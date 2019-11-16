#include <common.h>
#include <modules/fstab.h>
#include <modules/kill_software_update.h>
#include <modules/n1ghtshade_app.h>
#include <modules/non_default_apps.h>
#include <modules/hacktivate.h>


#define INSTALL_N1GHTSHADE_APP
#define JB_INIT
#define PATCH_FSTAB
#define SHOW_NON_DEFAULT_SYSTEM_APPS
#define KILL_SOFTWARE_UPDATE
#define HACKTIVATE

int install_physostigmine() {
	int ret;

#ifdef INSTALL_N1GHTSHADE_APP
	ret = install_n1ghtshade_app();
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef JB_INIT
	ret = install_jb_init();
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef PATCH_FSTAB
	ret = patch_fstab();
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef SHOW_NON_DEFAULT_SYSTEM_APPS
	ret = patch_systen_apps();
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef KILL_SOFTWARE_UPDATE
	ret = kill_software_update();
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef HACKTIVATE
	ret = hacktivate();
	if(ret != 0) {
		return -1;
	}
#endif

	return 0;
}