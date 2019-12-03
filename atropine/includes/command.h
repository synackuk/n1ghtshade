#ifndef COMMAND_H
#define COMMAND_H

typedef struct command_args {
	signed int unk1;
	unsigned int uinteger;
	signed int integer;
	unsigned int type;
	unsigned char* string;
} command_args;

typedef int(*command)(int argc, command_args* argv);

typedef struct command_descriptor {
	char* name;
	command handler;
	char* description;
} command_descriptor;

int help_command(int argc, command_args* argv);
int add_command(char* name, command handler, char* description);
int parse_command(int argc, command_args* argv);

extern int num_commands;

extern command_descriptor* commands[];

#endif