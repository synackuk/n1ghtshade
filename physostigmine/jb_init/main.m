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
	int console = open("/dev/console", O_WRONLY);
	dup2(console, 1);
	dup2(console, 2);

	if(argc > 1 && !strcmp(argv[1], "-machineBoot")) {
		mkdir_recursive("/var/tmp/launchd", 700);
	}

	printf("Launching Daemons\n");

	system("ls /Library/LaunchDaemons | while read a; do launchctl load /Library/LaunchDaemons/$a; done;");
	system("for file in /etc/rc.d/*; do $file; done;");

	system("launchctl unload /Library/LaunchDaemons/com.openssh.sshd.plist;/usr/libexec/sshd-keygen-wrapper");

	printf("Done\n");

	close(console);
	return 0;
}
