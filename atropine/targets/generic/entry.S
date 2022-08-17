.arm
.ltorg
.code 32

.global _start

_start:
	push {r0}
fix_got:
	mov	r5, pc @ Find the load address
	sub r5, #0xC
	ldr	r0, =_GLOBAL_OFFSET_TABLE_ @ The offset to the GOT not including the length of the _start function
	add	r0, r5 @ add the load address to the GOT offset
	add	r0, #(.Lstart_func_size) @ Add the start function size
	ldr	r1, .Ledata @ len of GOT
fix_got_loop:
	sub	r1, #4 @ Go through each GOT entry until r1 = 0
	cmp r1, #0 
	ldr	r2, [r0, r1]
	add	r2, r5 @ add the load address to each entry
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

clear_caches:
	mov r0, #0
	mcr	p15, 0, r0, c7, c5, 0
	dsb	sy
	isb	sy

ready:
	pop {r0}
	blx	main

.Lend:	.long	end(GOTOFF)
.Ledata:.long	edata(GOTOFF)
.Lstart_func_size:
	.size	_start, .-_start
