#import <Cocoa/Cocoa.h>


int log_callback(const char* fmt, ...) {
	va_list args;
	char* output_str;
	va_start(args, fmt);
	vasprintf(&output_str, fmt, args);
	va_end(args);
	[[NSNotificationCenter defaultCenter] postNotificationName: @"update_log" object:nil userInfo:@{@"message": [NSString stringWithUTF8String: output_str]}];
	free(output_str);
	return 0;
}

int progress_callback(double progress) {
	[[NSNotificationCenter defaultCenter] postNotificationName: @"update_progress" object:nil userInfo:@{@"progress": [NSNumber numberWithDouble:progress]}];
	return 0;
}

int error_callback(const char* fmt, ...) {
	va_list args;
	char* output_str;
	va_start(args, fmt);
	vasprintf(&output_str, fmt, args);
	va_end(args);
	[[NSNotificationCenter defaultCenter] postNotificationName: @"error" object:nil userInfo:@{@"error": [NSString stringWithUTF8String: output_str]}];
	free(output_str);
	return 0;
}
