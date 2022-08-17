#include <kernel.h>

uint8_t* kernel_dump;

mach_port_t get_kernel_task() {
	static mach_port_t kernel_task = 0;
	if(!kernel_task) {
		if(task_for_pid(mach_task_self(), 0, &kernel_task) != KERN_SUCCESS) {
			return 0;
		}
	}
	return kernel_task;
}

vm_address_t get_kernel_region() {
	static vm_address_t region = 0;
	if(region) {
		return region;
	}
	task_t kernel_task = get_kernel_task();
	vm_region_submap_info_data_64_t info;
	vm_size_t size;
	mach_msg_type_number_t info_count = VM_REGION_SUBMAP_INFO_COUNT_64;
	unsigned int depth = 0;
	uintptr_t addr = KERNEL_SEARCH_ADDRESS;

	while (1) {
		if (KERN_SUCCESS != vm_region_recurse_64(kernel_task, (vm_address_t *)&addr, &size, &depth, (vm_region_info_t) &info, &info_count)){
		  break;
		}
		if (size > 1024 * 1024 * 1024) {
			/*
			 * https://code.google.com/p/iphone-dataprotection/
			 * hax, sometimes on iOS7 kernel starts at +0x200000 in the 1Gb region
			 */
			pointer_t buf;
			mach_msg_type_number_t sz = 0;
			addr += 0x200000;
			vm_read(kernel_task, addr + IMAGE_OFFSET, 512, &buf, &sz);
			if (*((uint32_t *)buf) != MACHO_HEADER_MAGIC) {
				addr -= 0x200000;
				vm_read(kernel_task, addr + IMAGE_OFFSET, 512, &buf, &sz);
				if (*((uint32_t*)buf) != MACHO_HEADER_MAGIC) {
					break;
				}
			}
			region = addr;
			return region;
		}
		addr += size;
	}
	return 0;
}

uint32_t rk32(uint32_t kaddr) {
	kern_return_t err;
	uint32_t val = 0;
	vm_size_t outsize = 0;

	err = vm_read_overwrite(get_kernel_task(), (mach_vm_address_t)kaddr, (mach_vm_size_t)sizeof(uint32_t), (mach_vm_address_t)&val, &outsize);
	
	if (err != KERN_SUCCESS) {
		return 0;
	}
	
	if (outsize != sizeof(uint32_t)) {
		return 0;
	}
	
	return val;
}

int wk32(uint32_t addr, uint32_t val) {
	kern_return_t err;
	mach_port_t kernel_task = get_kernel_task();

	err = vm_write(kernel_task, addr, (vm_address_t) &val, sizeof(uint32_t));

	return err;
}



const char* kern_bootargs() {
	static char buffer[256];
	size_t buffer_size = sizeof(buffer);
	sysctlbyname("kern.bootargs", buffer, &buffer_size, NULL, 0);
	return buffer;
}

void dump_kernel(task_t kernel_task, vm_address_t kernel_base, uint8_t *dest) {
	for (vm_address_t addr = kernel_base + IMAGE_OFFSET, e = 0; addr < kernel_base + KERNEL_DUMP_SIZE; addr += CHUNK_SIZE, e += CHUNK_SIZE) {
		pointer_t buf = 0;
		mach_msg_type_number_t sz = 0;
		vm_read(kernel_task, addr, CHUNK_SIZE, &buf, &sz);
		if (buf == 0 || sz == 0){
			continue;
		}
		bcopy((uint8_t *)buf, dest + e, CHUNK_SIZE);
	}
}

int init_kernel() {

	/* First get tfp0 */
	task_t tfp0 = get_kernel_task();
	if(!tfp0) {
		printf("Failed to get tfp0\n");
		return -1;
	}
	printf("tfp0: 0x%08x\n", tfp0);

	/* Next get the kernel base address (will be different everytime due to ASLR) */
	uint32_t base_address = get_kernel_region();
	if(!base_address) {
		printf("Failed to get base_address\n");
		return -1;
	}
	printf("base_address: 0x%08x\n", base_address);

	/* Finally dump the kernel so that we can use it to find addresses we need to patch */
	kernel_dump = malloc(KERNEL_SIZE);
	if(!kernel_dump) {
		printf("Failed to allocate kernel_dump\n");
		return -1;
	}

	dump_kernel(tfp0, base_address, kernel_dump);

	printf("Successfully dumped kernel\n");

	return 0;
}
