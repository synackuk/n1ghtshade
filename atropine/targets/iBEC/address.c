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
jumpto_t jumpto;

#define GET_ENV_UINT_SEARCH "bootdelay"
#define GET_ENV_SEARCH "boot-command"
#define PRINTF_SEARCH "Memory image not valid\n"
#define FRAMEBUFFER_ADDRESS_SEARCH "framebuffer"
#define DISPLAY_TIMING_SEARCH "display-timing"
#define JUMPTO_SEARCH "Memory image not valid\n"

int find_functions() {
	base_address = (uint32_t*)get_iboot_base_address(NULL);
	if(!base_address) {
		return -1;
	}
	printf = find_printf();
	if(!printf) {
		return -1;
	}
	printf("Found base address at %p\n", base_address);
	printf("Found printf at %p\n", printf);
	get_env_uint = find_get_env_uint();
	if(!get_env_uint) {
		return -1;
	}
	printf("Found get_env_uint at %p\n", get_env_uint);

	get_env = find_get_env();
	if(!get_env) {
		return -1;
	}
	printf("Found get_env at %p\n", get_env);
	framebuffer_address = find_framebuffer_address();
	if(!framebuffer_address) {
		return -1;
	}
	printf("Found framebuffer_address at %p\n", framebuffer_address);
	display_timings_address = find_display_timings_address();
	if(!display_timings_address) {
		return -1;
	}
	printf("Found display_timings_address at %p\n", display_timings_address);
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

	jumpto = find_jumpto_func();
	if(!jumpto) {
		return -1;
	}
	printf("Found jumpto at %p\n", jumpto);

	return 0;
}

void* quick_find(char* str, size_t len) {

	/* Find a reference to the search string */
	void* ref = memmem(base_address, IBOOT_LEN, str, len);
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
	return (uint32_t*)get_env_uint(FRAMEBUFFER_ADDRESS_SEARCH, 0);
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