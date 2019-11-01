#include <common.h>

int num_commands = 0;

command_descriptor* commands[50];

int add_command(char* name, command handler, char* description) {
	if(num_commands >= 50) {
		error("Maximum commands reached.\n");
		return -1;
	}
	commands[num_commands] = malloc(sizeof(command_descriptor));
	commands[num_commands]->name = name;
	commands[num_commands]->handler = handler;
	commands[num_commands]->description = description;
	num_commands += 1;
	return 0;
}

int parse_command(int argc, command_args* argv){
	for(int i = 0; i < num_commands; i += 1) {
		if(!strcmp(commands[i]->name, argv[0].string)) {
			commands[i]->handler(argc, argv);
			return 0;
		}
	}
	printf("Unrecognised command \"%s\".\n", argv[0].string);
	return 0;
}