#include <libsolanine.h>
#include <stdlib.h>
#include <common.h>
#include <xpwn/libxpwn.h>
#include <xpwn/nor_files.h>
#include <dmg/dmg.h>
#include <dmg/filevault.h>
#include <xpwn/ibootim.h>
#include <plist/plist.h>
#include <xpwn/outputstate.h>
#include <hfs/hfslib.h>
#include <dmg/dmglib.h>
#include <xpwn/pwnutil.h>
#include <curl/curl.h>
#include <libfragmentzip/libfragmentzip.h>
#include <unistd.h>

#include <asr_bypass.h>

#define LOG solanine_log

#define API_URL "https://api.ipsw.me"
#define API_KEYS_URL API_URL "/v4/keys/ipsw/%s/%s"
#define DEVICE_URL API_URL "/v4/device/%s"
#define DEVICE_KEY_URL API_URL "/v4/keys/device/%s"
#define IPSW_URL API_URL "/v4/ipsw/%s/%s"

solanine_log_cb solanine_log = NULL;
CURL *hnd;

static void default_log_cb(char* msg) {
	printf("%s", msg);
}


struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t curl_download_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	
	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if(ptr == NULL) {
		return 0;
	}
	
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}

static int download_file(char* url, char** buf, size_t* len) {
	struct MemoryStruct chunk;
	
	chunk.memory = malloc(1);
	chunk.size = 0;
	CURLcode ret;
	struct curl_slist *slist1;
	
	slist1 = NULL;
	slist1 = curl_slist_append(slist1, "Accept: application/x-plist");
	slist1 = curl_slist_append(slist1, "Cache-Control: no-cache");
	
	
	curl_easy_setopt(hnd, CURLOPT_URL, url);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_HEADER, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.54.0");
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_download_cb);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(hnd, CURLOPT_HEADER, 0);
	ret = curl_easy_perform(hnd);
	if(ret != 0) {
		return -1;
	}
	curl_slist_free_all(slist1);
	slist1 = NULL;

	if(!chunk.memory || !chunk.size) {
		return -1;
	}
	
	*buf = chunk.memory;
	*len = chunk.size;
	return 0;
}

static int load_buffer_from_output_state(OutputState** output_state, char* path, char** buf, size_t* len) {
	AbstractFile* f = getFileFromOutputState(output_state, path);
	if (!f) {
		return -1;
	}
	*len = f->getLength(f);
	if(!*len) {
		return -1;
	}
	*buf = malloc(*len);
	if(!*buf) {
		*len = 0;
		return -1;
	}
	f->read(f, *buf, *len);
	f->close(f);
	return 0;
}

static int write_buffer_to_output_state(OutputState** output_state, char* path, char* buf, size_t len) {
	addToOutput(output_state, path, buf, len);
	return 0;
}

static int load_manifest_from_output_state(OutputState** output_state, plist_t* manifest) {
	int ret;
	char* buf;
	size_t len;

	ret = load_buffer_from_output_state(output_state, "BuildManifest.plist", &buf, &len);
	if(ret != 0) {
		return -1;
	}
	plist_from_xml(buf, len, manifest);
	free(buf);
	return 0;
}

static int get_image_data(char* buildid, char* identifier, char* component, char** key, char** iv, char** path) {
	int ret;
	*key = NULL;
	*iv = NULL;
	*path = NULL;
	char* url;
	asprintf(&url, API_KEYS_URL, curl_easy_escape(hnd, identifier, 0), buildid);
	char* response;
	size_t response_len;
	ret = download_file(url, &response, &response_len);
	free(url);
	if(ret != 0) {
		return -1;
	}
	plist_t response_plist = NULL;
	plist_from_xml(response, response_len, &response_plist);
	free(response);
	if(!response_plist) {
		return -1;
	}
	plist_t keys = plist_dict_get_item(response_plist, "Keys");
	if(!keys) {
		plist_free(response_plist);
		return -1;
	}
	plist_array_iter iter = NULL;
	plist_array_new_iter(keys, &iter);
	if(!iter) {
		plist_free(response_plist);
		return -1;
	}
	plist_t key_for_img = NULL;
	plist_array_next_item(keys, iter, &key_for_img);
	if(!key_for_img) {
		free(iter);
		return -1;
	}
	while(key_for_img) {
		plist_t image_plist = plist_dict_get_item(key_for_img, "Image");
		if(!image_plist) {
			free(iter);
			plist_free(response_plist);
			return -1;
		}
		char* image = NULL;
		plist_get_string_val(image_plist, &image);
		if(!strcmp(component, image)) {
			plist_t key_plist = plist_dict_get_item(key_for_img, "Key");
			if(!key_plist) {
				free(iter);
				plist_free(response_plist);
				return -1;
			}
			plist_get_string_val(key_plist, key);
			plist_t iv_plist = plist_dict_get_item(key_for_img, "IV");
			if(!iv_plist) {
				free(iter);
				plist_free(response_plist);
				return -1;
			}
			plist_get_string_val(iv_plist, iv);
			plist_t path_plist = plist_dict_get_item(key_for_img, "Filename");
			if(!path_plist) {
				free(iter);
				plist_free(response_plist);
				return -1;
			}
			plist_get_string_val(path_plist, path);
		}
		free(image);
		plist_array_next_item(keys, iter, &key_for_img);
	}
	free(iter);
	plist_free(response_plist);
	if(!*path) {
		return -1;
	}
	return 0;
}

