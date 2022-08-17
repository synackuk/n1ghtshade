

.arm
.ltorg
.code 32

.global _start

_start:
	push	{r0-r12, lr}
	ldr r4, .Ldone
	cmp r4, #1
	beq ready
@ We fix the GOT by adding the start address to each GOT offset
fix_got:
	mov	r5, pc @ Find the load address
	sub r5, #0x18
	ldr r4, .Loff
	ldr	r0, =_GLOBAL_OFFSET_TABLE_ @ The offset to the GOT not including the length of the _start function
	add	r0, r5 @ add the load address to the GOT offset
	add	r0, #(.Lstart_func_size) @ Add the start function size
	ldr	r1, .Ledata @ len of GOT
fix_got_loop:
	sub	r1, #4 @ Go through each GOT entry until r1 = 0
	cmp r1, #0 
	ldr	r2, [r0, r1]
	add	r2, r5 @ add the load address to each entry
	add r2, r4 @ add the payload offset to each entry
	str	r2, [r0, r1]
	bne fix_got_loop

zero_bss:
	ldr	r1, .Lend
	add	r1, r0 @ The address of the end of the payload
	ldr	r2, .Ledata @ The address of the end of the data segment
	add	r0, r2
	mov	r2, #0
zero_bss_loop:
	sub	r1, #4
	cmp	r1, r0
	str	r2, [r1]
	bne	zero_bss_loop

copy_payload:
	mov r0, #1
	str r0, .Ldone
	mov r0, r5
	add r1, r5, r4 
	bl copy @ copy(load_address, load_address + offset)

clear_caches:
	mov r0, #0
	mcr	p15, 0, r0, c7, c5, 0
	dsb	sy
	isb	sy

	add r4, r5
	blx r4
	b done

ready:
	ldr	r0, [sp, #0x0]
	ldr	r1, [sp, #0x4]
	ldr	r2, [sp, #0x8]
	ldr	r3, [sp, #0xc]
	blx	main
	str r0, [sp, #0x0] @ push r0 return onto the stack to be returned

done:
	pop	{r0-r12, pc}

copy:
	mov	r2, #0x0100000
copy_loop:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	subs r2, r2, #4
	bne	copy_loop
	bx lr

.Ldone: .long 0x0
.Loff:	.long	0x10000000
.Lend:	.long	end(GOTOFF)
.Ledata:.long	edata(GOTOFF)
.Lstart_func_size:
	.size	_start, .-_start
