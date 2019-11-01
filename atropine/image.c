#include <common.h>

int bdev_read(void *bdev, void *buf, long long offset, long long size)
{
	if (version < 1940) {
		return ((struct bdev6_t *)bdev)->bdev_read(bdev, buf, offset, size);
	}
	return ((struct bdev_t *)bdev)->bdev_read(bdev, buf, offset, size);
}

int load_image_from_bdev(char* address, uint32_t tag, size_t* len) {
	int ret;
	struct firmware_image* image = ((firmware_image*)image_list)->next;
	while(image != image_list) {
		if(image->info.type == tag) {
			break;
		}
		image = image->next;
	}

	if(image == image_list) {
		return -1;
	}

	ret = bdev_read(image->bdev, address, image->offset_lo, image->info.size);
	if(ret != image->info.size) {
		return -1;
	}
	*len = image->info.size;
	return 0;
}