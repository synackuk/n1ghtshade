#include <libc.h>

int atoi(const char* ptr) {
	int val = 0;
	int fact = 1;
	if(*ptr == '-') {
		/* If the value is negative we can multiply the output by -1 to reflect this */
		fact = -1;
		ptr += 1;
	}
	while(1) {
		if(*ptr < '0' || *ptr > '9') {
			break;
		}
		/* As we read the number from back to front, leading zeros are irrelivant, therefore we can simply multiply by ten each time we want to move onto the next coluumn */
		val *= 10;
		/* Nice ascii trick - as the numbers 0 - 9 come one after eachother in ascii, by simply subtracting the ascii char '0', we get the actual number */
		val += (*ptr - '0');
		ptr += 1;
	}
	return val * fact;
}
