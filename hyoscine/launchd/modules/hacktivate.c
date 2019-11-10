#include <common.h>

int hacktivate() {
	int ret;
	
	ret = install("/modules/hacktivate.dylib", "/mnt/usr/lib/hacktivate.dylib", 0, 80, 0755);
	if(ret != 0) {
		return -1;
	}
	ret = install("/mnt/usr/libexec/lockdownd", "/mnt/usr/libexec/real_lockdownd", 0, 80, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = unlink("/mnt/usr/libexec/lockdownd");
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/lockdownd", "/mnt/usr/libexec/lockdownd", 0, 80, 0755);
	if(ret != 0) {
		return -1;
	}

	return 0;
}