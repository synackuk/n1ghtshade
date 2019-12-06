#ifndef RESTORE_VIEW_H
#define RESTORE_VIEW_H
#import <Cocoa/Cocoa.h>

NSWindow* window;
NSView* restore_view;

void init_restore_view();

@interface RestoreView : NSObject {}
@end

#endif