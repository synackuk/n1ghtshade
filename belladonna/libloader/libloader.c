#include <libloader.h>
#include <device_identifiers.h>
#include <crc32_table.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 
#include <sys/time.h> 
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#define crc32_step(a,b) \
	a = (crc32_lookup_t1[(a & 0xFF) ^ ((unsigned char)b)] ^ (a >> 8))

static libusb_context* libloader_context;

void libloader_init() {
	libusb_init(&libloader_context);
}

void libloader_exit() {
	if(!libloader_context) {
		return;
	}
	libusb_exit(libloader_context);
	libloader_context = NULL;
}

int libloader_is_checkm8_dfu(libloader_device_t dev) {
	char* pwnd_ptr = strstr((char*)dev->serial, "PWND:[checkm8]");
	if(!pwnd_ptr) {
		return 0;
	}
	return 1;
}

int libloader_is_pwned_dfu(libloader_device_t dev) {
	char* pwnd_ptr = strstr((char*)dev->serial, "PWND");
	if(!pwnd_ptr) {
		return 0;
	}
	return 1;
}

uint16_t libloader_get_cpid(libloader_device_t dev) {
	char* cpid_ptr = strstr((char*)dev->serial, "CPID:");
	if(!cpid_ptr) {
		return 0;
	}
	uint16_t cpid;
	sscanf(cpid_ptr, "CPID:%hx", &cpid);
	return cpid;
}

int libloader_get_bdid(libloader_device_t dev) {
	char* bdid_ptr = strstr((char*)dev->serial, "BDID:");
	if(!bdid_ptr) {
		return -1;
	}
	uint32_t bdid;
	sscanf(bdid_ptr, "BDID:%x", &bdid);
	return (uint8_t)bdid;
}

char* libloader_get_identifier(libloader_device_t dev) {
	int i = 0;
	uint16_t cpid = libloader_get_cpid(dev);
	if(!cpid) {
		return NULL;
	}
	int bdid = libloader_get_bdid(dev);
	if(bdid == -1) {
		return NULL;
	}
	while(device_identifiers[i].product_identifier != NULL) {
		if(device_identifiers[i].cpid == cpid && device_identifiers[i].bdid == bdid) {
			return device_identifiers[i].product_identifier;
		}
		i += 1;
	}
	return NULL;
}

int libloader_is_dfu(libloader_device_t dev) {
	return dev->mode == DFU_MODE;
}

libloader_device_t libloader_get_device_handle() {
	int ret;
	libloader_device_t dev = malloc(sizeof(struct libloader_device_t));
	if(!dev) {
		return NULL;
	}
	memset(dev, '\x00', sizeof(struct libloader_device_t));
	libusb_device **list = NULL;
	struct libusb_device_descriptor desc;
	int count = libusb_get_device_list(libloader_context, &list);
	libusb_device* usb_dev = NULL;
	for(int i = 0; i < count; i++) {
		usb_dev = list[i];
		ret = libusb_get_device_descriptor(usb_dev, &desc);
		if(ret != 0) {
			return NULL;
		}
		if(desc.idVendor == APPLE_VID && (desc.idProduct == DFU_MODE || desc.idProduct == RECOVERY_MODE_1 || desc.idProduct == RECOVERY_MODE_2 || desc.idProduct == RECOVERY_MODE_3 || desc.idProduct == RECOVERY_MODE_4 || desc.idProduct == NORMAL_MODE)) {
			break;
		}
		libusb_unref_device(usb_dev);
		usb_dev = NULL;
	}
	if(usb_dev == NULL) {
		return NULL;
	}
	dev->mode = desc.idProduct;
	ret = libusb_open(usb_dev, &dev->dev);
	if(ret != 0){
		return NULL;
	}
	libusb_get_string_descriptor_ascii(dev->dev, desc.iSerialNumber, dev->serial, 255);
	libusb_free_device_list(list, 0);
	if(ret != 0){
		libusb_close(dev->dev);
		free(dev);
		return NULL;
	}
	ret = libusb_set_configuration(dev->dev, 1);
	if(ret != 0){
		libloader_close(dev);
		return NULL;
	}
	if(dev->mode > RECOVERY_MODE_2) {
		ret = libusb_claim_interface(dev->dev, 1);
		if(ret != 0){
			libloader_close(dev);
			return NULL;
		}
		ret = libusb_set_interface_alt_setting(dev->dev, 1, 1);
		if (ret != 0) {
			libloader_close(dev);
			return NULL;
		}
	}
	else {
		ret = libusb_claim_interface(dev->dev, 0);
		if(ret != 0){
			libloader_close(dev);
			return NULL;
		}
	}
	return dev;
}

void libloader_close(libloader_device_t dev) {
	if(!dev) {
		return;
	}
	if(dev->mode > RECOVERY_MODE_2) {
		libusb_release_interface(dev->dev, 1);
	}
	else {
		libusb_release_interface(dev->dev, 0);
	}
	libusb_close(dev->dev);
	free(dev);
}

int libloader_reset(libloader_device_t dev) {
	return libusb_reset_device(dev->dev);
}

int libloader_get_status(libloader_device_t dev) {
	unsigned char buf[6];
	int ret = libloader_ctrl_transfer(dev, 0xA1, 3, 0, 0, buf, 6, USB_TIMEOUT);
	if(ret < 0){
		return -1;
	}
	return 0;
}

int libloader_request_image_validation(libloader_device_t dev) {
	int ret;
	libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, 0, 0, USB_TIMEOUT);
	for(int i = 0; i < 3; i++) {
		ret = libloader_get_status(dev);
		if(ret != 0) {
			return -1;
		}
	}
	libloader_reset(dev);
	return 0;
}

