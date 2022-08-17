#ifndef PATCHERS_H
#define PATCHERS_H

#include <libc/libc.h>

int patch_amfi_substrate(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
int patch_mapforio(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
int patch_task_for_pid_0(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
int patch_i_can_has_debugger(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
int patch_sandbox(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
#endif