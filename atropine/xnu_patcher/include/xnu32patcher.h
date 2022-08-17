#ifndef XNU32PATCHER_H
#define XNU32PATCHER_H

#include <libc/libc.h>

int patch_xnu(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
uint32_t find_rootvnode(void* kernel_base, uint32_t phys_base, uint32_t virt_base);

#endif