#ifndef MAINVIEW_H
#define MAINVIEW_H
#import <Cocoa/Cocoa.h>

extern NSView* main_view;

@interface MainView : NSView {}

@property (assign) NSButton* boot_tethered_button;
@property (assign) NSButton* other_button;

@property (assign) NSTextView* credits_textbox;

@property (assign) NSTextField* welcome_label;
@property (assign) NSTextField* help_label;
@property (assign) NSTextField* boot_tethered_label;
@property (assign) NSTextField* other_label;

@property (assign) NSImageView* logo;

@end

#endif