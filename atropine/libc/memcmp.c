#include <libc.h>


int memcmp(const void *s1, const void *s2, size_t n) {
	char* search1 = (char*)s1;
	char* search2 = (char*)s2;
	for(size_t i = 0; i < n; i += 1) {
		if(search1[i] != search2[i]) {
			return -1;
		}
	}
	return 0;
}