#import <Cocoa/Cocoa.h>
#import <common.h>

#define MAX_VIEW 20

static NSView* views[MAX_VIEW] = {NULL};
static unsigned int curr_view = 0;

void new_view(NSView* view) {
	curr_view += 1;
	if(curr_view == MAX_VIEW) {
		exit(-1);
	}
	views[curr_view] = view;
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[window setContentView:views[curr_view]];
	});
}

void swap_view(NSView* view) {
	views[curr_view] = view;
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[window setContentView:views[curr_view]];
	});
}

void prev_view() {
	curr_view -= 1;
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[window setContentView:views[curr_view]];
	});
}

// From greenpois0n : https://github.com/Chronic-Dev/doctors/blob/master/macosx/appFunctions.m#L11

void create_menus() {
	
	// Create basic menus:
	
	[NSApp setMainMenu:[[NSMenu alloc] init]];
	
	NSMenu *appleMenu;
	NSMenuItem *menuItem;
	NSString *title;
	NSMenu *windowMenu;
	NSMenuItem  *windowMenuItem;
	
	// Application menu
	
	appleMenu = [[NSMenu alloc] initWithTitle:@""];
	[appleMenu addItemWithTitle:@"About n1ghtshade" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
	[appleMenu addItem:[NSMenuItem separatorItem]];
	[appleMenu addItemWithTitle:@"Hide n1ghtshade" action:@selector(hide:) keyEquivalent:@"h"];
	menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];
	[appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
	[appleMenu addItem:[NSMenuItem separatorItem]];
	[appleMenu addItemWithTitle:@"Quit n1ghtshade" action:@selector(terminate:) keyEquivalent:@"q"];

	// Window menu
	
	windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
	menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
	[windowMenu addItem:menuItem];
	[menuItem release];
	
	// Put menus into the menu bar
	
	menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
	[menuItem setSubmenu:appleMenu];
	[[NSApp mainMenu] addItem:menuItem];
	[NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:appleMenu];
	windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
	[windowMenuItem setSubmenu:windowMenu];
	[[NSApp mainMenu] addItem:windowMenuItem];
	[NSApp setWindowsMenu:windowMenu];

	[appleMenu release];
	[menuItem release];
	[windowMenu release];
	[windowMenuItem release];
}

NSButton* create_button(NSString* title, int x, int y, int width, int height, SEL selector, NSView* view) {
	NSButton *btn = [[NSButton alloc] initWithFrame:NSMakeRect(x, y, width, height)]; 
	[btn setBezelStyle:NSBezelStyleRounded];
	[btn setTitle:title];
	[btn setTarget:view];
	[btn setAction:selector];
	[view addSubview:btn]; 
	return btn;
}

NSTextView* create_textbox(NSString* text, int x, int y, int width, int height, NSView* view) {
	NSTextView* label = [[NSTextView alloc] initWithFrame:NSMakeRect(x, y, width, height)];
	[label setString:text];
	[label setDrawsBackground:NO];
	[label setEditable:NO];
	[label setSelectable:NO];

	[view addSubview:label];
	return label;	
}

NSScrollView* create_scrollbox(NSString* text, int x, int y, int width, int height, NSView* view) {
	NSTextView* label = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
	[label setString:text];
	[label setEditable:NO];
	[label setDrawsBackground:NO];
	NSScrollView* scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(x, y, width, height)];
	[scroll setDocumentView: label];
	[scroll setDrawsBackground:YES];
	[scroll setWantsLayer:YES];
	[scroll.layer setCornerRadius:3.0f];
	[scroll.contentView setWantsLayer:YES];
	[scroll.contentView.layer setCornerRadius:3.0f];
	[view addSubview:scroll];

	return scroll;	
}

NSTextField* create_label(NSString* title, int x, int y, int width, int height, NSView* view) {
	NSTextField* label = [[NSTextField alloc] initWithFrame:NSMakeRect(x, y, width, height)];
	[label setStringValue:title];
	[label setBezeled:NO];
	[label setBordered:NO];
	[label setDrawsBackground:NO];
	[label setEditable:NO];
	[label setSelectable:NO];

	[view addSubview:label];
	return label;
}

NSProgressIndicator* create_progress_bar(int x, int y, int width, int height, NSView* view) {
	NSProgressIndicator*  progress_bar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(x, y, width, height)];
	[progress_bar setIndeterminate:YES];
	[progress_bar setMinValue:0.0];
	[progress_bar setMaxValue:100.0];
	[progress_bar startAnimation:nil];
	[view addSubview:progress_bar];
	return progress_bar;
}

