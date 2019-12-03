#ifndef MAINVIEW_H
#define MAINVIEW_H
#import <Cocoa/Cocoa.h>

NSWindow* window;
NSView* main_view;

void init_main_view();

@interface MainView : NSObject {}
@end

#endif