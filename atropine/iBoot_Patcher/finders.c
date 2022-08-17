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
#include <stdlib.h>
#include <include/finders.h>
#include <include/functions.h>
#include <include/iBoot32Patcher.h>

void* find_cmd_handler(void* iboot_buf, size_t iboot_len, char* cmd_name, size_t cmd_len) {

	uint32_t command_ref =(uint32_t)memmem(iboot_buf, iboot_len, cmd_name, cmd_len);
	if(!command_ref) {
		return NULL;
	}
	/* Shift the reference by 1 so that we have the address of "command\0" */
	command_ref += 1;

	/* The xref is the first attribute of the command struct */

	return memmem(iboot_buf, iboot_len, &command_ref, sizeof(uintptr_t));

}

void* find_jumpto(struct iboot_img* iboot_in) {
	/* In DFU loaders (iBSS and LLB), we call jumpto like this jumpto(2, load_address, 0); */

	/* loop through every movs r0, #2 */
	void* movs_r0_2 = movs_r0_2_search_down(iboot_in->buf, iboot_in->len);
	while(movs_r0_2) {

		/* If there's a nearby movs r2, #0 we've found the function */
		void* movs_r2_0 = movs_r2_0_search_down(movs_r0_2, 0x8);
		if(movs_r2_0) {
			void* bw = bw_search_down(movs_r2_0, 0x8);
			if(bw) {
				return resolve_bl32(bw);
			}
			void* bl = bl_search_down(movs_r2_0, 0x8);
			if(bl) {
				return resolve_bl32(bl);
			}
		}
		movs_r0_2 += 0x4;
		movs_r0_2 = movs_r0_2_search_down(movs_r0_2, iboot_in->len - (size_t)(movs_r0_2 - iboot_in->buf));
	}
	return NULL;
}


void* find_bl_verify_shsh(struct iboot_img* iboot_in) {
	int os_vers = get_os_version(iboot_in);

	/* Use the os-specific method for finding BL verify_shsh... */
	if(os_vers >= 5 && os_vers <= 7) {
		return find_bl_verify_shsh_5_6_7(iboot_in);
	}

	return find_bl_verify_shsh_generic(iboot_in);
}

void* find_bl_verify_shsh_5_6_7(struct iboot_img* iboot_in) {

	/* Find the MOVW Rx, #'RT' instruction... */
	void* movw = find_next_MOVW_insn_with_value(iboot_in->buf, iboot_in->len, 'RT');
	if(!movw) {
		return NULL;
	}


	/* Resolve the BL verify_shsh routine from found instruction... */
	void* bl_verify_shsh = find_bl_verify_shsh_insn(iboot_in, movw);
	if(!bl_verify_shsh) {
		return NULL;
	}



	return bl_verify_shsh;
}

void* find_bl_verify_shsh_generic(struct iboot_img* iboot_in) {

	/* Find the LDR Rx, ='CERT' instruction... */
	void* ldr_insn = find_next_LDR_insn_with_value(iboot_in, 'CERT');
	if(!ldr_insn) {
		return NULL;
	}


	/* Resolve the BL verify_shsh routine from found instruction... */
	void* bl_verify_shsh = find_bl_verify_shsh_insn(iboot_in, ldr_insn);
	if(!bl_verify_shsh) {
		return NULL;
	}



	return bl_verify_shsh;
}

void* find_bl_verify_shsh_insn(struct iboot_img* iboot_in, void* pc) {
	/* Find the top of the function... */
	void* function_top = find_verify_shsh_top(pc);
	if(!function_top) {
		return NULL;
	}

	/* Find the BL insn resolving to this function... (BL verify_shsh seems to only happen once) */
	void* bl_verify_shsh = find_next_bl_insn_to(iboot_in, (uint32_t) ((uintptr_t)GET_IBOOT_FILE_OFFSET(iboot_in, function_top)));
	if(!bl_verify_shsh) {
		return NULL;
	}

	return bl_verify_shsh;
}

void* find_verify_shsh_top(void* ptr) {
	void* top = push_r4_r7_lr_search_up(ptr, 0x500);
	if(!top) {
		return NULL;
	}
	top++; // Thumb
	return top;
}
