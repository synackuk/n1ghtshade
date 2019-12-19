#include <common.h>

static void* image_find_tag(void* image, unsigned int tag, unsigned int size) {
	unsigned int i = 0;
	unsigned int* current = image;
	current = image;
	for (i = 0; i < size; i++) {
		if (*current == tag) {
			return current;
		}
		current++;
	}
	return 0;
}

int decrypt_image(void* image) {
	int ret;
	img3_root* header = (img3_root*) image;
	size_t len = header->tag.size;
	image_kbag* kbag = (image_kbag*)image_find_tag(image, IMAGE_KBAG, len);
	if(!kbag) {
		error("Failed to find kbag");
		return -1;
	}
	ret = aes_crypto_cmd(AES_DECRYPT, kbag->iv, kbag->iv, AES_KBAG_SIZE, AES_GID, 0, 0);
	if(ret != 0) {
		error("Failed to decrypt kbag");
		return -1;
	}
	img3_tag* data_head = (img3_tag*)image_find_tag(image, IMAGE_DATA, len);
	if(!data_head) {
		error("Failed to find data tag");
		return -1;
	}
	void* data = (void*)data_head + sizeof(img3_tag);
	size_t data_len = (data_head->dataSize - (data_head->dataSize % 16));
	ret = aes_crypto_cmd(AES_DECRYPT, data, data, data_len, AES_256, kbag->key, kbag->iv);
	if(ret != 0) {
		error("Failed to decrypt image data");
		return -1;
	}
	memcpy(image, data, (data_head->dataSize - (data_head->dataSize % 16)));
	return 0;
}

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