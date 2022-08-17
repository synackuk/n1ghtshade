#import <Foundation/Foundation.h>

int main() {

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];


	NSFileManager* file_manager = [NSFileManager defaultManager];

	/* First we wipe out the software update launch daemons to break updates */

	[file_manager removeItemAtPath:@"/System/Library/LaunchDaemons/com.apple.mobile.softwareupdated.plist" error:NULL];
	[file_manager removeItemAtPath:@"/System/Library/LaunchDaemons/com.apple.softwareupdateservicesd.plist" error:NULL];
	[file_manager removeItemAtPath:@"/System/Library/PrivateFrameworks/MobileSoftwareUpdate.framework/Versions/A/Resources/softwareupdated" error:NULL];
	[file_manager removeItemAtPath:@"/System/Library/PrivateFrameworks/SoftwareUpdateServices.framework/Support/softwareupdateservicesd" error:NULL];
	[file_manager removeItemAtPath:@"/System/Library/CoreServices/OTACrashCopier" error:NULL];
	[file_manager removeItemAtPath:@"/usr/libexec/OTATaskingAgent" error:NULL];
	
	/* Next we remove the software update option from System Preferences */

	NSMutableDictionary *general_plist = [[NSMutableDictionary alloc] initWithContentsOfFile:@"/Applications/Preferences.app/General.plist"];
	if(!general_plist) {
		return -1;
	}
	NSMutableArray* general_items = general_plist[@"items"];
		for(int i = 0; i < [general_items count]; i++) {
			NSDictionary* general_item = general_items[i];
			if([general_item[@"label"]  isEqual: @"SOFTWARE_UPDATE"]) {
				[general_items removeObjectAtIndex:i];
				break;
			}
		}
	general_plist[@"items"] = general_items;
	[general_plist writeToFile:@"/Applications/Preferences.app/General.plist" atomically:YES];

	[pool drain];
	return 0;
}