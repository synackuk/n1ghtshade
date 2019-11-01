#ifndef CONSTANTS_H
#define CONSTANTS_H

extern uintptr_t* base_address;
extern uintptr_t* load_address;
extern uintptr_t* framebuffer_address;
extern uint32_t display_width;
extern uint32_t display_height;
extern void* image_list;
extern int version;
extern command_descriptor* cmd_ptr_address;
extern uintptr_t* boot_args_string;
extern uintptr_t* kern_load_target;

typedef int (*printf_t)(const char *fmt, ...);
extern printf_t _printf;
#define printf _printf

typedef void* (*malloc_t)(size_t size);
extern malloc_t _malloc;
#define malloc _malloc

typedef char* (*get_env_t)(char* env);
extern get_env_t _get_env;
#define get_env _get_env

typedef uintptr_t (*get_env_uint_t)(char* env);
extern get_env_uint_t _get_env_uint;
#define get_env_uint _get_env_uint


typedef void (*jumpto_t)(int, void *, int, int) __attribute__((noreturn));
extern jumpto_t _jumpto;
#define jumpto _jumpto

typedef int (*aes_crypto_cmd_t)(int crypt_type, void *inbuf, void *outbuf, unsigned int inbuf_len, unsigned int aes_key_type, char *key, char *iv);
extern aes_crypto_cmd_t _aes_crypto_cmd;
#define aes_crypto_cmd _aes_crypto_cmd

typedef int (*fs_mount_t)(const char *partition, const char *fstype, const char *mountpoint);
extern fs_mount_t _fs_mount;
#define fs_mount _fs_mount

typedef int (*fs_load_file_t)(const char* path, void *address, unsigned int *size);
extern fs_load_file_t _fs_load_file;
#define fs_load_file _fs_load_file

typedef int (*set_env_uint_t)(char* env, uint32_t val, int unk);
extern set_env_uint_t _set_env_uint;
#define set_env_uint _set_env_uint

int init_constants();

#endif