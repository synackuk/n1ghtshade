#include <iBoot_Patcher/include/finders.h>
#include <command.h>
#include <address.h>

#define CMD_HANDLER "\0go"

int main(int argc, cmd_arg_t* argv);

int fix_cmd_handler() {
	cmd_handler_t* handler = find_cmd_handler(base_address, IBOOT_LEN, CMD_HANDLER, sizeof(CMD_HANDLER));
	if(!handler) {
		return -1;
	}
	printf("Found command handler at %p\n", handler);
	
	/* Change the name to atropine and point the function to our main function (since we relocated the code) */
	handler->name = "atropine";

	handler->func = main;
	return 0;
}