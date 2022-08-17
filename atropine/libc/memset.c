#include <libc.h>

void* memset(void* b, int c, size_t len) {
	char* chr_buf = (char*)b;
	for(size_t i = 0; i < len; i += 1) {
		chr_buf[i] = c;
	}
	return b;
}