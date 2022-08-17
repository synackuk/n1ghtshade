#include <address.h>
#include <iBoot_Patcher/include/functions.h>

uint32_t* base_address;
printf_t printf;
get_env_uint_t get_env_uint;
get_env_t get_env;
uint32_t* framebuffer_address;
uint32_t* display_timings_address;
uint32_t display_width;
uint32_t display_height;
char* boot_args_ref;
fsboot_t fsboot;
jumpto_t jumpto;
uint32_t* ramdisk_address_ptr;
size_t* ramdisk_size_ptr;
void* loadaddr;

#define GET_ENV_UINT_SEARCH "bootdelay"
#define GET_ENV_SEARCH "boot-command"
#define PRINTF_SEARCH "Memory image not valid\n"
#define FRAMEBUFFER_ADDRESS_SEARCH "framebuffer"
#define DISPLAY_TIMING_SEARCH "display-timing"
#define BOOTARGS_SEARCH "rd=md0 nand-enable-reformat=1 -progress"
#define FSBOOT_SEARCH "fsboot"
#define JUMPTO_SEARCH "Memory image not valid\n"
#define LOADADDR_SEARCH "loadaddr"

int find_functions() {
	base_address = (uint32_t*)get_iboot_base_address(NULL);
	if(!base_address) {
		return -1;
	}
	printf = find_printf();
	if(!printf) {
		return -1;
	}
	printf("Found base address at %08x\n", base_address);
	printf("Found printf at %08x\n", printf);
	get_env_uint = find_get_env_uint();
	if(!get_env_uint) {
		return -1;
	}
	printf("Found get_env_uint at %08x\n", get_env_uint);

	get_env = find_get_env();
	if(!get_env) {
		return -1;
	}
	printf("Found get_env at %08x\n", get_env);
	framebuffer_address = find_framebuffer_address();
	if(!framebuffer_address) {
		return -1;
	}
	printf("Found framebuffer_address at %08x\n", framebuffer_address);
	display_timings_address = find_display_timings_address();
	if(!display_timings_address) {
		return -1;
	}
	printf("Found display_timings_address at %08x\n", display_timings_address);
	display_width = find_display_width();
	if(!display_width) {
		return -1;
	}
	printf("Found display_width %d\n", display_width);
	display_height = find_display_height();
	if(!display_height) {
		return -1;
	}
	printf("Found display_height %d\n", display_height);

	boot_args_ref = find_boot_args_ref();
	if(!boot_args_ref) {
		return -1;
	}
	printf("Found boot_args_ref at %08x\n", boot_args_ref);

	fsboot = find_fsboot();
	if(!fsboot) {
		return -1;
	}
	printf("Found fsboot at %08x\n", fsboot);

	jumpto = find_jumpto_func();
	if(!jumpto) {
		return -1;
	}
	printf("Found jumpto at %08x\n", jumpto);

	ramdisk_address_ptr = find_ramdisk_address_ptr();
	if(!ramdisk_address_ptr) {
		return -1;
	}
	printf("Found ramdisk_address_ptr at %08x\n", ramdisk_address_ptr);

	ramdisk_size_ptr = find_ramdisk_size_ptr();
	if(!ramdisk_size_ptr) {
		return -1;
	}
	printf("Found ramdisk_size_ptr at %08x\n", ramdisk_size_ptr);

	loadaddr = find_load_address();
	if(!loadaddr) {
		return -1;
	}
	printf("Found load_address at %08x\n", loadaddr);

	return 0;
}

void* quick_find(char* str, size_t len) {

	/* Find a reference to the search string */
	void* ref = memmem(base_address, IBOOT_LEN, str, len);
	if(!ref) {
		return NULL;
	}

	/* Find a reference to the search strings address */
	void* xref = memmem(base_address, IBOOT_LEN, &ref, sizeof(ref));
	if(!xref) {
		return NULL;
	}

	/* Find the ldr which loads this address as an argument */
	void* ldr = ldr_to(xref);
	if(!ldr) {
		return NULL;
	}

	/* Find the bl which calls the function using this argument */
	void* bl = bl_search_down(ldr, 0x10);
	if(!bl) {
		return NULL;
	}

	/* Get the address of the function */
	return resolve_bl32(bl);
}

void* last_xref(char* pattern, size_t len) {

	/* Find a reference to the search string */
	uintptr_t* ref = memmem(base_address, IBOOT_LEN, pattern, len);
	if(!ref) {
		return 0;
	}

	/* Find a reference to the search strings address */
	uintptr_t* curr_xref = memmem(base_address, IBOOT_LEN, &ref, sizeof(ref));
	if(!curr_xref) {
		return 0;
	}
	uintptr_t* prev_xref;

	/* Keep finding references until the last one has been found */
	while(curr_xref != 0) {
		prev_xref = curr_xref;
		curr_xref = memmem(curr_xref + 0x4, IBOOT_LEN, &ref, sizeof(ref));
	}
	return prev_xref;
}

