#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOTypes.h>

#include <IOUSBDeviceControllerLib.h>

IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreateWithType(CFAllocatorRef, CFStringRef);

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

/* reversed from restored_external */
int usb_init(void)
{
	int i;
	CFNumberRef n;
	io_service_t service;
	CFMutableDictionaryRef dict;
	IOUSBDeviceDescriptionRef desc;
	IOUSBDeviceControllerRef controller;

	desc = IOUSBDeviceDescriptionCreateWithType(kCFAllocatorDefault, CFSTR("standardMuxOnly")); /* standardRestore */
	if (!desc) {
		return -1;
	}
	IOUSBDeviceDescriptionSetSerialString(desc, CFSTR("hyoscine jailbreakd"));

	controller = 0;
	while (IOUSBDeviceControllerCreate(kCFAllocatorDefault, &controller)) {
		printf("Unable to get USB device controller\n");
		sleep(3);
	}
	IOUSBDeviceControllerSetDescription(controller, desc);

	CFRelease(desc);
	CFRelease(controller);

	service = get_service("AppleUSBDeviceMux", 10);
	if (!service) {
		return -1;
	}

	dict = CFDictionaryCreateMutable(NULL, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	i = 7;
	n = CFNumberCreate(NULL, kCFNumberIntType, &i);
	CFDictionarySetValue(dict, CFSTR("DebugLevel"), n);
	CFRelease(n);

	i = IORegistryEntrySetCFProperties(service, dict);
	CFRelease(dict);
	IOObjectRelease(service);

	return i;
}
