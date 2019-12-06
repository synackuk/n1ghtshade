#include <libbelladonna.h>

#import <main_view.h>
#import <jailbreak_view.h>
#import <Cocoa/Cocoa.h>
#import <common.h>

static NSButton* jailbreak_button;
static NSButton* back_button;
static NSTextField* progress_label;
static NSProgressIndicator* progress_bar;

static void operation_in_progress(BOOL is_operation_in_progress) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"jb_operation_in_progress" object:nil userInfo:@{@"in_progress": [NSNumber numberWithBool:is_operation_in_progress]}];
}

static void message_callback(char* msg) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"jb_update_message" object:nil userInfo:@{@"message": [NSString stringWithUTF8String: msg]}];
}

static void progress_callback(unsigned int progress) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"jb_update_progress" object:nil userInfo:@{@"progress": [NSNumber numberWithInteger:progress]}];
}

@implementation JailbreakView

- (void) operation_in_progress:(id)sender {
	BOOL is_operation_in_progress = [[sender userInfo][@"in_progress"] boolValue];
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[jailbreak_button setEnabled: !is_operation_in_progress];
		[back_button setEnabled: !is_operation_in_progress];
		[progress_bar setIndeterminate:is_operation_in_progress];
		[progress_bar startAnimation:nil];
	});
}

- (void) update_message:(id)sender {
	NSString* progress_text = [sender userInfo][@"message"];
	NSLog(@"%@", progress_text);
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[progress_label setStringValue:progress_text];
	});
}

- (void) update_progress:(id)sender {
	unsigned int progress = [[sender userInfo][@"progress"] intValue];
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[progress_bar setIndeterminate: ((progress == 100) ? YES : NO)];
		[progress_bar setDoubleValue:(double)progress];
		[progress_bar startAnimation:nil];
	});
}

- (void) jailbreak_button {
	operation_in_progress(YES);
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
		int ret;
		libbelladonna_init();
		libbelladonna_set_log_cb(&message_callback);
		libbelladonna_set_prog_cb(&progress_callback);
		while(libbelladonna_get_device() != 0) {
			message_callback("Waiting for device in DFU mode\n");
			sleep(1);
		}
		ret = libbelladonna_compatible();
		if(ret != 0) {
			operation_in_progress(NO);
			libbelladonna_exit();
			message_callback("Device not compatible\n");
			return ;
		}
		ret = libbelladonna_exploit(); 
		if (ret != 0) {
			operation_in_progress(NO);
			libbelladonna_exit();
			message_callback("Failed to enter Pwned DFU mode\n");
			return ;
		}
		ret = libbelladonna_enter_recovery();
		if (ret != 0) {
			operation_in_progress(NO);
			libbelladonna_exit();
			message_callback("Failed to enter Pwned Recovery mode\n");
			return ;
		}
		ret = libbelladonna_boot_ramdisk();
		if (ret != 0) {
			operation_in_progress(NO);
			libbelladonna_exit();
			message_callback("Failed to boot jailbreak ramdisk\n");
			return ;
		}
		operation_in_progress(NO);
		message_callback("Done\n");
		libbelladonna_exit();
	});
}

- (void) back_button {
	[window setContentView:main_view];
}

@end

void init_jailbreak_view() {
	JailbreakView* jailbreak_view_cb = [[JailbreakView alloc] init];
	jailbreak_view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 350, 420)];
	jailbreak_button = create_button(@"Jailbreak", 95, 182, 160, 24, jailbreak_view_cb, @selector(jailbreak_button), jailbreak_view);
	back_button = create_button(@"Back", 95, 10, 160, 24, jailbreak_view_cb, @selector(back_button), jailbreak_view);
	progress_label = create_label(@"Press \"Jailbreak\" to begin the jailbreak", 25, 60, 300, 20, jailbreak_view);

	progress_bar = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(25, 40, 300, 20)];
	[progress_bar setIndeterminate:NO];
	[progress_bar setMinValue:0.0];
	[progress_bar setMaxValue:100.0];
	[jailbreak_view addSubview:progress_bar];

	[[NSNotificationCenter defaultCenter] addObserver:jailbreak_view_cb selector:@selector(operation_in_progress:) name:@"jb_operation_in_progress" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:jailbreak_view_cb selector:@selector(update_message:) name:@"jb_update_message" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:jailbreak_view_cb selector:@selector(update_progress:) name:@"jb_update_progress" object:nil];

}