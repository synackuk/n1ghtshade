
@ Based on code from greenpois0n cyanide : https://github.com/Chronic-Dev/cyanide/blob/master/entry.S

#include <target.h>

.arm
.ltorg
.code 32
.global _start
_start:
	b reset

.pool
.set MAGIC_OFFSET, 0x200
.set PWND_MAGIC, 0x646E7770
.code 32
reset:
	push	{r0-r12, lr}
	ldr r1, =#LOADER_MAGIC
	ldr r7, =#MAGIC_OFFSET
	ldr	r5, [r7]
	cmp	r5, r1
	bne continue @ If the iboot magic has been modified we can call the main function
	mov	r5, pc @ Otherwise find the load address
	lsr	r5, #24
	lsl r5, #24
	ldr r6, =LOADADDRESS_OFFSET
	add r1, r5, r6 @ And add a specified offset so that the payload survives other files being uploaded to the buffer
	mov r0, r5
	b relocate @ relocate(loadaddr, loadaddr + LOADADDRESS_OFFSET)

continue:
	ldr	r0, [sp, #0x0]
	ldr	r1, [sp, #0x4]
	ldr	r2, [sp, #0x8]
	ldr	r3, [sp, #0xc]
	bl	main
	str r0, [sp, #0x0] @ push r0 return onto the stack to be returned

done:
	pop	{r0-r12, pc}

relocate:
	mov	r4, r1
	bl	copy @ Copy the whole payload from the load address to the load address + the offset
	ldr r1, =PWND_MAGIC
	str r1, [r7] @ Replace the iBoot Magic with our pwnd magic.
	bl	clear_icache
	bl flush_dcache
	blx	r4
	b done

.code 32
copy:
	mov	r2, #0x0100000
copy_loop:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	subs	r2, r2, #4
	bne	copy_loop
	bx lr

.code 32
flush_dcache:
	mrc	p15, 0, r0, c1, c0, 1
	bx lr

.code 32
clear_icache:
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5
	mcr	p15, 0, r0, c7, c5, 4  @ Flush Prefetch Buffer
	nop
	nop
	nop
	nop
	bx lr

