#include <libbelladonna.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void usage(char** argv) {
	printf("Usage: %s [options]\n", argv[0]);
	printf("\t-b [boot_args]\t\tBoot device tethered\n");
	printf("\t-j\t\t\tBoot jailbreak ramdisk\n");
	printf("\t-w <path>\t\tRestore Patched IPSW at path\n");
}

int main(int argc, char** argv) {
	printf("n1ghtshade by @synackuk\n");
	int ret;
	char* input_ipsw;
	char boot_args[255];
	bzero(boot_args, 255);
	int patch_ipsw;
	int restore_ipsw;
	int tethered_boot;
	int ramdisk_boot;

	if(argc == 1) {
		usage(argv);
		return -1;
	}
	else if(!strcmp(argv[1], "-b")) {
		tethered_boot = 1;
		if(argc > 2) {
			strncpy(boot_args, argv[2], 255);
		}
	}
	else if(!strcmp(argv[1], "-j")) {
		ramdisk_boot = 1;
	}
	else if(!strcmp(argv[1], "-w")) {
		if(argc < 3) {
			usage(argv);
			return -1;
		}
		input_ipsw = argv[2];
		restore_ipsw = 1;
	}
	else {
		usage(argv);
		return -1;
	}

	libbelladonna_init();

	while(libbelladonna_get_device() != 0) {
		printf("Waiting for device\n");
		sleep(1);
	}

	ret = libbelladonna_compatible();
	if(ret != 0) {
		libbelladonna_exit();
		printf("Device not compatible\n");
		return -1;
	}

	ret = libbelladonna_exploit(); 
	if (ret != 0) {
		libbelladonna_exit();
		printf("Failed to enter Pwned DFU mode\n");
		return -1;
	}
	ret = libbelladonna_enter_recovery();
	if (ret != 0) {
		libbelladonna_exit();
		printf("Failed to enter Pwned Recovery mode\n");
		return -1;
	}

	if(tethered_boot) {
		ret = libbelladonna_boot_tethered(boot_args);
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to boot tethered\n");
			return -1;
		}
	}

	if(ramdisk_boot) {
		ret = libbelladonna_boot_ramdisk();
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to boot ramdisk\n");
			return -1;
		}
	}

	if(restore_ipsw) {
		ret = libbelladonna_restore_ipsw(input_ipsw);
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to restore device\n");
			return -1;
		}
	}

	libbelladonna_exit();
	return 0;
}