/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  IOUSBDeviceControllerLib.h
 *  IOUSBDeviceFamily
 *
 *  Created by Paul Chinn on 11/6/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#ifndef _IOKIT_IOUSBDEVICECONTROLLERLIB_H_
#define _IOKIT_IOUSBDEVICECONTROLLERLIB_H_

#include <IOKit/IOTypes.h>
#include <IOKit/IOReturn.h>
#include <CoreFoundation/CoreFoundation.h>

/*!
 @header IOUSBDeviceControllerLib
 IOUSBDeviceControllerLib provides some API to access devicce-mode-usb controllers.
 */

__BEGIN_DECLS


/*! @typedef IOUSBDeviceControllerRef
 @abstract This is the type of a reference to the IOUSBDeviceController.
 */
typedef struct __IOUSBDeviceController* IOUSBDeviceControllerRef;

/*! @typedef IOUSBDeviceDescriptionRef
 @abstract Object that describes the device, configurations and interfaces of a IOUSBDeviceController.
 */
typedef struct __IOUSBDeviceDescription* IOUSBDeviceDescriptionRef;

/*! @typedef IOUSBDeviceArrivalCallback
 @abstract Function callback for notification of asynchronous arrival of an IOUSBDeviceController .
 */
typedef void (*IOUSBDeviceArrivalCallback) ( 
											void *                  context,
											IOUSBDeviceControllerRef    device);
/*!
 @function   IOUSBDeviceControllerGetTypeID
 @abstract   Returns the type identifier of all IOUSBDeviceController instances.
 */
CF_EXPORT
CFTypeID IOUSBDeviceControllerGetTypeID(void) ;

/*!
 @function   IOUSBDeviceDescriptionGetTypeID
 @abstract   Returns the type identifier of all IOUSBDeviceDescription instances.
 */
CF_EXPORT
CFTypeID IOUSBDeviceDescriptionGetTypeID(void) ;

/*!
 @function   IOUSBDeviceControllerCreate
 @abstract   Creates an IOUSBDeviceController object.
 @discussion Creates a CF object that provides access to the kernel's IOUSBDeviceController IOKit object.
 @param      allocator Allocator to be used during creation.
 @param      deviceRef The newly created object. Only valid if the call succeeds.
 @result     The status of the call. The call will fail if no IOUSBDeviceController exists in the kernel.
 */
CF_EXPORT 
IOReturn IOUSBDeviceControllerCreate(     
								   CFAllocatorRef                  allocator,
									IOUSBDeviceControllerRef* deviceRef
									);

/*!
 @function   IOUSBDeviceControllerGoOffAndOnBus
 @abstract   Cause the controller to drop off bus and return.
 @discussion The controller will drop off USB appearing to the host as if it has been unlugged. After the given msecDelay
 has elapsed, it will come back on bus.
 @param      deviceRef The controller object
 @param      msecDelay The time in milliseconds to stay off-bus.
 @result     The status of the call.
 */
CF_EXPORT
IOReturn IOUSBDeviceControllerGoOffAndOnBus(IOUSBDeviceControllerRef device, uint32_t msecDelay);

/*!
 @function   IOUSBDeviceControllerForceOffBus
 @abstract   Cause the controller to stay off.
 @discussion The controller will drop off USB appearing to the host as if it has been unlugged.
 @param      deviceRef The controller object
 @param      enable If true the controller is dropped off the bus and kept off. When false the controller will no longer be forced off.
 @result     The status of the call.
 */
CF_EXPORT
IOReturn IOUSBDeviceControllerForceOffBus(IOUSBDeviceControllerRef device, int enable);

/*! @function   IOUSBDeviceControllerRegisterArrivalCallback
 @abstract   Schedules async controller arrival with a run loop
 @discussion Establishs a callback to be invoked when an IOUSBDeviceController becomes available in-kernel.
 @param      callback The function invoked when the controller arrives. It receives a IOUSBDeviceControllerRef annd the caller-provided context. 
 @param      context A caller-specified pointer that is provided when the callback is invoked. 
 @param      runLoop RunLoop to be used when scheduling any asynchronous activity.
 @param      runLoopMode Run loop mode to be used when scheduling any asynchronous activity.
 */
CF_EXPORT
IOReturn IOUSBDeviceControllerRegisterArrivalCallback(IOUSBDeviceArrivalCallback callback, void *context, CFRunLoopRef runLoop, CFStringRef runLoopMode);

CF_EXPORT
void IOUSBDeviceControllerRemoveArrivalCallback();

/*! @function   IOUSBDeviceControllerSetDescription
 @abstract   Provide the information required to configure the IOUSBDeviceController in kernel
 @param      device The controller instance to receive the description
 @param      description The description to use.
 */
CF_EXPORT
IOReturn IOUSBDeviceControllerSetDescription(IOUSBDeviceControllerRef device, IOUSBDeviceDescriptionRef description);

