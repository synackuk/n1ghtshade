#ifndef LIB_BELLADONNA_H
#define LIB_BELLADONNA_H

#include <libirecovery.h>
#include <usbmuxd.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*belladonna_log_t)(const char* fmt, ...);
typedef int (*belladonna_progress_t)(double progress);

typedef enum {
	RECOVERY_IMG_SECUREROM = 0,
	RECOVERY_IMG_PWND_DFU,
	RECOVERY_IMG_WTF,
	RECOVERY_IMG_IBSS,
	RECOVERY_IMG_IBEC,
	RECOVERY_IMG_IBOOT,
} belladonna_recovery_images_t;

struct belladonna_ctx {
	irecv_client_t client;
	irecv_device_t dev_info;
	belladonna_log_t debug_log;
	belladonna_log_t error_log;
	belladonna_progress_t progress_log;
	belladonna_recovery_images_t loaded_img;
};

typedef struct belladonna_ctx* belladonna_ctx_t;

/* libbelladonna.c */
int belladonna_init(belladonna_ctx_t* ctx);
void belladonna_set_progress_cb(belladonna_ctx_t ctx, belladonna_progress_t progress_cb);
void belladonna_set_log_cb(belladonna_ctx_t ctx, belladonna_log_t debug_cb);
void belladonna_set_error_cb(belladonna_ctx_t ctx, belladonna_log_t error_cb);
int belladonna_exit(belladonna_ctx_t ctx);
int belladonna_restore(belladonna_ctx_t ctx, char* path);
int belladonna_enter_iboot(belladonna_ctx_t ctx);
int belladonna_tethered_boot(belladonna_ctx_t ctx,  const char* boot_args);

/* device.c */
int belladonna_get_device(belladonna_ctx_t ctx);
int belladonna_close_device(belladonna_ctx_t ctx);
int belladonna_reconnect(belladonna_ctx_t ctx, int wait_time);

/* dfu.c */
int belladonna_enter_pwned_dfu(belladonna_ctx_t ctx);
int belladonna_execute_first_stage_loader(belladonna_ctx_t ctx, const char* loader_in, size_t real_loader_length);

/* recovery.c */
int belladonna_ramdisk_boot(belladonna_ctx_t ctx, const char* ramdisk, size_t ramdisk_length);
int belladonna_set_boot_args(belladonna_ctx_t ctx, const char* boot_args);
int belladonna_execute_atropine(belladonna_ctx_t ctx);

/* ramdisk.c */
int belladonna_boot_hyoscine(belladonna_ctx_t ctx);
int belladonna_jailbreak(belladonna_ctx_t ctx, int should_hacktivate);

#endif