#include <stddef.h>
#include <iBoot_Patcher/include/iBoot32Patcher.h>

int __attribute__((__section__(".text.startup"))) _start(void* buf, size_t len) {
	return patch_iboot(buf, len);
}