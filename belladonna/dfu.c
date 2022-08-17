#include <stdlib.h>
#include <string.h>

#include <libbelladonna.h>

#include <exploits/exploits.h>
#include <payloads/atropine_securerom_payload.h>

static int checkm8_command(belladonna_ctx_t ctx, unsigned char* command, size_t command_len);

int belladonna_enter_pwned_dfu(belladonna_ctx_t ctx) {
	int ret = 0;

	if(!ctx->client) {
		ctx->error_log("No device connected\n");
		return -1;
	}

	/* Make sure we're not already in pwned DFU */
	if(ctx->loaded_img == RECOVERY_IMG_PWND_DFU) {
		return 0;
	}


	/* Verify device is in DFU mode */
	if(ctx->loaded_img != RECOVERY_IMG_SECUREROM) {
		ctx->error_log("Device not in DFU mode\n");
		return -1;
	}

	/* We search for an appropriate exploit in the list of exploits provided */
	for(int i = 0; i < exploits_len; i += 1) {
		exploit_decl_t exploit = exploits[i];
		ret = exploit.supported(ctx);
		if(ret != 0) {
			continue;
		}
		ctx->debug_log("Found supported exploit %s\n", exploit.name);
		return exploit.exploit(ctx);
	}

	/* If no supported exploits are found then we fail */
	ctx->error_log("Device not supported\n");
	return -1;
}

int belladonna_execute_first_stage_loader(belladonna_ctx_t ctx, const char* loader_in, size_t real_loader_length) {
	int ret;

	/* First we add the atropine securerom payload to the end of the ibss or llb so it's executed before the bootloader is loaded */
	size_t loader_len = real_loader_length + atropine_securerom_payload_length;
	char* loader = malloc(loader_len);
	if(!loader) {
		ctx->debug_log("Failed to allocate image buffer\n");
		return -1;
	}
	/* We append our payload to the end of the loader */
	memcpy(loader, loader_in, real_loader_length);
	memcpy(&loader[real_loader_length], atropine_securerom_payload, atropine_securerom_payload_length);

	/* Next we execute it, our checkm8 payload takes the IMG3 magic as a command */
	ret = checkm8_command(ctx, (unsigned char*)loader, loader_len);
	free(loader);
	return ret;
}

static int checkm8_command(belladonna_ctx_t ctx, unsigned char* command, size_t command_len) {
	unsigned char buf[16] = { '\0' };
	int ret;

	/* checkm8 commands require this set of precurser packets, else crash */
	ctx->progress_log(0);
	ret = irecv_usb_control_transfer(ctx->client, 0x21, 1, 0, 0, buf, 16, 5000);
	if(ret < 0) {
		ctx->error_log("Failed to send fake data\n");
		return -1;
	}
	ctx->progress_log(25);
	ret = irecv_usb_control_transfer(ctx->client, 0x21, 1, 0, 0, 0, 0, 5000);
	if(ret != 0) {
		ctx->error_log("Failed to send blank packet\n");
		return -1;
	}
	ctx->progress_log(50);
	ret = irecv_usb_control_transfer(ctx->client, 0xA1, 3, 0, 0, buf, 6, 5000);
	if(ret < 0) {
		ctx->error_log("Failed to request status\n");
		return -1;
	}
	ctx->progress_log(75);
	ret = irecv_usb_control_transfer(ctx->client, 0xA1, 3, 0, 0, buf, 6, 5000);
	if(ret < 0) {
		ctx->error_log("Failed to request status\n");
		return -1;
	}
	ctx->progress_log(100);
	ctx->debug_log("Uploading command\n");
	size_t len = 0;

	/* We upload the command like this to prevent a crc checksum being provided alongside it */
	while(len < command_len) {
		ctx->progress_log(len/command_len * 100);
		size_t size = ((command_len - len) > 0x800) ? 0x800 : (command_len - len);
		size_t sent = irecv_usb_control_transfer(ctx->client, 0x21, 1, 0, 0, &command[len], size, 5000);
		if(sent != size) {
			ctx->error_log("Failed to upload command\n");
			return -1;
		}
		len += size;
	}
	ctx->progress_log(0);
	ctx->debug_log("Executing command\n");
	irecv_usb_control_transfer(ctx->client, 0xA1, 2, 0xFFFF, 0, buf, 0, 5000);
	ctx->progress_log(100);
	return 0;
}