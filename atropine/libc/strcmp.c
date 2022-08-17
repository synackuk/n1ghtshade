#include <libc.h>

int strcmp(const char *s1, const char *s2) {
	int i = 0;
	while(1) {
		if(s1[i] == '\0' && s2[i] == '\0') {
			return 0;
		}
		if(s1[i] != s2[i]) {
			return -1;
		}
		i += 1;
	}
}