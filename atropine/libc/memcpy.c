#include <libc.h>

void* memcpy(void* dest, const void* src, size_t len) {
	const char* source = src;
	char* destination = dest;
	for(int i = 0; i < len; i += 1) {
		destination[i] = source[i];
	}
	return dest;
}