static int get_board_config(char* identifier, char** board_config) {
	int ret;
	char* url;
	asprintf(&url, DEVICE_URL, curl_easy_escape(hnd, identifier, 0));
	char* response;
	size_t response_len;
	ret = download_file(url, &response, &response_len);
	free(url);
	if(ret != 0) {
		return -1;
	}
	plist_t response_plist = NULL;
	plist_from_xml(response, response_len, &response_plist);
	free(response);
	if(!response_plist) {
		return -1;
	}
	plist_t board_config_plist = plist_dict_get_item(response_plist, "BoardConfig");
	if(!board_config_plist) {
		plist_free(response_plist);
		return -1;
	}
	plist_get_string_val(board_config_plist, board_config);
	plist_free(response_plist);
	return 0;
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
		return -1;
	}
	ret = fragmentzip_download_file(ipsw, component_path, tmp_path, NULL);
	
	fragmentzip_close(ipsw);
	
	if(ret != 0) {
		return -1;
	}
	FILE* f = fopen(tmp_path, "rb");
	if(!f) {
		return -1;
	}
	fseek(f, 0, SEEK_END);
	*component_len = ftell(f);
	fseek(f, 0, SEEK_SET);

	*out_buf = malloc(*component_len);
	if(!*out_buf) {
		return -1;
	}
	ret = fread(*out_buf, 1, *component_len, f);
	fclose(f);
	if(ret != *component_len) {
		free(*out_buf);
		*out_buf = NULL;
		*component_len = 0;
		return -1;
	}
	unlink(tmp_path);
	return 0;
}

static int get_latest_firmware(char* identifier, char** out_url, char** buildid) {
	int ret;
	char* url;
	asprintf(&url, DEVICE_KEY_URL, curl_easy_escape(hnd, identifier, 0));
	char* response;
	size_t response_len;
	ret = download_file(url, &response, &response_len);
	free(url);
	if(ret != 0) {
		return -1;
	}
	plist_t response_plist = NULL;
	plist_from_xml(response, response_len, &response_plist);
	free(response);
	if(!response_plist) {
		return -1;
	}
	plist_array_iter iter = NULL;
	plist_array_new_iter(response_plist, &iter);
	if(!iter) {
		return -1;
	}
	plist_t firmware = NULL;
	plist_array_next_item(response_plist, iter, &firmware);
	if(!firmware) {
		free(iter);
		return -1;
	}
	plist_t behind = NULL;
	while(firmware) {
		behind = firmware;
		plist_array_next_item(response_plist, iter, &firmware);
	}
	free(iter);
	plist_t buildid_plist = plist_dict_get_item(behind, "BuildID");
	plist_get_string_val(buildid_plist, buildid);
	plist_free(response_plist);

	asprintf(&url, IPSW_URL, curl_easy_escape(hnd, identifier, 0), *buildid);
	ret = download_file(url, &response, &response_len);
	free(url);
	if(ret != 0) {
		return -1;
	}
	response_plist = NULL;
	plist_from_xml(response, response_len, &response_plist);
	free(response);
	if(!response_plist) {
		return -1;
	}
	plist_t url_plist = plist_dict_get_item(response_plist, "URL");
	plist_get_string_val(url_plist, out_url);
	plist_free(response_plist);
	return 0;
}


