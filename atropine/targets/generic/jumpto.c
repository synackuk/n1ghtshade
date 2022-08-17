#include <jumpto.h>
#include <cpu.h>

__attribute__((noreturn)) void* jumpto(entry_t entry, void* args) {

	/* We must de-initialise the CPU first */
	armv7_cpu_exit();

	/* Then we jump into our new address, providing our boot argument and setting r1-r3 to NULL */
	entry(args, 0, 0, 0);

	/* We should never return */
	while(1);
}