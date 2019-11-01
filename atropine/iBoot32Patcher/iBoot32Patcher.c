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

#include <include/arm32_defs.h>
#include <include/finders.h>
#include <include/functions.h>
#include <include/iBoot32Patcher.h>
#include <include/patchers.h>

int patch_iboot(char* buf, size_t len, char* boot_args) {
	int ret = 0;
	struct iboot_img iboot_in;

	memset(&iboot_in, 0, sizeof(iboot_in));

	debug("Starting...\n");

	iboot_in.len = len;
	iboot_in.buf = buf;
	
	uint32_t image_magic = *(uint32_t*)iboot_in.buf;
	
	if(image_magic == IMAGE3_MAGIC) {
		error("The supplied image appears to be in an img3 container. Please ensure that the image is decrypted and that the img3 header is stripped.");
		return -1;
	}

	if(image_magic != IBOOT32_RESET_VECTOR_BYTES) {
		error("The supplied image is not a valid 32-bit iBoot.");
		return -1;
	}

	const char* iboot_vers_str = (iboot_in.buf + IBOOT_VERS_STR_OFFSET);

	iboot_in.VERS = atoi(iboot_vers_str);
	if(!iboot_in.VERS) {
		error("No iBoot version found!");
		return -1;
	}

	debug("iBoot-%d inputted.\n", iboot_in.VERS);
	
	/* Check to see if the loader has a kernel load routine before trying to apply custom boot args + debug-enabled override. */
	if(has_kernel_load(&iboot_in)) {
		if(boot_args) {
			ret = patch_boot_args(&iboot_in, boot_args);
			if(!ret) {
				error("Error doing patch_boot_args()!");
				return -1;
			}
		}
		ret = patch_ticket_check(&iboot_in);
		if(!ret) {
			error("Error doing patch_ticket_check()!");
			return -1;
		}

	}

	/* All loaders have the RSA check. */
	
	ret = patch_rsa_check(&iboot_in);
	if(!ret) {
		error("Error doing patch_rsa_check()!");
		return -1;
	}

	debug("Quitting...\n");
	return 0;
}
