#import <other_options_view.h>
#import <main_view.h>

#import <Cocoa/Cocoa.h>
#import <common.h>
#import <dfu_enter_view.h>
#import <tasks_view.h>


@implementation OtherOptionsView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	self.restore_button = create_button(@"Restore", 20, 160, 160, 28, @selector(restore_btn), other_options_view);
	self.restore_label = create_label(@"Restore a device without SHSH blobs.", 205, 156, 250, 28, other_options_view);
	
	self.jailbreak_button = create_button(@"Jailbreak", 20, 112, 160, 28, @selector(jailbreak_btn), other_options_view);
	self.jailbreak_label = create_label(@"Install a tethered jailbreak.", 205, 108, 250, 28, other_options_view);

	self.back_button = create_button(@"Back", 300, 20, 160, 28, @selector(back_btn), other_options_view);
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