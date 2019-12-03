#import <tethered_boot_view.h>
#import <Cocoa/Cocoa.h>
#import <common.h>

@implementation TetheredBootView

@end

void init_tethered_boot_view() {
	TetheredBootView *tethered_boot_view_cb = [[TetheredBootView alloc] init];
	tethered_boot_view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 350, 420)];
}