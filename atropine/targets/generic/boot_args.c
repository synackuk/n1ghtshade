#include <boot_args.h>
#include <kernel.h>
#include <printf.h>
#include <libc/libc.h>
#include <xnu_patcher/include/xnu32patcher.h>

int update_boot_args(boot_args_t* args) {
	/* First we must find the length of the boot-args to start with to ensure we don't overflow the buffer */
	size_t args_len = strlen(args->CommandLine);
	printf("Old boot-args = \"%s\"\n", args->CommandLine);
#ifdef MODE_RELEASE
	/* This is enough to wipe out amfi, along with our PE_i_can_has_debugger patch */
	strncat(args->CommandLine, " amfi=0xff cs_enforcement_disable=1", 255 - args_len);
#else
	/* We add debugging boot-args if we're a debug build */
	strncat(args->CommandLine, " amfi=0xff cs_enforcement_disable=1 -v serial=3 keepsyms=1", 255 - args_len);
#endif
	/* We must also append the rootvnode_addr and pmap_addr */
	uint32_t rootvnode_offset = find_rootvnode(kernel_base, phys_base, virt_base);
	if(!rootvnode_offset) {
		printf("Failed to find _rootvnode\n");
		return -1;
	}
	char rootvnode_arg[64] = {0};
	snprintf(rootvnode_arg, 63, " rootvnode_addr=0x%p", rootvnode_offset);
	strncat(args->CommandLine, rootvnode_arg, 255 - args_len);


	printf("New boot-args = \"%s\"\n", args->CommandLine);
	return 0;
}