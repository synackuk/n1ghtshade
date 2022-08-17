#ifndef FINDERS_H
#define FINDERS_H

#include <libc/libc.h>
#include <xnu_patcher/include/mach.h>

void* find_amfi_substrate_function_top(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
void* find_map_for_io_locked(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
void* find_task_for_pid(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
void* find_i_can_has_debugger(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
void* find_sbops(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
void* find_kernel_entry(char* kernel_base);
mach_trap_old_t* find_old_mach_trap_table(void* kernel_base, uint32_t phys_base, uint32_t virt_base);
mach_trap_t* find_mach_trap_table(void* kernel_base, uint32_t phys_base, uint32_t virt_base);

#endif