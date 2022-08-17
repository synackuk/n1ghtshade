#ifndef REMOUNT_H
#define REMOUNT_H

#include <hfs_mount.h>
#include <sys/mount.h>

#define KSTRUCT_OFFSET_MOUNT_MNT_FLAG 0x3c
#define KSTRUCT_OFFSET_VNODE_V_UN 0x84

int do_remount();

#endif