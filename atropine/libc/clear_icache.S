.global clear_icache

.code 16

clear_icache:
	push {r4, lr}
	mov r4, #0
	mcr p15, 0, r4, c7, c5, 0
	mcr p15, 0, r4, c7, c5, 4
	dsb
	isb
	pop {r4, pc}