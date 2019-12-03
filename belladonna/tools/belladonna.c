#include <libbelladonna.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>

void usage(char** argv) {
	printf("Usage: %s [options]\n", argv[0]);
	printf("\t-p\t\tPut device in pwned DFU mode\n");
	printf("\t-r\t\tPut device in pwned Recovery mode\n");
	printf("\t-b\t\tBoot device tethered\n");
	printf("\t-j\t\tBoot jailbreak ramdisk\n");
	printf("\t-w <path>\tRestore Patched IPSW at path\n");
}

int main(int argc, char** argv) {
	int ret;
	int pwned_dfu = 0;
	int pwned_recovery = 0;
	int tethered_boot = 0;
	int ramdisk_boot = 0;
	char* restore_path = NULL;
	char boot_args[255];
	bzero(boot_args, 255);

	if(argc == 1) {
		usage(argv);
		return -1;
	}
	if(!strcmp(argv[1], "-p")) {
		pwned_dfu = 1;
	}
	else if(!strcmp(argv[1], "-r")) {
		pwned_dfu = 1;
		pwned_recovery = 1;
	}
	else if(!strcmp(argv[1], "-b")) {
		pwned_dfu = 1;
		pwned_recovery = 1;
		tethered_boot = 1;
		if(argc > 2) {
			strncpy(boot_args, argv[2], 255);
		}
	}
	else if(!strcmp(argv[1], "-j")) {
		pwned_dfu = 1;
		pwned_recovery = 1;
		ramdisk_boot = 1;
	}
	else if(!strcmp(argv[1], "-w")) {
		if(argc < 3) {
			usage(argv);
			return -1;
		}
		pwned_dfu = 1;
		pwned_recovery = 1;
		restore_path = argv[2];
	}
	else {
		usage(argv);
		return -1;
	}
	libbelladonna_init();
	while(libbelladonna_get_device() != 0 || libbelladonna_exploit_for_mode() != 0) {
		printf("Waiting for device in DFU mode\n");
		sleep(1);
	}
	ret = libbelladonna_compatible();
	if(ret != 0) {
		libbelladonna_exit();
		printf("Device not compatible\n");
		return -1;
	}
	if(pwned_dfu) {
		ret = libbelladonna_exploit(); 
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to enter Pwned DFU mode\n");
			return -1;
		}
	}
	if(pwned_recovery){
		ret = libbelladonna_enter_recovery();
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to enter Pwned Recovery mode\n");
			return -1;
		}
	}
	if(tethered_boot){
		ret = libbelladonna_boot_tethered(boot_args);
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to boot tethered\n");
			return -1;
		}
	}
	if(ramdisk_boot){
		ret = libbelladonna_boot_ramdisk();
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to boot ramdisk\n");
			return -1;
		}
	}
	if(restore_path) {
		ret = libbelladonna_restore_ipsw(restore_path);
		if (ret != 0) {
			libbelladonna_exit();
			printf("Failed to restore device\n");
			return -1;
		}
	}
	libbelladonna_exit();
	return 0;
}