#ifndef DFU_ENTER_VIEW_H
#define DFU_ENTER_VIEW_H
#import <Cocoa/Cocoa.h>


@interface DFUEnterView : NSView {}

@property (assign) NSButton* start_button;
@property (assign) NSButton* ipsw_button;
@property (assign) NSButton* back_button;
@property (assign) NSButton* should_hacktivate_button;

@property (assign) NSTextView* instructions_textbox;

@property (assign) NSTextField* step_1_label;
@property (assign) NSTextField* step_2_label;
@property (assign) NSTextField* boot_args_label;

@property (assign) NSTextField* boot_args_enter;




@end

extern DFUEnterView* dfu_enter_view;


#endif