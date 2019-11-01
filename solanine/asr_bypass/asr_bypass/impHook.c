/*
 *  impHook.cpp
 *  impHook
 *
 *  Created by msftguy on 6/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "asr_bypass.h"

int import_api_verbose = 0;

uintptr_t* get_import_ptr(const macho_header* mh, const char* importName) {
	const macho_nlist*	symbolTable = NULL;
	const char*			stringTable = NULL;
	const uint8_t*		linkEditBase = NULL;
	const uint32_t*		indirectSymbolTable = NULL;
	intptr_t			slide = 0;
	
	// do this work only on first call
	uint32_t i;
	// symbol table, indirect symbol table
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	// first pass at load commands gets linkEditBase
	for (i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const macho_segment_command* seg = (macho_segment_command*)cmd;
			if ( strcmp(seg->segname,"__TEXT") == 0 ) 
				slide = (uintptr_t)mh - seg->vmaddr;
			else if ( strcmp(seg->segname,"__LINKEDIT") == 0 ) 
				linkEditBase = (uint8_t*)(seg->vmaddr + slide - seg->fileoff);
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	// next pass at load commands gets symbolTable, stringTable
	cmd = cmds;
	for (i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd ) {
			case LC_SYMTAB:
			{
				const struct symtab_command* symtab = (struct symtab_command*)cmd;
				stringTable = (const char*)&linkEditBase[symtab->stroff];
				symbolTable = (macho_nlist*)(&linkEditBase[symtab->symoff]);
			}
				break;
			case LC_DYSYMTAB:
			{
				const struct dysymtab_command* dsymtab = (struct dysymtab_command*)cmd;
				indirectSymbolTable = (uint32_t*)(&linkEditBase[dsymtab->indirectsymoff]);
			}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	
	// find lazy dylib pointer section
	//	const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
	cmd = cmds;
	// walk sections to find one with this lazy pointer
	for (i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const macho_segment_command* seg = (macho_segment_command*)cmd;
			const macho_section* const sectionsStart = (macho_section*)((char*)seg + sizeof(macho_segment_command));
			const macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
			const macho_section* sect;
			for (sect=sectionsStart; sect < sectionsEnd; ++sect) {
				const uint8_t type = sect->flags & SECTION_TYPE;
				if ( type == S_LAZY_DYLIB_SYMBOL_POINTERS 
					|| type == S_LAZY_SYMBOL_POINTERS 
					|| type == S_NON_LAZY_SYMBOL_POINTERS ) {
					const uint32_t pointerCount = sect->size / sizeof(uintptr_t);
					uintptr_t* const symbolPointers = (uintptr_t*)(sect->addr + slide);
					const uint32_t indirectTableOffset = sect->reserved1;
					for (uint32_t lazyIndex = 0; lazyIndex < pointerCount; lazyIndex++) {
						uint32_t symbolIndex = indirectSymbolTable[indirectTableOffset + lazyIndex];
						if ( symbolIndex != INDIRECT_SYMBOL_ABS && symbolIndex != INDIRECT_SYMBOL_LOCAL ) {
							// found symbol for this lazy pointer, now lookup address
							const char* symbolName = &stringTable[symbolTable[symbolIndex].n_un.n_strx];
							uintptr_t* result = symbolPointers + lazyIndex;
							if (import_api_verbose > 0) {
								printf("%s at %p\n", symbolName, result);
							}
							if (0 == strcmp(symbolName, importName)) {
								return result;
							}
						}
					}
				}
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}	
	return NULL;
}