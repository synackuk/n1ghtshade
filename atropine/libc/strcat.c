#include <libc.h>

char* strcat(char *restrict s1, const char *restrict s2) {
	size_t s1_len = strlen(s1);
	size_t i = 0;
	while(s2[i] != '\0') {
		s1[s1_len + i] = s2[i];
		i += 1;
	}
	s1[s1_len + i] = '\0';
	return s1;
}

char* strncat(char *restrict s1, const char *restrict s2, size_t n) {
	size_t s1_len = strlen(s1);
	size_t i = 0;
	while(s2[i] != '\0' && i < n) {
		s1[s1_len + i] = s2[i];
		i += 1;
	}
	s1[s1_len + i] = '\0';
	return s1;
}