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

#include <stdlib.h>
#include <include/finders.h>
#include <include/functions.h>
#include <include/patchers.h>
#include <include/iBoot32Patcher.h>
#include <trampoline.h>

/* We remove the conditional checks around boot arguments, they'll always be added. */
int patch_boot_args(struct iboot_img* iboot_in) {

	/* Find the pre-defined boot-args from iBoot "rd=md0 ..." */
	void* default_boot_args_str_loc = memstr(iboot_in->buf, iboot_in->len, DEFAULT_BOOTARGS_STR);
	if(!default_boot_args_str_loc) {
		return -1;
	}

	/* Find the boot-args string xref within the kernel load routine. */
	void* default_boot_args_xref = iboot_memmem(iboot_in, default_boot_args_str_loc);
	if(!default_boot_args_xref) {
		return -1;
	}

	/* Patch out the conditional branches... */
	void* _ldr_rd_boot_args = ldr_to(default_boot_args_xref);
	if(!_ldr_rd_boot_args) {
		uintptr_t default_boot_args_str_loc_with_base = (uintptr_t) GET_IBOOT_FILE_OFFSET(iboot_in, default_boot_args_str_loc) + get_iboot_base_address(iboot_in->buf);

		_ldr_rd_boot_args = find_next_LDR_insn_with_value(iboot_in, (uint32_t) default_boot_args_str_loc_with_base);
		if(!_ldr_rd_boot_args) {
			return -1;
		}
	}

	struct arm32_thumb_LDR* ldr_rd_boot_args = (struct arm32_thumb_LDR*) _ldr_rd_boot_args;

	/* Find next CMP Rd, #0 instruction... */
	void* _cmp_insn = find_next_CMP_insn_with_value(ldr_rd_boot_args, 0x100, 0);
	if(!_cmp_insn) {
		return -1;
	}

	void* arm32_thumb_IT_insn = _cmp_insn;


	/* Find the next IT EQ/IT NE instruction following the CMP Rd, #0 instruction... (kinda hacky) */
	while(*(uint16_t*)arm32_thumb_IT_insn != ARM32_THUMB_IT_EQ && *(uint16_t*)arm32_thumb_IT_insn != ARM32_THUMB_IT_NE) {
		arm32_thumb_IT_insn = (void*)((uintptr_t)arm32_thumb_IT_insn + 1);
	}


	/* MOV Rd, Rs instruction usually follows right after the IT instruction. */
	struct arm32_thumb_hi_reg_op* mov_insn = (struct arm32_thumb_hi_reg_op*) ((uintptr_t)arm32_thumb_IT_insn + 2);


	/* Find the last LDR Rd which holds the null string pointer... */
	int null_str_reg = (ldr_rd_boot_args->rd == mov_insn->rs) ? mov_insn->rd : mov_insn->rs;

	/* + 0x10: Some iBoots have the null string load after the CMP instruction... */
	void* ldr_null_str = find_last_LDR_rd(((uintptr_t)_cmp_insn + 0x10), 0x200, null_str_reg);
	if(!ldr_null_str) {
		return -1;
	}


	/* Calculate the new PC relative load from the default boot args xref to the LDR Rd, =null_string location. */
	uint32_t diff = (uint32_t) ((uint32_t)GET_IBOOT_FILE_OFFSET(iboot_in, default_boot_args_xref) - (uint32_t)GET_IBOOT_FILE_OFFSET(iboot_in, ldr_null_str));

	/* T1 LDR PC-based instructions use the immediate 8 bits multiplied by 4. */
	struct arm32_thumb_LDR* ldr_rd_null_str = (struct arm32_thumb_LDR*) ldr_null_str;
	ldr_rd_null_str->imm8 = (diff / 4);

	return 0;
}


/* Patches the llb to load an image with tag 'ibox' instead of 'ibot' */
int patch_llb_load(struct iboot_img* iboot_in) {
	int os_vers = get_os_version(iboot_in);

	/* Use the os-specific method for patching the tag */
	if(os_vers >= 5 && os_vers <= 7) {
		/* Find the movw tx, #'ot' instruction... */
		void* movw_ot = find_next_MOVW_insn_with_value(iboot_in->buf, iboot_in->len, 'ot');
		if(!movw_ot) {
			return -1;
		}
		/* Set the movw to tx, #'ox' */
		set_MOVW_val(movw_ot, 'ox');
		return 0;

	}
	uint32_t ibot_tag = 'ibot';
	uint32_t* ibot_tag_ptr = memmem(iboot_in->buf, iboot_in->len, &ibot_tag, sizeof(uint32_t));
	if(!ibot_tag_ptr) {
		return -1;
	}
	*ibot_tag_ptr = 'ibox';
	return 0;
}

int patch_rsa_check(struct iboot_img* iboot_in) {

	/* Find the BL verify_shsh instruction... */
	void* bl_verify_shsh = find_bl_verify_shsh(iboot_in);
	if(!bl_verify_shsh) {
		return -1;
	}


	/* BL verify_shsh --> MOVS R0, #0; STR R0, [R3] */
	*(uint32_t*)bl_verify_shsh = bswap32(0x00201860);

	return 0;
}

