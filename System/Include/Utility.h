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

//----------------------------------------------------------------------------
// Returns the system's button state
//----------------------------------------------------------------------------
tButtonData		GetButtonState(void);
tButtonData2	GetButtonState2(void);

//----------------------------------------------------------------------------
// Sets the system's button state
//----------------------------------------------------------------------------
void SetButtonState(tButtonData2 button_data);

LF_END_BRIO_NAMESPACE()

#endif // LF_BRIO_UTILITY_H
// EOF
