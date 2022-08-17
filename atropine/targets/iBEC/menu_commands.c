#include <command.h>
#include <address.h>
#include <libc/libc.h>

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