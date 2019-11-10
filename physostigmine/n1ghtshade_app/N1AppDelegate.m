#import "N1AppDelegate.h"
#import "N1RootViewController.h"

@implementation N1AppDelegate

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	_window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	_root_view_controller = [[UINavigationController alloc] initWithRootViewController:[[N1RootViewController alloc] initWithStyle:UITableViewStyleGrouped]];
	_window.rootViewController = _root_view_controller;
	[_window makeKeyAndVisible];
}

- (void)dealloc {
	[_window release];
	[_root_view_controller release];
	[super dealloc];
}

@end
