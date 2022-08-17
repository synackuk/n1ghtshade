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
#include <string.h>
#include <stdint.h>

#include <include/arm32_defs.h>
#include <include/finders.h>
#include <include/functions.h>
#include <include/iBoot32Patcher.h>
#include <include/patchers.h>

int patch_iboot(void* buf, size_t len) {
	int ret = 0;
	struct iboot_img iboot_in;
	iboot_in.buf = buf;
	iboot_in.len = len;

	char* iboot_vers_str = (iboot_in.buf + IBOOT_VERS_STR_OFFSET);

	iboot_in.VERS = atoi(iboot_vers_str);

	ret = patch_rsa_check(&iboot_in);
	if(ret != 0) {
		return -1;
	}

#ifdef BUILD_SECUREROM
	/* This is allowed to fail - it's only relevent to tethered boots, not when booting an iBSS */
	patch_llb_load(&iboot_in);

	ret = patch_command_handler(&iboot_in);
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef BUILD_IBOOT
	ret = patch_boot_args(&iboot_in);
	if(ret != 0) {
		return -1;
	}
#endif

#ifdef BUILD_IBEC
	ret = patch_ticket_check(&iboot_in);
	if(ret != 0) {
		return -1;
	}
#endif

	return 0;
}
