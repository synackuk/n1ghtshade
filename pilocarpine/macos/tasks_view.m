#import <main_view.h>
#import <tasks_view.h>
#import <Cocoa/Cocoa.h>
#import <common.h>
#import <dfu_enter_view.h>


#include <dimensions.h>
#include <callback.h>
#include <libbelladonna.h>

int hacktivate = 0;
char* input_ipsw = NULL;
char* boot_args = NULL;

options_t option;

@implementation TasksView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	self.log_scrollbox = create_scrollbox(@"", PADDING, 110, PAD_W(PADDING), PAD_H(110), self);
	self.back_button = create_button(@"Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, @selector(back_btn), self);
	self.progress_label = create_label(@"", PADDING, 60, TEXT_WIDTH, TEXT_HEIGHT, self);
	self.progress_label.alignment = NSTextAlignmentCenter;
	self.progress_bar = create_progress_bar(PADDING, 80, PAD_W(PADDING), PROGRESS_BAR_HEIGHT, self);
	self.error_alert = [[NSAlert alloc] init];
	self.has_error = false;
}

- (void)viewDidMoveToWindow {
	[self.back_button setEnabled: NO];

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
		int ret;
		dispatch_async(dispatch_get_main_queue(), ^(void){
			[self.progress_label setStringValue:@"Entering Pwned DFU"];
		});
		ret = belladonna_enter_pwned_dfu(ctx);
		if(ret != 0) {
			belladonna_close_device(ctx);
			dispatch_async(dispatch_get_main_queue(), ^(void){
				[self.back_button setEnabled: YES];
			});
			return;
		}
		if(option == restore) {
			dispatch_async(dispatch_get_main_queue(), ^(void){
				[self.progress_label setStringValue:@"Restoring Device"];
			});
			ret = belladonna_restore(ctx, input_ipsw);
		}
		else if(option == jailbreak) {
			dispatch_async(dispatch_get_main_queue(), ^(void){
				[self.progress_label setStringValue:@"Jailbreaking"];
			});
			ret = belladonna_boot_hyoscine(ctx);
			if(ret != 0) {
				dispatch_async(dispatch_get_main_queue(), ^(void){
					[self.back_button setEnabled: YES];
				});
				return;
			}
			ret = belladonna_jailbreak(ctx, hacktivate);
		}
		else if(option == boot_tethered) {
			dispatch_async(dispatch_get_main_queue(), ^(void){
				[self.progress_label setStringValue:@"Booting Tethered"];
			});
			ret = belladonna_tethered_boot(ctx, boot_args);
		}
		dispatch_async(dispatch_get_main_queue(), ^(void){
			[self.back_button setEnabled: YES];
		});
		if(ret == 0) {
			dispatch_async(dispatch_get_main_queue(), ^(void){
				[self.progress_label setStringValue:@"Done"];
			});

		}
		belladonna_close_device(ctx);
		progress_callback(0);
	});
	
}

- (void) back_btn {
	prev_view();
}

- (void) update_log:(id)sender {
	NSString* progress_text = [sender userInfo][@"message"];
	NSLog(@"%@", progress_text);
	dispatch_async(dispatch_get_main_queue(), ^(void){
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