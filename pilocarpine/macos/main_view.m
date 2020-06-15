#import <main_view.h>

#import <other_options_view.h>

#import <Cocoa/Cocoa.h>
#import <common.h>

#import <dfu_enter_view.h>
#import <tasks_view.h>


@implementation MainView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	self.welcome_label = create_label(@"Welcome to n1ghtshade by synackuk.", 10, 235, 480, 15, self);
	[self.welcome_label setFont: [NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
	self.help_label = create_label(@"To begin, select one of the options below.", 10, 210, 480, 15, self);
	self.logo = [[NSImageView alloc] initWithFrame:NSMakeRect(396, 186, 64, 64)];
	[self.logo setImage:[NSImage imageNamed:@"n1ghtshade.icns"]];
	[self addSubview: self.logo];

	self.boot_tethered_button = create_button(@"Boot Tethered", 20, 160, 160, 28, @selector(tethered_boot_btn), self);
	self.boot_tethered_label = create_label(@"Tether boot a device.", 205, 156, 250, 28, self);

	self.other_button = create_button(@"Other", 20, 112, 160, 28, @selector(other_btn), self);
	self.other_label = create_label(@"Functions for restoring and jailbreaking.", 205, 108, 250, 28, self);

	self.credits_textbox = create_textbox(@"With thanks to: axi0mX, Daniel Volt, Chronic Dev, xerub, iH8Sn0w, tihmstar, nyan_satan, libimobiledevice and RealiMuseum.", 10, 10, 460, 60, self);
	
}

- (void) tethered_boot_btn {
	option = boot_tethered;
	new_view(dfu_enter_view);
}

- (void) other_btn {
	new_view(other_options_view);
}

@end