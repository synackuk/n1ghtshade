#include <common.h>

#define PRINTF_SEARCH "jumping into image at"
#define MALLOC_SEARCH "_malloc"
#define GET_ENV_SEARCH "display-color-space"
#define GET_ENV_UINT_SEARCH "boot-partition"
#define CMD_PTR_SEARCH "\0" OVERWRITE_COMMAND
#define JUMPTO_SEARCH "jumping into image at"
#define AES_CRYPTO_CMD_SEARCH "aes_crypto_cmd"
#define FS_MOUNT_SEARCH "fs_mount:"
#define FS_LOAD_FILE_SEARCH "Permission Denied"
#define IMAGE_LIST_SEARCH "image %p: bdev %p type %c%c%c%c offset 0x%llx"
#define VERSION_OFFSET 0x280 + 6
#define SET_ENV_UINT_SEARCH "loadaddr"
#define BOOT_ARGS_STRING_SEARCH "Reliance on this certificate"
#define KERNEL_PATCH_SEARCH "Uncompressed kernel cache at %p\n"

/* much of the code below was snarfed from iDove v3 and iBex */

static void* pattern_search(const void* addr, int len, int pattern, int mask, int step) {
	int i;
	char* caddr = (char*)addr;
	if (len <= 0)
		return NULL;
	if (step < 0) {
		len = -len;
		len &= ~-(step+1);
	} else {
		len &= ~(step-1);
	}
	for (i = 0; i != len; i += step) {
		int x = *(int*)(caddr + i);
		if ((x & mask) == pattern)
			return (void*)(caddr + i);
	}
	return NULL;
}

static void* bl_search_down(const void* start_addr, int len) {
	/* BL pattern is xx Fx xx F8+ */
	return pattern_search(start_addr, len, 0xD000F000, 0xD000F800, 2);
}

static void* blx_search_down(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0xC000F000, 0xD001F800, 2);
}

static void* bw_search_down(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x9000F000, 0xD000F800, 2);
}

static void* ldr_search_down(const void* start_addr, int len) {
	/* LDR pattern is xx xx 48 xx ( 00 00 f8 00 ) */
	return pattern_search(start_addr, len, 0x00004800, 0x0000F800, 2);
}

static void* ldr_search_up(const void* start_addr, int len) {
	/* LDR pattern is xx xx 48 xx ( 00 00 f8 00 ) */
	return pattern_search(start_addr, len, 0x00004800, 0x0000F800, -2);
}

static void* ldr32_search_up(const void* start_addr, int len) {
	/* LDR32 pattern is DF F8 xx xx */
	return pattern_search(start_addr, len, 0x0000F8DF, 0x0000FFFF, -2);
}

static void* ldr_to(const void* loc) {
	int dw, ldr_target;
	int xref_target = (int)loc;
	int i = xref_target;
	int min_addr = xref_target - 0x420;
	for(; i > min_addr; i -= 2) {
		i = (int)ldr_search_up((void*)i, i - min_addr);
		if (i == 0) {
			return NULL;
		}
		
		dw = *(int*)i;
		ldr_target = ((i + 4) & ~3) + ((dw & 0xff) << 2);
		if (ldr_target == xref_target) {
			return (void*)i;
		}
		i -= 2;
	}
	
	i = xref_target;
	min_addr = xref_target - 0x1000;
	for(; i > min_addr; i -= 4) {
		i = (int)ldr32_search_up((void*)i, i - min_addr);
		if (i == 0) {
			break;
		}
		dw = *(int*)i;
		ldr_target = ((i + 4) & ~3) + ((dw >> 16) & 0xfff);
		if (ldr_target == xref_target) {
			return (void*)i;
		}
	}
	return NULL;
}

static void* push_lr_search_up(const void* start_addr, int len) {
	/* F0 B5 <-- PUSH LR */
	/* F0 BD <-- POP PC */
	return pattern_search(start_addr, len, 0x0000B580, 0x0000FF80, -2);
}

static void* push_r4_r5_r7_lr_search_up(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x0000B5B0, 0x0000FFFF, -2);
}

