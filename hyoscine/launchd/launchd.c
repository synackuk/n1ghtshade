#include <common.h>
#include <hfs_mount.h>
#include <install.h>

int console;

char* fsck_system_args[] = { "/sbin/fsck_hfs", "-fy", "/dev/rdisk0s1s1", NULL };
char* fsck_user_args[] = { "/sbin/fsck_hfs", "-fy", "/dev/rdisk0s1s2", NULL };

__attribute__((noreturn)) void done() {
	sync();
	puts("Unmounting filesystem\n");
	rmdir("/mnt/private/var2");
	unmount("/mnt/private/var2", 0);
	unmount("/mnt/dev", 0);
	unmount("/mnt", 0);
	puts("Flushing buffers\n");
	sync();
	puts("Rebooting\n");
	close(console);
	reboot(1);
	while(1){}
}

int main(int argc, char** argv, char** env) {
	int ret;
	struct stat status;

	console = open("/dev/console", O_WRONLY);
	dup2(console, 1);
	dup2(console, 2);

	puts("Running hyoscine\n");

	while (stat("/dev/disk0s1s1", &status) != 0) {
		puts("Waiting for disk\n");
		sleep(1);
	}

	puts("Mounting filesystem r/o\n");
	
	ret = hfs_mount("/dev/disk0s1s1", "/mnt", MNT_ROOTFS | MNT_RDONLY);
	if(ret != 0) {
		puts("Failed to mount filesystem r/o\n");
		done();
	}

	puts("Mounting dev filesystem\n");
	
	ret = mount("devfs", "/mnt/dev", 0, NULL);
	if(ret != 0) {
		puts("Failed to mount dev filesystem\n");
		done();
	}

	puts("Checking filesystem\n");

	ret = fsexec(fsck_system_args, cache_env, 1);
	if(ret != 0) {
		puts("Failed to check filesystem\n");
		done();
	}

	puts("Mounting filesystem r/w\n");

	ret = hfs_mount("/dev/disk0s1s1", "/mnt", MNT_ROOTFS | MNT_UPDATE);
	if(ret != 0) {
		puts("Failed to mount filesystem r/w\n");
		done();
	}

	puts("Checking user filesystem\n");

	ret = fsexec(fsck_user_args, cache_env, 1);
	if(ret != 0) {
		puts("Failed to check filesystem\n");
		done();
	}

	mkdir("/mnt/private/var2", 0755);

	puts("Mounting user filesystem\n");

	if (hfs_mount("/dev/disk0s1s2", "/mnt/private/var2", 0) != 0) {
		puts("Failed to mount user filesystem\n");
		done();
	}


	puts("Installing physostigmine\n");

	ret = install_physostigmine();
	if(ret != 0) {
		puts("Failed to install physostigmine\n");
		done();
	}

	puts("Installed physostigmine\n");

	done();

	return 0;
}