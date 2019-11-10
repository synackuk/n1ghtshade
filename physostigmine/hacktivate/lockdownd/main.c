#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char *argv[], char *envp[]) {
	setenv("DYLD_INSERT_LIBRARIES", "/usr/lib/hacktivate.dylib", 1);
	execv("/usr/libexec/real_lockdownd", argv);
	return 0;
}
