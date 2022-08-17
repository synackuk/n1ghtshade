.code 16
.global armv7_cpu_init
.global armv7_cpu_exit

armv7_cpu_init:
	push {r4, r5, lr}
	mrc p15, 0, r4, c1, c0, 0
	mov r5, #0x2
	bic r4, r4, r5 @ Enable unaligned memory access
	movw r5, #0x800
	orr r4, r4, r5 @ Turn on branch prediction
	movw r5, #0x1000
	orr r4, r4, r5 @ Turn on icache
	movw r5, #0x4
	orr r4, r4, r5 @ Turn on dcache
	mcr p15, 0, r4, c1, c0, 0
	pop {r4, r5, pc}


armv7_cpu_exit:
	push {r4, r5, lr}
	mrc p15, 0, r4, c1, c0, 0
	mov r5, #0x2
	orr r4, r4, r5 @ Disable unaligned memory access
	movw r5, #0x800
	bic r4, r4, r5 @ Turn off branch prediction
	movw r5, #0x1000
	bic r4, r4, r5 @ Turn off icache
	mov r5, #0x4
	bic r4, r4, r5 @ Turn off dcache
	mcr p15, 0, r4, c1, c0, 0
	pop {r4, r5, pc}
