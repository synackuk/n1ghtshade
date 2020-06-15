#import <Cocoa/Cocoa.h>
#import <tasks_view.h>
#import <dfu_enter_view.h>
#import <common.h>

#include <libbelladonna.h>

static NSButton* start_button;

@implementation DFUEnterView

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
	[self.subviews makeObjectsPerformSelector: @selector(removeFromSuperview)];
	if(option == restore) {
		self.instructions_textbox = create_textbox(@"To begin, you must select the IPSW you wish to restore with the \"Restore IPSW\" button. Then you must ensure your device in DFU mode. To do this, press \"Start\" when you are ready and follow the instructions.", 10, 175, 460, 60, self);
	}
	else {
		self.instructions_textbox = create_textbox(@"To begin, you must ensure your device is in DFU mode. To do this, press \"Start\" when you are ready and follow the instructions.", 10, 175, 460, 60, self);
	}
	self.start_button = create_button(@"Start", 20, 150, 160, 28, @selector(start_btn), self);

	self.step_1_label = create_label(@"Hold power and home for 10 seconds.", 205, 144, 250, 28, self);
	self.step_2_label = create_label(@"Hold home for 10 seconds.", 205, 106, 250, 28, self);
	[self set_stage: 0 withString: NULL];

	self.back_button = create_button(@"Back", 300, 20, 160, 28, @selector(back_btn), self);

	if(option == restore) {
		self.ipsw_button = create_button(@"Select IPSW", 20, 110, 160, 28, @selector(ipsw_select_btn), self);
	}

}

- (void) ipsw_select_btn {
	input_ipsw = NULL;
	NSOpenPanel* ipsw_selector = [NSOpenPanel openPanel];
	[ipsw_selector setCanChooseFiles:YES];
	[ipsw_selector setAllowsMultipleSelection:NO];
	[ipsw_selector setCanChooseDirectories:NO];
	[ipsw_selector setTitle:@"Please select your IPSW"];
	if ([ipsw_selector runModal] == NSModalResponseOK) {
		input_ipsw = (char *)[[[[ipsw_selector URL] path] retain] UTF8String];
	}
	else {
		return;
	}
}

- (void) set_stage: (int)stage withString: (NSString*) new_string {
	dispatch_async(dispatch_get_main_queue(), ^(void){
		if(stage == 1) {
			self.step_1_label.textColor = [self.step_1_label.textColor colorWithAlphaComponent: 1];
			self.step_2_label.textColor = [self.step_2_label.textColor colorWithAlphaComponent: 0.5];
			[self.step_1_label setStringValue: new_string];
			[self.start_button setEnabled: NO];
			[self.back_button setEnabled: NO];
			if(option == restore) {
				[self.ipsw_button setEnabled: NO];
			}
		}
		else if(stage == 2) {
			self.step_1_label.textColor = [self.step_1_label.textColor colorWithAlphaComponent: 0.5];
			self.step_2_label.textColor = [self.step_2_label.textColor colorWithAlphaComponent: 1];
			[self.step_2_label setStringValue: new_string];
			[self.start_button setEnabled: NO];
			[self.back_button setEnabled: NO];
			if(option == restore) {
				[self.ipsw_button setEnabled: NO];
			}
		}
		else {
			self.step_1_label.textColor = [self.step_1_label.textColor colorWithAlphaComponent: 0.5];
			self.step_2_label.textColor = [self.step_2_label.textColor colorWithAlphaComponent: 0.5];
			[self.step_1_label setStringValue: @"Hold power and home for 10 seconds."];
			[self.step_2_label setStringValue: @"Hold home for 10 seconds."];

			[self.start_button setEnabled: YES];
			[self.back_button setEnabled: YES];
			if(option == restore) {
				[self.ipsw_button setEnabled: YES];
			}

			if(new_string) {
				[self.start_button setTitle: new_string];
			}
			else {
				[self.start_button setTitle: @"Start"];
			}
		}
	});
}

- (void) start_btn {
	if(option == restore && !input_ipsw) {
		[self.instructions_textbox setString: @"Select an IPSW first.\n"];
		return;
	}
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
		for(int i = 1; i <= 10; i += 1) {
			NSString* new_string = [NSString stringWithFormat:@"Hold power and home for %d seconds.", 10 - i];
			[self set_stage: 1 withString: new_string];
			int ret = belladonna_get_device();
			if(ret == 0) {
				swap_view(tasks_view);
				return;
			}
			sleep(1);
		}
		for(int i = 1; i <= 10; i += 1) {
			NSString* new_string = [NSString stringWithFormat:@"Hold home for %d seconds.", 10 - i];
			[self set_stage: 2 withString: new_string];
			int ret = belladonna_get_device();
			if(ret == 0) {
				swap_view(tasks_view);
				return;
			}
			sleep(1);
		}
		[self set_stage: 0 withString: @"Try again?"];

	});
}

- (void) back_btn {
	prev_view();
}

@end