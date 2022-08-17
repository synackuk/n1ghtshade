/* It is recommended to use dlopen & dlsym to call these functions, and use this header as a reference.
Example:
void *IOMobileFramebuffer = dlopen("/System/Library/PrivateFrameworks/IOMobileFramebuffer.framework/IOMobileFramebuffer", RTLD_LAZY);
IOMobileFramebufferReturn (*IOMobileFramebufferGetMainDisplay)(IOMobileFramebufferRef *pointer) = dlsym(IOMobileFramebuffer, "IOMobileFramebufferGetMainDisplay");
dlclose(IOMobileFramebuffer); */

/* You may have to include your IOSurface header to compile, because of the IOMobileFramebufferGetLayerDefaultSurface function. If you do not have it, you may just uncomment the typedef to an IOSurface below. */
#include <stdio.h>
#include <sys/mman.h>
#include <mach/mach.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kIOMobileFramebufferError 0xE0000000

/* If you do not have IOKit headers, you must uncomment the next following commented lines: */
/* typedef kern_return_t IOReturn;
typedef mach_port_t io_service_t; */
typedef io_service_t IOMobileFramebufferService;
typedef IOReturn IOMobileFramebufferReturn;
typedef struct __IOMobileFramebuffer *IOMobileFramebufferRef;
typedef CGSize IOMobileFramebufferDisplaySize;
typedef CGFloat IOMobileFramebufferDisplayArea;
typedef CFTypeID IOMobileFramebufferID;
typedef int IOMobileFramebufferContrastLevel;
typedef int IOMobileFramebufferDotPitch;
/* typedef struct __IOSurface *IOSurfaceRef; */

/* Options for the "IOMobileFramebufferOpenByName" function. */
typedef CFStringRef IOMobileFramebufferDisplayType;
static const IOMobileFramebufferDisplayType IOMobileFramebufferDisplayTypePrimary = CFSTR("primary");
static const IOMobileFramebufferDisplayType IOMobileFramebufferDisplayTypeExternal = CFSTR("external");
static const IOMobileFramebufferDisplayType IOMobileFramebufferDisplayTypeWireless = CFSTR("wireless");

typedef struct {
    uint32_t values[0xc0c / sizeof(uint32_t)];
} IOMobileFramebufferGammaTable;

typedef long s1516;
typedef struct {
    union {
        s1516 values[9];
    } content;
} IOMobileFramebufferGamutMatrix;

typedef NS_ENUM(int, IOMobileFramebufferColorRemapMode) {
    IOMobileFramebufferColorRemapModeError = -1,
    IOMobileFramebufferColorRemapModeNormal = 0,
    IOMobileFramebufferColorRemapModeInverted = 1,
    IOMobileFramebufferColorRemapModeGrayscale = 2,
    IOMobileFramebufferColorRemapModeGrayscaleIncreaseContrast = 3,
    IOMobileFramebufferColorRemapModeInvertedGrayscale = 4
};

typedef NS_ENUM(uint32_t, IOMobileFramebufferBrightnessCorrection) {
    IOMobileFramebufferBrightnessCorrectionReducedWhitepoint = 57344,
    IOMobileFramebufferBrightnessCorrectionDefault = 65535
};
    
typedef NS_ENUM(BOOL, IOMobileFramebufferWhiteOnBlackMode) {
    IOMobileFramebufferWhiteOnBlackModeEnabled = YES,
    IOMobileFramebufferWhiteOnBlackModeDisabled = NO
};

typedef NS_ENUM(BOOL, IOMobileFramebufferVideoPowerSavingsMode) {
    IOMobileFramebufferVideoPowerSavingsModeEnabled = YES,
    IOMobileFramebufferVideoPowerSavingsModeDisabled = NO
};
    
/*!
@function IOMobileFramebufferOpen
@abstract Works the same way as IOServiceOpen
@param service - the IOMobileFramebufferService from IOServiceGetMatchingService
@param owningTask - use mach_task_self_ or mach_task_self()
@param type - unknown, use 0
@param pointer - a new IOMobileFramebufferRef pointer
@result An IOMobileFramebufferReturn code
*/
    
