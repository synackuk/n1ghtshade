#include <drivers/drivers.h>

#include "image.h"

display_info_t display_info;

void display_init(uint32_t* framebuffer, uint32_t width, uint32_t height) {
	display_info.framebuffer = framebuffer;
	display_info.width = width;
	display_info.height = height;
	display_info.background_colour = *display_coords_to_address(0, 0);
	display_info.foreground_colour = ~ display_info.background_colour;
	display_set_sf(2);
	display_clear();
	display_logo();

	display_progress_bar(0);

	/* We print a whole line of = signs for our banner */
	for(int i = 0; i < display_info.width/FONT_WIDTH; i += 1) {
		display_debug_print("=");
	}
	display_debug_print("\n");
	display_debug_print("hyoscine by synackuk\n");
	display_debug_print("Part of the n1ghtshade jailbreak\n");
	display_debug_print("With thanks to:\n");
	display_debug_print("axi0mx, nyansatan, iH8sn0w,\n");
	display_debug_print("linus henze, xerub and tihmstar\n");
	display_debug_print("\n");

	/* We print a whole line of = signs for our banner */
	for(int i = 0; i < display_info.width/FONT_WIDTH; i += 1) {
		display_debug_print("=");
	}
	display_debug_print("\n");
}


uint32_t* display_coords_to_address(int x, int y) {
	/* Since the framebuffer is just a chunk of memory the y-coordinate is simply when the memory wraps around (if you go through the whole width then you'll start on the next row down at the start again) */
	uint32_t offset = y * display_info.width + x;
	return &display_info.framebuffer[offset];
}

void display_write_pixel(int x, int y, uint32_t colour) {
	uint32_t* pixel = display_coords_to_address(x, y);
	*pixel = colour;
}

void display_clear() {
	for(int y = 0; y < display_info.height; y += 1) {
		for(int x = 0; x < display_info.width; x += 1) {
			display_write_pixel(x, y, display_info.background_colour);
		}
	}
}

void display_invert() {
	display_info.background_colour = ~display_info.background_colour;
	display_info.foreground_colour = ~display_info.foreground_colour;
	for(int y = 0; y < display_info.height; y += 1) {
		for(int x = 0; x < display_info.width; x += 1) {
				uint32_t* pixel = display_coords_to_address(x, y);
				uint32_t colour = *pixel;
				*pixel = ~colour;
		}
	}
}

void display_set_sf(int sf) {
	display_info.sf = sf;
}

void display_progress_print(char* str) {
	size_t len = strlen(str);

	/* Scale progress text to fit onto one line */

	for(int sf = 4; sf != 0; sf -= 1) {

		/* If the scale factor = zero the text won't fit on one line and we need to return */
		if(sf == 0) {
			return;
		}

		display_set_sf(sf);
		if((FONT_WIDTH * len) < display_info.width) {
			break;
		}
	}


	int x = (display_info.width / 2) - ((FONT_WIDTH * len) / 2);
	int y = (3 * display_info.height) / 4;

	/* Clear the area where text is to be written */
	display_fill_rect(0, y, display_info.width, y + 32, display_info.background_colour);

	display_write(str, x, y);
}

void display_debug_print(char* str) {
	display_set_sf(2);
	display_write_debug(str);
}

void display_debug_print_char(char c) {
	display_set_sf(2);
	display_write_debug_char(c);
}

void display_logo() {

	/* Find the correct coordinates to move the logo to the middle of the display */
	int x = (display_info.width / 2) - (image_width / 2);
	int y = (display_info.height / 2) - (image_height / 2);
	draw_image((uint32_t*)image, x, y, image_width, image_height);
}


void display_progress_bar(int progress) {
	int start_x = 50;
	int start_y = (2 * display_info.height) / 3;
	int end_x = display_info.width - 50;
	int end_y = start_y + 25;


	/* Clear the progress bar */
	display_fill_rect(start_x, start_y, end_x, end_y, display_info.background_colour);


	/* Stroke the outline of the progress bar */
	display_stroke_rect(start_x, start_y, end_x, end_y, display_info.foreground_colour);

	/* Calculate the % of the bar to fill */
	int progress_x = (int)((double)(end_x) * (((double)progress)/100.00));

	/* Fill the completed % */
	display_fill_rect(start_x, start_y, progress_x, end_y, display_info.foreground_colour);
}