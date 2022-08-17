#ifndef KERNEL_H
#define KERNEL_H

#include <mach/mach.h>
#include <mach-o/nlist.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach/vm_types.h>
#include <mach-o/loader.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/sysctl.h>

#define MACHO_HEADER_MAGIC MH_MAGIC
#define KERNEL_SEARCH_ADDRESS 0x81200000
#define IMAGE_OFFSET 0x1000
#define KERNEL_DUMP_SIZE 0xd00000
#define KERNEL_SIZE KERNEL_DUMP_SIZE + IMAGE_OFFSET
#define CHUNK_SIZE 2048

extern uint8_t* kernel_dump;

task_t get_kernel_task();
vm_address_t get_kernel_region();
uint32_t rk32(uint32_t kaddr);
int wk32(uint32_t addr, uint32_t val);
const char* kern_bootargs();
void dump_kernel(task_t kernel_task, vm_address_t kernel_base, uint8_t *dest);
int init_kernel();

#endif