#ifndef FINDERS_H
#define FINDERS_H

#define MASK(x, y, z) (((x) >> (y)) & ((1 << (z)) - 1))

uintptr_t* find_base_address();
uintptr_t* find_load_address();
uintptr_t* find_cmd_ptr();
printf_t find_printf();
malloc_t find_malloc();
get_env_t find_get_env();
get_env_uint_t find_get_env_uint();
uintptr_t* find_framebuffer_address();
uintptr_t* find_display_timing_address();
uint32_t find_display_width();
uint32_t find_display_height();
jumpto_t find_jumpto();
aes_crypto_cmd_t find_aes_crypto_cmd();
fs_mount_t find_fs_mount();
fs_load_file_t find_fs_load_file();
void* find_image_list();
int find_version();
set_env_uint_t find_set_env_uint();
uintptr_t* find_boot_args_string();
uintptr_t* find_kern_load_target();

#endif