static void* resolve_bl32(const void* bl) {
	int jump = 0;
	unsigned short bits = ((unsigned short *)bl)[0];
	unsigned short exts = ((unsigned short *)bl)[1];
	jump |= MASK(bits, 10, 1) << 24;
	jump |= (~(MASK(bits, 10, 1) ^ MASK(exts, 13, 1)) & 0x1) << 23;
	jump |= (~(MASK(bits, 10, 1) ^ MASK(exts, 11, 1)) & 0x1) << 22;
	jump |= MASK(bits, 0, 10) << 12;
	jump |= MASK(exts, 0, 11) << 1;
	jump |= MASK(exts, 12, 1);
	jump <<= 7;
	jump >>= 7;
	return (void*)((int)bl + 4 + jump);
}

static void* xref(char* pattern, size_t len) {
	uintptr_t* ref = memmem(base_address, 0x50000, pattern, len);
	if(!ref) {
		return 0;
	}
	uintptr_t* xref = memmem(base_address, 0x50000, &ref, sizeof(ref));
	if(!xref) {
		return 0;
	}
	return xref;
}

static void* last_xref(char* pattern, size_t len) {
	uintptr_t* ref = memmem(base_address, 0x50000, pattern, len);
	if(!ref) {
		return 0;
	}
	uintptr_t* curr_xref = memmem(base_address, 0x50000, &ref, sizeof(ref));
	if(!curr_xref) {
		return 0;
	}
	uintptr_t* prev_xref;
	while(curr_xref != 0) {
		prev_xref = curr_xref;
		curr_xref = memmem(curr_xref + 0x4, 0x50000, &ref, sizeof(ref));
	}
	return prev_xref;
}

static void* find_next_LDR_insn_with_string(char* string, size_t len) {
	uintptr_t* ref = xref(string, len);
	if(!ref) {
		return 0;
	}
	uintptr_t* ldr = ldr_to(ref);
	if(!ldr) {
		return 0;
	}
	return ldr;
}

uintptr_t* find_base_address() {
	return (uintptr_t*)(*(uint32_t*)(0x20) & ~0xFFFFF);
}

static command_descriptor* find_cmd(char* cmd, size_t len) {
	uintptr_t str_loc = (uintptr_t)memmem(base_address, 0x50000, cmd,len);
	if(!str_loc) {
		error("Failed to find string loc");
		return 0;
	}
	str_loc += 1;
	uintptr_t* addr = memmem(base_address, 0x50000, &str_loc, sizeof(uintptr_t));
	if(!addr) {
		error("Failed to find addr");
		return 0;
	}
	return (command_descriptor*)addr;
}

printf_t find_printf() {
	uintptr_t* ldr = find_next_LDR_insn_with_string(PRINTF_SEARCH, strlen(PRINTF_SEARCH));
	if(!ldr) {
		return 0;
	}
	uintptr_t* bl = bl_search_down(ldr, 8);
	if (!bl) {
		return 0;
	}
	uintptr_t* ref = resolve_bl32(bl);
	if (!ref) {
		return 0;
	}
	return (printf_t)ref;
}

malloc_t find_malloc() {
	uintptr_t* ref = xref(MALLOC_SEARCH, strlen(MALLOC_SEARCH));
	if(!ref) {
		return 0;
	}
	uintptr_t* func = push_r4_r5_r7_lr_search_up(ref, 0x200);
	if(!func) {
		return 0;
	}
	return (malloc_t)func + 1;
}

get_env_t find_get_env() {
	uintptr_t* ldr = find_next_LDR_insn_with_string(GET_ENV_SEARCH, strlen(GET_ENV_SEARCH));
	if (!ldr) {
		return 0;
	}
	uintptr_t* bl = bl_search_down(ldr, 0x10);
	if (!bl) {
		return 0;
	}
	uintptr_t* ref = resolve_bl32(bl);
	if (!ref) {
		return 0;
	}
	return (get_env_t)ref;
}

get_env_uint_t find_get_env_uint() {
	uintptr_t* ldr = find_next_LDR_insn_with_string(GET_ENV_UINT_SEARCH, strlen(GET_ENV_UINT_SEARCH));
	uintptr_t* bl = bl_search_down(ldr, 8);
	if (!bl) {
		return 0;
	}
	uintptr_t* ref = resolve_bl32(bl);
	if (!ref) {
		return 0;
	}
	return (get_env_uint_t)ref;
}

uintptr_t* find_load_address() {
	return (uintptr_t*)get_env_uint("loadaddr");	
}

uintptr_t* find_framebuffer_address() {
	return (uintptr_t*)get_env_uint("framebuffer");
}

