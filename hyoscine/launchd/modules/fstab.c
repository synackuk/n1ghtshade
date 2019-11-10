#include <common.h>

char* fstab_args[] = { "/fstab", NULL };

int patch_fstab() {
	int ret;

	ret = install("/modules/fstab", "/mnt/fstab", 0, 80, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = fsexec(fstab_args, cache_env, 1);
	if(ret != 0) {
		return -1;
	}

	ret = unlink("/mnt/fstab");
	if(ret != 0) {
		return -1;
	}

	return 0;
}