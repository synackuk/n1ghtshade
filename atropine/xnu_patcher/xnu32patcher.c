#include <libc/libc.h>
#include <include/xnu32patcher.h>
#include <include/patchers.h>
#include <include/functions.h>

int patch_xnu(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	int ret = 0;

	uint32_t version = find_version(kernel_base);
	if(!version) {
		return -1;
	}

	/* The AMFI substrate patch and mapForIO patch only exist in iOS 8+ */ 
	if(version >= 0x00080000) {
		ret = patch_mapforio(kernel_base, phys_base, virt_base);
		if(ret != 0) {
			return -1;
		}
		ret = patch_amfi_substrate(kernel_base, phys_base, virt_base);
		if(ret != 0) {
			return -2;
		}
	}
	ret = patch_task_for_pid_0(kernel_base, phys_base, virt_base);
	if(ret != 0) {
		return -3;
	}
	ret = patch_i_can_has_debugger(kernel_base, phys_base, virt_base);
	if(ret != 0) {
		return -4;
	}
	ret = patch_sandbox(kernel_base, phys_base, virt_base);
	if(ret != 0) {
		return -5;
	}
	return 0;
}

uint32_t find_rootvnode(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	uint32_t rootvnode = (uint32_t)find_sym(kernel_base, "_rootvnode", phys_base, virt_base);
	rootvnode -= (uint32_t)kernel_base;
	return rootvnode;
}