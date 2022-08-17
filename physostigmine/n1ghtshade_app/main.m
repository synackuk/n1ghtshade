#import "N1AppDelegate.h"

int main(int argc, char *argv[]) {
	setuid(0);
	setgid(0);
	@autoreleasepool {
		return UIApplicationMain(argc, argv, nil, NSStringFromClass(N1AppDelegate.class));
	}
}
