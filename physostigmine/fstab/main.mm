#import <Foundation/Foundation.h>

int main() {

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSString *fstab = [NSString stringWithContentsOfFile:@"/private/etc/fstab" encoding:NSUTF8StringEncoding error:nil];
	if(!fstab) {
		return -1;
	}
	fstab = [fstab stringByReplacingOccurrencesOfString:@"ro" withString:@"rw"];
	fstab = [fstab stringByReplacingOccurrencesOfString:@",nosuid,nodev" withString:@""];
	[fstab writeToFile:@"/private/etc/fstab" atomically:YES encoding:NSWindowsCP1250StringEncoding error:nil];

	[pool drain]; 
	return 0;
}
