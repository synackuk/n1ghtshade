#ifndef COMMON_H
#define COMMON_H

#include <target.h>
#include <stdint.h>
#include <stdlib.h>
#include <plib.h>
#include <relocate.h>
#include <command.h>
#include <addresses.h>
#include <constants.h>
#include <memory.h>
#include <finders.h>
#include <framebuffer.h>
#include <menu_commands.h>
#include <include/iBoot32Patcher.h>
#include <image.h>
#include <kernel.h>

#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...) 
#endif

#define error(x, args...) printf("Error in %s (%s:%d) \""x"\"\n", __FUNCTION__, __FILE__, __LINE__, ##args)

#define log(x) printf("%s", x); \
			   fb_print(x);

#endif