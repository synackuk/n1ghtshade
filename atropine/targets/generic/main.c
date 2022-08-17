#include <libc/libc.h>
#include <display/display.h>
#include <xnu_patcher/include/xnu32patcher.h>

#include <printf.h>
#include <cpu.h>
#include <jumpto.h>
#include <kernel.h>
#include <boot_args.h>
#include <panic.h>

int main(boot_args_t* args) {
	int ret = 0;

	/* First initialise the CPU (enable caches etc) */
	armv7_cpu_init();

	/* Next initialise display output */
	display_init((uint32_t*)args->Video.v_baseAddr, args->Video.v_width, args->Video.v_height);

	/* Initialise the kernel */

	display_progress_bar(25);
	display_progress_print("Initialising xnu");
	ret = initialise_kernel(args);
	if(ret != 0) {
		printf("initialise_kernel failed with error %d\n", ret);
		panic();
	}


	/* Patch the kernel */

	display_progress_bar(50);
	display_progress_print("Patching xnu");
	ret = patch_xnu(kernel_base, phys_base, virt_base);
	if(ret != 0) {
		printf("patch_xnu failed with error %d\n", ret);
		panic();
	}

	/* Update the boot-args */

	display_progress_bar(75);
	display_progress_print("Updating boot-args");
	ret = update_boot_args(args);
	if(ret != 0) {
		printf("update_boot_args failed with error %d\n", ret);
		panic();
	}
	
	display_progress_bar(100);
	display_progress_print("Booting...");

	/* Leave just our logo on the framebuffer for bootup */

	display_clear();
	display_logo();

	/* Boot the kernel */

	jumpto(kernel_entrypoint, args);

	/* Should never get here */
	panic();
	return 0;
}
