#ifndef TARGET_H
#define TARGET_H

// These values are constant to this type of loader - atropine is designed to work regardless of the loader used

// The offset from the loadaddress we copy atropine to after the first run

#define LOADADDRESS_OFFSET 0x14100000

// The loader magic

#define LOADER_MAGIC 0x43454269

// Command that has been relocated to the load address

#define OVERWRITE_COMMAND "go"

// Whether the loader supports menu commands

#define MENU_COMMANDS

// Whether the loader supports display output

#define DISPLAY_OUTPUT

// Whether we have NVRAM 

#define NVRAM

// Whether this is a debug build

//#define DEBUG

// Whether or not to relocate the payload

#define RELOCATE

// Whether to patch the image the payload is executed against

#define PATCH

#endif