#ifndef COMMAND_H
#define COMMAND_H

typedef struct {
	signed int unk1;
	unsigned int uinteger;
	signed int integer;
	unsigned int type;
	char* string;
} cmd_arg_t;

typedef int (*cmd_func_t)(int argc, cmd_arg_t* argv);

typedef struct {
	char* name;
	cmd_func_t func;
	char* description;
} cmd_handler_t;

extern const void commands;
extern const void ecommands;

int iterate_commands(int argc, cmd_arg_t* argv);


/* Commands are stored in a linker section called .commands, this macro creates the struct declerations in the correct section */
#define ADD_COMMAND(__name__, __handler__, __description__) cmd_handler_t __attribute__((section(".commands"))) __handler__##_struct = {.name = __name__, .func = __handler__, .description = __description__};

#endif