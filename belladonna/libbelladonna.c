#include <libbelladonna.h>
#include <device_loaders.h>
#include <libfragmentzip/libfragmentzip.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <payloads/atropine.h>
#include <payloads/hyoscine.h>
#include <libidevicerestore/idevicerestore.h>
#ifdef WIN32
#include <windows.h>
#endif

#include <exploits/checkm8/checkm8.h>

exploit_list* exploits = NULL;
libloader_device_t dev = NULL;
belladonna_log_cb belladonna_log = NULL;

static void default_log_cb(char* msg) {
	printf("%s", msg);
}

static void exploit_add(char* name, exploit_supported supported, exploit_func exploit) {
	exploit_list* new_exploit = malloc(sizeof(exploit_list));
	new_exploit->name = name;
	new_exploit->supported = supported;
	new_exploit->exploit = exploit;
	new_exploit->next = exploits;
	exploits = new_exploit;
}

static void exploits_init() {
#ifdef CHECKM8
	exploit_add("checkm8", &checkm8_supported, &checkm8_exploit);
#endif

#ifdef SHATTER
	exploit_add("shatter", &shatter_supported, &shatter_exploit);
#endif

#ifdef LIMERA1N
	exploit_add("limera1n", &limera1n_supported, &limera1n_exploit);
#endif

#ifdef STEAKS4UCE
	exploit_add("steaks4uce", &steaks4uce_exploit, &steaks4uce_exploit);
#endif

#ifdef PWNAGE2
	exploit_add("pwnage2", &pwnage2_supported, &pwnage2_exploit);
#endif
}

static void exploits_exit() {
	exploit_list* exploit = exploits;
	exploit_list* to_free;
	while(exploit != NULL) {
		to_free = exploit;
		exploit = to_free->next;
		free(to_free);
	}
	exploits = NULL;
}

void libbelladonna_set_log_cb(belladonna_log_cb new_cb) {
	belladonna_log = new_cb;
}

void libbelladonna_init() {
#ifdef __APPLE__
	system("killall -9 iTunesHelper 2> /dev/null");
#endif
	libloader_init();
	exploits_init();
	libbelladonna_set_log_cb(&default_log_cb);
}

void libbelladonna_exit() {
	if(dev){
		libloader_close(dev);
	}
	libloader_exit();
	exploits_exit();
}

int libbelladonna_get_device() {
	dev = libloader_get_device_handle();
	if(!dev) {
		return -1;
	}
	if(!libloader_is_dfu(dev)) {
		return -1;
	}
	return 0;
}

int libbelladonna_compatible() {
	exploit_list* curr = exploits;
	int ret;
	while(curr != NULL) {
		ret = curr->supported(dev);
		if(ret == 0){
			return 0;
		}
		curr = curr->next;
	}
	error("Device not supported.");
	return -1;
}

int libbelladonna_exploit() {
	int ret;
	ret = libloader_is_pwned_dfu(dev);
	if(ret) {
		return 0;
	}
	exploit_list* curr = exploits;
	while(curr != NULL) {
		ret = curr->supported(dev);
		if(ret == 0){
			dev = curr->exploit(dev);
			if(!dev) {
				error("Failed to enter Pwned DFU mode.");
				return -1;
			}
			return 0;
		}
		curr = curr->next;
	}
	return -1;
}

