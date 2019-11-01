#include <libsolanine.h>
#include <stdio.h>

void usage(char** argv) {
	printf("Usage: %s <input_ipsw> <output_ipsw> <device_identifier>\n", argv[0]);
}

int main(int argc, char** argv) {
	int ret;
	if(argc != 4) {
		usage(argv);
		return -1;
	}
	libsolanine_init();
	ret = libsolanine_patch_ipsw(argv[1], argv[2], argv[3]);
	if(ret != 0) {
		printf("Failed to patch ipsw\n");
		return -1;
	}
	libsolanine_exit();
	return 0;
}