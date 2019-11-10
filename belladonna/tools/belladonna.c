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
}

int main(int argc, char** argv) {
	int ret;
	int pwned_dfu = 0;
	int pwned_recovery = 0;
	int tethered_boot = 0;
	int ramdisk_boot = 0;

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
	}
	else if(!strcmp(argv[1], "-j")) {
		pwned_dfu = 1;
		pwned_recovery = 1;
		ramdisk_boot = 1;
	}
	else {
		usage(argv);
		return -1;
	}
	libbelladonna_init();
	printf("Waiting for device in DFU mode\n");
	while(libbelladonna_get_device() != 0) {
		sleep(1);
	}
	ret = libbelladonna_compatible();
	if(ret != 0) {
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
		ret = libbelladonna_boot_tethered("-v");
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
			printf("Failed to boot tethered\n");
			return -1;
		}
	}
	libbelladonna_exit();
	return 0;
}