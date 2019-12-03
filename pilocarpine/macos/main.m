#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <main_view.h>
#import <tethered_boot_view.h>
#import <common.h>

int main(int argc, const char * argv[]) {

	// Autorelease Pool:
	// Objects declared in this scope will be automatically
	// released at the end of it, when the pool is "drained".
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	// Create a shared app instance.
	// This will initialize the global variable
	// 'NSApp' with the application instance.
	[NSApplication sharedApplication];

	//
	// Create a window:
	//

	// Style flags:
	NSUInteger windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

	// Window bounds (x, y, width, height).
	NSRect windowRect = NSMakeRect(250, 312, 350, 336);
	window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
	[window autorelease];

	// Window controller:
	NSWindowController * windowController = [[NSWindowController alloc] initWithWindow:window];
	[windowController autorelease];

	create_menus();

	// Init views

	init_main_view();
	init_tethered_boot_view();

	// Set the main view as the visible one

	[window setContentView:main_view];

	// Show window and run event loop.
	[window setTitle:@"n1ghtshade"];
	[window makeKeyAndOrderFront:nil];
	[window center];
	[NSApp run];

	[pool drain];

	return NSApplicationMain(argc, (const char **)argv);
}