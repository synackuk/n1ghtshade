@ This trampoline searches for the go command handler struct
@ It then replaces the go handler function pointer with the load address
@ This enables us to load a payload from the load address by executing the go command

.code 16

.set GO_SEARCH,				0x006f6700 @ \0go\0
.set IBEC_LOAD_ADDRESS,		0x80000000
.set JUMPTO_ADDRESS,		0xFEEDFACE @ This is specified by the patcher

	mov r6, r1
	push {r0 - r3}
	ldr r0, =GO_SEARCH
	mov r1, r6
	bl find @ find the \0go\0 string in memory
	sub r0, r0, r6
	add r5, r0, #1 @ add 1 to move the pointer to go\0
	mov r0, r6
	bl get_base_address
	add r0, r0, r5
	mov r1, r6
	bl find @ find the pointer to the name of the go command
	ldr r5, =IBEC_LOAD_ADDRESS
	str r5, [r0, #4] @ write the load address to the handler of the go command
	pop {r0 - r3}
	ldr r6, =JUMPTO_ADDRESS
	blx r6

get_base_address: @ get_base_address(iboot_base)
	push {r4, r5, lr}
	mov r4, r0
	add r4, #0x20
	ldr r5, =0xFFF00000
	ldr r0, [r4]
	and r0, r0, r5 
	pop {r4, r5, pc}



find: @(needle, haystack)
_find_loop:
	ldr	r3,	[r1]
	cmp	r3,	r0
	beq	_find_ret
	add	r1,	#1
	b _find_loop

_find_ret:
	mov	r0,	r1
	bx lr

