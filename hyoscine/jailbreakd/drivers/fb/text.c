#include <drivers/drivers.h>

#include "font.h"

static int debug_x = 0;
static int debug_y = 0;


void display_write_char(int chr, int x, int y) {
	/* Get the bitmap of the character */
	char* char_bitmap = font[chr];

	/* Iterate through the x and y coordinates of the bitmap */
	for(int chr_y = 0; chr_y < FONT_WIDTH; chr_y += 1) {
		for(int chr_x = 0; chr_x < FONT_WIDTH; chr_x += 1) {

			/* Get the x and y coordinate of the char before scaling */
			int bitmap_x = chr_x / display_info.sf;
			int bitmap_y = chr_y / display_info.sf;

			/* Check if the pixel is meant to be active or not */
			if(char_bitmap[bitmap_y] & 1 << (bitmap_x)) {

				/* If the pixel is meant to be active set it to the foreground colour */
				display_write_pixel(x + chr_x, y + chr_y, display_info.foreground_colour);
			}
		}
	}
}

void display_write(char* s, int x, int y) {
	size_t len = strlen(s);
	for(int i = 0; i < len; i += 1) {
		char chr = s[i];

		/* For newline just move on to the next line */

		if(chr == '\n') {
			x = 0;
			y += FONT_HEIGHT;
			continue;
		}
		display_write_char(chr, x, y);

		/* Move onto the next character and next row if needed */
		x += FONT_WIDTH;
		if (x >= display_info.width) {
			x = 0;
			y += FONT_HEIGHT;
		}
	}
}

void display_write_debug_char(char chr) {
	/* For newline just move on to the next line */
	if(chr == '\n') {
		debug_x = 0;
		debug_y += FONT_HEIGHT;
		return;
	}
	display_write_char(chr, debug_x, debug_y);

	/* Move onto the next character and next row if needed */
	debug_x += FONT_WIDTH;
	if (debug_x >= display_info.width) {
		debug_x = 0;
		debug_y += FONT_HEIGHT;
	}
}

void display_write_debug(char* s) {
	size_t len = strlen(s);
	for(int i = 0; i < len; i += 1) {
		display_write_debug_char(s[i]);
	}
}