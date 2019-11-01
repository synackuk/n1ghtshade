#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
	setenv("DYLD_INSERT_LIBRARIES", "/usr/lib/asr_bypass.dylib", 1);
	execv("/usr/sbin/real_asr", argv);
	return 0;
}