static int patch_ramdisk(OutputState** output_state, char* identifier, char* buildid, char* board_config) {
	int ret;
	char* str_key = NULL;
	char* str_iv = NULL;
	char* ramdisk_path = NULL;
	ret = get_image_data(buildid, identifier, "RestoreRamdisk", &str_key, &str_iv, &ramdisk_path);
	if(ret != 0) {
		return -1;
	}

	unsigned int* key = NULL;
	unsigned int* iv = NULL;

	size_t bytes;
	hexToInts(str_key, &key, &bytes);
	hexToInts(str_iv, &iv, &bytes);
	free(str_key);
	free(str_iv);

	AbstractFile* ramdisk_f = openAbstractFile2(getFileFromOutputStateForOverwrite(output_state, ramdisk_path), key, iv);
	free(ramdisk_path);
	if(!ramdisk_f) {
		return -1;
	}

	size_t base_ramdisk_len = (size_t) ramdisk_f->getLength(ramdisk_f);
	if(!base_ramdisk_len) {
		return -1;
	}
	char* base_ramdisk_buf = (char*) malloc(base_ramdisk_len);
	if(!base_ramdisk_buf) {
		return -1;
	}
	ramdisk_f->read(ramdisk_f, base_ramdisk_buf, base_ramdisk_len);

	io_func* ramdisk_file_system = IOFuncFromAbstractFile(ramdisk_f);

	if(!ramdisk_file_system) {
		free(base_ramdisk_buf);
		return -1;
	}

	Volume* ramdisk_volume = openVolume(ramdisk_file_system);
	if(!ramdisk_volume) {
		free(base_ramdisk_buf);
		return -1;
	}

	grow_hfs(ramdisk_volume, (ramdisk_volume->volumeHeader->totalBlocks * 3) * ramdisk_volume->volumeHeader->blockSize);
	add_hfs(ramdisk_volume, createAbstractFileFromMemoryFile((void**)&base_ramdisk_buf, &base_ramdisk_len), "/real_ramdisk.dmg");
	free(base_ramdisk_buf);
	
	LOG("Adding ASR bypass\n");

	move("/usr/sbin/asr", "/real_asr", ramdisk_volume);
	move("/real_asr", "/usr/sbin/real_asr", ramdisk_volume);
	void* asr_patch_tar_wr = malloc(asr_patch_tar_length);
	if(!asr_patch_tar_wr) {
		return -1;
	}
	memcpy(asr_patch_tar_wr, asr_patch_tar, asr_patch_tar_length);
	hfs_untar(ramdisk_volume, createAbstractFileFromMemoryFile((void**)&asr_patch_tar_wr, (size_t*)&asr_patch_tar_length));
	free(asr_patch_tar_wr);

	LOG("Patching options.plist\n");

	board_config[3] = '\x00';
	char* options_path;
	asprintf(&options_path, "/usr/local/share/restore/options.%s.plist", board_config);
	
	HFSPlusCatalogRecord* record = getRecordFromPath(options_path, ramdisk_volume, NULL, NULL);
	if(!record) {
		free(options_path);
		return -1;
	}
	HFSPlusCatalogFile* options_file = (HFSPlusCatalogFile*)record;
	size_t options_len = 512;
	char* options_xml = malloc(options_len);
	if(!options_xml) {
		free(options_path);
		return -1;
	}
	writeToFile(options_file, createAbstractFileFromMemory((void**)&options_xml, options_len), ramdisk_volume);
	plist_t options_plist = NULL;
	plist_from_xml(options_xml, strlen(options_xml), &options_plist);
	free(options_xml);
	if(!options_plist) {
		free(options_path);
		return -1;
	}

	plist_dict_set_item(options_plist, "UpdateBaseband", plist_new_bool(0));
	char* options_xml_patched;
	plist_to_xml(options_plist, &options_xml_patched, (uint32_t*)&options_len);
	plist_free(options_plist);
	if(!options_xml_patched) {
		free(options_path);
		return -1;
	}

	add_hfs(ramdisk_volume, createAbstractFileFromMemory((void**)&options_xml_patched, options_len), options_path);
	
	free(options_xml_patched);
	free(options_path);

	closeVolume(ramdisk_volume);
	CLOSE(ramdisk_file_system);
	return 0;
}

