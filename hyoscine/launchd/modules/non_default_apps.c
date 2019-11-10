#include <common.h>

char* patch_systen_apps_args[] = { "/non_default_apps", NULL };

int patch_systen_apps() {
	int ret;

	ret = install("/modules/non_default_apps", "/mnt/non_default_apps", 0, 80, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = fsexec(patch_systen_apps_args, cache_env, 1);
	if(ret != 0) {
		return -1;
	}

	ret = unlink("/mnt/non_default_apps");
	if(ret != 0) {
		return -1;
	}

	return 0;
}