#ifndef COMMON_H
#define COMMON_H
#import <Cocoa/Cocoa.h>

NSWindow* window;

void new_view(NSView* view);
void swap_view(NSView* view);
void prev_view();
void create_menus();
NSButton* create_button(NSString* title, int x, int y, int width, int height, SEL selector, NSView* view);
NSTextView* create_textbox(NSString* text, int x, int y, int width, int height, NSView* view);
NSScrollView* create_scrollbox(NSString* text, int x, int y, int width, int height, NSView* view);
NSTextField* create_label(NSString* title, int x, int y, int width, int height, NSView* view);
NSProgressIndicator* create_progress_bar(int x, int y, int width, int height, NSView* view);

#endif