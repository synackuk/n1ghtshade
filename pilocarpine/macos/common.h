#ifndef COMMON_H
#define COMMON_H
#import <Cocoa/Cocoa.h>
#include <libbelladonna.h>

extern NSWindow* window;
extern belladonna_ctx_t ctx;

void new_view(NSView* view);
void swap_view(NSView* view);
void prev_view();
void create_menus();
NSButton* create_button(NSString* title, int x, int y, int width, int height, SEL selector, NSView* view);
NSButton* create_checkbox(NSString* title, int x, int y, int width, int height, NSView* view);
NSTextView* create_textbox(NSString* text, int x, int y, int width, int height, NSView* view);
NSScrollView* create_scrollbox(NSString* text, int x, int y, int width, int height, NSView* view);
NSTextField* create_label(NSString* title, int x, int y, int width, int height, NSView* view);
NSTextField* create_editbox(NSString* title, int x, int y, int width, int height, NSView* view);
NSProgressIndicator* create_progress_bar(int x, int y, int width, int height, NSView* view);
NSImageView* create_logo(int x, int y, int width, int height, NSImage* image, NSView* view);

#endif