IOMobileFramebufferReturn IOMobileFramebufferOpen(IOMobileFramebufferService service, task_port_t owningTask, unsigned int type, IOMobileFramebufferRef *pointer);

/*!
@function IOMobileFramebufferOpenByName
@abstract Opens the framebuffer service by specifying the display type
@param name - the display type/name
@param pointer - a new IOMobileFramebufferRef pointer
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferOpenByName(IOMobileFramebufferDisplayType name, IOMobileFramebufferRef *pointer);

/*!
@function IOMobileFramebufferGetLayerDefaultSurface
@abstract Gets an IOSurfaceRef from the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param surface - the ID of the surface
@param buffer - a new IOSurface pointer
@result An IOMobileFramebufferReturn code
*/
    
IOMobileFramebufferReturn IOMobileFramebufferGetLayerDefaultSurface(IOMobileFramebufferRef pointer, int surface, IOSurfaceRef *buffer);
    
/*!
@function IOMobileFramebufferGetMainDisplay
@abstract Gets a new and valid pointer for an IOMobileFramebufferRef
@param pointer - a new IOMobileFramebufferRef pointer
@result An IOMobileFramebufferReturn code
*/
    
IOMobileFramebufferReturn IOMobileFramebufferGetMainDisplay(IOMobileFramebufferRef *pointer);

/* IOMobileFramebufferSwap functions */
IOMobileFramebufferReturn IOMobileFramebufferSwapBegin(IOMobileFramebufferRef pointer, int *token);
IOMobileFramebufferReturn IOMobileFramebufferSwapEnd(IOMobileFramebufferRef pointer);
IOMobileFramebufferReturn IOMobileFramebufferSwapSetLayer(IOMobileFramebufferRef pointer, int layerid, IOSurfaceRef buffer);
IOMobileFramebufferReturn IOMobileFramebufferSwapWait(IOMobileFramebufferRef pointer, int token, int something);

/*!
@function IOMobileFramebufferGetDisplayArea
@abstract Gets the area of the display from service opened using IOMobileFramebufferOpen or IOMobileFramebufferGetMainDisplay
@param pointer - a valid IOMobileFramebufferRef
@param displayArea - a new IOMobileFramebufferDisplayArea pointer
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferGetDisplayArea(IOMobileFramebufferRef pointer, IOMobileFramebufferDisplayArea *displayArea);
    
/*!
@function IOMobileFramebufferGetDisplaySize
@abstract Creates a CGSize pointer with the screen resolution in pixels
@param pointer - a valid IOMobileFramebufferRef
@param size - a new IOMobileFramebufferDisplaySize pointer
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferGetDisplaySize(IOMobileFramebufferRef pointer, IOMobileFramebufferDisplaySize *size);
    
/*!
@function IOMobileFramebufferGetGammaTable
@abstract Gets the gamma table data of the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param table - a pointer to a valid IOMobileFramebufferGammaTable
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferGetGammaTable(IOMobileFramebufferRef pointer, IOMobileFramebufferGammaTable *table);
    
/*!
@function IOMobileFramebufferSetGammaTable
@abstract Sets the gamma table data to the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param table - a pointer to a new IOMobileFramebufferGammaTable
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferSetGammaTable(IOMobileFramebufferRef pointer, IOMobileFramebufferGammaTable *table);
    
/*!
@function IOMobileFramebufferSetContrast
@abstract Sets the contrast of the framebuffer (function seems broken, but a reboot will fix any issues)
@param pointer - a valid IOMobileFramebufferRef
@param level - an integer containing the contrast level
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferSetContrast(IOMobileFramebufferRef pointer, IOMobileFramebufferContrastLevel level);

/*!
@function IOMobileFramebufferGetColorRemapMode
@abstract Retrieves the current color remap setting of the display
@param pointer - a valid IOMobileFramebufferRef
@param mode - the current IOMobileFramebufferColorRemapMode, use IOMobileFramebufferColorRemapModeNormal as default
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferGetColorRemapMode(IOMobileFramebufferRef pointer, IOMobileFramebufferColorRemapMode *mode);

/*!
@function IOMobileFramebufferSetColorRemapMode
@abstract Sets the desired IOMobileFramebufferColorRemapMode
@param pointer - a valid IOMobileFramebufferRef
@param mode - the desired IOMobileFramebufferColorRemapMode to be set
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferSetColorRemapMode(IOMobileFramebufferRef pointer, IOMobileFramebufferColorRemapMode mode);

/*!
@function IOMobileFramebufferSetWhiteOnBlackMode
@abstract Does the same thing as "Invert Colors" in accessability options
@param pointer - a valid IOMobileFramebufferRef
@param enabled - the BOOL that determines if white on black mode should be enabled or not
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferSetWhiteOnBlackMode(IOMobileFramebufferRef pointer, IOMobileFramebufferWhiteOnBlackMode enabled);
    
/*!
@function IOMobileFramebufferSetBrightnessCorrection
@abstract Does the same thing as "Reduce White Point" in accessability options, but with a customizable white point value if wanted
@param pointer - a valid IOMobileFramebufferRef
@param value - the IOMobileFramebufferBrightnessCorrectionValue to determine the new white point
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferSetBrightnessCorrection(IOMobileFramebufferRef pointer, IOMobileFramebufferBrightnessCorrection value);
    
/*!
@function IOMobileFramebufferGetMatrix
@abstract Gets the current gamma matrix of the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param matrix - a pointer to the gamut matrix you would like to retrieve
@result An IOMobileFramebufferReturn code
*/