int patch_command_handler(struct iboot_img* iboot_in) {

	/* Copy the iBSS trampoline to the free space at 0x2A0 */
	char* trampoline_dest = iboot_in->buf + 0x2A0;
	memcpy(trampoline_dest, trampoline, trampoline_length);
	
	/* Find the jumpto function */
	void* jumpto = find_jumpto(iboot_in);
	if(!jumpto) {
		return -1;
	}
	/* Add the address of the jumpto function to our trampoline so that we can call it */
	uint32_t jumpto_addr = (uintptr_t)GET_IBOOT_ADDR(iboot_in, jumpto);
	char* jumpto_addr_offset = (trampoline_dest + JUMPTO_ADDR_OFFSET);
	*(uint32_t*)(jumpto_addr_offset) = jumpto_addr;


	/* Replace each jumpto call with a call to our trampoline */
	char* jumpto_call = find_next_bl_insn_to(iboot_in, (uint32_t) ((uintptr_t)GET_IBOOT_FILE_OFFSET(iboot_in, jumpto)));
	if(!jumpto_call) {
		return -1;
	}
	while(jumpto_call) {
		build_bl(jumpto_call, trampoline_dest);
		jumpto_call += 4;
		jumpto_call = find_next_bl_insn_to(iboot_in, (uint32_t) ((uintptr_t)GET_IBOOT_FILE_OFFSET(iboot_in, jumpto)));
	}
	return 0;
}

int patch_ticket_check(struct iboot_img *iboot_in) {
#define pointer(p) (__pointer[0] = (uint32_t)p & 0xff, __pointer[1] = ((uint32_t)p / 0x100) & 0xff, __pointer[2] = ((uint32_t)p / 0x10000) & 0xff, __pointer[3] = ((uint32_t)p / 0x1000000) & 0xff, _pointer)
	char __pointer[4];
	char *_pointer = __pointer;
	char *NOPstart = NULL;
	char *NOPstop = NULL;

	/* find iBoot_vers_str */
	const char *iboot_vers_str = memstr(iboot_in->buf, iboot_in->len, "iBoot-");
	if (!iboot_vers_str) {
		return -1;
	}

	/* find pointer to vers_str (should be a few bytes below string) */
	uint32_t vers_str_iboot = (uint32_t)GET_IBOOT_ADDR(iboot_in, iboot_vers_str);
	char *str_pointer = MEMMEM_RELATIVE(iboot_in, iboot_vers_str, pointer(vers_str_iboot), 4);
	if (!str_pointer) {
		return -1;
	}


	/* find 3rd xref */
	uint32_t str_pointer_iboot = (uint32_t)GET_IBOOT_ADDR(iboot_in, str_pointer);
	char *iboot_str_3_xref = iboot_in->buf;
	for (int i = 0; i < 3; i++) {
		if (!(iboot_str_3_xref = MEMMEM_RELATIVE(iboot_in, iboot_str_3_xref + 1, pointer(str_pointer_iboot), 4))) {
			return -1;
		}
	}


	/* find ldr rx = iboot_str_3_xref */
	char *ldr_intruction = ldr_pcrel_search_up(iboot_str_3_xref, 0x100);
	if (!ldr_intruction) {
		return -1;
	}


	char *last_good_bl = bl_search_down(ldr_intruction, 0x100);
	if (!last_good_bl) {
		return -1;
	}
	last_good_bl += 4;

	char *next_pop = pop_search(last_good_bl, 0x100, 0);
	if (!next_pop) {
		return -1;
	}

	char *last_branch = branch_search(next_pop, 0x20, 1);
	char *prev_mov_r0_fail = pattern_search(next_pop, 0x20, bswap32(0x4ff0ff30), bswap32(0x4ff0ff30), -2);

	if (prev_mov_r0_fail && prev_mov_r0_fail > last_branch) {
		last_branch = prev_mov_r0_fail - 2; //last branch is a BL
	}

	if (!last_branch) {
		return -1;
	}
	uint32_t to_write[2];
	to_write[0] = bswap32(0x4ff00000);
	to_write[1] = bswap32(0x4ff00001);
	memcpy(last_good_bl, to_write, sizeof(to_write));
	last_good_bl += 8;

	NOPstart = last_good_bl;
	NOPstop = last_branch + 2;

	//because fuck clean patches

	while (NOPstart < NOPstop) {
		NOPstart[0] = 0x00;
		NOPstart[1] = 0xBF; //NOP
		NOPstart += 2;
	}

	if (*(uint32_t *)NOPstop == bswap32(0x4ff0ff30)) { //mov.w      r0, #0xffffffff
		/* mov.w      r0, #0xffffffff -->  mov.w      r0, #0x0 */
		*(uint32_t *)NOPstop = bswap32(0x4ff00000);
	}

	return 0;
}