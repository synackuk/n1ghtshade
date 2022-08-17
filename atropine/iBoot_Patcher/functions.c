/*
 * Copyright 2013-2016, iH8sn0w. <iH8sn0w@iH8sn0w.com>
 *
 * This file is part of iBoot32Patcher.
 *
 * iBoot32Patcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iBoot32Patcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iBoot32Patcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <include/functions.h>
#include <include/iBoot32Patcher.h>

uint32_t sign_extend_11_32(uint32_t x) {
	const int bits = 11;
	uint32_t m = 1u << (bits - 1);
	return (x ^ m) - m;
}

void build_bl(void* insn, void* target) {
	uintptr_t pc = (uintptr_t)insn + 0x4;
	uintptr_t offset = (uintptr_t)target - pc;
	uint16_t high_bits = sign_extend_11_32(offset >> 12) - 0xff000;
	uint16_t low_bits = (offset & 0xFFF) >> 1;
	struct arm32_thumb_BL* bl = (struct arm32_thumb_BL*) insn;
	bl->h = 0x0;
	bl->offset = high_bits;
	bl->padd = 0xf;
	bl->h2 = 0x1;
	bl->offset2 = low_bits;
	bl->padd2 = 0xf;
}

void* bl_search_down(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0xD000F000, 0xD000F800, 1);
}

void* bw_search_down(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x9000F000, 0xD000F800, 2);
}

void* find_last_LDR_rd(uintptr_t start, size_t len, const uint8_t rd) {
	for(uintptr_t i = start; i > 0; i -= sizeof(uint16_t)) {
		void* prev_ldr = pattern_search((void*) i, len, 0x00004800, 0x0000F800, -2);
		struct arm32_thumb_LDR* ldr = (struct arm32_thumb_LDR*) prev_ldr;

		if(ldr == NULL) {
			break;
		} else if(ldr->rd == rd) {
			return (void*) prev_ldr;
		}
		i = ((uintptr_t) prev_ldr - sizeof(uint16_t));
	}
	return NULL;
}

void* find_next_bl_insn_to(struct iboot_img* iboot_in, uint32_t addr) {
	for(int i = 0; i < iboot_in->len - sizeof(uint32_t); i += 1) {
		void* possible_bl = iboot_in->buf + i;
		/* Verify we've got a bw or bl */
		if((*(uint32_t*)possible_bl & 0xD000F800) != 0xD000F000 && (*(uint32_t*)possible_bl & 0xD000F800) != 0x9000F000) {
			continue;
		}
		void* bl_addr = resolve_bl32(possible_bl);
		uint32_t resolved = (uintptr_t) GET_IBOOT_FILE_OFFSET(iboot_in, bl_addr);
		if(resolved == addr) {
			return (void*) (iboot_in->buf + i);
		}
	}
	return NULL;
}

void* find_next_CMP_insn_with_value(void* start, size_t len, const uint8_t val) {
	for(int i = 0; i < len; i += sizeof(uint16_t)) {
		struct arm32_thumb* insn = (struct arm32_thumb*) (start + i);
		if(insn->op == ARM32_THUMB_CMP && insn->offset == val) {
			return (void*) insn;
		}
	}
	return NULL;
}

void* find_next_LDR_insn_with_value(struct iboot_img* iboot_in, uint32_t value) {
	void* ldr_xref = (void*) memmem(iboot_in->buf, iboot_in->len, &value, sizeof(value));
	if(!ldr_xref) {
		return 0;
	}
	void* ldr_insn = ldr_to(ldr_xref);
	if(!ldr_insn) {
		return 0;
	}
	return ldr_insn;
}

void* find_next_MOVW_insn_with_value(void* start, size_t len, const uint16_t val) {
	for(int i = 0; i < len; i += sizeof(uint16_t)) {
		struct arm32_thumb_MOVW* candidate = (struct arm32_thumb_MOVW*) (start + i);
		if(is_MOVW_insn(start + i) && get_MOVW_val(candidate) == val) {
			return (void*) candidate;
		}
	}
	return NULL;
}

uint32_t get_iboot_base_address(void* iboot_buf) {
	return *(uint32_t*)(iboot_buf + 0x20) & ~0xFFFFF;
}

int get_os_version(struct iboot_img* iboot_in) {
	for(int i = 0; i < sizeof(iboot_intervals)/sizeof(struct iboot_interval); i++) {
		if(iboot_in->VERS >= iboot_intervals[i].low && iboot_in->VERS <= iboot_intervals[i].high) {
			return iboot_intervals[i].os;
		}
	}
	return 0;
}

uint16_t get_MOVW_val(struct arm32_thumb_MOVW* movw) {
	return (uint16_t) (((movw->imm4 << 4) + (movw->i << 3) + movw->imm3) << 8) + movw->imm8;
}

void set_MOVW_val(struct arm32_thumb_MOVW* movw, uint16_t val) {
	movw->imm8 = val & 0xFF;
	uint8_t byte2 = val >> 8;
	movw->imm3 = byte2 & 0b111;
	movw->i = (byte2 >> 3) & 1;
	movw->imm4 = (byte2 >> 4) & 0xF;
}

void* iboot_memmem(struct iboot_img* iboot_in, void* pat) {
	uint32_t new_pat = (uintptr_t) GET_IBOOT_ADDR(iboot_in, pat);

	return (void*) memmem(iboot_in->buf, iboot_in->len, &new_pat, sizeof(uint32_t));
}

