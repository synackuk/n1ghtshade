// Based on code by msftguy : https://github.com/msftguy/ios-jb-tools/tree/master/tools_src/asrpass
// Idea courtesy of parrotgeek

#include "asr_bypass.h"

int my_open(const char* path, int flags) {
	return open("/real_ramdisk.dmg", flags);
}

int my_ioctl(int fd, unsigned long op, void* a3) {
	switch (op) {
		case IOCTL_GET_BLOCK_SIZE:
			*(int*)a3 = RAMDISK_BLOCKSIZE;
			break;
		case IOCTL_GET_BLOCK_COUNT:
			{
				struct stat st;
				fstat(fd, &st);
				int blockCount = st.st_size / RAMDISK_BLOCKSIZE;
				*(int64_t*)a3 = blockCount;
			}
			break;
		default:
			return ioctl(fd, op, a3);
			break;
	}
	return 0;
}

__attribute__((constructor)) void  dlentry() {
	const struct mach_header* mh = NULL;
	
	for (int i = 0; i < _dyld_image_count(); ++i) {
		if (NULL != strstr(_dyld_get_image_name(i), MODULE_NAME)) {
			mh = _dyld_get_image_header(i);
			break;
		}
	}
	if (mh == NULL) {
		return;
	}
	
	
	uintptr_t* pOpenImp = get_import_ptr(mh, "_open");
	uintptr_t* pIoctlImp = get_import_ptr(mh, "_ioctl");

	*pOpenImp = (uintptr_t)my_open;
	*pIoctlImp = (uintptr_t)my_ioctl;
}