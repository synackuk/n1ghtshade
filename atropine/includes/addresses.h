#ifndef ADDRESSES_H
#define ADDRESSES_H

#define BASE_ADDRESS find_base_address()
#define LOAD_ADDRESS find_load_address()
#define PRINTF_ADDRESS find_printf()
#define MALLOC_ADDRESS find_malloc()
#define CMD_PTR_ADDRESS find_cmd_ptr()
#define GET_ENV_ADDRESS find_get_env()
#define GET_ENV_UINT_ADDRESS find_get_env_uint()
#define FRAMEBUFFER_ADDRESS find_framebuffer_address()
#define DISPLAY_WIDTH find_display_width()
#define DISPLAY_HEIGHT find_display_height()
#define JUMPTO_ADDRESS find_jumpto()
#define AES_CRYPTO_CMD_ADDRESS find_aes_crypto_cmd()
#define FS_MOUNT_ADDRESS find_fs_mount()
#define FS_LOAD_FILE_ADDRESS find_fs_load_file()
#define IMAGE_LIST_ADDRESS find_image_list()
#define VERSION find_version()
#define SET_ENV_UINT_ADDRESS find_set_env_uint()
#define BOOT_ARGS_STRING_ADDRESS find_boot_args_string()
#define KERN_LOAD_TARGET_ADDRESS find_kern_load_target()

#endif