#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOTypes.h>

static io_service_t get_service(const char *name, unsigned int retry) {
	io_service_t service;
	CFDictionaryRef match = IOServiceMatching(name);

	while (1) {
		CFRetain(match);
		service = IOServiceGetMatchingService(kIOMasterPortDefault, match);
		if (service || !retry--) {
			break;
		}
		printf("Didn't find %s, trying again\n", name);
		sleep(1);
	}

	CFRelease(match);
	return service;
}

int watchdog_init() {
	io_service_t service = get_service("IOWatchDogTimer", 0);
	if (service) {
		int zero = 0;
		CFNumberRef n = CFNumberCreate(NULL, kCFNumberSInt32Type, &zero);
		IORegistryEntrySetCFProperties(service, n);
		CFRelease(n);
		IOObjectRelease(service);
	}
	return 0;
}