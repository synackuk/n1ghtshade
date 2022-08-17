#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libbelladonna.h>
#include <loaders.h>

#include <idevicerestore/common.h>
#include <idevicerestore/dfu.h>
#include <idevicerestore/recovery.h>
#include <idevicerestore/download.h>
#include <idevicerestore/idevicerestore.h>

/* hooks we use for idevicerestore */

static int idevicerestore_ibss_hook(struct idevicerestore_client_t* client, void* to_load, size_t load_len);
static int idevicerestore_ibec_hook(struct idevicerestore_client_t* client);

/* The function we use for getting the LLB for tethered booting */

static int get_device_llb(belladonna_ctx_t ctx, char** llb, size_t* llb_len);

static int default_prog_cb(double progress) {
	return 0;
}


int belladonna_init(belladonna_ctx_t* ctx) {
	belladonna_ctx_t tmp_context;

	/* Allocate the context and zero it out */
	tmp_context = (belladonna_ctx_t)malloc(sizeof(struct belladonna_ctx));
	if(!tmp_context) {
		printf("Failed to allocate context\n");
		return -1;
	}
	memset(tmp_context, 0, sizeof(struct belladonna_ctx));

	/* We set default logging functions so that we can get output */
	belladonna_set_progress_cb(tmp_context, default_prog_cb);
	belladonna_set_log_cb(tmp_context, printf);
	belladonna_set_error_cb(tmp_context, printf);
	*ctx = tmp_context;
	return 0;
}

void belladonna_set_progress_cb(belladonna_ctx_t ctx, belladonna_progress_t progress_cb) {
	ctx->progress_log = progress_cb;
}

void belladonna_set_log_cb(belladonna_ctx_t ctx, belladonna_log_t debug_cb) {
	ctx->debug_log = debug_cb;
	debug = printf;
	info = debug_cb;
}

void belladonna_set_error_cb(belladonna_ctx_t ctx, belladonna_log_t error_cb) {
	ctx->error_log = error_cb;
	error = error_cb;
}

int belladonna_exit(belladonna_ctx_t ctx) {
	int ret = 0;
	if(ctx->client) {
		ret = belladonna_close_device(ctx);
	}
	free(ctx);
	return ret;
}

static void idevicerestore_prog_wrapper(int step, double step_progress, void* userdata) {
	belladonna_ctx_t ctx = (belladonna_ctx_t)userdata;
	ctx->progress_log(step_progress * 100.0);
}

int belladonna_restore(belladonna_ctx_t ctx, char* path) {
	int ret;

	/* Verify we're in the correct mode */
	if(ctx->loaded_img != RECOVERY_IMG_PWND_DFU) {
		ctx->error_log("Device not in pwned DFU mode\n");
		return -1;
	}

	ctx->debug_log("Restoring device\n");

	/* idevicerestore opens its own device handle */
	ret = belladonna_close_device(ctx);

	if(ret != 0) {
		ctx->error_log("Failed to close device handle\n");
		return -1;
	}
	
	struct idevicerestore_client_t* client = idevicerestore_client_new();
	if (!client) {
		ctx->error_log("Failed to create restore client\n");
		return -1;
	}

	/* We set our flags */
	client->flags |= FLAG_ERASE;
	client->flags |= FLAG_DOWNGRADE;
	client->flags &= ~FLAG_INTERACTIVE;
	client->ipsw = strdup(path);


	client->user_data = ctx;
	client->ibss_load_hook = &idevicerestore_ibss_hook;
	client->ibec_post_load = &idevicerestore_ibec_hook;
	idevicerestore_set_progress_callback(client, idevicerestore_prog_wrapper, ctx);

	ret = idevicerestore_start(client);
	if(ret != 0) {
		ctx->error_log("Failed to restore device\n");
		return ret;
	}
	idevicerestore_client_free(client);
	return 0;
}

