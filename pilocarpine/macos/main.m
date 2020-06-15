#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <main_view.h>
#import <tasks_view.h>
#import <other_options_view.h>
#import <dfu_enter_view.h>
#import <common.h>
#import <callback.h>
#include <libbelladonna.h>


NSView* main_view;
NSView* other_options_view;
NSView* dfu_enter_view;
NSView* tasks_view;

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
	NSRect windowRect = NSMakeRect(250, 312, 480, 270);
	window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
	[window autorelease];

	// Window controller:
	NSWindowController * windowController = [[NSWindowController alloc] initWithWindow:window];
	[windowController autorelease];

	create_menus();

	// Init views

	main_view = [[MainView alloc] initWithFrame:NSMakeRect(0, 0, 480, 270)];
	other_options_view = [[OtherOptionsView alloc] initWithFrame:NSMakeRect(0, 0, 480, 270)];

	dfu_enter_view = [[DFUEnterView alloc] initWithFrame:NSMakeRect(0, 0, 480, 270)];
	tasks_view = [[TasksView alloc] initWithFrame:NSMakeRect(0, 0, 480, 270)];

	// Setup callbacks

	[[NSNotificationCenter defaultCenter] addObserver:tasks_view selector:@selector(update_log:) name:@"update_log" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:tasks_view selector:@selector(update_progress:) name:@"update_progress" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:tasks_view selector:@selector(error:) name:@"error" object:nil];

	// Set the main view as the visible one

	new_view(main_view);


	// initialise belladonna

	belladonna_init();
	belladonna_set_log_cb(log_callback);
	belladonna_set_error_cb(error_callback);
	belladonna_set_prog_cb(progress_callback);

	// Show window and run event loop.
	[window setTitle:@"n1ghtshade"];
	[window makeKeyAndOrderFront:nil];
	[window center];
	[NSApp run];

	[pool drain];

	belladonna_exit();

	return NSApplicationMain(argc, (const char **)argv);
}