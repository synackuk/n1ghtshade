#include <libc/libc.h>
#include <include/functions.h>
#include <include/finders.h>
#include <include/patchers.h>
#include <include/sbops.h>

#include <i_can_has_debugger_patch.h>

int patch_amfi_substrate(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	void* function_top = find_amfi_substrate_function_top(kernel_base, phys_base, virt_base);
	if(!function_top) {
		return -1;
	}
	/* We search down for two ittt ne. By removing these we grant every process the CS_INSTALLER and CS_GET_TASK_ALLOW entitlements */
	
	/*
	For reference:
		ITTT NE ; To remove
		LDRNE.W R0, [R10]
		ORRNE.W R0, R0, #4 ; CS_GET_TASK_ALLOW
		STRNE.W R0, [R10]
		LDRB.W  R0, [R7,#var_22]
		CMP R0, #0
		ITTT NE  ; To remove
		LDRNE.W R0, [R10]
		ORRNE.W R0, R0, #8;  CS_INSTALLER
	*/
	for(int i = 0; i < 2; i += 1) {
		uint16_t* ittt_ne = memmem(function_top, KERNEL_LEN, "\x1E\xBF", sizeof("\x1E\xBF") - 1);
		if(!ittt_ne) {
			return -1;
		}
		*ittt_ne = 0xbf00;
	}
	return 0;
}

int patch_mapforio(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	void* map_for_io_locked = find_map_for_io_locked(kernel_base, phys_base, virt_base);
	if(!map_for_io_locked) {
		return -1;
	}

	uint16_t* error_ldr = ldr_to(map_for_io_locked);
	/* For each LDR to this error code we NOP the check */
	while(error_ldr) {
		uint16_t* check = error_ldr;
		/* ldr rx, =LOCKED_ERROR */
		/* cmp ry, #0 */
		/* bne error_out */

		/* So we NOP all three instructions */
		for(int i = 0; i < 3; i += 1) {
			if(insn_is_32bit(check)) {
				check[0] = 0xbf00;
				check = &check[1];
			}
			check[0] = 0xbf00;
			check = &check[1];
		}
		error_ldr = ldr_to(map_for_io_locked);
	}
	return 0;
}

