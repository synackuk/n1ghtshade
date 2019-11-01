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

#include <include/finders.h>
#include <include/functions.h>
#include <include/patchers.h>
#include <include/iBoot32Patcher.h>

#define MEMMEM_RELATIVE(iboot_in, bufstart, needle, needleLen) memmem(bufstart, iboot_in->len - ((char*)(bufstart) - (char*)iboot_in->buf), needle, needleLen)


int patch_boot_args(struct iboot_img* iboot_in, const char* boot_args) {
	debug("Entering...\n");

	/* Find the pre-defined boot-args from iBoot "rd=md0 ..." */
	void* default_boot_args_str_loc = memstr(iboot_in->buf, iboot_in->len, DEFAULT_BOOTARGS_STR);
	if(!default_boot_args_str_loc) {
		debug("Unable to find default boot-args string!");
		return 0;
	}
	debug("Default boot-args string is at %p\n", (void*) GET_IBOOT_FILE_OFFSET(iboot_in, default_boot_args_str_loc));

	/* Find the boot-args string xref within the kernel load routine. */
	void* default_boot_args_xref = iboot_memmem(iboot_in, default_boot_args_str_loc);
	if(!default_boot_args_xref) {
		debug("Unable to find default boot-args string xref!");
		return 0;
	}
	debug("boot-args xref is at %p\n", (void*) GET_IBOOT_FILE_OFFSET(iboot_in, default_boot_args_xref));

	/* If new boot-args length exceeds the pre-defined one in iBoot, we need to point the xref somewhere else... */
	if(strlen(boot_args) > strlen(DEFAULT_BOOTARGS_STR)) {
		debug("Relocating boot-args string...\n");

		/* Find the "Reliance on this cert..." string. */
		char* reliance_cert_str_loc = (char*) memstr(iboot_in->buf, iboot_in->len, RELIANCE_CERT_STR);
		if(!reliance_cert_str_loc) {
			debug("Unable to find \"%s\" string!", RELIANCE_CERT_STR);
			return 0;
		}
		debug("\"%s\" string found at %p\n", RELIANCE_CERT_STR, GET_IBOOT_FILE_OFFSET(iboot_in, reliance_cert_str_loc));

		/* Point the boot-args xref to the "Reliance on this cert..." string. */
		debug("Pointing default boot-args xref to %p...\n", GET_IBOOT_ADDR(iboot_in, reliance_cert_str_loc));
		*(uint32_t*)default_boot_args_xref = (uintptr_t) GET_IBOOT_ADDR(iboot_in, reliance_cert_str_loc);

		default_boot_args_str_loc = reliance_cert_str_loc;
	}
	debug("Applying custom boot-args \"%s\"\n", boot_args);
	strcpy(default_boot_args_str_loc, boot_args);

	/* This is where things get tricky... (Might run into issues on older loaders)*/

	/* Patch out the conditional branches... */
	void* _ldr_rd_boot_args = ldr_to(default_boot_args_xref);
	if(!_ldr_rd_boot_args) {
		uintptr_t default_boot_args_str_loc_with_base = (uintptr_t) GET_IBOOT_FILE_OFFSET(iboot_in, default_boot_args_str_loc) + get_iboot_base_address(iboot_in->buf);

		_ldr_rd_boot_args = find_next_LDR_insn_with_value(iboot_in, (uint32_t) default_boot_args_str_loc_with_base);
		if(!_ldr_rd_boot_args) {
			debug("debug locating LDR Rx, =boot_args!");
			return 0;
		}
	}

	struct arm32_thumb_LDR* ldr_rd_boot_args = (struct arm32_thumb_LDR*) _ldr_rd_boot_args;
	debug("Found LDR R%d, =boot_args at %p\n", ldr_rd_boot_args->rd, GET_IBOOT_FILE_OFFSET(iboot_in, _ldr_rd_boot_args));

	/* Find next CMP Rd, #0 instruction... */
	void* _cmp_insn = find_next_CMP_insn_with_value(ldr_rd_boot_args, 0x100, 0);
	if(!_cmp_insn) {
		debug("debug locating next CMP instruction!");
		return 0;
	}

	struct arm32_thumb* cmp_insn = (struct arm32_thumb*) _cmp_insn;
	void* arm32_thumb_IT_insn = _cmp_insn;

	debug("Found CMP R%d, #%d at %p\n", cmp_insn->rd, cmp_insn->offset, GET_IBOOT_FILE_OFFSET(iboot_in, _cmp_insn));

	/* Find the next IT EQ/IT NE instruction following the CMP Rd, #0 instruction... (kinda hacky) */
	while(*(uint16_t*)arm32_thumb_IT_insn != ARM32_THUMB_IT_EQ && *(uint16_t*)arm32_thumb_IT_insn != ARM32_THUMB_IT_NE) {
		arm32_thumb_IT_insn++;
	}

	debug("Found IT EQ/IT NE at %p\n", GET_IBOOT_FILE_OFFSET(iboot_in, arm32_thumb_IT_insn));

	/* MOV Rd, Rs instruction usually follows right after the IT instruction. */
	struct arm32_thumb_hi_reg_op* mov_insn = (struct arm32_thumb_hi_reg_op*) (arm32_thumb_IT_insn + 2);

	debug("Found MOV R%d, R%d at %p\n", mov_insn->rd, mov_insn->rs, GET_IBOOT_FILE_OFFSET(iboot_in, arm32_thumb_IT_insn + 2));

	/* Find the last LDR Rd which holds the null string pointer... */
	int null_str_reg = (ldr_rd_boot_args->rd == mov_insn->rs) ? mov_insn->rd : mov_insn->rs;

	/* + 0x10: Some iBoots have the null string load after the CMP instruction... */
	void* ldr_null_str = find_last_LDR_rd((uintptr_t) (_cmp_insn + 0x10), 0x200, null_str_reg);
	if(!ldr_null_str) {
		debug("Unable to find LDR R%d, =null_str", null_str_reg);
		return 0;
	}

	debug("Found LDR R%d, =null_str at %p\n", null_str_reg, GET_IBOOT_FILE_OFFSET(iboot_in, ldr_null_str));

	/* Calculate the new PC relative load from the default boot args xref to the LDR Rd, =null_string location. */
	uint32_t diff = (uint32_t) (GET_IBOOT_FILE_OFFSET(iboot_in, default_boot_args_xref) - GET_IBOOT_FILE_OFFSET(iboot_in, ldr_null_str));

	/* T1 LDR PC-based instructions use the immediate 8 bits multiplied by 4. */
	struct arm32_thumb_LDR* ldr_rd_null_str = (struct arm32_thumb_LDR*) ldr_null_str;
	debug("Pointing LDR R%d, =null_str to boot-args xref...\n", ldr_rd_null_str->rd);
	ldr_rd_null_str->imm8 = (diff / 0x4);

	debug("Leaving...\n");
	return 1;
}