static int upgrade_bootchain(OutputState** output_state, char* identifier, char* buildid) {
	int ret;
	char* new_buildid;
	char* new_url;
	ret = get_latest_firmware(identifier, &new_url, &new_buildid);
	if(ret != 0) {
		return -1;
	}
	LOG("Upgrading LLB\n");

	char* old_llb_path;
	char* new_llb_path;
	char* key;
	char* iv;
	ret = get_image_data(buildid, identifier, "LLB", &key, &iv, &old_llb_path);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		return -1;
	}
	free(key);
	free(iv);

	ret = get_image_data(new_buildid, identifier, "LLB", &key, &iv, &new_llb_path);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		free(old_llb_path);
		return -1;
	}
	free(key);
	free(iv);

	char* new_llb_buf;
	size_t new_llb_len;
	ret = download_firmware_component(new_url, new_llb_path, &new_llb_buf, &new_llb_len);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		free(old_llb_path);
		free(new_llb_path);
		return -1;
	}
	ret = write_buffer_to_output_state(output_state, old_llb_path, new_llb_buf, new_llb_len);
	if(ret != 0) {
		free(new_llb_buf);
		free(new_buildid);
		free(new_url);
		free(old_llb_path);
		free(new_llb_path);
		return -1;
	}
	free(old_llb_path);
	free(new_llb_path);


	LOG("Upgrading iBoot\n");

	char* old_iboot_path;
	char* new_iboot_path;
	ret = get_image_data(buildid, identifier, "iBoot", &key, &iv, &old_iboot_path);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		return -1;
	}
	free(key);
	free(iv);

	ret = get_image_data(new_buildid, identifier, "iBoot", &key, &iv, &new_iboot_path);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		free(old_iboot_path);
		return -1;
	}
	free(key);
	free(iv);

	char* new_iboot_buf;
	size_t new_iboot_len;
	ret = download_firmware_component(new_url, new_iboot_path, &new_iboot_buf, &new_iboot_len);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		free(old_iboot_path);
		free(new_iboot_path);
		return -1;
	}
	ret = write_buffer_to_output_state(output_state, old_iboot_path, new_iboot_buf, new_iboot_len);
	if(ret != 0) {
		free(new_buildid);
		free(new_url);
		free(old_iboot_path);
		free(new_iboot_path);
		free(new_iboot_buf);
		return -1;
	}
	free(new_buildid);
	free(new_url);

	free(old_iboot_path);
	free(new_iboot_path);

	return 0;
}

void libsolanine_set_log_cb(solanine_log_cb new_cb) {
	solanine_log = new_cb;
}

void libsolanine_init() {
	libsolanine_set_log_cb(&default_log_cb);
	hnd = curl_easy_init();
	char* argv = "";
	int argc = 0;
	init_libxpwn(&argc, &argv);
}

void libsolanine_exit() {
	curl_easy_cleanup(hnd);
	hnd = NULL;
}

int libsolanine_patch_ipsw(char* input, char* output, char* identifier) {
	int ret;
	OutputState* output_state = loadZip2(input, 1);
	if(!output_state) {
		return -1;
	}

	plist_t manifest = NULL;

	ret = load_manifest_from_output_state(&output_state, &manifest);
	if(ret != 0) {
		return -1;
	}

	plist_t node = plist_dict_get_item(manifest, "ProductBuildVersion");
	if (!node || plist_get_node_type(node) != PLIST_STRING) {
		plist_free(manifest);
		return -1;
	}
	char* buildid;
	plist_get_string_val(node, &buildid);

	node = plist_dict_get_item(manifest, "ProductBuildVersion");
	if (!node || plist_get_node_type(node) != PLIST_STRING) {
		plist_free(manifest);
		return -1;
	}
	char* board_config;
	ret = get_board_config(identifier, &board_config);
	if(ret != 0) {
		return -1;
	}

	LOG("Adding signed bootchain\n");
	ret = upgrade_bootchain(&output_state, identifier, buildid);
	if(ret != 0) {
		return -1;
	}

	LOG("Patching ramdisk\n");
	ret = patch_ramdisk(&output_state, identifier, buildid, board_config);
	if(ret != 0) {
		return -1;
	}

	free(board_config);
	free(buildid);
	writeOutput(&output_state, output);

	return 0;
}