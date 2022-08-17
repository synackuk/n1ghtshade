#include <launch_daemons.h>
#include <remount.h>
#include <kernel.h>
#include <stdio.h>
#include <sys/stat.h>

// https://stackoverflow.com/a/2336245

static void mkdir_recursive(const char *dir, int mode) {
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp),"%s",dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/'){
		tmp[len - 1] = 0;
	}
	for(p = tmp + 1; *p; p++){
		if(*p == '/') {
			*p = 0;
			mkdir(tmp, mode);
			*p = '/';
		}
	}
	mkdir(tmp, mode);
}

int main(int argc, char *argv[], char *envp[]) {
	/* First give ourself root credentials, this bypasses the adding of NOSUID when mounting the Data partition */
	setuid(0);
	setgid(0);

	int ret;
	/* We want our output in the console so we can view it when we boot */
	int console = open("/dev/console", O_WRONLY);
	dup2(console, 1);
	dup2(console, 2);

	/* Do the job of dirhelper */
	if(argc > 1 && !strcmp(argv[1], "-machineBoot")) {
		mkdir_recursive("/var/tmp/launchd", 700);
	}

	printf("n1ghtshade jb loader by synackuk\n");

	printf("Initialising kernel.\n");

	/* Start by preparing the kernel, dumping the kernel */
	ret = init_kernel();
	if(ret != 0) {
		printf("Failed to initialise kernel\n");
		return -1;
	}

	printf("Remounting r/w.\n");

	/* Remount the root filesystem r/w */
	ret = do_remount();
	if(ret != 0) {
		printf("Failed to remount r/w\n");
		return -1;
	}

	char* boot_args = (char*)kern_bootargs();

	/* If we're being executed on a ramdisk we don't need to boot launch daemons */
	if(!strstr(boot_args, "rd=md0")) {
		printf("Launching daemons.\n");
		/* Finally load our launch daemons and exit */
		load_launch_daemons();
	}

	printf("Done.\n");


	close(console);
	return 0;
}
