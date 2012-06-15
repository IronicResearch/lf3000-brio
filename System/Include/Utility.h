#ifndef LF_BRIO_UTILITY_H
#define LF_BRIO_UTILITY_H
//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Utility.h
//
// Description:
//		Utility functions.
//
//============================================================================
#include <stdio.h>
#include <vector>	
#include <string>	
#include <fstream> 
#include <fcntl.h>
#include <math.h>
#include <SystemTypes.h>
#include <CoreTypes.h>	
#include <PowerTypes.h>
#include <USBDeviceTypes.h>
#include <ButtonTypes.h>
#include <CartridgeTypes.h>
#include <StringTypes.h>
#include <boost/shared_array.hpp>	

#include "Wrappers.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif 

using namespace std;
LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Constants
//============================================================================
const string kDirSeparatorString("\\");	

typedef vector<U32>  PointerOffsets;

typedef struct tagAppRsrcDataSet {
		tVersion	fmtVersion;	
		tVersion 	cntVersion;	
		U32 		id;
		void* 	pDataSet;
}tAppRsrcDataSet;

/* app monitor communication */

/* socket that monitord listens on */
#define MONITOR_SOCK    "/tmp/monitoring_socket"
/* socket on which monitord reports USB events */
#define USB_SOCK        "/tmp/usb_events_socket"
/* socketn on which monitord reports Power events */
#define POWER_SOCK      "/tmp/power_events_socket"

#define APP_MSG_GET_USB         0
#define APP_MSG_SET_USB         1
#define APP_MSG_GET_POWER       2
#define APP_MSG_SET_POWER       3


struct app_message {
	unsigned int type;
	unsigned int payload;
} __attribute__((__packed__));

/****************************************************************************
 * 																			*
 * Dataset loader functions.												*
 * 		StripTrailing() -remove whitespace when their second parameter      *
 * 						 is defaulted, but can also be used to remove 		*
 * 						 leading and trailing quote characters, etc.		*
 * 		FileSize(a)		-returns size of a.                         		*
 * 		OffsetsToPtrs()	-convert the offsets into pointers for a given   	*
 * 						 pointer address.									*
 * 		LoadDataset()	-loads the dataset into RAM, fixes up any pointers	*
 * 						 and returns a pointer to the dataset.	            *
 * **************************************************************************/
//----------------------------------------------------------------------------
string StripTrailing( const string& in, const string& remove);

//----------------------------------------------------------------------------
size_t FileSize( const string& file );

//----------------------------------------------------------------------------
string GetFileExtension(const string& file);

//----------------------------------------------------------------------------
void OffsetsToPtrs( U8* pData, const PointerOffsets& ptr_offsets );

//----------------------------------------------------------------------------
//tAppRsrcDataSet* 
boost::shared_array<U8> LoadDataset(const string& binPath, const string& relinkPath);


//----------------------------------------------------------------------------
// Returns free memory snapshot in user space (in Kb)
//----------------------------------------------------------------------------
int GetFreeMem(void);

//----------------------------------------------------------------------------
// Returns file name descriptor for the keyboard device
//----------------------------------------------------------------------------
char *GetKeyboardName(void);

//----------------------------------------------------------------------------
// Returns a listening socket at the specified path
//----------------------------------------------------------------------------
int CreateListeningSocket(const char *path);

int CreateReportSocket(const char *path);

//----------------------------------------------------------------------------
// Returns the system's power state
//----------------------------------------------------------------------------
enum tPowerState GetCurrentPowerState(void);

//----------------------------------------------------------------------------
// Returns the system's USB device state
//----------------------------------------------------------------------------
tUSBDeviceData GetCurrentUSBDeviceState(void);
void SetCachedUSBDeviceState(tUSBDeviceData);

//----------------------------------------------------------------------------
// Returns the system's button state
//----------------------------------------------------------------------------
tButtonData		GetButtonState(void);
tButtonData2	GetButtonState2(void);

//----------------------------------------------------------------------------
// Sets the system's button state
//----------------------------------------------------------------------------
void SetButtonState(tButtonData2 button_data);


//----------------------------------------------------------------------------
// Returns the system's cartridge state
//----------------------------------------------------------------------------
tCartridgeData		GetCartridgeState(void);

//----------------------------------------------------------------------------
// Sets the system's cartridge state
//----------------------------------------------------------------------------
void SetCartridgeState(tCartridgeData cartridge_data);

//----------------------------------------------------------------------------
// Find device input event nodes
//----------------------------------------------------------------------------
int find_input_device(const char *input_name, char *dev);
int open_input_device(const char *input_name);

//----------------------------------------------------------------------------
// Prints uptime to console in uniform manner
//----------------------------------------------------------------------------
void PrintUptime(const char *tag);

