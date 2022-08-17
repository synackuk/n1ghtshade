#include <libbelladonna.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

belladonna_ctx_t ctx = NULL;



static int read_file_into_buffer(char* path, char** buf, size_t* len) {
	FILE* f = fopen(path, "rb");
	if(!f) {
		return -1;
	}
	fseek(f, 0, SEEK_END);
	*len = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(!*len) {
		return -1;
	}

	*buf = malloc(*len);
	if(!*buf) {
		return -1;
	}
	fread(*buf, 1, *len, f);
	fclose(f);
	return 0;
}

int main(int argc, char** argv) {
	int ret = 0;
	int boot_tethered = 0;
	int jailbreak = 0;
	char* restore_path = NULL;
	char* ramdisk_path = NULL;
	int opt;
	/* We check for the arguments 'b' for tethered boot and 'r' for device restore */
	while ((opt = getopt(argc, argv, "bjr:d:")) != -1) {
		switch(opt) {
			case 'b':
				if(!restore_path && !ramdisk_path) {
					boot_tethered = 1;
				}
				break;
			case 'r':
				if(!boot_tethered && !ramdisk_path) {
					restore_path = optarg;
				}
				break;
			case 'd':
				if(!boot_tethered && !restore_path ) {
					ramdisk_path = optarg;
				}
				break;
			case 'j':
				if(!boot_tethered && !restore_path && !ramdisk_path) {
					jailbreak = 1;
				}
				break;
			default:
				printf("Usage:\n");
				printf("\t%s -b : for tethered boot\n", argv[0]);
				printf("\t%s -r <ipsw> : for restore\n", argv[0]);
				printf("\t%s -d <ramdisk> : for ramdisk boot\n", argv[0]);
				printf("\t%s -j : for jailbreak using hyoscine\n", argv[0]);
				return -1;
		}
	}
	/* Verify we chose an option */
	if(!boot_tethered && !restore_path && !ramdisk_path && !jailbreak) {
		printf("Usage:\n");
		printf("\t%s -b : for tethered boot\n", argv[0]);
		printf("\t%s -r <ipsw> : for restore\n", argv[0]);
		printf("\t%s -d <ramdisk> : for ramdisk boot\n", argv[0]);
		printf("\t%s -j : for jailbreak using hyoscine\n", argv[0]);
		return -1;
	}
	/* We always put the device in pwned DFU mode */
	ret = belladonna_init(&ctx);
	if(ret != 0) {
		return -1;
	}
	ret = belladonna_get_device(ctx);
	if(ret != 0) {
		belladonna_exit(ctx);
		return -1;
	}
	ret = belladonna_enter_pwned_dfu(ctx);
	if(ret != 0) {
		belladonna_exit(ctx);
		return -1;
	}
	/* Then we do extra as required */
	if(restore_path) {
		ret = belladonna_restore(ctx, restore_path);
	}
	if(boot_tethered) {
		ret = belladonna_tethered_boot(ctx, "-v");
	}
	if(ramdisk_path) {
		char* ramdisk = NULL;
		size_t ramdisk_len = 0;
		ret = read_file_into_buffer(ramdisk_path, &ramdisk, &ramdisk_len);
		if(ret != 0) {
			printf("Failed to read ramdisk at \"%s\"\n", ramdisk_path);
			belladonna_exit(ctx);
			return -1;
		}
		ret = belladonna_ramdisk_boot(ctx, ramdisk, ramdisk_len);
		if(ret != 0) {
			printf("Failed to boot ramdisk\n");
			belladonna_exit(ctx);
			return -1;
		}
		ret = belladonna_jailbreak(ctx, 1);
	}
	if(jailbreak) {
		ret = belladonna_boot_hyoscine(ctx);
		if(ret != 0) {
			printf("Failed to boot ramdisk\n");
			belladonna_exit(ctx);
			return -1;
		}
		ret = belladonna_jailbreak(ctx, 1);
	}
	if(ret != 0) {
		belladonna_exit(ctx);
		return -1;
	}

	belladonna_close_device(ctx);
	ret = belladonna_exit(ctx);
	if(ret != 0) {
		return -1;
	}
	return 0;
}