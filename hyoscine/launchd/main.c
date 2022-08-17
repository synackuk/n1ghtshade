#include <common.h>

static char* fsck_system_args[] = { "/sbin/fsck_hfs", "-fy", "/dev/rdisk0s1s1", NULL };
static char* fsck_user_args[] = { "/sbin/fsck_hfs", "-fy", "/dev/rdisk0s1s2", NULL };
static char* jb_init_args[] = { "/jb_init", NULL };
static char* jailbreakd_args[] = { "/jailbreakd", NULL };

/* Finally, we've evolved to the userland */
int main(int argc, char** argv, char** env) {
	int ret;
	struct stat status;

	/* First copy STDIN, STDOUT and STDERR file descriptors to the console descriptor(this lets our debug output show up in verbose boot) */

	close(0);
	close(1);
	close(2);

	int console = open("/dev/console", O_WRONLY);

	dup2(console, 0);
	dup2(console, 1);
	dup2(console, 2);

	puts("Running hyoscine\n");

	/* Wait for the NAND flash to be initialised */

	while (stat("/dev/disk0s1s1", &status) != 0) {
		puts("Waiting for disk\n");
		sleep(1);
	}

	/* Start mounting filesystems */

	puts("Mounting root filesystem r/o... ");

	ret = hfs_mount("/dev/disk0s1s1", "/mnt", MNT_ROOTFS | MNT_RDONLY);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}
	puts("done.\n");

	puts("Mounting devfs...");
	if (mount("devfs", "/mnt/dev", 0, NULL) != 0) {
		puts("failed.\n");
		reboot(1);
	}
	puts("done.\n");
	
	puts("Checking filesystem...");

	ret = fsexec(fsck_system_args, cache_env);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}
	ret = fsexec(fsck_user_args, cache_env);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	puts("done.\n");

	puts("Mounting root filesystem r/w... ");

	ret = hfs_mount("/dev/disk0s1s1", "/mnt", MNT_ROOTFS | MNT_UPDATE);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}
	puts("done.\n");

	puts("Mounting user filesystem... ");

	ret = hfs_mount("/dev/disk0s1s2", "/mnt2", 0);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}
	puts("done.\n");

	/* Next, we need to replace dyld. The dyld we provide with the jailbreak will only work with iOS 9.3.5, so we must use the system dyld for binaries that need actual linking */

	puts("Remounting ramdisk... ");

	/* To do this, we use jb_init to remove the MNT_ROOTFS flag so we can remount r/w */
	install("/files/jb_init", "/mnt/jb_init", 0, 80, 0755);

	ret = fsexec(jb_init_args, cache_env);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	ret = hfs_mount("/dev/md0", "/", MNT_UPDATE);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	/* We run jb_init again to add the MNT_ROOTFS flag */
	ret = fsexec(jb_init_args, cache_env);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	unlink("/mnt/jb_init");

	puts("done.\n");

	puts("Symlinking /usr... ");

	/* Now, we need to delete our /usr folder */

	ret = unlink("/usr/lib/dyld");
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	ret = rmdir("/usr/lib");
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	ret = rmdir("/usr");
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	/* Finally, we symlink /mnt/usr to /usr */

	ret = symlink("/mnt/usr", "/usr");
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}

	puts("done.\n");

	/* Now we hand over to jailbreakd */

	do {
		puts("Waiting for jailbreakd\n");
		sleep(1);
	} while(stat("/jailbreakd", &status) != 0);


	puts("Launching daemons...");

	ret = exec(jailbreakd_args, cache_env);
	if(ret != 0) {
		puts("failed.\n");
		reboot(1);
	}
	puts("done.\n");

	/* Finally we tidy up and reboot */

	puts("Unmounting filesystems...");

	unmount("/mnt/dev", 0);
	unmount("/mnt2", 0);
	unmount("/mnt", 0);

	puts("done.\n");

	puts("Flushing buffers...");

	sync();
	sync();
	sync();

	puts("done.\n");
	
	reboot(1);
	return 0;
}