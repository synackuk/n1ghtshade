/**
  * GreenPois0n Medicine - sachet.mm
  * Copyright (C) 2000 Chronic-Dev Team
  * Copyright (C) 2000 Dustin Howett
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#import <Foundation/Foundation.h>
#include <sys/sysctl.h>

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Usage:\n");
		printf("%s <path_to_appbundle_to_add>\n", argv[0]);
		return 0;
	}

	NSAutoreleasePool *p = [[NSAutoreleasePool alloc] init];

	NSString *appPath = [[NSString stringWithUTF8String:argv[1]] retain];
	NSBundle *appBundle = [NSBundle bundleWithPath:appPath];
	if(!appBundle) {
		printf("Failed to find app bundle: %s.\n", argv[1]);
		return 0;
	}

	NSString *miPath = @"/mnt2/mobile/Library/Caches/com.apple.mobile.installation.plist";
	NSMutableDictionary *miDict = [[NSMutableDictionary alloc] initWithContentsOfFile:miPath];
	NSMutableDictionary *system = [[miDict objectForKey:@"System"] mutableCopy];
	if(!system) {
		printf("Failed to find system key in installation plist\n");
		return 0;
	}

	NSMutableDictionary *added = [[appBundle infoDictionary] mutableCopy];
	[added setObject:@"System" forKey:@"ApplicationType"];
	[added setObject:appPath forKey:@"Path"];

	NSString *cfbi = [added objectForKey:@"CFBundleIdentifier"];
	if(!cfbi) {
		printf("Failed to find CFBundleIdentifier.\n");
		return 0;
	}

	[system setObject:added forKey:cfbi];
	[miDict setObject:system forKey:@"System"];

	[miDict writeToFile:miPath atomically:YES];
	chown([miPath UTF8String], 501, 501);

	[p drain];
	return 0;
}
