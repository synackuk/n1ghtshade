// Umbrella header for asr_bypass.
// Add import lines for each public header, like this: #import <asr_bypass/XXXAwesomeClass.h>
// Don’t forget to also add them to asr_bypass_PUBLIC_HEADERS in your Makefile!

#include <unistd.h>
#include <fcntl.h>

#include <mach-o/dyld.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <stddef.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/dyld.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>


#ifndef LC_LAZY_LOAD_DYLIB
#define LC_LAZY_LOAD_DYLIB  0x20
#endif
#ifndef S_LAZY_DYLIB_SYMBOL_POINTERS
#define S_LAZY_DYLIB_SYMBOL_POINTERS  0x10
#endif

#define IOCTL_GET_BLOCK_SIZE 0x40046418
#define IOCTL_GET_BLOCK_COUNT 0x40086419

#define RAMDISK_BLOCKSIZE 0x1000

#define MODULE_NAME "asr"

#define LC_SEGMENT_COMMAND		LC_SEGMENT
#define LC_ROUTINES_COMMAND		LC_ROUTINES

typedef struct mach_header		macho_header;
typedef struct section			macho_section;
typedef struct nlist			macho_nlist;
typedef struct segment_command	macho_segment_command;

uintptr_t* get_import_ptr(const macho_header* mh, const char* importName);


