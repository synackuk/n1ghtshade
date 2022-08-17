#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

int cp(const char *src, const char *dest) {
	int count = 0;
	char buf[0x800];
	struct stat status;

	if (stat(src, &status) != 0) {
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

int main(int argc, char *argv[], char *envp[]) {
	/* If the iOS version is newer than iOS 6 then we can simply delete Setup.app */
	if(SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(@"7.0")) {
		printf("Wiping out Setup.app\n");
		unlink("/Applications/Setup.app");
		return 0;
	}
	/* Otherwise we need to use a hack dylib */
	printf("Installing lockdownd hack\n");
	install("/usr/libexec/lockdownd", "/usr/libexec/real_lockdownd", 0, 80, 0755);
	unlink("/usr/libexec/lockdownd");
	install("/files/lockdownd", "/usr/libexec/lockdownd", 0, 80, 0755);
	install("/files/hacktivate.dylib", "/usr/lib/hacktivate.dylib", 0, 80, 0755);
	return 0;
}
