#include <IOSurface/IOSurface.h>
#include <IOMobileFramebuffer.h>


#include <drivers/drivers.h>

int fb_init() {
	IOMobileFramebufferRef conn = 0;

	IOSurfaceRef surface_buffer;
	void* framebuffer;

	IOMobileFramebufferGetMainDisplay(&conn);

	if(!conn) {
		printf("Failed to get main display\n");
		return -1;
	} 


	IOMobileFramebufferGetLayerDefaultSurface(conn, 0, &surface_buffer);

	if(!surface_buffer) {
		printf("Failed to get surface buffer\n");
		return -1;
	} 

	int display_height = IOSurfaceGetHeight(surface_buffer);
	if(!display_height) {
		printf("Failed to get display height\n");
		return -1;
	}
	int display_width = IOSurfaceGetWidth(surface_buffer);
	if(!display_width) {
		printf("Failed to get display width\n");
		return -1;
	}
	IOSurfaceLock(surface_buffer, 3, NULL);
	framebuffer = IOSurfaceGetBaseAddress(surface_buffer);
	IOSurfaceUnlock(surface_buffer, 3, NULL);

	if(!framebuffer) {
		printf("Failed to get framebuffer address\n");
		return -1;
	}

	display_init(framebuffer, display_width, display_height);

	return 0;
}