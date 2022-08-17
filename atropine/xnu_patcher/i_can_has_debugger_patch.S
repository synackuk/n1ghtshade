@ This patches PE_i_can_has_debugger to always return true and with debugging enabled
.code 16
_PE_i_can_has_debugger:
	push {r4, lr}
	cmp r0, #0 @ Check if an argument has been provided
	beq _return @ if not simply return
_add_debug_options:
	movw r4, #0x14e @ No idea why this value, just seems to be the one to use
	str r4, [r0] @ A call to _PE_i_can_has_debugger can choose to request the boot_args debug value, we hardcode it
_return:
	mov r0, #1 @ Force return true
	pop {r4, pc}
