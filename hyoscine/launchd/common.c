#include <common.h>
#include <hfs_mount.h>

char* cache_env[] = {
		"DYLD_SHARED_CACHE_DONT_VALIDATE=1",
		"DYLD_SHARED_CACHE_DIR=/System/Library/Caches/com.apple.dyld",
		"DYLD_SHARED_REGION=private"
};

void sleep(unsigned int seconds) {
	int i = 0;
	for(i = seconds * 10000000; i > 0; i--) {}
}

void _puts(const char* s) {
	while((*s) != '\0') {
		write(1, s, 1);
		s++;
	}
	sync();
}

int fsexec(char* argv[], char* env[], int pause) {
	int pid = vfork();
	if (pid != 0) {
		if (pause) {
			while (wait4(pid, NULL, WNOHANG, NULL) <= 0) {
				sleep(1);
			}
		} else
			return pid;
	} else {
		chdir("/mnt");
		if (chroot("/mnt") != 0) {
			return -1;
		}
		execve(argv[0], argv, env);
	}
	return 0;
}


int hfs_mount(char* device, const char* mountdir, int options) {
	struct hfs_mount_args args;
	args.fspec = device;
	return mount("hfs", mountdir, options, &args);
}

int install(const char* src, const char* dst, int uid, int gid, int mode) {
	int ret = 0;

	ret = cp(src, dst);
	if (ret < 0) {
		return ret;
	}

	ret = chown(dst, uid, gid);
	if (ret < 0) {
		return ret;
	}

	ret = chmod(dst, mode);
	if (ret < 0) {
		return ret;
	}

	return 0;
}


int cp(const char *src, const char *dest) {
	int count = 0;
	char buf[0x800];
	struct stat status;

	while (stat(src, &status) != 0) {
		return -1;
	}

	int in = open(src, O_RDONLY, 0);
	if (in < 0) {
		return -1;
	}

	int out = open(dest, O_WRONLY | O_CREAT, 0);
	if (out < 0) {
		close(in);
		return -1;
	}

	do {
		count = read(in, buf, 0x800);
		if (count > 0) {
			count = write(out, buf, count);
		}
	} while (count > 0);

	close(in);
	close(out);

	if (count < 0) {
		return -1;
	}

	return 0;
}
