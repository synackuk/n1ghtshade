#ifndef COMMON_H
#define COMMON_H

#import <Cocoa/Cocoa.h>

void create_menus();
NSButton* create_button(NSString* title, int x, int y, int width, int height, NSObject* callback, SEL selector, NSView* view);

#endif