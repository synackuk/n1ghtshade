#include <libc/libc.h>

#include <xnu_patcher/include/finders.h>

#include <kernel.h>
#include <printf.h>

void* kernel_entrypoint;
void* kernel_base;
uint32_t virt_base;
uint32_t phys_base;


int initialise_kernel(boot_args_t* args) {
	/* just parses addresses from the bootargs */

	virt_base = args->virtBase;
	printf("virtBase = 0x%p\n", virt_base);
	phys_base = args->physBase;
	printf("physBase = 0x%p\n", phys_base);

	/* Since the kernel mach header is loaded 0x1000 above the phys_base, we store that address as the kernel base */
	kernel_base = (void*)(phys_base + 0x1000);

	/* We need the entrypoint so we know where to jump into when we want to execute it */
	kernel_entrypoint = find_kernel_entry(kernel_base);
	if(!kernel_entrypoint) {
		return -1;
	}
	printf("kernel_entrypoint = 0x%p\n", kernel_entrypoint);
	return 0;
}