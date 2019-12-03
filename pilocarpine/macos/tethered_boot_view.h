#ifndef TETHEREDBOOTVIEW_H
#define TETHEREDBOOTVIEW_H
#import <Cocoa/Cocoa.h>

NSWindow* window;
NSView* tethered_boot_view;

void init_tethered_boot_view();

@interface TetheredBootView : NSObject {}
@end

#endif