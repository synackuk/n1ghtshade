#include <commands/commands.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

command_t commands[MAX_COMMANDS];
int num_commands = 0;

int parse_command(char* command) {
	int argc = 0;

	/* static so that we survive function calls */
	static char* argv[255];
	bzero(argv, 255 * sizeof(char*));

	/* Populate argc and argv for passing to commands */
	char* token = strtok(command, " ");
	while (token) {
		argv[argc] = strdup(token);
		argc += 1;
		token = strtok(NULL, " ");
	}

	/* Avoid crash when argv[0] is empty */
	if(!argv[0]) {
		return 0;
	}

	/* Search the command list for the command */
	for(int i = 0; i < num_commands; i += 1) {
		if(!strcmp(argv[0], commands[i].name)) {
			return commands[i].handler(argc - 1, &argv[1]);
		}
	}
	printf("Unrecognised command \"%s\" try \"help\" for a list of commands\n", argv[0]);

	return 0;
}

int add_command(char* name, command_handler_t handler, char* description) {
	if(num_commands >= MAX_COMMANDS) {
		printf("max commands reached\n");
		return -1;
	}

	commands[num_commands] = (command_t){.name = name, .handler = handler, .description = description};

	num_commands += 1;
	return 0;
}