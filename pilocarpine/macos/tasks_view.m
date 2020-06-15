#import <main_view.h>
#import <tasks_view.h>
#import <Cocoa/Cocoa.h>
#import <common.h>
#include <callback.h>

#include <libbelladonna.h>

char* input_ipsw = NULL;

options_t option;

@implementation TasksView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	self.log_scrollbox = create_scrollbox(@"", 25, 110, 430, 150, self);
	self.back_button = create_button(@"Back", 300, 20, 160, 28, @selector(back_btn), self);
	self.progress_label = create_label(@"", 10, 60, 460, 15, self);
	self.progress_label.alignment = NSTextAlignmentCenter;
	self.progress_bar = create_progress_bar(25, 80, 430, 20, self);
	self.error_alert = [[NSAlert alloc] init];
	self.has_error = false;
}

- (void)viewDidMoveToWindow {
	[self.back_button setEnabled: NO];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
		int ret;
		if(option == restore) {
			ret = belladonna_restore_ipsw(input_ipsw);
			if(ret != 0) {
				error_callback("Failed to restore IPSW.\n");
			}
		}
		else if(option == jailbreak) {
			ret = belladonna_boot_ramdisk();
			if(ret != 0) {
				error_callback("Failed to jailbreak device.\n");
			}
		}
		else if(option == boot_tethered) {
			ret = belladonna_boot_tethered();
			if(ret != 0) {
				error_callback("Failed to boot tethered.\n");
			}
		}
		else { // Should never get there
			error_callback("Unknown instruction.\n");
		}
		if(ret == 0) {
			log_callback("Done.");
			progress_callback(0);
		}
		dispatch_async(dispatch_get_main_queue(), ^(void){
			[self.back_button setEnabled: YES];
		});
	});
}

- (void) back_btn {
	prev_view();
}

- (void) update_log:(id)sender {
	NSString* progress_text = [sender userInfo][@"message"];
	NSLog(@"%@", progress_text);
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[self.progress_label setStringValue:progress_text];
		NSTextView* log_text = self.log_scrollbox.documentView;
		NSString* new_log = [[[log_text textStorage] string] stringByAppendingString: progress_text];
		[log_text setString:new_log];
	});
}

- (void) update_progress:(id)sender {
	unsigned int progress = [[sender userInfo][@"progress"] intValue];
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[self.progress_bar setIndeterminate: ((progress == 100) ? YES : NO)];
		[self.progress_bar setDoubleValue:(double)progress];
		[self.progress_bar startAnimation:nil];
	});
}

- (void) error:(id)sender {
	NSString* error_text = [sender userInfo][@"error"];
	NSLog(@"%@", error_text);
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[self.progress_label setStringValue:error_text];
		NSTextView* log_text = self.log_scrollbox.documentView;
		NSString* new_log = [[[log_text textStorage] string] stringByAppendingString: error_text];
		[log_text setString:new_log];
		if(!self.has_error) {
			[self.error_alert setMessageText:error_text];
			[self.error_alert beginSheetModalForWindow:self.window completionHandler:nil];
		}
		self.has_error = true;
	});
}

@end