/* If iOS version >= 8.3 & < 9.0, the name of the function is "IOMobileFramebufferGetGamutCSC". With versions >= 9.0 & < 9.3, it is "IOMobileFramebufferGetGamutMatrix". Versions >= 9.3 are below. The function does not exist at all on versions < 8.3 */
IOMobileFramebufferReturn IOMobileFramebufferGetMatrix(IOMobileFramebufferRef pointer, IOMobileFramebufferGamutMatrix *matrix);

/*!
@function IOMobileFramebufferSetMatrix
@abstract Sets a custom gamma matrix to the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param matrix - a pointer to the new gamut matrix you would like to set
@result An IOMobileFramebufferReturn code
*/

/* If iOS version >= 8.3 & < 9.0, the name of the function is "IOMobileFramebufferSetGamutCSC". With versions >= 9.0 & < 9.3, it is "IOMobileFramebufferSetGamutMatrix". Versions >= 9.3 are below. The function does not exist at all on versions < 8.3 */
IOMobileFramebufferReturn IOMobileFramebufferSetMatrix(IOMobileFramebufferRef pointer, IOMobileFramebufferGamutMatrix *matrix);

/*!
@function IOMobileFramebufferGetID
@abstract Gets the identifier of the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param outID - a new IOMobileFramebufferID pointer
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferGetID(IOMobileFramebufferRef pointer, IOMobileFramebufferID *outID);

/*!
@function IOMobileFramebufferGetDotPitch
@abstract Gets the number of pixels per inch of the display
@param pointer - a valid IOMobileFramebufferRef
@param dotPitch - a new IOMobileFramebufferDotPitch pointer
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferGetDotPitch(IOMobileFramebufferRef pointer, IOMobileFramebufferDotPitch *dotPitch);

/*!
@function IOMobileFramebufferIsMainDisplay
@abstract Determines whether the IOMobileFramebuffer service is the main display
@param pointer - a valid IOMobileFramebufferRef
@param isMainDisplay - a BOOL that is YES or NO depending on whether the IOMobileFramebuffer service is the main display
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferIsMainDisplay(IOMobileFramebufferRef pointer, BOOL *isMainDisplay);

/*!
@function IOMobileFramebufferEnableDisableVideoPowerSavings
@abstract Enables or disables the power saving mode for the framebuffer
@param pointer - a valid IOMobileFramebufferRef
@param enabled - a IOMobileFramebufferVideoPowerSavingsMode pointer that sets video power savings on or off
@result An IOMobileFramebufferReturn code
*/

IOMobileFramebufferReturn IOMobileFramebufferEnableDisableVideoPowerSavings(IOMobileFramebufferRef pointer, IOMobileFramebufferVideoPowerSavingsMode enabled);

#ifdef __cplusplus
}
#endif