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

#ifndef IBOOT32PATCHER_H
#define IBOOT32PATCHER_H

#include <common.h>
#include <stdbool.h>

#define bswap32(x) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | (( x >> 8) & 0xff00) | (( x << 24) & 0xff000000))
#define bswap16(x) ((x >> 8) | (x << 8))

#define GET_IBOOT_FILE_OFFSET(iboot_in, x) (x - (uintptr_t) iboot_in->buf)
#define GET_IBOOT_ADDR(iboot_in, x) (x - (uintptr_t) iboot_in->buf) + get_iboot_base_address(iboot_in->buf)

#define IMAGE3_MAGIC 'Img3'
#define IBOOT_VERS_STR_OFFSET 0x286
#define IBOOT32_RESET_VECTOR_BYTES bswap32(0x0E0000EA)

struct iboot_img {
	void* buf;
	size_t len;
	uint32_t VERS;
} __attribute__((packed));

struct iboot32_cmd_t {
	uint32_t cmd_str_ptr;
	uint32_t cmd_ptr;
	uint32_t cmd_desc_str_ptr;
} __attribute__((packed));

struct iboot64_cmd_t {
	uintptr_t cmd_str_ptr;
	uintptr_t cmd_ptr;
	uintptr_t cmd_desc_str_ptr;
} __attribute__((packed));

struct iboot_interval {
	uint32_t low, high, os;
} __attribute__((packed));

static const struct iboot_interval iboot_intervals[] = {
	{320, 590, 2},
	{594, 817, 3},
	{889, 1072, 4},
	{1218, 1220, 5},
	{1537, 1537, 6},
	{1940, 1940, 7},
	{2261, 2261, 8},
	{2817, 2817, 9},
	{3393, 3393, 10},
};

uint32_t get_iboot_base_address(void* iboot_buf);
int patch_iboot(char* buf, size_t len, char* boot_args);

#endif