static int download_firmware_component(char* ipsw_url, char* component_path, char** out_buf, size_t* component_len) {
	int ret;
#ifdef WIN32
	char tmp_path[512];
	char tmp_file_path[512];
	GetTempPath(512, tmp_file_path);
	GetTempFileName(tmp_file_path, "comp", 0, tmp_path);
#else
	char* tmp_path = "/tmp/component";
#endif
	fragmentzip_t *ipsw = fragmentzip_open(ipsw_url);
	if (!ipsw) {
		error("failed to open ipsw.");
		return -1;
	}
	ret = fragmentzip_download_file(ipsw, component_path, tmp_path, NULL);
	
	fragmentzip_close(ipsw);
	
	if(ret != 0) {
		error("Failed to download firmware component.");
		return -1;
	}
	FILE* f = fopen(tmp_path, "rb");
	if(!f) {
		error("Failed to open firmware component.");
		return -1;
	}
	fseek(f, 0, SEEK_END);
	*component_len = ftell(f);
	fseek(f, 0, SEEK_SET);

	*out_buf = malloc(*component_len);
	if(!*out_buf) {
		error("Out of memory.");
		return -1;
	}
	ret = fread(*out_buf, 1, *component_len, f);
	fclose(f);
	if(ret != *component_len) {
		error("Failed to read in firmware component.");
		free(*out_buf);
		*out_buf = NULL;
		*component_len = 0;
		return -1;
	}
	unlink(tmp_path);
	return 0;
}

static int load_ibss(char** ibss, size_t* ibss_len) {
	int ret;
	char* identifier = libloader_get_identifier(dev);
	if(!identifier) {
		error("Failed to get identifier.");
		return -1;
	}
	char* ipsw_url = NULL;
	char* ibss_path = NULL;
	int i = 0;
	LOG("Finding iBSS for device\n");
	while(device_loaders[i].identifier != NULL) {
		if(!strcmp(device_loaders[i].identifier, identifier)) {
			ipsw_url = device_loaders[i].ipsw_url;
			ibss_path = device_loaders[i].ibss_path;
			break;
		}
		i += 1;
	}
	if(!ipsw_url || !ibss_path) {
		error("Failed to find correct loader for your device.");
		return -1;
	}
	LOG("Downloading iBSS\n");
	ret = download_firmware_component(ipsw_url, ibss_path, ibss, ibss_len);
	if(ret != 0) {
		error("Failed to download iBSS.");
		*ibss = NULL;
		*ibss_len = 0;
		return -1;
	}
	return 0;
}

static int boot_ibss_checkm8(char* ibss, size_t ibss_len) {
	unsigned char buf[16];
	bzero(buf, 16);
	int ret;
	ret = libloader_send_buffer(dev, buf, 16);
	if(ret != 0) {
		error("Failed to send blank packet.");
		return -1;
	}
	ret = libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, 0, 0, 100);
	if(ret != 0) {
		error("Failed to send blank packet.");
		return -1;
	}
	ret = libloader_ctrl_transfer(dev, 0xA1, 3, 0, 0, buf, 6, 100);
	if(ret < 0) {
		error("Failed to request status.");
		return -1;
	}
	ret = libloader_ctrl_transfer(dev, 0xA1, 3, 0, 0, buf, 6, 100);
	if(ret < 0) {
		error("Failed to request status.");
		return -1;
	}
	LOG("Uploading iBSS\n");
	libloader_send_buffer(dev, (unsigned char*)ibss, ibss_len);
	LOG("Executing iBSS\n");
	libloader_ctrl_transfer(dev, 0xA1, 2, 0xFFFF, 0, buf, 0, 100);
	return 0;
}

static int boot_ibss_normal(char* ibss, size_t ibss_len) {
	int ret;
	LOG("Uploading iBSS\n");
	ret = libloader_send_buffer(dev, (unsigned char*)ibss, ibss_len);
	if(ret != 0) {
		error("Failed to upload iBSS.");
		return -1;
	}
	LOG("Executing iBSS\n");
	ret = libloader_request_image_validation(dev);
	if(ret != 0) {
		error("Failed to execute iBSS.");
		return -1;
	}
	return 0;
}
static int boot_ibss() {
	int ret;
	char* ibss;
	size_t ibss_len;
	ret = load_ibss(&ibss, &ibss_len);
	if(ret != 0) {
		error("Failed to load iBSS.");
		return -1;
	}
	ret = libloader_is_checkm8_dfu(dev);
	if(ret) {
		ret = boot_ibss_checkm8(ibss, ibss_len);
	}
	else {
		ret = boot_ibss_normal(ibss, ibss_len);
	}
	free(ibss);
	if(ret != 0) {
		error("Failed to boot iBSS.");
		return -1;
	}
	return 0;
}

