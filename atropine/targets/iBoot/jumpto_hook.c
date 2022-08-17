#include <libc/libc.h>
#include <address.h>

#include <build/generic_payload.h>

static uint64_t original_instructions = 0;

__attribute__((noreturn)) static void hooker(int type, void* jump, void* arg) {
	/* jumpto is a thumb instruction meaning the last bit is set */
	uintptr_t jumpto_addr = (uintptr_t)jumpto;
	jumpto_addr &= ~1;

	/* Restore the original jumpto instruction */
	memcpy((void*)jumpto_addr, &original_instructions, sizeof(uint64_t));

	/* Clear the instruction cache to make sure we're up to date when we call jumpto */
	clear_icache();

	/* Call jumpto to our payload instead of the actual target */
	jumpto(type, (void*)generic_payload, arg);
	while(1);
}

int hook_jumpto() {
	void* hook = &hooker;

	/* jumpto is a thumb instruction meaning the last bit is set */
	uintptr_t jumpto_addr = (uintptr_t)jumpto;
	jumpto_addr &= ~1;

	/* Save the original first instruction so that we can fixup jumpto after we've ran our hook */
	memcpy(&original_instructions, (void*)jumpto_addr, sizeof(uint64_t));

	/* Replace the instruction with a branch to our hooker function */
	memcpy((void*)jumpto_addr, "\x00\x4b\x98\x47", 4);
	jumpto_addr += 4;
	memcpy((void*)jumpto_addr, &hook, 4);
	return 0;
}