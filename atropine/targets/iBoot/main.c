#include <libc/libc.h>
#include <address.h>
#include <display/display.h>
#include <command.h>
#include <iBoot_Patcher/include/iBoot32Patcher.h>
#include <handler.h>
#include <jumpto_hook.h>

int init = 0;

int atropine_init() {
	int ret;

	/* First we have to find all the functions we want to use so that we can output information and etc */
	ret = find_functions();
	if(ret != 0) {
		return -1;
	}

	/* Next we initialise the display with the framebuffer, width and height we've found */
	display_init(framebuffer_address, display_width, display_height);

	display_progress_print("Patching iBoot");
	printf("Patching iBoot\n");

	display_progress_bar(20);

	/* We patch the rest of the iBoot, mainly sigchecks */
	ret = patch_iboot(base_address, IBOOT_LEN);
	if(ret != 0) {
		return ret;
	}

	printf("Fixing command handler\n");

	display_progress_bar(40);

	/* Since we relocate the payload when we first run it, we need to move the command handler to the new location; we also rename it for good measure */
	ret = fix_cmd_handler();
	if(ret != 0) {
		return ret;
	}

	printf("Hooking jumpto\n");
	display_progress_bar(60);

	/* Whenever iBoot tries to boot a new image, it calls jumpto. Here we patch that call so that we instead execute our own image, this allows us to patch the kernel before it's loaded */
	ret = hook_jumpto();
	if(ret != 0) {
		return ret;
	}

	/* Clear the instruction cache so that we know our changes are correctly updated */
	clear_icache();
	printf("Ready to boot\n");
	display_progress_print("Ready to boot");
	display_progress_bar(80);


	
	return 0;
}

int main(int argc, cmd_arg_t* argv) {
	if(!init) {
		init = 1;
		return atropine_init();
	}
	if(argc == 1) {
		return 0;
	}
	return iterate_commands(argc - 1, &argv[1]);
}
