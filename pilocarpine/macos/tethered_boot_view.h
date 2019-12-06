#ifndef TETHERED_BOOT_VIEW_H
#define TETHERED_BOOT_VIEW_H
#import <Cocoa/Cocoa.h>

NSWindow* window;
NSView* tethered_boot_view;

void init_tethered_boot_view();

@interface TetheredBootView : NSObject {}
@end

#endif