#include <stdio.h>

#include <drivers/drivers.h>
#include <commands/menu_commands.h>
#include <server.h>


int main(int argc, char *argv[], char *envp[]) {
	int ret = 0;

	/* We start by initialising the watchdog to stop jailbreakd being ended */
	ret = watchdog_init();
	if(ret != 0) {
		printf("Failed to initalise watchdog\n");
		return -1;
	}

	/* We now initialise the TCP stack */
	ret = tcp_init();
	if(ret != 0) {
		printf("Failed to initalise TCP\n");
		return -1;
	}

	/* Now we initialise USB so that we can communicate with jailbreakd */
	ret = usb_init();
	if(ret != 0) {
		printf("Failed to initalise USB\n");
		return -1;
	}

	/* We initialise the framebuffer so that we can output to the display */
	ret = fb_init();
	if(ret != 0) {
		printf("Failed to initalise framebuffer\n");
		return -1;
	}

	/* We setup our commands for our interactive shell */
	ret = menu_commands_init();
	if(ret != 0) {
		printf("Failed to initalise commands\n");
		return -1;
	}

	/* Finally, we hand over control to the server and therefore our shell */
	return server_init();
}
