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
#include <include/iBoot32Patcher.h>

void* find_bl_verify_shsh(struct iboot_img* iboot_in) {
	int os_vers = get_os_version(iboot_in);

	/* Use the os-specific method for finding BL verify_shsh... */
	if(os_vers >= 5 && os_vers <= 7) {
		return find_bl_verify_shsh_5_6_7(iboot_in);
	}

	return find_bl_verify_shsh_generic(iboot_in);
}

void* find_bl_verify_shsh_5_6_7(struct iboot_img* iboot_in) {
	debug("Entering...\n");

	/* Find the MOVW Rx, #'RT' instruction... */
	void* movw = find_next_MOVW_insn_with_value(iboot_in->buf, iboot_in->len, 'RT');
	if(!movw) {
		debug("Unable to find MOVW instruction!");
		return 0;
	}

	debug("Found MOVW instruction at %p\n", GET_IBOOT_FILE_OFFSET(iboot_in, movw));

	/* Resolve the BL verify_shsh routine from found instruction... */
	void* bl_verify_shsh = find_bl_verify_shsh_insn(iboot_in, movw);
	if(!bl_verify_shsh) {
		debug("Unable to find a BL verify_shsh! (Image may already be patched?)");
		return 0;
	}

	debug("Found BL verify_shsh at %p\n", GET_IBOOT_FILE_OFFSET(iboot_in, bl_verify_shsh));

	debug("Leaving...\n");

	return bl_verify_shsh;
}

void* find_bl_verify_shsh_generic(struct iboot_img* iboot_in) {
	debug("Entering...\n");

	/* Find the LDR Rx, ='CERT' instruction... */
	void* ldr_insn = find_next_LDR_insn_with_value(iboot_in, 'CERT');
	if(!ldr_insn) {
		debug("Unable to find LDR insn!");
		return 0;
	}

	debug("Found LDR instruction at %p\n", GET_IBOOT_FILE_OFFSET(iboot_in, ldr_insn));

	/* Resolve the BL verify_shsh routine from found instruction... */
	void* bl_verify_shsh = find_bl_verify_shsh_insn(iboot_in, ldr_insn);
	if(!bl_verify_shsh) {
		debug("Unable to find a BL verify_shsh! (Image may already be patched?)");
		return 0;
	}

	debug("Found BL verify_shsh at %p\n", GET_IBOOT_FILE_OFFSET(iboot_in, bl_verify_shsh));

	debug("Leaving...\n");

	return bl_verify_shsh;
}

void* find_bl_verify_shsh_insn(struct iboot_img* iboot_in, void* pc) {
	/* Find the top of the function... */
	void* function_top = find_verify_shsh_top(pc);
	if(!function_top) {
		debug("Unable to find top of verify_shsh!");
		return 0;
	}

	/* Find the BL insn resolving to this function... (BL verify_shsh seems to only happen once) */
	void* bl_verify_shsh = find_next_bl_insn_to(iboot_in, (uint32_t) ((uintptr_t)GET_IBOOT_FILE_OFFSET(iboot_in, function_top)));
	if(!bl_verify_shsh) {
		return 0;
	}

	return bl_verify_shsh;
}

void* find_verify_shsh_top(void* ptr) {
	void* top = push_r4_r7_lr_search_up(ptr, 0x500);
	if(!top) {
		return 0;
	}
	top++; // Thumb
	return top;
}

