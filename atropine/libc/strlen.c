#include <libc.h>

size_t strlen(const char* s) {
	size_t len = 0;
	while(1) {
		if(s[len] == '\0') {
			return len;
		}
		len += 1;
	}
}