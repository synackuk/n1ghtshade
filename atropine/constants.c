#include <common.h>

command_descriptor* cmd_ptr_address;
uintptr_t* base_address;
uintptr_t* load_address;
uintptr_t* kern_load_target;
printf_t _printf;
malloc_t _malloc;
get_env_t _get_env;
get_env_uint_t _get_env_uint;
uintptr_t* framebuffer_address;
uint32_t display_width;
uint32_t display_height;
jumpto_t _jumpto;
aes_crypto_cmd_t _aes_crypto_cmd;
fs_mount_t _fs_mount;
fs_load_file_t _fs_load_file;
void* image_list;
int version;
set_env_uint_t _set_env_uint;
uintptr_t* boot_args_string;

int init_constants() {
	base_address = BASE_ADDRESS;
	if(!base_address) {
		return -1;
	}	
	_printf = PRINTF_ADDRESS;
	if(!_printf) {
		return -1;
	}
	debug("base address: %p\n", base_address);
	debug("printf: %p\n", _printf);
	_malloc = MALLOC_ADDRESS;
	if(!_malloc) {
		return -1;
	}
	debug("malloc: %p\n", _malloc);

#ifdef MENU_COMMANDS
	cmd_ptr_address = (command_descriptor*)CMD_PTR_ADDRESS;
	if(!cmd_ptr_address) {
		return -1;
	}
	debug("cmd_ptr_address: %p\n", cmd_ptr_address);
	_jumpto = JUMPTO_ADDRESS;
	if(!_jumpto) {
		return -1;
	}
	debug("jumpto: %p\n", _jumpto);
#endif

#ifdef NVRAM
	_get_env = GET_ENV_ADDRESS;
	if(!_get_env) {
		return -1;
	}
	debug("get_env: %p\n", _get_env);
	_get_env_uint = GET_ENV_UINT_ADDRESS;
	if(!_get_env_uint) {
		return -1;
	}
	debug("get_env_uint: %p\n", _get_env_uint);
	load_address = LOAD_ADDRESS;
	if(!load_address) {
		return -1;
	}
	debug("load address: %p\n", load_address);

#ifdef DISPLAY_OUTPUT
	framebuffer_address = FRAMEBUFFER_ADDRESS;
	if(!framebuffer_address) {
		return -1;
	}
	debug("framebuffer address: %p\n", framebuffer_address);
	display_width = DISPLAY_WIDTH;
	if(!display_width) {
		return -1;
	}
	debug("display width: %u\n", display_width);
	display_height = DISPLAY_HEIGHT;
	if(!display_height) {
		return -1;
	}
	debug("display height: %u\n", display_height);
#endif
#endif
	_aes_crypto_cmd = AES_CRYPTO_CMD_ADDRESS;
	if(!_aes_crypto_cmd) {
		return -1;
	}
	debug("aes_crypto_cmd: %p\n", _aes_crypto_cmd);

	_fs_mount = FS_MOUNT_ADDRESS;
	if(!_fs_mount) {
		return -1;
	}
	debug("fs_mount: %p\n", _fs_mount);

	_fs_load_file = FS_LOAD_FILE_ADDRESS;
	if(!_fs_load_file) {
		return -1;
	}
	debug("fs_load_file: %p\n", _fs_load_file);

	image_list = IMAGE_LIST_ADDRESS;
	if(!image_list) {
		return -1;
	}
	debug("image_list: %p\n", image_list);

	version = VERSION;
	if(!version) {
		return -1;
	}
	debug("version: %d\n", version);

	_set_env_uint = SET_ENV_UINT_ADDRESS;
	if(!_set_env_uint) {
		return -1;
	}
	debug("set_env_uint: %p\n", _set_env_uint);

	boot_args_string = BOOT_ARGS_STRING_ADDRESS;
	if(!boot_args_string) {
		return -1;
	}
	debug("boot_args_string: %p\n", boot_args_string);

	kern_load_target = KERN_LOAD_TARGET_ADDRESS;
	if(!kern_load_target) {
		return -1;
	}
	debug("kern_load_target: %p\n", kern_load_target);

	debug("Initialised constants.\n");
	return 0;
}