static int load_ibec(char** ibec, size_t* ibec_len) {
	int ret;
	char* identifier = libloader_get_identifier(dev);
	if(!identifier) {
		error("Failed to get identifier.");
		return -1;
	}
	char* ipsw_url = NULL;
	char* ibec_path = NULL;
	int i = 0;
	LOG("Finding iBEC for device\n");
	while(device_loaders[i].identifier != NULL) {
		if(!strcmp(device_loaders[i].identifier, identifier)) {
			ipsw_url = device_loaders[i].ipsw_url;
			ibec_path = device_loaders[i].ibec_path;
			break;
		}
		i += 1;
	}
	if(!ipsw_url || !ibec_path) {
		error("Failed to find correct loader for your device.");
		return -1;
	}
	LOG("Downloading iBEC\n");
	ret = download_firmware_component(ipsw_url, ibec_path, ibec, ibec_len);
	if(ret != 0) {
		error("Failed to download iBEC.");
		*ibec = NULL;
		*ibec_len = 0;
		return -1;
	}
	return 0;
}

static int boot_ibec() {
	int ret;
	char* ibec;
	size_t ibec_len;
	ret = load_ibec(&ibec, &ibec_len);
	if(ret != 0) {
		error("Failed to load iBEC.");
		return -1;
	}
	LOG("Uploading iBEC\n");
	ret = libloader_send_buffer(dev, (unsigned char*)ibec, ibec_len);
	free(ibec);
	if(ret != 0) {
		error("Failed to upload iBEC.");
		return -1;
	}
	LOG("Executing iBEC\n");
	ret = libloader_request_image_validation(dev);
	if(ret != 0) {
		error("Failed to execute iBEC.");
		return -1;
	}
	return 0;
}

static int load_payload() {
	int ret;
	LOG("Uploading iBoot payload\n");
	ret = libloader_send_buffer(dev, (unsigned char*)atropine, atropine_length);
	if(ret != 0) {
		error("Failed to upload iBoot payload.");
		return -1;
	}
	LOG("Executing iBoot payload\n");
	ret = libloader_send_cmd(dev, "go");
	if(ret != 0) {
		error("Failed to execute iBoot payload.");
		return -1;
	}
	return 0;
}

int libbelladonna_enter_recovery() {
	int ret;
	ret = libloader_is_pwned_dfu(dev);
	if(!ret) {
		error("Device not in Pwned DFU mode.");
		return -1;
	}
	ret = boot_ibss();
	if(ret != 0) {
		error("Failed to boot iBSS.");
		return -1;
	}
	dev = libloader_reconnect(dev, 2);
	if(!dev) {
		error("Failed to reconnect to device.");
		return -1;
	}
	ret = boot_ibec();
	if(ret != 0) {
		error("Failed to boot iBEC.");
		return -1;
	}
	dev = libloader_reconnect(dev, 2);
	if(!dev) {
		error("Failed to boot iBEC.");
		return -1;
	}
	ret = libloader_is_dfu(dev);
	if(ret) {
		error("Failed to boot iBEC.");
		return -1;
	}
	ret = load_payload();
	if(ret != 0) {
		error("Failed to load iBoot payload");
		return -1;
	}
	return 0;
}