int patch_rsa_check(struct iboot_img* iboot_in) {
    debug("Entering...\n");
    
    /* Find the BL verify_shsh instruction... */
    void* bl_verify_shsh = find_bl_verify_shsh(iboot_in);
    if(!bl_verify_shsh) {
        debug("Unable to find BL verify_shsh!");
        return 0;
    }
    
    debug("Patching BL verify_shsh at %p...\n", GET_IBOOT_FILE_OFFSET(iboot_in, bl_verify_shsh));
    
    /* BL verify_shsh --> MOVS R0, #0; STR R0, [R3] */
    *(uint32_t*)bl_verify_shsh = bswap32(0x00201860);
    
    debug("Leaving...\n");
    return 1;
}

int patch_ticket_check(struct iboot_img* iboot_in) {
#define pointer(p) (__pointer[0] = (uint32_t)p & 0xff, __pointer[1] = ((uint32_t)p/0x100) & 0xff, __pointer[2] = ((uint32_t)p/0x10000) & 0xff, __pointer[3] = ((uint32_t)p/0x1000000) & 0xff, _pointer)
    char __pointer[4];
    char *_pointer = __pointer;
    printf("%s: Entering...\n", __FUNCTION__);
    char *NOPstart = NULL;
    char *NOPstop = NULL;
    
    /* find iBoot_vers_str */
    const char* iboot_vers_str = memstr(iboot_in->buf, iboot_in->len, "iBoot-");
    if (!iboot_vers_str) {
        printf("%s: Unable to find iboot_vers_str!\n", __FUNCTION__);
        return 0;
    }
    printf("%s: Found iBoot baseaddr %p\n", __FUNCTION__, get_iboot_base_address(iboot_in->buf));
    printf("%s: Found iboot_vers_str at %p\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, iboot_vers_str));
    
    
    /* find pointer to vers_str (should be a few bytes below string) */
    uint32_t vers_str_iboot = (uint32_t)GET_IBOOT_ADDR(iboot_in,iboot_vers_str);
    char *str_pointer = MEMMEM_RELATIVE(iboot_in, iboot_vers_str, pointer(vers_str_iboot), 4);
    if (!str_pointer) {
        printf("%s: Unable to find str_pointer!\n", __FUNCTION__);
        return 0;
    }
    
    printf("%s: Found str_pointer at %p\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, str_pointer));


    /* find 3rd xref */
    uint32_t str_pointer_iboot = (uint32_t)GET_IBOOT_ADDR(iboot_in,str_pointer);
    char *iboot_str_3_xref = iboot_in->buf;
    for (int i=0; i<3; i++) {
        if (!(iboot_str_3_xref = MEMMEM_RELATIVE(iboot_in, iboot_str_3_xref+1, pointer(str_pointer_iboot), 4))){
            printf("%s: Unable to find %d iboot_str_3_xref!\n", __FUNCTION__,i+1);
            return 0;
        }
    }
    
    printf("%s: Found iboot_str_3_xref at %p\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, iboot_str_3_xref));
    
    /* find ldr rx = iboot_str_3_xref */
    char *ldr_intruction = ldr_pcrel_search_up(iboot_str_3_xref, 0x100);
    if (!ldr_intruction) {
        printf("%s: Unable to find ldr_intruction!\n", __FUNCTION__);
        return 0;
    }
    
    printf("%s: Found ldr_intruction at %p\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, ldr_intruction));
    
    char *last_good_bl = bl_search_down(ldr_intruction,0x100);
    if (!last_good_bl) {
        printf("%s: Unable to find last_good_bl!\n", __FUNCTION__);
        return 0;
    }
    printf("%s: Found last_good_bl at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, last_good_bl));
    last_good_bl +=4;
    
    char *next_pop = pop_search(last_good_bl,0x100,0);
    if (!next_pop) {
        printf("%s: Unable to find next_pop!\n", __FUNCTION__);
        return 0;
    }
    printf("%s: Found next_pop at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, next_pop));
    printf("%s: Found next_pop at %p...\n", __FUNCTION__, GET_IBOOT_ADDR(iboot_in, next_pop));
    
    char *last_branch = branch_search(next_pop,0x20,1);
    char *prev_mov_r0_fail = pattern_search(next_pop, 0x20, bswap32(0x4ff0ff30), bswap32(0x4ff0ff30), -2);

    if (prev_mov_r0_fail && prev_mov_r0_fail > last_branch) {
        printf("%s: Detected prev_mov_r0_fail at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, prev_mov_r0_fail));
        last_branch = prev_mov_r0_fail-2; //last branch is a BL
    }
    
    if (!last_branch) {
        printf("%s: Unable to find last_branch!\n", __FUNCTION__);
        return 0;
    }
    printf("%s: Found last_branch at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, last_branch));
    
    
    printf("%s: Patching in mov.w r0, #0 at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, last_good_bl));
    *(uint32_t*)last_good_bl = bswap32(0x4ff00000);
    last_good_bl +=4;
    
    printf("%s: Patching in mov.w r1, #0 at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, last_good_bl));
    *(uint32_t*)last_good_bl = bswap32(0x4ff00001);
    last_good_bl +=4;
    
    NOPstart = last_good_bl;
    NOPstop = last_branch+2;
    
    //because fuck clean patches
    printf("%s: NOPing useless stuff at %p to %p ...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, NOPstart),  GET_IBOOT_FILE_OFFSET(iboot_in, NOPstop));
    
    while (NOPstart<NOPstop) {
        NOPstart[0] = 0x00;
        NOPstart[1] = 0xBF; //NOP
        NOPstart +=2;
    }
    
    if (*(uint32_t*)NOPstop == bswap32(0x4ff0ff30)){ //mov.w      r0, #0xffffffff
        printf("%s: Detected mov r0, #0xffffffff at NOPstop\n", __FUNCTION__);
        printf("%s: Applying additional mov.w r0, #0 patch at %p...\n", __FUNCTION__, GET_IBOOT_FILE_OFFSET(iboot_in, NOPstop));
        /* mov.w      r0, #0xffffffff -->  mov.w      r0, #0x0 */
        *(uint32_t*)NOPstop = bswap32(0x4ff00000);
    }
        
    
    printf("%s: Leaving...\n", __FUNCTION__);
    return 1;
}
