#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_COMMANDS 50

typedef int(*command_handler_t)(int argc, char** argv);

typedef struct {
	char* name;
	command_handler_t handler;
	char* description;
} command_t;

extern command_t commands[];
extern int num_commands;

int add_command(char* name, command_handler_t handler, char* description);
int parse_command(char* command);

#endif