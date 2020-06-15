#import <Cocoa/Cocoa.h>


void log_callback(char* msg) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"update_log" object:nil userInfo:@{@"message": [NSString stringWithUTF8String: msg]}];
}

void progress_callback(unsigned int progress) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"update_progress" object:nil userInfo:@{@"progress": [NSNumber numberWithInteger:progress]}];
}

void error_callback(char* error) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"error" object:nil userInfo:@{@"error": [NSString stringWithUTF8String: error]}];
}
