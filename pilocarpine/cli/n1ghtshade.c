#include <libbelladonna.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

const struct option longopts[] = {
	{"pwned-dfu", no_argument, NULL, 'p'},
	{"pwned-rec", no_argument, NULL, 'r'},
	{"tether", no_argument, NULL, 'b'},
	{"jailbreak", no_argument, NULL, 'j'},
	{"restore", required_argument, NULL, 'w'},
	{NULL, 0, NULL, 0}
};

void usage(char** argv) {
	printf("Usage: %s [options]\n", argv[0]);
	printf("\t-p\t\tPut device in pwned DFU mode\n");
	printf("\t-r\t\tPut device in pwned Recovery mode\n");
	printf("\t-b\t\tBoot device tethered\n");
	printf("\t-j\t\tBoot jailbreak ramdisk\n");
	printf("\t-w <path>\tRestore Patched IPSW at path\n");
	belladonna_exit();
	exit(0);
}

int main(int argc, char** argv) {
	int ret;
	int optindex = 0;
	int pwned_dfu = 0;
	int pwned_recovery = 0;
	int tethered_boot = 0;
	int ramdisk_boot = 0;
	char* restore_path = NULL;
	belladonna_init();
	int opt;
	if(argc < 2) {
		usage(argv);
	} 
	while ((opt = getopt_long(argc, argv, "prbjw:", longopts, &optindex)) > 0) {
		switch (opt) {
			case 'p':
				pwned_dfu = 1;
				break;
			break;
			case 'r':
				pwned_dfu = 1;
				pwned_recovery = 1;
				break;
			break;
			case 'b':
				pwned_dfu = 1;
				pwned_recovery = 1;
				tethered_boot = 1;
				break;
			break;
			case 'j':
				pwned_dfu = 1;
				pwned_recovery = 1;
				ramdisk_boot = 1;
				break;
			case 'w':
				if(!optarg) {
					usage(argv);
				}
				pwned_dfu = 1;
				pwned_recovery = 1;
				restore_path = optarg;
				break;
			default:
				usage(argv);
				break;
		}
	}
	while(belladonna_get_device() != 0) {
		printf("Waiting for device in DFU mode\n");
		sleep(1);
	}
	if(pwned_dfu) {
		ret = belladonna_exploit(); 
		if (ret != 0) {
			belladonna_exit();
			printf("Failed to enter Pwned DFU mode\n");
			return -1;
		}
	}
	if(pwned_recovery){
		ret = belladonna_enter_recovery();
		if (ret != 0) {
			belladonna_exit();
			printf("Failed to enter Pwned Recovery mode\n");
			return -1;
		}
	}
	if(tethered_boot){
		ret = belladonna_boot_tethered();
		if (ret != 0) {
			belladonna_exit();
			printf("Failed to boot tethered\n");
			return -1;
		}
	}
	if(ramdisk_boot){
		ret = belladonna_boot_ramdisk();
		if (ret != 0) {
			belladonna_exit();
			printf("Failed to boot ramdisk\n");
			return -1;
		}
	}
	if(restore_path) {
		ret = belladonna_restore_ipsw(restore_path);
		if (ret != 0) {
			belladonna_exit();
			printf("Failed to restore device\n");
			return -1;
		}
	}
	belladonna_exit();
	return 0;
}
