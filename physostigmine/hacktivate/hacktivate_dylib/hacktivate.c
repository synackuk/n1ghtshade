#include <unistd.h>
#include <sys/sysctl.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int my_sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp, size_t newlen) {
	int ret;
	if(strcmp(name, "hw.model") == 0 && oldp != NULL) {
		ret = sysctlbyname(name, oldp, oldlenp, newp, newlen);
		if(ret != 0) {
			return -1;
		}
		size_t len = strlen(oldp) + 1;
		memcpy(oldp + (len - 4), "DEV", 4);
		return 0;
	}
	return sysctlbyname(name, oldp, oldlenp, newp, newlen);
}

const struct {void *n; void *o;} interposers[] __attribute((section("__DATA, __interpose"))) = {
	{ (void *)my_sysctlbyname, (void *)sysctlbyname }
};