#include <libloader.h>
#include <device_identifiers.h>
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

static unsigned int crc32_lookup_t1[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

#define crc32_step(a,b) \
	a = (crc32_lookup_t1[(a & 0xFF) ^ ((unsigned char)b)] ^ (a >> 8))

static libusb_context* libloader_context;

void libloader_init() {
	libusb_init(&libloader_context);
}

void libloader_exit() {
	libusb_exit(libloader_context);
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

uint8_t libloader_get_bdid(libloader_device_t dev) {
	char* bdid_ptr = strstr((char*)dev->serial, "BDID:");
	if(!bdid_ptr) {
		return 0;
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
	uint8_t bdid = libloader_get_bdid(dev);
	if(!bdid) {
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
		if(desc.idVendor == APPLE_VID && (desc.idProduct == DFU_MODE || desc.idProduct == RECOVERY_MODE_1 || desc.idProduct == RECOVERY_MODE_2 || desc.idProduct == RECOVERY_MODE_3 || desc.idProduct == RECOVERY_MODE_4)) {
			break;
		}
	}
	dev->mode = desc.idProduct;
	ret = libusb_open(usb_dev, &dev->dev);
	if(ret != 0){
		return NULL;
	}
	libusb_get_string_descriptor_ascii(dev->dev, desc.iSerialNumber, dev->serial, 255);
	libusb_free_device_list(list, 1);
	if(ret != 0){
		return NULL;
	}
	ret = libusb_claim_interface(dev->dev, 0);
	if(ret != 0){
		return NULL;
	}
	return dev;
}

void libloader_close(libloader_device_t dev) {
	if(!dev) {
		return;
	}
	libusb_release_interface(dev->dev, 0);
	libusb_close(dev->dev);
	free(dev);
}

int libloader_reset(libloader_device_t dev) {
	return libusb_reset_device(dev->dev);
}

int libloader_get_status(libloader_device_t dev) {
	unsigned char buf[6];
	int ret = libloader_ctrl_transfer(dev, 0xA1, 3, 0, 0, buf, 6, 1000);
	if(ret < 0){
		return -1;
	}
	return 0;
}

int libloader_request_image_validation(libloader_device_t dev) {
	int ret;
	libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, 0, 0, 1000);
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
					ret = libloader_ctrl_transfer(dev, 0x21, 1, i, 0, &buf[i * packet_size], size, 1000);
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
				ret = libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, (unsigned char*) newbuf, size, 1000);
				if(ret != size) {
					return -1;
				}
				free(newbuf);
			} else {
				ret = libloader_ctrl_transfer(dev, 0x21, 1, 0, 0, &buf[i * packet_size], size, 1000);
				if(ret != size) {
					return -1;
				}
			}
		}
	}
	else {
		libloader_ctrl_transfer(dev, 0x41, 0, 0, 0, NULL, 0, 1000);
		for (int i = 0; i < packets; i++) {
			int bytes;
			int size = (i + 1) < packets ? packet_size : last;
			ret = libloader_bulk_transfer(dev, 0x04, &buf[i * packet_size], size, &bytes, 1000);
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
	while((get_nanos() - start) < (timeout * (10 * 6)));
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
		sleep(wait);
	}
	libloader_device_t new_dev = NULL;
	float start = clock();
	for(int i = 0; i < 5; i++){
		new_dev = libloader_get_device_handle();
		if(new_dev){
			return new_dev;
		}
		sleep(1);
	}
	return NULL;
}	


