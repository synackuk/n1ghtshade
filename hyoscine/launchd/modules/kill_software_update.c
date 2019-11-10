#include <common.h>

char* kill_software_update_args[] = { "/kill_software_update", NULL };

int kill_software_update() {
	int ret;

	ret = install("/modules/kill_software_update", "/mnt/kill_software_update", 0, 80, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = fsexec(kill_software_update_args, cache_env, 1);
	if(ret != 0) {
		return -1;
	}

	ret = unlink("/mnt/kill_software_update");
	if(ret != 0) {
		return -1;
	}

	return 0;
}