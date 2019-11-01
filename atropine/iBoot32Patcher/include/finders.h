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

#ifndef FINDERS_H
#define FINDERS_H

#include <include/iBoot32Patcher.h>

void* find_bl_verify_shsh(struct iboot_img* iboot_in);
void* find_bl_verify_shsh_5_6_7(struct iboot_img* iboot_in);
void* find_bl_verify_shsh_generic(struct iboot_img* iboot_in);
void* find_bl_verify_shsh_insn(struct iboot_img* iboot_in, void* pc);
void* find_verify_shsh_top(void* ptr);

#endif
