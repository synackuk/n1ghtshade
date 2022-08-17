#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdint.h>
#include <stddef.h>

#define IBOOT_LEN 0x50000

typedef size_t (*get_env_uint_t)(char* name, size_t default_val);
typedef char* (*get_env_t)(char* name);
typedef int (*printf_t)(char* fmt, ...);
typedef void (*jumpto_t)(int type, void* jump, void* arg) __attribute__((noreturn));

extern uint32_t* base_address;
extern uint32_t* framebuffer_address;
extern uint32_t* display_timings_address;
extern uint32_t display_width;
extern uint32_t display_height;
extern printf_t printf;
extern get_env_uint_t get_env_uint;
extern get_env_t get_env;
extern jumpto_t jumpto;

int find_functions();
void* quick_find(char* str, size_t len);
void* last_xref(char* pattern, size_t len);
printf_t find_printf();
get_env_uint_t find_get_env_uint();
get_env_t find_get_env();
uint32_t* find_framebuffer_address();
uint32_t* find_display_timings_address();
uint32_t find_display_width();
uint32_t find_display_height();
jumpto_t find_jumpto_func();

#endif