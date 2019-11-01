#include <common.h>

int init_relocate() {
	if(!cmd_ptr_address) {
		return -1;
	}
	cmd_ptr_address->name = "atropine";
	cmd_ptr_address->handler += LOADADDRESS_OFFSET;
	debug("Payload Relocated.\n");
	return 0;
}