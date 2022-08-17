#include <libbelladonna.h>
#include <payloads/atropine_ibec_payload.h>
#include <payloads/atropine_iboot_payload.h>

int belladonna_ramdisk_boot(belladonna_ctx_t ctx, const char* ramdisk, size_t ramdisk_length) {
	int ret = 0;

	ret = belladonna_enter_iboot(ctx);
	if(ret < 0) {
		ctx->error_log("Failed to boot iBoot\n");
		return ret;
	}

	/* We execute our iBoot payload, which patches sigchecks and exposes fsboot */
	ret = belladonna_execute_atropine(ctx);
	if(ret != 0) {
		ctx->error_log("Failed to initialise atropine\n");
		return -1;
	}

	ctx->debug_log("Uploading ramdisk\n");

	ret = irecv_send_buffer(ctx->client, (unsigned char*)ramdisk, ramdisk_length, 0);
	if(ret < 0) {
		ctx->error_log("Failed to upload ramdisk\n");
		return ret;
	}

	ctx->debug_log("Setting ramdisk\n");
	ret = irecv_send_command(ctx->client, "atropine ramdisk");
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to set ramdisk\n");
		return ret;
	}

	ctx->debug_log("Executing kernel\n");
	ret = irecv_send_command(ctx->client, "atropine rdboot");
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to execute kernel\n");
		return ret;
	}
	belladonna_close_device(ctx);
	return 0;
}

int belladonna_set_boot_args(belladonna_ctx_t ctx, const char* boot_args) {
	int ret = 0;

	/* atropine only supports custom boot-args for iBoots. */
	if(ctx->loaded_img != RECOVERY_IMG_IBOOT) {
		ctx->error_log("Custom boot-args only work on iBoot\n");
		return -1;
	}

	/* atropine will load the boot-args string from the environment variables before boot */
	ret = irecv_setenv(ctx->client, "boot-args", boot_args);
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to set new boot-args\n");
		return ret;
	}
	ret = irecv_saveenv(ctx->client);
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to save new boot-args\n");
		return ret;
	}
	return 0;
}

int belladonna_execute_atropine(belladonna_ctx_t ctx) {
	unsigned char* payload = NULL;
	size_t payload_len = 0;

	ctx->debug_log("Finding appropriate atropine payload\n");
	/* To find the payload we need we check whether the image loaded is an iBoot or an iBEC */
	if(ctx->loaded_img == RECOVERY_IMG_IBOOT) {
		payload = (unsigned char*)atropine_iboot_payload;
		payload_len = atropine_iboot_payload_length;
	}
	if(ctx->loaded_img == RECOVERY_IMG_IBEC) {
		payload = (unsigned char*)atropine_ibec_payload;
		payload_len = atropine_ibec_payload_length;
	}
	if(!payload || !payload_len) {
		ctx->error_log("Failed to find a payload for image\n");
		return -1;
	}

	ctx->debug_log("Uploading atropine\n");
	/* Now we upload our payload to the device */
	int ret = irecv_send_buffer(ctx->client, payload, payload_len, 0);
	if(ret < 0) {
		ctx->error_log("Failed to upload atropine payload\n");
		return ret;
	}

	ctx->debug_log("Executing atropine\n");
	/* We use the 'go' command to execute it */
	ret = irecv_send_command(ctx->client, "go");
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to send go command\n");
		return ret;
	}
	return 0;
}
