#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <libc/libc.h>
#include <include/mach.h>


#define PHYS_TO_VIRT(x) (((uint32_t)x - (uint32_t)phys_base) + (uint32_t)virt_base)
#define VIRT_TO_PHYS(x) (((uint32_t)x - (uint32_t)virt_base) + (uint32_t)phys_base)

#define WRITE_IF_NOT_ZERO(test, set) if(test) test = set

struct segment_command *find_segment(struct mach_header *mh, const char *segname);
struct load_command *find_load_command(struct mach_header *mh, uint32_t cmd);
struct section *find_section(struct segment_command *seg, const char *name);
void* find_sym(struct mach_header *mh, const char *name, uint32_t phys_base, uint32_t virt_base);
uint32_t find_version(struct mach_header *mh);
void* find_kext_start(void* kernel_base, uint32_t phys_base, uint32_t virt_base, char* kext_name);
void* push_lr_search_up(const void* start_addr, int len);
int insn_is_bne(uint16_t* i);
int insn_is_beq(uint16_t* i);
int insn_is_beqw(uint16_t* i);
int insn_is_32bit(uint16_t* i);
void* ldr_to(const void* loc);
uint16_t* find_literal_ref(uint8_t* kdata, size_t ksize, uint16_t* insn, uintptr_t address);

#endif