uintptr_t* find_display_timing_address() {
	static uintptr_t* timing_address;
	if(timing_address != 0) {
		return timing_address;
	}
	char* type = get_env("display-timing");
	if (!type) {
		return 0;
	}
	timing_address = last_xref(type, strlen(type) + 1);
	if (!timing_address) {
		return 0;
	}
	return timing_address;

}
uint32_t find_display_width() {
	uintptr_t* width = (find_display_timing_address() + (sizeof(uint32_t)));
	if(!width) {
		return 0;
	}

	return *(uint32_t*)width;

}
uint32_t find_display_height() {
	uintptr_t* height = (find_display_timing_address() + (sizeof(uint32_t) * 0x2));
	if(!height) {
		return 0;
	}
	return *(uint32_t*)height;
}

uintptr_t* find_cmd_ptr() {
	uintptr_t* cmd_ptr = (uintptr_t*)find_cmd(CMD_PTR_SEARCH, sizeof(CMD_PTR_SEARCH));
	return cmd_ptr;
}

jumpto_t find_jumpto() {
	uintptr_t* go_cmd_jump = find_next_LDR_insn_with_string(JUMPTO_SEARCH, strlen(JUMPTO_SEARCH));
	if(!go_cmd_jump) {
		return 0;
	}
	uintptr_t* go_bl_1 = bl_search_down(go_cmd_jump, 8);
	if(!go_bl_1) {
		return 0;
	}
	uintptr_t* jumto_bl = bl_search_down((char *)go_bl_1 + 4, 20);
	if (!jumto_bl) {
		jumto_bl = bw_search_down((char *)go_bl_1 + 4, 24);
		if (!jumto_bl) {
			return 0;
		}
	}
	uintptr_t* jumpto_addr = resolve_bl32(jumto_bl);
	if(!go_bl_1) {
		return 0;
	}
	return (jumpto_t)jumpto_addr;
}

aes_crypto_cmd_t find_aes_crypto_cmd() {
	uintptr_t* ref = xref(AES_CRYPTO_CMD_SEARCH, strlen(AES_CRYPTO_CMD_SEARCH));
	if(!ref) {
		return 0;
	}
	uintptr_t* func = push_lr_search_up(ref, 0x200);
	if(!func) {
		return 0;
	}
	return (aes_crypto_cmd_t)func + 1;
}

fs_mount_t find_fs_mount() {
	uintptr_t* ref = xref(FS_MOUNT_SEARCH, strlen(FS_MOUNT_SEARCH));
	if(!ref) {
		return 0;
	}
	uintptr_t* func = push_lr_search_up(ref, 0x200);
	if(!func) {
		return 0;
	}
	return (fs_mount_t)func + 1;
}

fs_load_file_t find_fs_load_file() {
	uintptr_t* ref = last_xref(FS_LOAD_FILE_SEARCH, strlen(FS_LOAD_FILE_SEARCH));
	if(!ref) {
		return 0;
	}
	uintptr_t* func = push_lr_search_up(ref, 0x200);
	if(!func) {
		return 0;
	}
	return (fs_load_file_t)func + 1;
}

void* find_image_list() {
	uintptr_t* image_list_ref = xref(IMAGE_LIST_SEARCH, strlen(IMAGE_LIST_SEARCH));
	if (!image_list_ref) {
		return 0;
	}
	return (((void **)image_list_ref)[-1]);
}

int find_version() {
	return atoi((char *)base_address + VERSION_OFFSET);
}
set_env_uint_t find_set_env_uint() {
	uintptr_t* loadaddr_ptr = find_next_LDR_insn_with_string(SET_ENV_UINT_SEARCH, strlen(SET_ENV_UINT_SEARCH));
	if(!loadaddr_ptr) {
		return 0;
	}
	uintptr_t* func_ptr = bl_search_down(loadaddr_ptr, 0x20);
	if(!func_ptr) {
		return 0;
	}
	uintptr_t* func = resolve_bl32(func_ptr);
	if(!func) {
		return 0;
	}
	return (set_env_uint_t)func;
}

uintptr_t* find_boot_args_string() {
	uintptr_t* loc = memmem(base_address, 0x50000, BOOT_ARGS_STRING_SEARCH, strlen(BOOT_ARGS_STRING_SEARCH));
	if(!loc) {
		return 0;
	}
	return loc;
}

uintptr_t* find_kern_load_target() {
	uintptr_t* ptr = xref(KERNEL_PATCH_SEARCH, strlen(KERNEL_PATCH_SEARCH));
	if(!ptr) {
		return 0;
	}
	return ptr;
}