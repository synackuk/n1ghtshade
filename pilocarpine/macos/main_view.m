#import <main_view.h>

#import <other_options_view.h>

#import <Cocoa/Cocoa.h>
#import <common.h>

#import <dfu_enter_view.h>
#import <tasks_view.h>

#include <dimensions.h>

@implementation MainView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	self.welcome_label = create_label(@"Welcome to n1ghtshade by synackuk.", PADDING, CENTRE_SHIFT(PAD_H(LOGO_HEIGHT), LOGO_HEIGHT, BOLD_TEXT_HEIGHT), TEXT_WIDTH, BOLD_TEXT_HEIGHT, self);
	[self.welcome_label setFont: [NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];

	self.logo = create_logo(PAD_W(LOGO_WIDTH), PAD_H(LOGO_HEIGHT), LOGO_WIDTH, LOGO_HEIGHT, [NSImage imageNamed:@"n1ghtshade.icns"], self);

	self.boot_tethered_button = create_button(@"Boot Tethered", PADDING * 5, 256, BUTTON_WIDTH, BUTTON_HEIGHT, @selector(tethered_boot_btn), self);
	self.boot_tethered_label = create_label(@"Tether boot a device.", PADDING * 6 + BUTTON_WIDTH, CENTRE_SHIFT(256, BUTTON_HEIGHT, TEXT_HEIGHT), TEXT_WIDTH, TEXT_HEIGHT, self);

	self.other_button = create_button(@"Other", PADDING * 5, 208, BUTTON_WIDTH, BUTTON_HEIGHT, @selector(other_btn), self);
	self.other_label = create_label(@"Everything Else.", PADDING * 6 + BUTTON_WIDTH, CENTRE_SHIFT(208, BUTTON_HEIGHT, TEXT_HEIGHT), TEXT_WIDTH, TEXT_HEIGHT, self);

	self.credits_textbox = create_textbox(@"With thanks to: axi0mX, Daniel Volt, Chronic Dev, xerub, iH8Sn0w, tihmstar, nyan_satan, libimobiledevice and RealiMuseum.", PADDING, PADDING, TEXT_WIDTH, TEXT_HEIGHT, self);
	
}

- (void) tethered_boot_btn {
	option = boot_tethered;
	new_view(dfu_enter_view);
}

- (void) other_btn {
	new_view(other_options_view);
}

@end