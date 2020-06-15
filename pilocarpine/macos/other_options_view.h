#ifndef OTHER_OPTIONS_VIEW_H
#define OTHER_OPTIONS_VIEW_H
#import <Cocoa/Cocoa.h>

extern NSView* other_options_view;

@interface OtherOptionsView : NSView {}
@property (assign) NSButton* restore_button;
@property (assign) NSButton* jailbreak_button;
@property (assign) NSButton* back_button;

@property (assign) NSTextField* restore_label;
@property (assign) NSTextField* jailbreak_label;

@end

#endif