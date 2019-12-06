#ifndef LIBBELLADONNA_H
#define LIBBELLADONNA_H

#include "libloader/libloader.h"

typedef int(*exploit_supported)(libloader_device_t dev);
typedef libloader_device_t(*exploit_func)(libloader_device_t dev);
typedef void(*belladonna_log_cb)(char* msg);
typedef void(*belladonna_prog_cb)(unsigned int progress);

extern belladonna_log_cb belladonna_log;
extern belladonna_prog_cb belladonna_prog;

#define LOG belladonna_log

#define error(x, args...) printf("Error in %s (%s:%d) \""x"\"\n", __FUNCTION__, __FILE__, __LINE__, ##args)

typedef struct exploit_list {
	char* name;
	exploit_supported supported;
	exploit_func exploit;
	struct exploit_list* next;
} exploit_list;

int libbelladonna_enter_recovery();
void libbelladonna_set_log_cb(belladonna_log_cb new_cb);
void libbelladonna_set_prog_cb(belladonna_prog_cb new_cb);
void libbelladonna_init();
int libbelladonna_get_device();
int libbelladonna_compatible();
int libbelladonna_exploit();
int libbelladonna_boot_tethered(char* boot_args);
int libbelladonna_boot_ramdisk();
int libbelladonna_restore_ipsw(char* path);
void libbelladonna_exit();

#endif