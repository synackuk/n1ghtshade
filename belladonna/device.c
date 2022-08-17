#include <string.h>
#include <libbelladonna.h>

static int get_device_mode(belladonna_ctx_t ctx);

int belladonna_get_device(belladonna_ctx_t ctx) {
	/* Verify that the device handle isn't already open */
	if(ctx->client) {
		ctx->error_log("Device handle already open, close it first\n");
		return -1;
	}

	/* First we try and open a device handle */
	int ret = irecv_open_with_ecid(&ctx->client, 0);
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to get client (are you sure the device is attached?)\n");
		return ret;
	}

	/* Next we get the device details */
	ret = irecv_devices_get_device_by_client(ctx->client, &ctx->dev_info);
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to get device information for the client\n");
		return ret;
	}
	ctx->debug_log("Found %s\n", ctx->dev_info->display_name);

	/* And the mode the device is in */
	ret = get_device_mode(ctx);
	if(ret != 0) {
		ctx->error_log("Failed to get device mode\n");
		return ret;
	}
	return 0;
}

int belladonna_close_device(belladonna_ctx_t ctx) {
	/* Ensure the context exists before exit */
	if(!ctx->client) {
		return -1;
	}
	int ret = irecv_close(ctx->client);
	/* Ensure the context is null to prevent double free */
	ctx->client = NULL;
	return ret;
}

int belladonna_reconnect(belladonna_ctx_t ctx, int wait_time) {
	int ret = 0;
	if(!ctx->client) {
		ctx->error_log("No client to reconnect to\n");
		return -1;
	}
	ctx->client = irecv_reconnect(ctx->client, wait_time);
	if(!ctx->client) {
		ctx->error_log("Failed to reconnect to device.\n");
		return -1;
	}
	ret = get_device_mode(ctx);
	if(ret != 0) {
		ctx->error_log("Failed to get device mode\n");
		return ret;
	}
	return 0;
}


static int get_device_mode(belladonna_ctx_t ctx) {
	/* First check if device is in DFU, WTF or recovery mode */
	int mode = 0;
	int ret = irecv_get_mode(ctx->client, &mode);
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to get device mode\n");
		return ret;
	}

	/* We also need the device info (specifically the iBoot flags) */
	const struct irecv_device_info* dev_inf = irecv_get_device_info(ctx->client);
	unsigned int iboot_flags = dev_inf->ibfl;
	switch(mode) {
		case IRECV_K_DFU_MODE:
			/* The first bit of iboot flags is zero when a SecureROM is loaded and 1 when an iBSS is */
			if(iboot_flags & 1) {
				ctx->loaded_img = RECOVERY_IMG_IBSS;
			}
			else {
				/* Check for Pwned DFU */
				if(strstr(dev_inf->serial_string, "PWND:[")){
					ctx->loaded_img = RECOVERY_IMG_PWND_DFU;
				}
				else {
					/* Else regular secureROM */
					ctx->loaded_img = RECOVERY_IMG_SECUREROM;
				}
			}
			break;
		case IRECV_K_WTF_MODE:
			ctx->loaded_img = RECOVERY_IMG_WTF;
			break;
		case IRECV_K_RECOVERY_MODE_1:
		case IRECV_K_RECOVERY_MODE_2:
		case IRECV_K_RECOVERY_MODE_3:
		case IRECV_K_RECOVERY_MODE_4:
			/* The first bit of iboot flags is zero when an iBEC is loaded and 1 when an iBoot is */
			if(iboot_flags & 1) {
				ctx->loaded_img = RECOVERY_IMG_IBOOT;
			}
			else {
				ctx->loaded_img = RECOVERY_IMG_IBEC;
			}
			break;
		default:
			/* We have no idea what image is loaded */
			return -1;
	}
	return 0;
}