#include <common.h>
#include <include/functions.h>

// Uses code from CBPatcher.

#define BX_LR 0x4770

/* Find start of a load command in a macho */
static struct load_command *find_load_command(struct mach_header *mh, uint32_t cmd)
{
    struct load_command *lc, *flc;
    
    lc = (struct load_command *)((uintptr_t)mh + sizeof(struct mach_header));
    
    while (1) {
        if ((uintptr_t)lc->cmd == cmd) {
            flc = (struct load_command *)(uintptr_t)lc;
            break;
        }
        lc = (struct load_command *)((uintptr_t)lc + (uintptr_t)lc->cmdsize);
    }
    return flc;
}

/* Find offset of an exported symbol in a macho */
static void* find_sym(struct mach_header *mh, const char *name) {
    struct segment_command* first = (struct segment_command*) find_load_command(mh, LC_SEGMENT);
    struct symtab_command* symtab = (struct symtab_command*) find_load_command(mh, LC_SYMTAB);
    vm_address_t vmaddr_slide = (vm_address_t)mh - (vm_address_t)first->vmaddr;
    
    char* sym_str_table = (char*) (((char*)mh) + symtab->stroff);
    struct nlist* sym_table = (struct nlist*)(((char*)mh) + symtab->symoff);
    
    for (int i = 0; i < symtab->nsyms; i++) {
        if (sym_table[i].n_value && !strcmp(name,&sym_str_table[sym_table[i].n_un.n_strx])) {
            return (void*)(uintptr_t)(sym_table[i].n_value + vmaddr_slide);
        }
    }
    return 0;
}

int patch_amfi(char* address) {
	uint32_t memcmp_func = (uint32_t)find_sym((void*)address, "_memcmp");
	if(!memcmp_func) {
		return -1;
	}
	memcmp_func -= (uint32_t)address;

	uint32_t memcmp_address = (memcmp_func) + KERNEL_BASE_ADDRESS + 1;
	uint32_t mach_msg_rpc_from_kernel_proper_func = (uint32_t)find_sym((void*)address, "_mach_msg_rpc_from_kernel_proper");
	if(!mach_msg_rpc_from_kernel_proper_func) {
		return -1;
	}
	mach_msg_rpc_from_kernel_proper_func -= (uint32_t)address;
	uint32_t mach_msg_rpc_from_kernel_proper_address = (mach_msg_rpc_from_kernel_proper_func) + KERNEL_BASE_ADDRESS + 1;	
	uint32_t bx_lr_gadget = 0;
	for(uint32_t i = 0; i < 0xF00000; i++) {
		if(*(uint32_t*)&address[i] == 0x47702000) {
			bx_lr_gadget = i + KERNEL_BASE_ADDRESS + 1;
			break;
		}
	}
	if(!bx_lr_gadget) {
		return -1;
	}

	uint32_t* overwrite = 0;
	for(uint32_t i = 0; i < 0xF00000; i++) {
		if(*(uint32_t*)&address[i] == mach_msg_rpc_from_kernel_proper_address) {
			if(*(uint32_t*)&address[i + sizeof(uint32_t)] == memcmp_address) {
				overwrite = (uint32_t*)(address + i + sizeof(uint32_t));
				break;
			}
		}
	}
	if(!overwrite) {
		return -1;
	}

	*(uint32_t*)overwrite = bx_lr_gadget;
	return 0;
}

int patch_sandbox(char* address) {
	uint32_t* mac_label_get_ptr = find_sym((void*)address, "_mac_label_get");
	if(!mac_label_get_ptr) {
		return -1;
	}
	*(uint32_t*)mac_label_get_ptr = 0x47702000;
	return 0;
}

int patch_pe_i_can_has_debugger(char* address) {
	uint32_t* pe_i_can_has_debugger_ptr = find_sym((void*)address, "_PE_i_can_has_debugger");
	if(!pe_i_can_has_debugger_ptr) {
		return -1;
	}
	uint32_t* overwrite = 0;
	for(int i = 0; i < 0x100; i++) {
		if(*(uint16_t*)(pe_i_can_has_debugger_ptr + i) == BX_LR) {
			overwrite = (pe_i_can_has_debugger_ptr + i) - 4;
		}
	}
	if(!overwrite) {
		return -1;
	}

	*(uint32_t*)overwrite = 0x20012001;
	return 0;
}

int patch_kernel(char* address) {
	int ret;
	ret = patch_amfi(address);
	if(ret != 0) {
		return -1;
	}
	ret = patch_sandbox(address);
	if(ret != 0) {
		return -1;
	}
	ret = patch_pe_i_can_has_debugger(address);
	if(ret != 0) {
		return -1;
	}
	return 0;
}

void hooker(void* func_address, void* address) {
	int ret;
	ret = patch_kernel(address);
	if(ret != 0) {
		fb_print("hanging here.");
		while(1) {}
	}
	return;
}

void hook_kernel() {
	void* hook = &hooker;
	memcpy(kern_load_target, &hook, 4);
	void* target = ldr_to(kern_load_target);
	if(!target) {
		return;
	}
	void* bl = bl_search_down(target, 8);
	if(!bl) {
		return;
	}
	memcpy(bl, "\x80\x47\x00\xbf", 4);
}