bool is_MOVW_insn(void* offset) {
	struct arm32_thumb_MOVW* test = (struct arm32_thumb_MOVW*) offset;
	if(test->pad0 == 0x24 && test->pad1 == 0x1E && test->bit31 == 0) {
		return true;
	}
	return false;
}

void* ldr_pcrel_search_up(const void* start_addr, int len) {
	char *caddr = (char*)start_addr;
	for (unsigned char i = 0; i< 0xff; i++) {
		len -=4;
		if (len < 0)
			return NULL; //out of mem
		caddr -=4;
		
		uint32_t x = *(uint32_t*)caddr;
		if ((x & 0xF8FF0000) == (0x48000000 | (i<<16)))
			return (void*)(caddr+2);
		else if ((x & 0x0000F8FF) == (0x00004800 | i))
			return (void*)(caddr);
	}
	return NULL;
}

void* ldr_search_up(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x00004800, 0x0000F800, -1);
}

void* ldr32_search_up(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x0000F8DF, 0x0000FFFF, -1);
}

void* ldr_to(const void* loc) {
	uintptr_t xref_target = (uintptr_t)loc;
	uintptr_t i = xref_target;
	uintptr_t min_addr = xref_target - 0x420;
	for(; i > min_addr; i -= 2) {
		i = (uintptr_t)ldr_search_up((void*)i, i - min_addr);
		if (i == 0) {
			break;
		}

		uint32_t dw = *(uint32_t*)i;
		uintptr_t ldr_target = ((i + 4) & ~3) + ((dw & 0xff) << 2);
		if (ldr_target == xref_target) {
			return (void*)i;
		}
	}

	min_addr = xref_target - 0x1000;
	for(i = xref_target; i > min_addr; i -= 4) {
		i = (uintptr_t)ldr32_search_up((void*)i, i - min_addr);
		if (i == 0) {
			break;
		}
		uint32_t dw = *(uint32_t*)i;
		uintptr_t ldr_target = ((i + 4) & ~3) + ((dw >> 16) & 0xfff);
		if (ldr_target == xref_target) {
			return (void*)i;
		}
	}
	return NULL;
}

void* memstr(const void* mem, size_t size, const char* str) {
	return (void*) memmem(mem, size, str, strlen(str));
}

void* pattern_search(const void* addr, int len, int pattern, int mask, int step) {
	char* caddr = (char*)addr;
	if (len <= 0)
		return NULL;
	if (step < 0) {
		len = -len;
		len &= ~-(step+1);
	} else {
		len &= ~(step-1);
	}
	for (int i = 0; i != len; i += step) {
		uint32_t x = *(uint32_t*)(caddr + i);
		if ((x & mask) == pattern)
			return (void*)(caddr + i);
	}
	return NULL;
}

void* push_r4_r7_lr_search_up(const void* start_addr, int len) {
	// F0 B5
	return pattern_search(start_addr, len, 0x0000B5F0, 0x0000FFFF, -2);
}

void* movs_r0_2_search_down(const void* start_addr, int len) {
	// 02 20
	return pattern_search(start_addr, len, 0x00002002, 0x0000FFFF, 2);
}

void* movs_r2_0_search_down(const void* start_addr, int len) {
	// 00 22
	return pattern_search(start_addr, len, 0x00002200, 0x0000FFFF, 2);
}


void* pop_search(const void* start_addr, int len, int searchup) {
	return pattern_search(start_addr, len, 0b1011110<<9, 0xFE<<8, 2 - 4 * (searchup!=0));
}

void* branch_thumb_unconditional_search(const void* start_addr, int len, int searchup) {
	return pattern_search(start_addr, len, 0b11100<<11, 0b11111<<11, 2 - 4 * (searchup!=0));
}

void* branch_thumb_conditional_search(const void* start_addr, int len, int searchup) {
	return pattern_search(start_addr, len, 0b1101<<12, 0b1111<<12, 2 - 4 * (searchup!=0));
}

void* branch_search(const void* start_addr, int len, int searchup) {
	void *ret = 0;
	void *tmp = 0;
	if ((tmp = branch_thumb_unconditional_search(start_addr, len, searchup))){
		if (!ret || ((!searchup && tmp < ret) || (searchup && tmp > ret)))
			ret = tmp;
	}else if ((tmp = branch_thumb_conditional_search(start_addr, len, searchup))){
		if (!ret || ((!searchup && tmp < ret) || (searchup && tmp > ret)))
			ret = tmp;
	}
	return ret;
}

/* Taken from saurik's substrate framework. (See Hooker.cpp) */
void* resolve_bl32(const void* bl) {
	union {
		uint16_t value;

		struct {
			uint16_t immediate : 10;
			uint16_t s : 1;
			uint16_t : 5;
		};
	} bits = {*(uint16_t*)bl};

	union {
		uint16_t value;

		struct {
			uint16_t immediate : 11;
			uint16_t j2 : 1;
			uint16_t x : 1;
			uint16_t j1 : 1;
			uint16_t : 2;
		};
	} exts = {((uint16_t*)bl)[1]};

	int32_t jump = 0;
	jump |= bits.s << 24;
	jump |= (~(bits.s ^ exts.j1) & 0x1) << 23;
	jump |= (~(bits.s ^ exts.j2) & 0x1) << 22;
	jump |= bits.immediate << 12;
	jump |= exts.immediate << 1;
	jump |= exts.x;
	jump <<= 7;
	jump >>= 7;

	return (void*) (bl + 4 + jump);
}