/*! @function   IOUSBDeviceControllerSendCommand
 @abstract   Issue a command to the in-kernel usb-device stack
 @discussion This sends a command string and optional parameter object into the kernel. Commands are passed to the controller-driver, the
"device", then to the individual interface drivers, until one of those handles it.
 @param      device The controller instance to receive the command
 @param      command A string command. Valid commands are determined by the various in-kernel drivers comprising the usb-device stack
 @param		 param An optional, arbitrary object that is appropriate for the given command
 */
CF_EXPORT
IOReturn IOUSBDeviceControllerSendCommand(IOUSBDeviceControllerRef device, CFStringRef command, CFTypeRef param);

/*! @function   IOUSBDeviceControllerSetPreferredConfiguration
 @abstract   Sets the preferred configuration number to gain desired functionality on the host
 @param      device The controller instance to receive the description
 @param      config Preferred configuration number that will be sent to the host.
 */
CF_EXPORT
IOReturn IOUSBDeviceControllerSetPreferredConfiguration(IOUSBDeviceControllerRef device, int config);


CF_EXPORT
IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreate(CFAllocatorRef allocator);

/*! @function   IOUSBDeviceDescriptionCreateFromController
 @abstract   Retrieve the current description from the IOUSBDeviceController
 @discussion This retrieves the currently set description from the kernel's IOUSBDeviceController. It represents the full description of the device as
 it is currently presented on the USB. The call can fail if the controller exists but has not et received a description.
 @param		allocator	The CF allocator to use when creating the description
 @param      device The controller instance from which to receive the description
 */
CF_EXPORT
IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreateFromController(CFAllocatorRef allocator, IOUSBDeviceControllerRef);

/*! @function   IOUSBDeviceDescriptionCreateFromDefaults
 @abstract   Create a descripion based on the hardwares default usb description.
 @discussion This retrieves the default description for the device. It describes the main usb functionality provided by the device and is what is used for
 a normal system. Currently the description is retrieved from a plist on disk and is keyed to a sysctl that describes the hardware.
 @param		allocator	The CF allocator to use when creating the description
 */
CF_EXPORT
IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreateFromDefaults(CFAllocatorRef allocator);

CF_EXPORT
IOUSBDeviceDescriptionRef IOUSBDeviceDescriptionCreate(CFAllocatorRef allocator);

CF_EXPORT
uint8_t IOUSBDeviceDescriptionGetClass(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
void IOUSBDeviceDescriptionSetClass(IOUSBDeviceDescriptionRef ref, UInt8 bClass);

CF_EXPORT
void IOUSBDeviceDescriptionSetSubClass(IOUSBDeviceDescriptionRef devDesc, UInt8 bSubClass);

CF_EXPORT
uint8_t IOUSBDeviceDescriptionGetSubClass(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
void IOUSBDeviceDescriptionSetProtocol(IOUSBDeviceDescriptionRef devDesc, UInt8 protocol);

CF_EXPORT
uint8_t IOUSBDeviceDescriptionGetProtocol(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
uint16_t IOUSBDeviceDescriptionGetVendorID(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
void IOUSBDeviceDescriptionSetVendorID(IOUSBDeviceDescriptionRef devDesc, UInt16 vendorID);

CF_EXPORT
uint16_t IOUSBDeviceDescriptionGetProductID(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
void IOUSBDeviceDescriptionSetProductID(IOUSBDeviceDescriptionRef devDesc, UInt16 productID);

CF_EXPORT
uint16_t IOUSBDeviceDescriptionGetVersion(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
CFStringRef IOUSBDeviceDescriptionGetManufacturerString(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
CFStringRef IOUSBDeviceDescriptionGetProductString(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
CFStringRef IOUSBDeviceDescriptionGetSerialString(IOUSBDeviceDescriptionRef ref);

CF_EXPORT
void IOUSBDeviceDescriptionSetSerialString(IOUSBDeviceDescriptionRef ref, CFStringRef serial);

CF_EXPORT
int IOUSBDeviceDescriptionAppendInterfaceToConfiguration(IOUSBDeviceDescriptionRef devDesc, int config, CFStringRef name);;

CF_EXPORT
int IOUSBDeviceDescriptionAppendConfiguration(IOUSBDeviceDescriptionRef devDesc, CFStringRef textDescription, UInt8 attributes, UInt8 maxPower);;

CF_EXPORT
void IOUSBDeviceDescriptionRemoveAllConfigurations(IOUSBDeviceDescriptionRef devDesc);

CF_EXPORT
io_service_t IOUSBDeviceControllerGetService(IOUSBDeviceControllerRef controller);;

CF_EXPORT
int IOUSBDeviceDescriptionGetMatchingConfiguration(IOUSBDeviceDescriptionRef devDesc, CFArrayRef interfaceNames);;


/*! @function   IOUSBDeviceDescriptionCopyInterfaces
 @abstract   Return a an array of the interfaces on each configuration.
 @discussion This function returns an array of arrays of strings where each item in the top array corresponds to a single configuration and each string is the name of an interface in on the configuration.
 @param		devDesc	The USB device description to query
 */
CF_EXPORT
CFArrayRef IOUSBDeviceDescriptionCopyInterfaces(IOUSBDeviceDescriptionRef devDesc);

__END_DECLS

#endif