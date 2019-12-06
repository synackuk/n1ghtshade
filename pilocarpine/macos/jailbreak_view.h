#ifndef JAILBREAK_VIEW_H
#define JAILBREAK_VIEW_H
#import <Cocoa/Cocoa.h>

NSWindow* window;
NSView* jailbreak_view;

void init_jailbreak_view();

@interface JailbreakView : NSObject {}
@end

#endif