int patch_task_for_pid_0(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {

	/* First find the top of the task_for_pid function */
	char* task_for_pid = find_task_for_pid(kernel_base, phys_base, virt_base);
	if(!task_for_pid) {
		return -1;
	}
	/* The first conditional branch is verifying the process id isn't 0, and failing if it is. We must remove this check */
	uint32_t i = 0;
	while(i < 0x100) {
		uint16_t* insn = (uint16_t*)&task_for_pid[i];
		if(insn_is_beqw(insn)) {
			/* NOP out the check */
			insn[0] = 0xBF00;
			insn[1] = 0xBF00;
			return 0;
		}
		if(insn_is_beq(insn)) {
			/* NOP out the check */
			insn[0] = 0xBF00;
			return 0;
		}
		if(insn_is_bne(insn)) {
			/* Change the first byte to E0, this turns the bne into a branch, jumping over the error condition */
			insn[0] &= 0xE0FF;
			insn[0] |= 0xE000;
			return 0;
		}
		i += 2;
	}
	return -1;
}

int patch_i_can_has_debugger(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	void* i_can_has_debugger = find_i_can_has_debugger(kernel_base, phys_base, virt_base);
	if(!i_can_has_debugger) {
		return -1;
	}

	/* We just replace the function entirely with one that always returns true */
	memcpy(i_can_has_debugger, i_can_has_debugger_patch, i_can_has_debugger_patch_length);
	return 0;
}


int patch_sandbox(void* kernel_base, uint32_t phys_base, uint32_t virt_base) {
	void* sbops = find_sbops(kernel_base, phys_base, virt_base);
	if(!sbops) {
		return -1;
	}
	uint32_t version = find_version(kernel_base);
	if(!version) {
		return -1;
	}

	/* When the addresses are updated for KASLR we'll add a slide to them, by reversing this slide we can cancel it out to make a field = NULL */
	uint32_t inverse_slide = phys_base - virt_base;

	/* Obscenely long function because the mac_policy_ops change with device version... */
	if(version >= 0x00060000 && version < 0x00070000) {
		struct mac_policy_ops6* sbops_6 = sbops;

		/* If the policy isn't already zero, set our inverse slide */
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_accepted, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_deliver, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_kqfilter, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_select, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_listen, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_received, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_setsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_getsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_link, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_fsgetpath, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_mount_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_ioctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_label_associate_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_label_associate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_socket_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_mount_check_remount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_mount_check_fsctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_mount_check_mount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_access, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_chroot, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_mount_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_proc_check_get_task, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_deleteextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_exchangedata, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_exec, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_getattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_getextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_listextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_open, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_readlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_revoke, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_setattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_setextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_setflags, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_setmode, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_setowner, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_setutimes, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_truncate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_unlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_notify_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_uipc_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_vnode_check_uipc_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_proc_check_setauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_proc_check_getauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_proc_check_fork, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_6->mpo_file_check_mmap, inverse_slide);
	}
	else if(version >= 0x00070000 && version < 0x00080000) {
		struct mac_policy_ops7* sbops_7 = sbops;

		/* If the policy isn't already zero, set our inverse slide */
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_accepted, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_deliver, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_kqfilter, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_select, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_listen, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_received, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_setsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_getsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_link, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_fsgetpath, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_mount_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_ioctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_label_associate_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_label_associate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_socket_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_mount_check_remount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_mount_check_fsctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_mount_check_mount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_access, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_chroot, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_mount_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_proc_check_get_task, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_deleteextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_exchangedata, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_exec, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_getattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_getextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_listextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_open, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_readlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_revoke, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_setattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_setextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_setflags, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_setmode, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_setowner, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_setutimes, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_truncate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_unlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_notify_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_uipc_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_vnode_check_uipc_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_proc_check_setauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_proc_check_getauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_proc_check_fork, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_7->mpo_file_check_mmap, inverse_slide);
	}
	else if(version >= 0x00080000 && version < 0x00080100) {
		struct mac_policy_ops80* sbops_80 = sbops;

		/* If the policy isn't already zero, set our inverse slide */
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_accepted, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_deliver, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_kqfilter, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_select, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_listen, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_received, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_setsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_getsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_link, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_fsgetpath, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_mount_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_ioctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_label_associate_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_label_associate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_socket_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_mount_check_remount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_mount_check_fsctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_mount_check_mount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_rename, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_access, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_chroot, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_mount_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_proc_check_get_task, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_deleteextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_exchangedata, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_exec, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_getattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_getextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_listextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_open, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_readlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_revoke, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_setattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_setextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_setflags, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_setmode, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_setowner, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_setutimes, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_truncate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_unlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_notify_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_uipc_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_vnode_check_uipc_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_proc_check_setauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_proc_check_getauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_proc_check_fork, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_80->mpo_file_check_mmap, inverse_slide);
	}
	else if(version >= 0x00080100 && version < 0x00090000) {
		struct mac_policy_ops8* sbops_8 = sbops;

		/* If the policy isn't already zero, set our inverse slide */
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_accepted, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_deliver, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_kqfilter, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_select, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_listen, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_received, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_setsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_getsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_link, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_fsgetpath, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_mount_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_ioctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_label_associate_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_label_associate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_socket_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_mount_check_remount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_mount_check_fsctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_mount_check_mount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_rename, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_access, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_chroot, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_mount_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_proc_check_get_task, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_deleteextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_exchangedata, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_exec, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_getattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_getextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_listextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_open, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_readlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_revoke, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_setattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_setextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_setflags, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_setmode, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_setowner, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_setutimes, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_truncate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_unlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_notify_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_uipc_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_vnode_check_uipc_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_proc_check_setauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_proc_check_getauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_proc_check_fork, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_8->mpo_file_check_mmap, inverse_slide);
	}
	else if(version >= 0x00090000 && version < 0x00090200) {
		struct mac_policy_ops90* sbops_90 = sbops;

		/* If the policy isn't already zero, set our inverse slide */
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_accepted, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_deliver, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_kqfilter, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_select, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_listen, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_received, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_setsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_getsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_link, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_fsgetpath, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_mount_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_ioctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_label_associate_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_label_associate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_socket_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_mount_check_remount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_mount_check_fsctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_mount_check_mount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_rename, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_access, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_chroot, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_mount_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_proc_check_get_task, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_deleteextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_exchangedata, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_exec, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_getattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_getextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_listextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_open, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_readlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_revoke, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_setattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_setextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_setflags, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_setmode, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_setowner, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_setutimes, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_truncate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_unlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_notify_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_uipc_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_vnode_check_uipc_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_proc_check_setauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_proc_check_getauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_proc_check_fork, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_90->mpo_file_check_mmap, inverse_slide);
	}
	else if(version >= 0x00090200 && version < 0x000A0000) {
		struct mac_policy_ops9* sbops_9 = sbops;

		/* If the policy isn't already zero, set our inverse slide */
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_accepted, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_deliver, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_kqfilter, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_select, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_listen, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_received, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_setsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_getsockopt, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_link, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_fsgetpath, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_mount_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_ioctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_label_associate_accept, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_label_associate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_socket_check_label_update, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_mount_check_remount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_mount_check_fsctl, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_mount_check_mount, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_rename, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_access, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_chroot, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_mount_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_proc_check_get_task, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_deleteextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_exchangedata, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_exec, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_getattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_getextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_listextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_open, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_readlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_revoke, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_setattrlist, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_setextattr, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_setflags, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_setmode, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_setowner, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_setutimes, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_stat, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_truncate, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_unlink, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_notify_create, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_uipc_bind, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_vnode_check_uipc_connect, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_proc_check_setauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_proc_check_getauid, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_proc_check_fork, inverse_slide);
		WRITE_IF_NOT_ZERO(sbops_9->mpo_file_check_mmap, inverse_slide);
	}
	else {
		return -1;
	}

	return 0;
}