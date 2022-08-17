#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <commands/commands.h>
#include <drivers/drivers.h>

#define PORT 1337

static int process_client();
static int setup_socket();
static char* read_line();
static int new_stdio(int new_fd);

int server_init() {
	int ret = 0;

	/* First we initialise our socket */
	int socket_fd = setup_socket();
	if(socket_fd < 0) {
		return socket_fd;
	}
	struct sockaddr_in client_address;
	uint client_address_len = 0;


	while (1) {
		/* We wait for a new client to connect */
		int client_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);
		if (client_fd < 0) {
			printf("could not open a socket to accept data\n");
			return -1;
		}
		int enable = 1;
		ret = setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
		if(ret < 0) {
			printf("Failed to disable socket buffering\n");
			return -1;
		}
		
		/* We can only be accessed by one client at once, if we use multithreading then we can't modify the display */
		/* We setup our client file descriptor as stdio so that we can communicate using printf/fgets */
		ret = new_stdio(client_fd);
		if(ret != 0) {
			return -1;
		}

		/* Start our shell */
		process_client();
	}

	close(socket_fd);
	return 0;
}

static int new_stdio(int new_fd) {

	/* Close stdio */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Set our new file descriptor as stdio */

	dup2(new_fd, STDIN_FILENO);
	dup2(new_fd, STDOUT_FILENO);
	dup2(new_fd, STDERR_FILENO);

	close(new_fd);

	/* Disable buffering on stdio */

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	return 0;
}


static int process_client() {
	int ret;
	while(1) {

		/* Make the prompt pretty */
		printf("> ");

		/* read a line from stdin */
		char* command = read_line();
		if(!command) {
			break;
		}

		/* parse it for our output */
		ret = parse_command(command);
		free(command);
		if(ret != 0) {
			return 0;
		}
	}
	return 0;
}

static int setup_socket() {
	int ret = 0;
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	/* IPv4 */
	server_address.sin_family = AF_INET;

	/* Set the port and listen on all addresses */
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0) {
		printf("Failed to open socket\n");
		return ret;
	}

	/* Allow more than one client to attach to the server at once */
	int enable = 1;
	ret = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &enable, sizeof(enable));
	if(ret < 0) {
		printf("Failed to enable reuseaddr\n");
		return ret;
	}

	/* Bind to our port */

	ret = bind(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address));
	if(ret < 0) {
		printf("Failed to bind socket\n");
		return ret;
	}

	/* Start listening */

	ret = listen(socket_fd, 16);
	if(ret < 0) {
		printf("Failed to listen on socket\n");
		return ret;
	}

	return socket_fd;

}

static char* read_line() {
	char* buf = malloc(256);
	memset(buf, '\0', 256);
	int i = 0;
	while(i < 255) {
		int c = fgetc(stdin);

		/* If we failed to get a character (probably means the client has disconnected) */
		if(c < 0) {
			free(buf);
			return NULL;
		}

		/* Detect the end of the transmission */
		if(c == '\n' || c == '\0') {
			buf[i] = '\0';
			break;
		}
		buf[i] = c;
		i += 1;
	}
	return buf;
}