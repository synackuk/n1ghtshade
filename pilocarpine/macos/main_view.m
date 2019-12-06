#import <main_view.h>

#import <tethered_boot_view.h>
#import <jailbreak_view.h>
#import <restore_view.h>


#import <Cocoa/Cocoa.h>
#import <common.h>

@implementation MainView

- (void) jailbreak_btn {
	[window setContentView:jailbreak_view];

}
- (void) tethered_boot_btn {
	[window setContentView:tethered_boot_view];
}

- (void) restore_ipsw_btn {
	[window setContentView:restore_view];
}

@end

void init_main_view() {
	MainView* main_view_cb = [[MainView alloc] init];
	main_view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 350, 420)];

	create_button(@"Jailbreak", 95, 230, 160, 24, main_view_cb, @selector(jailbreak_btn), main_view);
	create_button(@"Boot Tethered", 95, 182, 160, 24, main_view_cb, @selector(tethered_boot_btn), main_view);
	create_button(@"Restore IPSW", 95, 134, 160, 24, main_view_cb, @selector(restore_ipsw_btn), main_view);
}