int belladonna_enter_iboot(belladonna_ctx_t ctx) {
	int ret;
	char* llb;
	size_t llb_len;

	/* Verify we're in the correct mode */
	if(ctx->loaded_img != RECOVERY_IMG_PWND_DFU) {
		ctx->error_log("Device not in pwned DFU mode\n");
		return -1;
	}

	/* First we need to download the LLB */
	ret = get_device_llb(ctx, &llb, &llb_len); 
	if(ret != 0) {
		ctx->error_log("Failed to download llb\n");
		return -1;
	}

	/* Next we execute the LLB, which will then boot the correct iBoot from the device */
	ctx->debug_log("Booting LLB\n");
	ret = belladonna_execute_first_stage_loader(ctx, llb, llb_len); 
	if(ret != 0) {
		ctx->error_log("Failed to boot LLB\n");
		return -1;
	}
	ctx->debug_log("Waiting for iBoot to connect\n");
	ret = belladonna_reconnect(ctx, 2);
	if(ret != 0) {
		ctx->error_log("Failed to reconnect to device.\n");
		return -1;
	}
	if(ctx->loaded_img != RECOVERY_IMG_IBOOT) {
		ctx->error_log("Failed to execute iBoot\n");
		return -1;
	}
	return 0;
}

int belladonna_tethered_boot(belladonna_ctx_t ctx,  const char* boot_args) {
	int ret;

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

	/* We set new boot-args */
	ctx->debug_log("Setting boot-args\n");
	ret = belladonna_set_boot_args(ctx, boot_args);
	if(ret != 0) {
		ctx->error_log("Failed to set boot-args\n");
		return ret;
	}

	/* We use our payload to call fsboot */
	ctx->debug_log("Executing fsboot\n");
	ret = irecv_send_command(ctx->client, "atropine fsboot");
	if(ret != IRECV_E_SUCCESS) {
		ctx->error_log("Failed to send fsboot command\n");
		return ret;
	}
	return 0;
}

static int idevicerestore_ibss_hook(struct idevicerestore_client_t* client, void* to_load, size_t load_len) {
	int ret;

	/* Dirty hack to give us a functional context to use */
	belladonna_ctx_t ctx = client->user_data;
	ctx->client = client->dfu->client;
	ret = belladonna_execute_first_stage_loader(ctx, to_load, load_len);
	ctx->client = NULL;
	return ret;
}

static int idevicerestore_ibec_hook(struct idevicerestore_client_t* client) {
	int ret;

	/* Dirty hack to give us a functional context to use */
	belladonna_ctx_t ctx = client->user_data;
	ctx->client = client->recovery->client;

	/* This hook is only called after an iBEC has been booted */
	ctx->loaded_img = RECOVERY_IMG_IBEC;
	ret = belladonna_execute_atropine(ctx);
	ctx->client = NULL;
	return ret;
}

static int get_device_llb(belladonna_ctx_t ctx, char** llb, size_t* llb_len) {
	int i = 0;
	char* ipsw_url = NULL;
	char* llb_path = NULL;

	/* Verify that a device is attached and that the product type has been properly identified */

	if(!ctx->client || ctx->dev_info->product_type == NULL) {
		ctx->error_log("Client not attached\n");
		return -1;
	}
	ctx->debug_log("Finding appropriate LLB\n");
	/* We loop through all the devices for which we have llbs */
	while(device_loaders[i].product_type != NULL) {
		/* If we have the correct device then get the required details*/
		if(!strcmp(ctx->dev_info->product_type, device_loaders[i].product_type)) {
			ipsw_url = (char*)device_loaders[i].ipsw_url;
			llb_path = (char*)device_loaders[i].llb_path;
			break;
		}
		i += 1;
	}
	if(!ipsw_url || !llb_path) {
		ctx->error_log("No LLB found for device %s\n", ctx->dev_info->product_type);
		return -1;
	}

	/* We download the llb to memory */
	ctx->debug_log("Downloading LLB\n");
	return download_firmware_component(ipsw_url, llb_path, llb, llb_len);
}
