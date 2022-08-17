 #include <remount.h>
#include <kernel.h>


static int hfs_mount(char* device, const char* mountdir, int options) {
	struct hfs_mount_args args;
	args.fspec = device;
	return mount("hfs", mountdir, options, &args);
}

static uint32_t find_v_mount() {
	static uint32_t vnode_mount = 0;
	if(vnode_mount) {
		return vnode_mount;
	}

	/* We store the rootvnode symbol offset in the boot-args */
	char* boot_args = (char*)kern_bootargs();
	uint32_t rootvnode_offset;
	char* rootvnode_boot_arg = strstr(boot_args, "rootvnode_addr=");
	if(!rootvnode_boot_arg) {
		return 0;
	}
	sscanf(rootvnode_boot_arg, "rootvnode_addr=%x", &rootvnode_offset);
	if(!rootvnode_offset) {
		return 0;
	}
	/* Get the address of the rootfs vnode */
	uint32_t rootfs_vnode = *(uint32_t*)&kernel_dump[rootvnode_offset];

	/* Read the vnode_mount struct for rootfs */
	vnode_mount = rk32(rootfs_vnode + KSTRUCT_OFFSET_VNODE_V_UN);
	if(!vnode_mount) {
		return 0;
	}
	return vnode_mount;
}

static uint32_t get_vfs_flags() {
	uint32_t vnode_mount = find_v_mount();
	if(!vnode_mount) {
		return 0;
	}
	/* Read the vnode_mount vfs_flags field */
	uint32_t vfs_flags = rk32(vnode_mount + KSTRUCT_OFFSET_MOUNT_MNT_FLAG + 1);
	return vfs_flags;
}

static int set_vfs_flags(uint32_t vfs_flags) {
	uint32_t vnode_mount = find_v_mount();
	if(!vnode_mount) {
		return -1;
	}
	/* Write to the vnode_mount vfs_flags field */
	return wk32(vnode_mount + KSTRUCT_OFFSET_MOUNT_MNT_FLAG + 1, vfs_flags);
}

int do_remount() {
	int ret = 0;
	char* boot_args = (char*)kern_bootargs();
	/* First get the current vnode flags */
	uint32_t vfs_flags = get_vfs_flags();
	if(!vfs_flags) {
		printf("Failed to get vfs_flags\n");
		return -1;
	}
	/* Then, modify the flags by unsetting the MNT_ROOTFS flag */
	ret = set_vfs_flags(vfs_flags & ~(MNT_ROOTFS >> 8));
	if(ret != 0) {
		printf("Failed to set vfs_flags\n");
		return -1;
	}

	/* If we're being executed on a ramdisk we're done here */
	if(strstr(boot_args, "rd=md0")) {

		/* If we haven't modified the flags (this means MNT_ROOTFS was already unset) and we're being executed on a ramdisk then we need to re-set MNT_ROOTFS */
		if(vfs_flags == get_vfs_flags()) {
			ret = set_vfs_flags(vfs_flags | (MNT_ROOTFS >> 8));
			if(ret != 0) {
				printf("Failed to set vfs_flags\n");
				return -1;
			}
		}
		return 0;
	}

	/* Now the MNT_ROOTFS flag is unset we can mount the root file system as r/w */
	ret = hfs_mount(strdup("/dev/disk0s1s1"), "/", MNT_UPDATE);
	if(ret != 0) {
		printf("Failed rootfs remount\n");
		return -1;
	}

	/* Finally we re-set the MNT_ROOTFS flag */
	ret = set_vfs_flags(vfs_flags);
	if(ret != 0) {
		printf("Failed to reset vfs_flags\n");
		return -1;
	}

	ret = hfs_mount(strdup("/dev/disk0s1s2"), "/private/var", MNT_UPDATE | MNT_CPROTECT);
	if(ret != 0) {
		printf("Failed userfs remount\n");
		return -1;
	}

	return 0;
}