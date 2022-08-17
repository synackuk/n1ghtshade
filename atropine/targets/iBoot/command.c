#include <command.h>
#include <address.h>
#include <libc/libc.h>

int iterate_commands(int argc, cmd_arg_t* argv) {
	cmd_handler_t* cmd = (cmd_handler_t*)&commands;

	/* Ensure the command is within the commands section */
	while(cmd != (cmd_handler_t*)&ecommands) {

		/* Check if the command is the one we want and if it is call it */
		if(!strcmp(cmd->name, argv[0].string)) {
			return cmd->func(argc - 1, &argv[1]);
		}

		/* Adding one to a pointer to is the same as adding the length of the struct */
		cmd += 1;
	}
	printf("Command not found, try help for a list of possible commands\n");
	return -1;
}