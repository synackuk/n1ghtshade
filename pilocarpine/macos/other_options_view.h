#ifndef OTHER_OPTIONS_VIEW_H
#define OTHER_OPTIONS_VIEW_H
#import <Cocoa/Cocoa.h>


@interface OtherOptionsView : NSView {}
@property (assign) NSButton* restore_button;
@property (assign) NSButton* jailbreak_button;
@property (assign) NSButton* back_button;

@property (assign) NSTextField* welcome_label;
@property (assign) NSTextField* restore_label;
@property (assign) NSTextField* jailbreak_label;

@property (assign) NSImageView* logo;


@end

extern OtherOptionsView* other_options_view;


#endif