#include <common.h>

int install_n1ghtshade_app() {
	int ret;

	mkdir("/mnt/Applications/n1ghtshade.app", 0755);

	ret = install("/modules/n1ghtshade.app/Info.plist", "/mnt/Applications/n1ghtshade.app/Info.plist", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/icon-ipad.png", "/mnt/Applications/n1ghtshade.app/icon-ipad.png", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/icon.png", "/mnt/Applications/n1ghtshade.app/icon.png", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/icon@2x.png", "/mnt/Applications/n1ghtshade.app/icon@2x.png", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/launch-568h.png", "/mnt/Applications/n1ghtshade.app/launch-568h.png", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/launch.png", "/mnt/Applications/n1ghtshade.app/launch.png", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/launch@2x.png", "/mnt/Applications/n1ghtshade.app/launch@2x.png", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/n1ghtshade", "/mnt/Applications/n1ghtshade.app/n1ghtshade", 0, 0, 6755);
	if(ret != 0) {
		return -1;
	}

	ret = install("/modules/n1ghtshade.app/Bootstrap", "/mnt/Applications/n1ghtshade.app/Bootstrap", 0, 0, 0755);
	if(ret != 0) {
		return -1;
	}

	return 0;
}