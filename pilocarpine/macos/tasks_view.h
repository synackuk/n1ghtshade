#ifndef TASKS_VIEW_H
#define TASKS_VIEW_H
#import <Cocoa/Cocoa.h>

typedef enum {
	boot_tethered = 0,
	restore = 1,
	jailbreak = 2
} options_t;

extern char* input_ipsw;

extern options_t option;

extern NSView* tasks_view;

@interface TasksView : NSView {}

@property (assign) NSButton* boot_button;
@property (assign) NSButton* back_button;

@property (assign) NSScrollView* log_scrollbox;

@property (assign) NSTextField* progress_label;

@property (assign) NSProgressIndicator* progress_bar;


@property (assign) NSAlert* error_alert;
@property (assign) BOOL has_error;

@end

#endif