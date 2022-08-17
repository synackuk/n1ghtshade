#import <other_options_view.h>
#import <main_view.h>

#import <Cocoa/Cocoa.h>
#import <common.h>
#import <dfu_enter_view.h>
#import <tasks_view.h>

#include <dimensions.h>

@implementation OtherOptionsView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	self.welcome_label = create_label(@"Welcome to n1ghtshade by synackuk.", PADDING, CENTRE_SHIFT(PAD_H(LOGO_HEIGHT), LOGO_HEIGHT, BOLD_TEXT_HEIGHT), TEXT_WIDTH, BOLD_TEXT_HEIGHT, self);
	[self.welcome_label setFont: [NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
	self.logo = create_logo(PAD_W(LOGO_WIDTH), PAD_H(LOGO_HEIGHT), LOGO_WIDTH, LOGO_HEIGHT, [NSImage imageNamed:@"n1ghtshade.icns"], self);
	
	self.restore_button = create_button(@"Restore", PADDING * 5, 256, BUTTON_WIDTH, BUTTON_HEIGHT, @selector(restore_btn), other_options_view);
	self.restore_label = create_label(@"Restore a device without SHSH blobs.", PADDING * 6 + BUTTON_WIDTH, CENTRE_SHIFT(256, BUTTON_HEIGHT, TEXT_HEIGHT), TEXT_WIDTH, TEXT_HEIGHT, self);
	
	self.jailbreak_button = create_button(@"Jailbreak", PADDING * 5, 208, BUTTON_WIDTH, BUTTON_HEIGHT, @selector(jailbreak_btn), other_options_view);
	self.jailbreak_label = create_label(@"Install a tethered jailbreak.", PADDING * 6 + BUTTON_WIDTH, CENTRE_SHIFT(208, BUTTON_HEIGHT, TEXT_HEIGHT), TEXT_WIDTH, TEXT_HEIGHT, self);


	self.back_button = create_button(@"Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, @selector(back_btn), other_options_view);
}

- (void) restore_btn {
	option = restore;
	new_view(dfu_enter_view);
}

- (void) jailbreak_btn {
	option = jailbreak;
	new_view(dfu_enter_view);
}

- (void) back_btn {
	prev_view();
}

@end