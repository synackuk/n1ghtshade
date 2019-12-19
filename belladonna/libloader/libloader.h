#ifndef LIBLOADER_H
#define LIBLOADER_H

#include <libusb-1.0/libusb.h>

#define APPLE_VID 0x5AC
#define DFU_MAX_PACKET_SIZE 0x800
#define RECVOVERY_MAX_PACKET_SIZE 0x8000

#define DFU_MODE 0x1227

#define RECOVERY_MODE_1 0x1280
#define RECOVERY_MODE_2 0x1281
#define RECOVERY_MODE_3 0x1282
#define RECOVERY_MODE_4 0x1283

#define NORMAL_MODE 0x1290

#define USB_TIMEOUT 1000

enum libloader_error {
    LIBLOADER_SUCCESS             = LIBUSB_SUCCESS,
    LIBLOADER_ERROR_IO            = LIBUSB_ERROR_IO,
    LIBLOADER_ERROR_INVALID_PARAM = LIBUSB_ERROR_INVALID_PARAM,
    LIBLOADER_ERROR_ACCESS        = LIBUSB_ERROR_ACCESS,
    LIBLOADER_ERROR_NO_DEVICE     = LIBUSB_ERROR_NO_DEVICE,
    LIBLOADER_ERROR_NOT_FOUND     = LIBUSB_ERROR_NOT_FOUND,
    LIBLOADER_ERROR_BUSY          = LIBUSB_ERROR_BUSY,
    LIBLOADER_ERROR_TIMEOUT       = LIBUSB_ERROR_TIMEOUT,
    LIBLOADER_ERROR_OVERFLOW      = LIBUSB_ERROR_OVERFLOW,
    LIBLOADER_ERROR_PIPE          = LIBUSB_ERROR_PIPE,
    LIBLOADER_ERROR_INTERRUPTED   = LIBUSB_ERROR_INTERRUPTED,
    LIBLOADER_ERROR_NO_MEM        = LIBUSB_ERROR_NO_MEM,
    LIBLOADER_ERROR_NOT_SUPPORTED = LIBUSB_ERROR_NOT_SUPPORTED,
    LIBLOADER_ERROR_OTHER         = LIBUSB_ERROR_OTHER,
};

typedef struct libloader_device_t{
	libusb_device_handle* dev;
	int mode;
	unsigned char serial[256];
}* libloader_device_t;

void libloader_init();
void libloader_exit();

int libloader_is_dfu(libloader_device_t dev);
int libloader_send_cmd(libloader_device_t dev, char* cmd);
int libloader_bulk_transfer(libloader_device_t dev, unsigned char endpoint, unsigned char* data, int length, int* transferred, unsigned int timeout);
int libloader_get_status(libloader_device_t dev);
int libloader_request_image_validation(libloader_device_t dev);
int libloader_is_checkm8_dfu(libloader_device_t dev);
int libloader_send_buffer(libloader_device_t dev, unsigned char* buf, size_t len);
int libloader_is_pwned_dfu(libloader_device_t dev);
uint16_t libloader_get_cpid(libloader_device_t dev);
int libloader_get_bdid(libloader_device_t dev);
char* libloader_get_identifier(libloader_device_t dev);
libloader_device_t libloader_get_device_handle();
void libloader_close(libloader_device_t dev);
int libloader_reset(libloader_device_t dev);
int libloader_ctrl_transfer(libloader_device_t dev, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, unsigned int timeout);
int libloader_async_ctrl_transfer(libloader_device_t dev, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, unsigned char *data, uint16_t w_length, float timeout);
libloader_device_t libloader_reconnect(libloader_device_t dev, float wait);

#endif