printf_t find_printf() {
	return (printf_t)quick_find(PRINTF_SEARCH, sizeof(PRINTF_SEARCH));
}

get_env_uint_t find_get_env_uint() {
	return (get_env_uint_t)quick_find(GET_ENV_UINT_SEARCH, sizeof(GET_ENV_UINT_SEARCH));
}

get_env_t find_get_env() {
	return (get_env_t)quick_find(GET_ENV_SEARCH, sizeof(GET_ENV_SEARCH));
}


uint32_t* find_framebuffer_address() {
	
	/* The framebuffer address is stored as an environment variable */
	return (uint32_t*)get_env_uint(FRAMEBUFFER_ADDRESS_SEARCH);
}

uint32_t* find_display_timings_address() {

	/* The display_timings struct starts with a reference to the contents of the display-timing environment variable */
	char* type = get_env(DISPLAY_TIMING_SEARCH);
	if (!type) {
		return 0;
	}

	/* display_timings is always the last reference to this string */
	return last_xref(type, strlen(type) + 1);
}

uint32_t find_display_width() {
	return display_timings_address[4];
}

uint32_t find_display_height() {
	return display_timings_address[8];
}

char* find_boot_args_ref() {
	/* Find a reference to the bootargs string */
	uintptr_t* ref = memmem(base_address, IBOOT_LEN, BOOTARGS_SEARCH, sizeof(BOOTARGS_SEARCH));
	if(!ref) {
		return 0;
	}

	/* Find a reference to the bootargs string address */
	return memmem(base_address, IBOOT_LEN, &ref, sizeof(ref));
}

fsboot_t find_fsboot() {

	/* Find the reference to the "fsboot" boot command string */
	uintptr_t* xref = last_xref(FSBOOT_SEARCH, sizeof(FSBOOT_SEARCH));
	if(!xref) {
		return 0;
	}

	/* The boot_commands are stored in a struct containing the name followed by a pointer to the commands */
	return (fsboot_t)xref[1];
}

jumpto_t find_jumpto_func() {

	/* Find a reference to the search string */
	void* ref = memmem(base_address, IBOOT_LEN, JUMPTO_SEARCH, sizeof(JUMPTO_SEARCH));
	if(!ref) {
		return NULL;
	}

	/* Find a reference to the search strings address */
	void* xref = memmem(base_address, IBOOT_LEN, &ref, sizeof(uintptr_t));
	if(!xref) {
		return NULL;
	}

	/* Find the ldr which loads this address as an argument */
	void* ldr = ldr_to(xref);
	if(!ldr) {
		return NULL;
	}

	/* The go command always sets the optional boot argument to NULL  */
	void* mov_r2_0 = movs_r2_0_search_down(ldr, 0x200);
	if(!mov_r2_0) {
		return NULL;
	}
	void* jumpto_bw = bw_search_down(mov_r2_0, 0x8);
	if (jumpto_bw) {
		return resolve_bl32(jumpto_bw);
	}
	/* may also be a bl instead of a bw */
	void* jumpto_bl = bl_search_down(mov_r2_0, 0x8);
	if (jumpto_bl) {
		return resolve_bl32(jumpto_bl);
	}

	return NULL;
}

uint32_t* find_ramdisk_address_ptr() {

	/* The xref before or after the boot-args ref always references the ramdisk_address pointer (It must always equal zero hence we can verify we have the correct pointer by checking if it's zero) */
	uint32_t* rd_ptr = (uint32_t*)(boot_args_ref - 4);
	struct iboot_img iboot_in;
	iboot_in.buf = base_address;
	iboot_in.len = IBOOT_LEN;
	char* iboot_vers_str = (iboot_in.buf + IBOOT_VERS_STR_OFFSET);
	iboot_in.VERS = atoi(iboot_vers_str);

	int os_vers = get_os_version(&iboot_in);
	if(os_vers >= 9) {
		rd_ptr = (uint32_t*)(boot_args_ref + 4);
	}
	return (uint32_t*)*rd_ptr;
}

size_t* find_ramdisk_size_ptr() {
	/* The ramdisk size variable is directly after the ramdisk address variable */
	return (size_t*)&ramdisk_address_ptr[1];
}

void* find_load_address() {
	return (void*)get_env_uint(LOADADDR_SEARCH);
}