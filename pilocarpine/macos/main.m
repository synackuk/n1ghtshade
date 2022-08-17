#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <main_view.h>
#import <tasks_view.h>
#import <other_options_view.h>
#import <dfu_enter_view.h>
#import <callback.h>
#import <common.h>
#include <libbelladonna.h>
#include <dimensions.h>

MainView* main_view;
OtherOptionsView* other_options_view;
DFUEnterView* dfu_enter_view;
TasksView* tasks_view;

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
	NSRect windowRect = NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
	[window autorelease];

	// Window controller:
	NSWindowController * windowController = [[NSWindowController alloc] initWithWindow:window];
	[windowController autorelease];

	create_menus();

	// Init views

	main_view = [[MainView alloc] initWithFrame:NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)];
	other_options_view = [[OtherOptionsView alloc] initWithFrame:NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)];

	dfu_enter_view = [[DFUEnterView alloc] initWithFrame:NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)];
	tasks_view = [[TasksView alloc] initWithFrame:NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)];

	// Setup callbacks

	[[NSNotificationCenter defaultCenter] addObserver:tasks_view selector:@selector(update_log:) name:@"update_log" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:tasks_view selector:@selector(update_progress:) name:@"update_progress" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:tasks_view selector:@selector(error:) name:@"error" object:nil];

	// Set the main view as the visible one

	new_view(main_view);

	belladonna_init(&ctx);
	belladonna_set_progress_cb(ctx, progress_callback);
	belladonna_set_log_cb(ctx, log_callback);
	belladonna_set_error_cb(ctx, error_callback);


	// Show window and run event loop.
	[window setTitle:@"n1ghtshade"];
	[window makeKeyAndOrderFront:nil];
	[window center];
	[NSApp run];

	[pool drain];

	belladonna_exit(ctx);

	return NSApplicationMain(argc, (const char **)argv);
}