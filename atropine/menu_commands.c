#include <common.h>

int echo_command(int argc, command_args* argv) {
	for(int i = 1; i < argc; i++) {
		printf("%s ", argv[i].string);
	}
	printf("\n");
	return 0;
}

int fb_echo_command(int argc, command_args* argv) {
	for(int i = 1; i < argc; i++) {
		 fb_print(argv[i].string);
	}
	fb_print("\n");
	return 0;
}

int help_command(int argc, command_args* argv) {
	printf("Availible commands:\n");
	for(int i = 0; i < num_commands; i++) {
		printf("\t%s\t%s\n", commands[i]->name, commands[i]->description);
	}
	return 0;
}

int jump_command(int argc, command_args* argv) {
	jumpto(0, load_address, 0, 0);
	return 0;
}

int patch_command(int argc, command_args* argv) {
	if(!strcmp(argv[1].string, "ibot")){
		char boot_args[255];
		for(int i = 2; i < argc; i++) {
			strncat(boot_args, argv[i].string, 255);
			strncat(boot_args, " ", 255);
		}
		patch_iboot((char*)load_address, get_env_uint("filesize"), boot_args);
	}
	else if(!strcmp(argv[1].string, "krnl")) {
		hook_kernel();
	}
	return 0;
}

int load_command(int argc, command_args* argv) {
	char* addr = (char*)load_address;
	unsigned int size;
	if(argc != 2) {
		return -1;
	}
	if(!strcmp(argv[1].string, "krnl")){
		/* load kernel, iH8sn0w style */
		fs_mount("nand0a", "hfs", "/boot");
		if ((size = 0x1000000, fs_load_file("/boot/System/Library/Caches/com.apple.kernelcaches/kernelcache", addr, &size)) &&
			(size = 0x1000000, fs_load_file("/boot/System/Library/Caches/com.apple.kernelcaches/kernelcache.s5l8920x", addr, &size)) &&
			(size = 0x1000000, fs_load_file("/boot/System/Library/Caches/com.apple.kernelcaches/kernelcache.s5l8922x", addr, &size)) &&
			(size = 0x1000000, fs_load_file("/boot/System/Library/Caches/com.apple.kernelcaches/kernelcache.s5l8720x", addr, &size))) {
			printf("kernel read err1\n");
			return -1;
		}
	}
	else if(!strcmp(argv[1].string, "dtre")) {
		load_image_from_bdev(addr, DEVICE_TREE_TAG, (size_t*)&size);
	}
	else if(!strcmp(argv[1].string, "logo")) {
		load_image_from_bdev(addr, LOGO_TAG, (size_t*)&size);
	}
	set_env_uint("filesize", size, 0);
	return 0;
}

int boot_args_command(int argc, command_args* argv) {
	char boot_args[255];
	for(int i = 1; i < argc; i++) {
		strncat(boot_args, argv[i].string, 255);
		strncat(boot_args, " ", 255);
	}
	memcpy(boot_args_string, boot_args, 255);
}

#ifdef DEBUG

int debug_command(int argc, command_args* argv) {
	if(argc != 4) {
		printf("Usage: debug <dump/hexdump/write> [options]\n");
		printf("\tdump <address> <length>             Dumps the specified memory to the display\n");
		printf("\thexdump <address> <length>          Hexdumps the specified memory to the display\n");
		printf("\twrite <address> <value>             Writes a 32bit integer to the specified address\n");
		return -1;
	}
	if(!strcmp(argv[1].string, "dump")) {
		char* address = (char*)argv[2].uinteger;
		uint32_t len = argv[3].uinteger;
		hexdump(address, len, 8, 0);
	}
	else if(!strcmp(argv[1].string, "hexdump")) {
		char* address = (char*)argv[2].uinteger;
		uint32_t len = argv[3].uinteger;
		hexdump(address, len, 8, 1);
	}
	else if(!strcmp(argv[1].string, "write")) {
		uint32_t* address = (uint32_t*)argv[2].uinteger;
		uint32_t value = argv[3].uinteger;
		*address = value;
	}
	else {
		printf("Usage: debug <dump/hexdump/write> [options]\n");
		printf("\tdump <address> <length>             Dumps the specified memory to the display\n");
		printf("\thexdump <address> <length>          Hexdumps the specified memory to the display\n");
		printf("\twrite <address> <value>             Writes a 32bit integer to the specified address\n");
		return -1;
	}
	return 0;
}

#endif

int init_menu_commands() {
	add_command("help", &help_command, "Shows this help dialog.");
	add_command("echo", &echo_command, "Writes arguments to stdout.");
	add_command("jump", &jump_command, "Jumps to an unpacked image at the load address.");
#ifdef DEBUG
	add_command("debug", &debug_command, "Commands to assist with debugging.");
#endif
#ifdef DISPLAY_OUTPUT
	add_command("fb_echo", &fb_echo_command, "Writes arguments to the framebuffer");
#endif
	add_command("patch", &patch_command, "Patches a decryped image uploaded to the load address");
	add_command("load", &load_command, "Loads on device images to the load addresss");
	add_command("boot-args", &boot_args_command, "Sets boot arguments for loader");

	debug("Initialised menu commands.\n");
	return 0;
}