int libbelladonna_boot_tethered(char* boot_args) {
	int ret;
	ret = libloader_is_dfu(dev);
	if(ret) {
		error("Device isn't in recovery mode.");
		return -1;
	}
	LOG("Setting apple logo\n");
	ret = libloader_send_cmd(dev, "atropine load logo");
	if(ret != 0) {
		error("Failed to load apple logo.");
		return -1;
	}
	ret = libloader_send_cmd(dev, "setpicture");
	if(ret != 0) {
		error("Failed to set applelogo.");
		return -1;
	}
	ret = libloader_send_cmd(dev, "bgcolor 0 0 0");
	if(ret != 0) {
		error("Failed to set background colour.");
		return -1;
	}
	LOG("Setting device tree\n");
	ret = libloader_send_cmd(dev, "atropine load dtre");
	if(ret != 0) {
		error("Failed to load devicetree.");
		return -1;
	}
	ret = libloader_send_cmd(dev, "devicetree");
	if(ret != 0) {
		error("Failed to set devicetree.");
		return -1;
	}
	LOG("Setting boot-args\n");
	char boot_arg_cmd[255];
	if(boot_args){
		snprintf(boot_arg_cmd, 255, "atropine boot-args rd=disk0s1s1 %s", boot_args);
	}
	else {
		strcpy(boot_arg_cmd, "atropine boot-args rd=disk0s1s1");
	}
	ret = libloader_send_cmd(dev, boot_arg_cmd);
	if(ret != 0) {
		error("Failed to set boot-args");
		return -1;
	}
	LOG("Patching Kernel\n");
	ret = libloader_send_cmd(dev, "atropine patch krnl");
	if(ret != 0) {
		error("Failed to patch kernelcache.");
		return -1;
	}
	LOG("Booting kernel\n");
	ret = libloader_send_cmd(dev, "atropine load krnl");
	if(ret != 0) {
		error("Failed to load kernel");
		return -1;
	}
	libloader_send_cmd(dev, "bootx");
	return 0;
}

static int load_ramdisk() {
	int ret;
	LOG("Uploading ramdisk\n");
	ret = libloader_send_buffer(dev, (unsigned char*)hyoscine, hyoscine_length);
	if(ret != 0) {
		error("Failed to upload ramdisk.");
		return -1;
	}
	LOG("Loading ramdisk\n");
	ret = libloader_send_cmd(dev, "ramdisk");
	if(ret != 0) {
		error("Failed to load ramdisk.");
		return -1;
	}
	return 0;
}

int libbelladonna_boot_ramdisk() {
	int ret;
	ret = libloader_is_dfu(dev);
	if(ret) {
		error("Device isn't in recovery mode.");
		return -1;
	}
	LOG("Setting device tree\n");
	ret = libloader_send_cmd(dev, "atropine load dtre");
	if(ret != 0) {
		error("Failed to load devicetree.");
		return -1;
	}
	ret = libloader_send_cmd(dev, "devicetree");
	if(ret != 0) {
		error("Failed to set devicetree.");
		return -1;
	}
	ret = load_ramdisk();
	if(ret != 0) {
		error("Failed to load ramdisk.");
		return -1;
	}
	LOG("Patching Kernel\n");
	ret = libloader_send_cmd(dev, "atropine patch krnl");
	if(ret != 0) {
		error("Failed to patch kernelcache.");
		return -1;
	}
	LOG("Booting kernel\n");
	ret = libloader_send_cmd(dev, "atropine load krnl");
	if(ret != 0) {
		error("Failed to load kernel");
		return -1;
	}
	libloader_send_cmd(dev, "bootx");
	return 0;
}

int libbelladonna_restore_ipsw(char* path) {
	int ret;
	ret = libloader_is_dfu(dev);
	if(ret) {
		error("Device isn't in recovery mode.");
		return -1;
	}
	LOG("Patching Kernel\n");
	ret = libloader_send_cmd(dev, "atropine patch krnl");
	if(ret != 0) {
		error("Failed to patch kernelcache.");
		return -1;
	}
	libloader_close(dev);
	dev = NULL;

	libloader_close(dev);
	struct idevicerestore_client_t* client = idevicerestore_client_new();
	if (!client) {
		error("Failed to create restore client.");
		return -1;
	}
	client->flags |= FLAG_ERASE;
	client->flags |= FLAG_LATEST_SHSH;
	client->flags |= FLAG_NO_IBEC_UPLOAD;
	client->flags &= ~FLAG_INTERACTIVE;
	client->ipsw = strdup(path);
	int result = idevicerestore_start(client);
	if(result != 0) {
		error("Failed to restore device.");
		return -1;
	}
	idevicerestore_client_free(client);
	
	return 0;
}