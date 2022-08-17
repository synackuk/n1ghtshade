#include <libc/libc.h>
#include <include/functions.h>
#include <include/finders.h>

void* find_amfi_substrate_function_top(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	/* First we find the start of the AMFI kext, this saves us from searching the whole kernel with find_literal_ref which is catastrophically slow */
	void* amfi_kext = find_kext_start(kernel_base, phys_base, virt_base, "AppleMobileFileIntegrity");

	/* Next we find a string referenced from the function we wish to patch */
	uint32_t amfi_str = (uint32_t)memmem(amfi_kext, KERNEL_LEN, "AMFI: hook..execve() killing pid %u: %s\n", sizeof("AMFI: hook..execve() killing pid %u: %s\n"));
	if(!amfi_str) {
		return NULL;
	}
	amfi_str -= phys_base;

	/* Then we find a reference to that String */
	void* amfi_ref = find_literal_ref((void*)phys_base, KERNEL_LEN, (uint16_t*)amfi_kext, amfi_str);
	if(!amfi_ref) {
		return NULL;
	}

	/* We search up for push {lr} as that gets us the function top */

	return push_lr_search_up(amfi_ref, 0x5000);
}

void* find_map_for_io_locked(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	/* The _mapForIO function is part of the LightweightVolumeManager kext, because it's generally towards the end of the kernel, we find it first to speed thigns up */
	void* lwvm_kext = find_kext_start(kernel_base, phys_base, virt_base, "LightweightVolumeManager");
	if(!lwvm_kext) {
		return NULL;
	}
	/* First we find the string "_mapForIO" */
	uint32_t mapforio_str = (uint32_t)memmem(lwvm_kext, KERNEL_LEN, "_mapForIO", sizeof("_mapForIO"));
	if(!mapforio_str) {
		return NULL;
	}
	mapforio_str -= phys_base;

	/* The reference to this string is in the _mapForIO function */
	void* mapforio_ref = find_literal_ref((void*)phys_base, KERNEL_LEN, (uint16_t*)lwvm_kext, mapforio_str);
	if(!mapforio_ref) {
		return NULL;
	}
	/* The part we're trying to patch is the LOCKED error, which is caused when we try to write to the root file system, so we search for the error code */
	uint32_t error_code = 0xE00002C4;
	return memmem(mapforio_ref, KERNEL_LEN, &error_code, sizeof(uint32_t));

}

void* find_task_for_pid(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	/* task_for_pid is a mach trap, stored in the mach trap table. */

	uint32_t task_for_pid_addr = 0;

	uint32_t version = find_version(kernel_base);
	if(!version) {
		return NULL;
	}

	/* pre-iOS7 uses an older style mach_trap struct */
	if(version < 0x00070000) {
		mach_trap_old_t* mach_trap_0_ptr = find_old_mach_trap_table(kernel_base, phys_base, virt_base);
		if(!mach_trap_0_ptr) {
			return NULL;
		}
		/* task_for_pid is the 45th mach trap */
		task_for_pid_addr = mach_trap_0_ptr[45].handler;

	}
	else {
		mach_trap_t* mach_trap_0_ptr = find_mach_trap_table(kernel_base, phys_base, virt_base);
		if(!mach_trap_0_ptr) {
			return NULL;
		}
		/* task_for_pid is the 45th mach trap */
		task_for_pid_addr = mach_trap_0_ptr[45].handler;
	}

	/* Unset the last bit (thumb) */
	task_for_pid_addr &= ~1;
	return (void*)VIRT_TO_PHYS(task_for_pid_addr);
}

void* find_i_can_has_debugger(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	return find_sym(kernel_base, "_PE_i_can_has_debugger", phys_base, virt_base);
}

