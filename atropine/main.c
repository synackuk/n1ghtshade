#include <common.h>

int init = 0;

int init_atropine() {
	int ret = 0;
	ret = init_constants();
	if(ret != 0) {
		return -1;
	}
#ifdef RELOCATE
	ret = init_relocate();
	if(ret != 0) {
		return -1;
	}
#endif
#ifdef PATCH
	ret = patch_iboot((char*)base_address, 0x50000, "amfi=0xff cs_enforcement_disable=1 rd=md0 -v");
	if(ret != 0) {
		return -1;
	}
#endif
#ifdef DISPLAY_OUTPUT
	ret = init_framebuffer();
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef MENU_COMMANDS
	ret = init_menu_commands();
	if(ret != 0) {
		return -1;
	}
#else
#error Commandless builds are not currently supported.
#endif

	return 0;
}

int main(int argc, command_args* argv) {
	int ret;
	if(!init) {
		ret = init_atropine();
		if(ret != 0) {
			error("Failed to initialise atropine.");
			return -1;
		}
		log("initialised atropine.\n");
		init = 1;
		return 0;
	}
#ifdef MENU_COMMANDS
	if(argc < 2) {
		return 0;
	}
	parse_command(argc - 1, &argv[1]);
#endif
	return 0;
}