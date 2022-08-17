#include <commands/menu_commands.h>
#include <commands/commands.h>
#include <drivers/drivers.h>
#include <untar.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdlib.h>
#include <spawn.h>

static int help_cmd(int argc, char** argv) {
	printf("Available commands:\n");

	for(int i = 0; i < num_commands; i += 1) {
		printf("\t%s\t%s\n", commands[i].name, commands[i].description);
	}
	return 0;
}

static int echo_cmd(int argc, char** argv) {
	for(int i = 0; i < argc; i += 1) {
		printf("%s ", argv[i]);
	}
	printf("\n");
	return 0;
}

static int fbecho_cmd(int argc, char** argv) {

	/* Max line length of 256 means that we can guarantee this is safe */
	char* progress_msg = malloc(256); 
	if(!progress_msg) {
		printf("Out of memory\n");
		return 0;
	}
	memset(progress_msg, '\0', 256);

	for(int i = 0; i < argc; i += 1) {
		strncat(progress_msg, argv[i], 255);
		strncat(progress_msg, " ", 255);
	}
	display_progress_print(progress_msg);
	free(progress_msg);
	return 0;
}

static int fbdebug_cmd(int argc, char** argv) {
	for(int i = 0; i < argc; i += 1) {
		display_debug_print(argv[i]);
		display_debug_print(" ");
	}
	display_debug_print("\n");
	return 0;
}

static int fbprog_cmd(int argc, char** argv) {
	if(argc != 1) {
		printf("Usage:\n");
		printf("fbprog <progress>\n");
		return 0;
	}
	int progress = atoi(argv[0]);
	if(progress > 100 || progress < 0) {
		printf("Progress must be between 0 and 100\n");
		return 0;
	}
	display_progress_bar(progress);
	return 0;
}

static int exec_cmd(int argc, char** argv) {
	if(argc < 1) {
		printf("Usage:\n");
		printf("exec <path> [args]\n");
		return 0;
	}
	NSFileManager* fm = [NSFileManager defaultManager];
	NSString* source = [NSString stringWithUTF8String: argv[0]];
	if (![fm isReadableFileAtPath:source]) {
		printf("Program file %s doesn't exist\n", argv[0]);
		return 0;
	}
	if(vfork() != 0) {
		while(wait4(-1, NULL, WNOHANG, NULL) <= 0) {
			sleep(1);
		}
	} else {
		execv(argv[0], argv);
	}
	return 0;
}

static int copy_cmd(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage:\n");
		printf("copy <source> <destination>\n");
		return 0;
	}
	int count = 0;
	char buf[0x800];
	struct stat status;

	while (stat(argv[0], &status) != 0) {
		puts("Unable to find source file\n");
		return -1;
	}

	int in = open(argv[0], O_RDONLY, 0);
	if (in < 0) {
		return -1;
	}

	int out = open(argv[1], O_WRONLY | O_CREAT, 0);
	if (out < 0) {
		close(in);
		return -1;
	}

	do {
		count = read(in, buf, 0x800);
		if (count > 0) {
			count = write(out, buf, count);
		}
	} while (count > 0);

	close(in);
	close(out);

	if (count < 0) {
		return -1;
	}

	return 0;
}

static int copyno_cmd(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage:\n");
		printf("copy <source> <destination>\n");
		return 0;
	}
	int count = 0;
	char buf[0x800];
	struct stat status;

	while (stat(argv[0], &status) != 0) {
		puts("Unable to find source file\n");
		return -1;
	}

	while (stat(argv[1], &status) == 0) {
		puts("Destination file exists\n");
		return 0;
	}

	int in = open(argv[0], O_RDONLY, 0);
	if (in < 0) {
		return -1;
	}

	int out = open(argv[1], O_WRONLY | O_CREAT, 0);
	if (out < 0) {
		close(in);
		return -1;
	}

	do {
		count = read(in, buf, 0x800);
		if (count > 0) {
			count = write(out, buf, count);
		}
	} while (count > 0);

	close(in);
	close(out);

	if (count < 0) {
		return -1;
	}

	return 0;
}


static int chmod_cmd(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage:\n");
		printf("chmod <file> <mode>\n");
		return 0;
	}
	chmod(argv[0], atoi(argv[1]));
	return 0;
}

static int mkdir_cmd(int argc, char** argv) {
	if(argc != 1) {
		printf("Usage:\n");
		printf("mkdir <path>\n");
		return 0;
	}
	mkdir(argv[0], 755);
	return 0;
}

static int touch_cmd(int argc, char** argv) {
	if(argc != 1) {
		printf("Usage:\n");
		printf("touch <path>\n");
		return 0;
	}
	FILE* f = fopen(argv[0], "wb");
	fclose(f);
	chmod(argv[0], 0644);
	return 0;
}

static int rm_cmd(int argc, char** argv) {
	if(argc != 1) {
		printf("Usage:\n");
		printf("rm <path>\n");
		return 0;
	}
	unlink(argv[0]);
	return 0;
}

static int cd_cmd(int argc, char** argv) {
	if(argc != 1) {
		printf("Usage:\n");
		printf("cd <path>\n");
		return 0;
	}
	chdir(argv[0]);
	return 0;
}

static int untar_cmd(int argc, char** argv) {
	if(argc != 2) {
		printf("Usage:\n");
		printf("untar <tar_file> <destination>\n");
		return 0;
	}
	untar(argv[0], argv[1]);
	return 0;
}

static int ls_cmd(int argc, char** argv) {
	if(argc > 1) {
		printf("Usage:\n");
		printf("ls [path]\n");
		return 0;
	}
	char* path = ".";
	if(argc == 1) {
		path = argv[0];
	}
	DIR* d = opendir(path);
	if(!d) {
		printf("Failed to open directory at \"%s\"\n", path);
		return 0;
	}

	struct dirent* dir = readdir(d);
	while(dir) {
		printf("%s ", dir->d_name);
		dir = readdir(d);
	}
	printf("\n");
	closedir(d);
	return 0;
}

static int exit_cmd(int argc, char** argv) {
	/* non-zero return exits the session */
	return -1;
}


static int reboot_cmd(int argc, char** argv) {
	exit(0);
	return 0;
}


int menu_commands_init() {
	add_command("help", help_cmd, "Prints this help dialog");
	add_command("echo", echo_cmd, "Repeats stdin to stdout");
	add_command("fbecho", fbecho_cmd, "Repeats stdin to the framebuffer progress text");
	add_command("fbdebug", fbdebug_cmd, "Repeats stdin to the framebuffer debug text");
	add_command("fbprog", fbprog_cmd, "Updates the progress bar");
	add_command("exec", exec_cmd, "Executes the binary at a given filepath");
	add_command("copy", copy_cmd, "Copies a file from source to destination");
	add_command("copyno", copyno_cmd, "Copies a file from source to destination as long as that file doesn't currently exist");
	add_command("chmod", chmod_cmd, "Sets new permissions for a given file");
	add_command("mkdir", mkdir_cmd, "Creates a directory at the given path");
	add_command("rm", rm_cmd, "Removes a file from the device");
	add_command("cd", cd_cmd, "Changes current working directory");
	add_command("untar", untar_cmd, "Untar a file to a given basepath");
	add_command("ls", ls_cmd, "List the files in a given directory");
	add_command("touch", touch_cmd, "Create a blank file");
	add_command("exit", exit_cmd, "Exits jailbreakd");
	add_command("reboot", reboot_cmd, "Exits jailbreakd and reboots the device");

	return 0;
}