void* find_sbops(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	
	/* The seatbelt sandbox policy struct and string are part of the Seatbelt sandbox policy kext, because it's generally towards the end of the kernel, we find it first to speed thigns up */
	void* sandbox_kext = find_kext_start(kernel_base, phys_base, virt_base, "Seatbelt sandbox policy");
	if(!sandbox_kext) {
		return NULL;
	}

	void* seatbelt_sandbox_str_addr = memmem(sandbox_kext, KERNEL_LEN, "Seatbelt sandbox policy", sizeof("Seatbelt sandbox policy"));
	
	if(!seatbelt_sandbox_str_addr) {
		return NULL;
	}

	char* seatbelt_sandbox_str_xref = memmem(sandbox_kext, KERNEL_LEN, &seatbelt_sandbox_str_addr, sizeof(uint32_t));
	if(!seatbelt_sandbox_str_xref) {
		return NULL;
	}

	uint32_t val = 1;
	
	uint32_t* sbops_address_precurser = memmem((void*)seatbelt_sandbox_str_xref, 0x10, &val, sizeof(uint32_t));

	if(!sbops_address_precurser) {
		return NULL;
	}
	/* sbops address is the address after the 1 */

	return (void*)sbops_address_precurser[1];
}

void* find_kernel_entry(char* kernel_base) {
	struct arm_thread_state* thread = (void*)((uint32_t)find_load_command((void*)kernel_base, LC_UNIXTHREAD) + sizeof(struct thread_command));
	return (void*)thread->pc;
}

mach_trap_old_t* find_old_mach_trap_table(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {

	/* This table is in the const section of the data segment, so we find that first */
	struct segment_command* data_segment = find_segment(kernel_base, SEGMENT_DATA);
	if(!data_segment) {
		return NULL;
	}

	struct section* const_section = find_section(data_segment, SECTION_CONST);

	if(!const_section) {
		return NULL;
	}
	mach_trap_old_t* mach_trap_0_ptr = NULL;

	char* const_section_addr = (char*)VIRT_TO_PHYS(const_section->addr);

	/* The first ten mach traps in the table are identical, so this is how we find it */
	for(uint32_t i = 0; i < const_section->size; i += 4) {
		mach_trap_old_t* potential = (void*)&const_section_addr[i];

		/* Verify that it's one of the mach traps we're looking for (takes no arguments and the handler is an address) */
		if(potential->num_args != 0 || (potential->handler & virt_base) != virt_base) {
			continue;
		}
		int found = 1;
		/* Check the next 9 mach traps are identical */
		for(int j = 1; j < 10; j += 1) {
			if(memcmp(potential, &potential[j], sizeof(mach_trap_old_t)) != 0) {
				found = 0;
				break;
			}
		}
		if(found) {
			mach_trap_0_ptr = potential;
			break;
		}
	}
	return mach_trap_0_ptr;
}

mach_trap_t* find_mach_trap_table(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {

	/* This table is in the const section of the data segment, so we find that first */
	struct segment_command* data_segment = find_segment(kernel_base, SEGMENT_DATA);
	if(!data_segment) {
		return NULL;
	}

	struct section* const_section = find_section(data_segment, SECTION_CONST);

	if(!const_section) {
		return NULL;
	}
	mach_trap_t* mach_trap_0_ptr = NULL;

	char* const_section_addr = (char*)VIRT_TO_PHYS(const_section->addr);

	/* The first ten mach traps in the table are identical, so this is how we find it */
	for(uint32_t i = 0; i < const_section->size; i += 4) {
		mach_trap_t* potential = (void*)&const_section_addr[i];

		/* Verify that it's one of the mach traps we're looking for (takes no arguments and the handler is an address) */
		if(potential->num_args != 0 || (potential->handler & virt_base) != virt_base || potential->num_u32 != 0) {
			continue;
		}
		int found = 1;
		/* Check the next 9 mach traps are identical */
		for(int j = 1; j < 10; j += 1) {
			if(memcmp(potential, &potential[j], sizeof(mach_trap_t)) != 0) {
				found = 0;
				break;
			}
		}
		if(found) {
			mach_trap_0_ptr = potential;
			break;
		}
	}
	return mach_trap_0_ptr;
}