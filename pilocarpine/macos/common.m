#import <Cocoa/Cocoa.h>


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

NSButton* create_button(NSString* title, int x, int y, int width, int height, NSObject* callback, SEL selector, NSView* view) {
	NSButton *btn = [[NSButton alloc] initWithFrame:NSMakeRect(x, y, width, height)]; 
	[btn setBezelStyle:NSBezelStyleRounded];
	[btn setTitle:title];
	[btn setTarget:callback];
	[btn setAction:selector];
	[view addSubview:btn]; 
	return btn;
}

NSTextField* create_label(NSString* title, int x, int y, int width, int height, NSView* view) {
	NSTextField* label = [[NSTextField alloc] initWithFrame:NSMakeRect(x, y, width, height)];
	[label setStringValue:title];
	[label setBezeled:NO];
	[label setBordered:NO];
	[label setDrawsBackground:NO];
	[label setEditable:NO];
	[label setSelectable:NO];
	[label setAlignment:NSTextAlignmentCenter];
	[view addSubview:label];
	return label;
}