int libloader_send_buffer(libloader_device_t dev, unsigned char* buf, size_t len) {
	int packet_size = libloader_is_dfu(dev) ? DFU_MAX_PACKET_SIZE : RECVOVERY_MAX_PACKET_SIZE;
	int last = len % packet_size;
	int packets = len / packet_size;
	int ret;
	if (last != 0) {
		packets++;
	} else {
		last = packet_size;
	}
	unsigned int h1 = 0xFFFFFFFF;
	unsigned char dfu_xbuf[12] = {0xff, 0xff, 0xff, 0xff, 0xac, 0x05, 0x00, 0x01, 0x55, 0x46, 0x44, 0x10};
	if(libloader_is_dfu(dev)){
		for (int i = 0; i < packets; i++) {
			int size = (i + 1) < packets ? packet_size : last;
			int j;
			for (int j = 0; j < size; j++) {
				crc32_step(h1, buf[i * packet_size + j]);
			}
			if (i+1 == packets) {
				if (size+16 > packet_size) {
					ret = libloader_ctrl_transfer(dev, 0x21, 1, i, 0, &buf[i * packet_size], size, USB_TIMEOUT);
					if (ret != size) {
						return -1;
					}
					size = 0;
				}
				for (j = 0; j < 2; j++) {
					crc32_step(h1, dfu_xbuf[j * 6 + 0]);
					crc32_step(h1, dfu_xbuf[j * 6 + 1]);
					crc32_step(h1, dfu_xbuf[j * 6 + 2]);
					crc32_step(h1, dfu_xbuf[j * 6 + 3]);
					crc32_step(h1, dfu_xbuf[j * 6 + 4]);
					crc32_step(h1, dfu_xbuf[j * 6 + 5]);
				}

				char* newbuf = (char*)malloc(size + 16);
				memcpy(newbuf, &buf[i * packet_size], size);
				memcpy(newbuf+size, dfu_xbuf, 12);
				newbuf[size+12] = h1 & 0xFF;
				newbuf[size+13] = (h1 >> 8) & 0xFF;
				newbuf[size+14] = (h1 >> 16) & 0xFF;
				newbuf[size+15] = (h1 >> 24) & 0xFF;
				size += 16;
				ret = libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, (unsigned char*) newbuf, size, USB_TIMEOUT);
				if(ret != size) {
					return -1;
				}
				free(newbuf);
			} else {
				ret = libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, &buf[i * packet_size], size, USB_TIMEOUT);
				if(ret != size) {
					return -1;
				}
			}
		}
	}
	else {
		libloader_ctrl_transfer(dev, 0x41, 0, 0, 0, NULL, 0, USB_TIMEOUT);
		for (int i = 0; i < packets; i++) {
			int bytes;
			int size = (i + 1) < packets ? packet_size : last;
			ret = libloader_bulk_transfer(dev, 0x04, &buf[i * packet_size], size, &bytes, USB_TIMEOUT);
			if(ret != 0 || bytes != size) {
				return -1;
			}
		}
	}
	return 0;
}

int libloader_send_cmd(libloader_device_t dev, char* cmd) {
	int ret;
	ret = libloader_ctrl_transfer(dev, 0x40, 0, 0, 0, (unsigned char*)cmd, strlen(cmd) + 1, 30000);
	if(ret != strlen(cmd) + 1) {
		return -1;
	}
	return 0;
}

int libloader_bulk_transfer(libloader_device_t dev, unsigned char endpoint, unsigned char* data, int length, int* transferred, unsigned int timeout) {
	return libusb_bulk_transfer(dev->dev, endpoint, data, length, transferred, timeout);
}

int libloader_ctrl_transfer(libloader_device_t dev, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int timeout) {
	return libusb_control_transfer(dev->dev, bm_request_type, b_request, w_value, w_index, data, w_length, timeout);
}

 // https://stackoverflow.com/a/36095407 and https://stackoverflow.com/a/6725161

static long get_nanos(void) {
	struct timespec ts;
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts.tv_sec = mts.tv_sec;
	ts.tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_REALTIME, &ts);
#endif
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}


int libloader_async_ctrl_transfer(libloader_device_t dev, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, float timeout) {
	int ret;
    long start = get_nanos();
	unsigned char* buffer = malloc(w_length + 8);
	struct libusb_transfer* transfer = libusb_alloc_transfer(0);
	if(!transfer) {
		return -1;
	}
	memcpy((buffer + 8), data, w_length);
    libusb_fill_control_setup(buffer, bm_request_type, b_request, w_value, w_index, w_length);
	libusb_fill_control_transfer(transfer, dev->dev, buffer, NULL, NULL, 0);
	transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
	ret = libusb_submit_transfer(transfer);
	if(ret != 0) {
		return -1;
	}
	while((get_nanos() - start) < (timeout * (1000000)));
	ret = libusb_cancel_transfer(transfer);
	if(ret != 0) {
		return -1;
	}
	free(buffer);
	return 0;
}

libloader_device_t libloader_reconnect(libloader_device_t dev, float wait) {
	libloader_close(dev);
	if(wait != 0){
		long start = get_nanos();
		while((get_nanos() - start) < (wait * (1000000000)));
	}
	libloader_device_t new_dev = NULL;
	for(int i = 0; i < 5; i++){
		new_dev = libloader_get_device_handle();
		if(new_dev){
			return new_dev;
		}
		sleep(1);
	}
	return NULL;
}	