/****************************************************************************
 * EnumFolder - Enumerates (non-recursively) files and dirs under a certain
 * folder path. Eliminates '.' and '..' from enumerations. Does not eliminate
 * hidden files/folders or files starting w/ '.' (eg: '.svn/').
 * Returns false if folder does not exist.
 * dirIn - the path of the folder to enumerate
 * f - the callback function which gets called for each new item enumerated.
 *     If return from f is false, EnumFolder stops enumerating. 'path' returned
 *     does not include a final '/' (if folder).
 * type - if kFoldersOnly, kFilesOnly, or kFilesAndFolders
 * userData - User data.
 ****************************************************************************/
typedef Boolean (*tFuncEnumFolder)(const CPath& path, void* userData);

enum tFileSelect {
	kFoldersOnly = 0,
	kFilesOnly,
	kFilesAndFolders,
	kFoldersNone
};

Boolean EnumFolder( const CPath& dirIn, tFuncEnumFolder f, tFileSelect type, void* userData);

//----------------------------------------------------------------------------
// Returns the USB device id of the gadget at a given sysfs path,
// e.g., "/sys/class/usb_device/usbdev1.2"
//----------------------------------------------------------------------------
U32 FindDevice(CPath path);

//----------------------------------------------------------------------------
/// <B>For use with Firmware 2.x or later only.</B>
/// \brief Returns the system platform ID
/// \return ID number for platform variants.
/// Use \ref GetPlatformName() or \ref GetPlatformFamily() instead.
//----------------------------------------------------------------------------
U32 GetPlatformID();

//----------------------------------------------------------------------------
/// <B>For use with Firmware 2.x or later only.</B>
/// \brief Returns the system platform name by string.
/// \return String with platform name ("Emerald", "Madrid", "LUCY", "VALENCIA").
//----------------------------------------------------------------------------
CString GetPlatformName();

//----------------------------------------------------------------------------
/// <B>For use with Firmware 3.x or later only.</B>
/// \brief Returns the system platform family by string.
/// \return String with platform family ("LEX", "LPAD").
//----------------------------------------------------------------------------
CString GetPlatformFamily();

//----------------------------------------------------------------------------
/// \brief Enumerated type for \ref HasPlatformCapability().
//----------------------------------------------------------------------------
enum tPlatformCaps {
	kCapsTouchscreen,				///< Has Touchscreen device?
	kCapsCamera,					///< Has Camera device?
	kCapsAccelerometer,				///< Has Accelerometer device?
	kCapsMicrophone,				///< Has Microphone device?
	kCapsScreenLEX,					///< Has LEX screen?
	kCapsScreenLPAD,				///< Has LPAD screen?
	kCapsLF1000,					///< Has LF1000 CPU?
	kCapsLF2000,					///< Has LF2000 CPU?
	kCapsWifi,						///< Has WiFi device?
	kCapsCameraFront,				///< Has Front-Facing Camera device?
	kCapsReserved1 = 0x10000000,
	kCapsReserved2 = 0x20000000,
	kCapsButtonSet = 0x40000000,	///< Has Button in kCapsButtonMask() set?
	kCapsButtonUp				= 0x40000001,
	kCapsButtonDown				= 0x40000002,
	kCapsButtonRight			= 0x40000004,
	kCapsButtonLeft				= 0x40000008,
	kCapsButtonA				= 0x40000010,
	kCapsButtonB				= 0x40000020,
	kCapsButtonLeftShoulder		= 0x40000040,
	kCapsButtonRightShoulder	= 0x40000080,
	kCapsButtonMenu				= 0x40000100,
	kCapsButtonHint				= 0x40000200,
	kCapsButtonPause			= 0x40000400,
	kCapsButtonBrightness		= 0x40000800,
	kCapsHeadphoneJackDetect  	= 0x40001000,
	kCapsCartridgeDetect      	= 0x40002000,
	kCapsButtonVolumeDown		= 0x40004000,
	kCapsButtonVolumeUp			= 0x40008000,
	kCapsButtonEscape			= 0x40010000,
};

//----------------------------------------------------------------------------
/// \brief Macro for Button capability using kButton* type mask.
/// For example, \ref HasPlatformCapability(kCapsButtonMask(kButtonPause)).
//----------------------------------------------------------------------------
#define kCapsButtonMask(x)		((enum tPlatformCaps)((U32)kCapsButtonSet | x))

//----------------------------------------------------------------------------
/// <B>For use with Firmware 2.x or later only.</B>
/// \brief Returns the supported platform capability.
/// \param caps Enumerated platform capability type.
/// \return True if capability supported on platform, or False otherwise.
//----------------------------------------------------------------------------
bool HasPlatformCapability(tPlatformCaps caps);

tDpadOrientation	GetDpadOrientationState();
tErrType			SetDpadOrientationState(tDpadOrientation dpad_orientation);

LF_END_BRIO_NAMESPACE()

#endif // LF_BRIO_UTILITY_H
// EOF
