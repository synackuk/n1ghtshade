#include <common.h>

void hexdump(char* start, int len, int row_len, int output_strings) {
	char str[row_len + 1];
	str[row_len] = '\0';
	int i;
	for(i = 0; i < len; i++) {
		if(i % row_len == 0) {
			if(i != 0) {
				if(output_strings){
					printf("|%s|", str);
				}
				printf("\n");
			}
			printf("%08x ", (uint32_t)start + i);
		}
		if ((uint8_t)start[i] < 0x20 || (uint8_t)start[i] > 0x7e) {
			str[i % row_len] = '.';
		}
		else{
			str[i % row_len] = (char)start[i];
		}
		printf("%02x ", (uint8_t)start[i]);

	}
	while(i % row_len != 0 && output_strings) {
		printf("   ");
		i++;
	}
	if(output_strings){
		printf("|%s|", str);
	}
	printf("\n");
}