#include <common.h>

int install_jb_init() {
	int ret;

	unlink("/mnt/usr/libexec/dirhelper");
	
	ret = install("/modules/jb_init", "/mnt/usr/libexec/dirhelper", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}
	return 0;
}