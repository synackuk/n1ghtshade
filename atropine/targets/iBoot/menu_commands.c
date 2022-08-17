#include <command.h>
#include <address.h>
#include <display/display.h>
#include <libc/libc.h>

extern uint32_t end;
static const char* rd_boot_args = "rd=md0";

int fsboot_cmd(int argc, cmd_arg_t* argv) {

	/* Sets the kernel boot-args to those in the boot-args environment variable */
	*(uint32_t*)boot_args_ref = (uint32_t)get_env("boot-args");
	clear_icache();

	/* Calls the fsboot boot command to boot the system */
	
	printf("Booting system\n");
	display_progress_print("Booting system...");

	display_progress_bar(100);
	fsboot();
	return 0;
}

ADD_COMMAND("fsboot", fsboot_cmd, "Mounts the system rootfs partition and executes the kernelcache");

int ramdisk_cmd(int argc, cmd_arg_t* argv) {

	*ramdisk_size_ptr = get_env_uint("filesize");
	if(!*ramdisk_size_ptr) {
		/* Default size of 30MB */
		*ramdisk_size_ptr = 30000000;
	}

	/* We use the end of the payload as the space to place our ramdisk */
	*ramdisk_address_ptr = (uint32_t)&end;

	memcpy((void*)*ramdisk_address_ptr, loadaddr, *ramdisk_size_ptr);
	return 0;
}

ADD_COMMAND("ramdisk", ramdisk_cmd, "Load a raw ramdisk to boot");

int rdboot_cmd(int argc, cmd_arg_t* argv) {
	/* Verify a ramdisk is loaded */
	if(!*ramdisk_size_ptr || !*ramdisk_address_ptr) {
		printf("No ramdisk loaded\n");
		return -1;
	}

	/* Reset the boot-args to a sensible default */
	*(uint32_t*)boot_args_ref = (uint32_t)rd_boot_args;

	/* Calls the fsboot boot command to boot the system */
	printf("Booting system\n");
	display_progress_print("Booting system...");

	display_progress_bar(100);
	fsboot();
	return 0;
}

ADD_COMMAND("rdboot", rdboot_cmd, "Boot a ramdisk loaded previously");

int echo_cmd(int argc, cmd_arg_t* argv) {
	for(int i = 0; i < argc; i += 1) {
		printf("%s ", argv[i].string);
	}
	printf("\n");
	return 0;
}

ADD_COMMAND("echo", echo_cmd, "Repeats stdin to stdout");


int help_cmd(int argc, cmd_arg_t* argv) {
	printf("Available commands:\n");
	cmd_handler_t* cmd = (cmd_handler_t*)&commands;

	/* Ensure the command is within the commands section */
	while(cmd != (cmd_handler_t*)&ecommands) {
		printf("\t%s\t%s\n", cmd->name, cmd->description);

		/* Adding one to a pointer to is the same as adding the length of the struct */
		cmd += 1;
	}
	return 0;
}

ADD_COMMAND("help", help_cmd, "Prints this help dialog");