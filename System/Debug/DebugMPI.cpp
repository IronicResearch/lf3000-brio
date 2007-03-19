//==============================================================================
// $Source: X:/SW/Concord/System/Debug/rcs/DebugMPI.cpp $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DebugMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the System Debug 
//		module.
//
//==============================================================================

//==============================================================================

// std includes

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>		// for vprintfs

// system includes
#include <SystemTypes.h>
#include <DebugMPI.h>

// Module includes
#include <DebugPrivate.h>

//==============================================================================
// Function Prototypes
//==============================================================================	
static void __assert(const char* formatString, va_list arguments);

//==============================================================================
// Module Names & Versioning
//
//	Version history:
//		v1.3 	- task creation & assert priority change use GetTaskPropertiesPreferences
//				- added Enable/DisableDebugOutTimestamp
//==============================================================================

//==============================================================================
// MPI Name & Version
//
//		This is the name & version of the MPI as defined in this header.
//		! NOTE: this may not match the MPI name & version implemented by the
//				installed module.  Always use the MPI's core functions to
//				resolve which version of the MPI is installed & available 
//				at runtime.
//==============================================================================
// info on implementation of this module
const CString   kDebugMPIName = "DebugMPI";
const tVersion  kDebugMPIVersion = MakeVersion(1,0);

const CString   kDebugModuleName = "DebugModule";
const CURI		kDebugModuleOriginURI = "/somewhere/module";
const tVersion  kDebugModuleVersion = MakeVersion(1,0);


// ROM the task name
//static const char kDebugTaskNameStr[] = kDebugTaskName;

//==============================================================================
// Defines
//==============================================================================

const char kAssertTagStr[] 				= "\n!ASSERT: ";
const char kWarnTagStr[] 				= "!WARNING: ";
const char kSavedAssertTagStr[] 		= "\n!SAVED ASSERT: ";
const char kDebugOutSignatureFmt[] 		= "[%d] ";
const char kDebugOutTimestampFmt[]		= "<@%d.%03dms> ";


//------------------------------------------------------------------------------
// tDebugEnvironment
//		The form of	Debug command task global environment data. This struct 
//		should be modified whenever it is desired to change the contents of 
//		the task's global data.
//------------------------------------------------------------------------------
tDebugEnvironment *gDebugEnvPtr = NULL;

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
Boolean CheckDebugOutIsEnabled(tDebugSignature sig, tDebugLevel lvl)
{
	// want to exit this as soon as possible for performance reasons so
	// n't take hit when debug switch off on shipping product, so we
	//  the checks in a very specific order...

	// If we're silenced...
	// 		...or level is > master level
	// 		...or signature out of range
	// 		...or output disabled for signature
	// 		then debug not enabled.
	// Else debug enabled for this signature.
	return ((lvl == kDbgLvlSilent) || 
			(lvl > gDebugEnvPtr->masterDebugLevel) ||
			 sig > kMaxDebugSig ||
			 !(gDebugEnvPtr->sigDbgBitVecArray[(sig >> 3)] & (1 << (sig & 0x7)))) ? false : true;
}
	
	
			  
//==============================================================================
// Function:
//		Init
//
// Parameters:
//		NA
//
// Returns:
//		Boolean - true on success or false otherwise.  
//
// Description:
//		Initializes the CDebugMPI Task. Allocates and initializes memory for the 
//		CDebugMPI environment. Creates and launches the CDebugMPI command task.
//		Registers several commands to allow for run-time printf control.  
//==============================================================================
CDebugMPI::CDebugMPI(void)
{
	U32 i;

	if (gDebugEnvPtr == NULL)
	{
		if((gDebugEnvPtr = (tDebugEnvironment*)malloc(sizeof(tDebugEnvironment))) == NULL)	 
		{
			const char formatString[] = "DebugMPI::Init alloc failed\n";
			printf(formatString);
		}

		// initialize the sigDbgBitVecArray so that DebugOut for all modules is 
		// turned on by default.
		for(i = 0 ; i < kSigBitVecArraySize ; i++)
			gDebugEnvPtr->sigDbgBitVecArray[i] = 0xff;

	}
}

//==============================================================================
// Function:
//		DeInit
//
// Parameters:
//		NA
//
// Returns:
//		void
//
// Description:
//		Unes the initialization performed by Init().    
//==============================================================================
CDebugMPI::~CDebugMPI(void)
{
	if(gDebugEnvPtr != NULL)
	{

//		CKernelMPI::Free(gDebugEnvPtr);
		free(gDebugEnvPtr);

		gDebugEnvPtr = NULL;
	}
}

//==============================================================================
// Function:
//		IsInited()
//
// Parameters:
//		none	
//
// Returns:
//		Boolean
//
// Description:
//		Returns true if the module has already been inited.
//==============================================================================

Boolean CDebugMPI::IsValid() const
{
	return (gDebugEnvPtr != NULL) ? true : false; 
}

//==============================================================================
// Function:
//		GetMPIVersion()
//		GetMPICoreVersion()
//		GetModuleVersion()
//		GetModuleName()
//		GetMPIName()
//
// Parameters:
//		none		
//
// Returns:
//		various
//
// Description:
//		Returns MPI & Module name & version info.
//==============================================================================

	
tErrType	CDebugMPI::GetMPIVersion(tVersion &version)  const
{
	version = kDebugMPIVersion;
	return kNoErr; 
}

tErrType	CDebugMPI::GetMPIName(ConstPtrCString &pName) const
{
	pName = &kDebugMPIName;
	return kNoErr; 
}	


tErrType	CDebugMPI::GetModuleVersion(tVersion &version) const
{
	version = kDebugModuleVersion;
	return kNoErr; 
}
	
tErrType	CDebugMPI::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kDebugModuleName;
	return kNoErr; 
}
 
tErrType	CDebugMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kDebugModuleOriginURI;
	return kNoErr; 
}	


//==============================================================================
// Function:
//		__VDebugOut
//
// Parameters:
//		tDebugSignature sig - the ID of the module requesting the Printf();
//		tDbgLvl	- the debug level for a Printf call (0 - kMaxDebugLevel)
//		char * 	- format string
//		va_list	- the argument list that completes the format string
//		U16		- debugOut format
//
// Returns:
//		void
//
// Description:
//		Provides Run-time control of debug output. sig and lvl are compared
//		against the current control values to determine if the Printf should
//		be allowed or ignored. Implementation of standard "C" printf.  
//		Calls the Kernel module's printf.  
//==============================================================================
static void __VDebugOut( tDebugSignature sig, tDebugLevel lvl, const char * formatString, 
						 va_list arguments, U16	debugOutFormat)
{
	// NOTE: get out as fast as possible if not going to print out.  Using
	//		 the helper fcn for this.
	if (CheckDebugOutIsEnabled(sig, lvl))
	{
		// Ready to output...

		// ...if timestamping all output, add timestamp first
		if (gDebugEnvPtr->timestampDebugOut)
		{
			U32 mSec, uSec;
//			mSec = CKernelMPI::GetElapsedPrecisionTime(&uSec);
			printf(kDebugOutTimestampFmt, mSec, uSec);
		}

		// ...if not asking for literal output, show the module signature
		if (!(debugOutFormat & kDebugOutFormatLiteral))
			printf(kDebugOutSignatureFmt, sig);

		// ...if this is a warning, output warning string
		if (debugOutFormat & kDebugOutFormatWarn)
			printf(kWarnTagStr);

		// ...now output the requested data.
		vprintf(formatString, arguments);
	}
}


//==============================================================================
// Function:
//		__assert
//
// Parameters:
//		char * 	- format string
//		va_list	- the argument list that completes the format string
//
// Returns:
//		void
//
// Description:
//		Handler for assertions based on tested assumptions in the code.  **We
//		only get here if the assertion tested was false**. Outputs information 
//		to identify the source of error.
//==============================================================================
static void __assert(const char *formatString, va_list arguments)
{

    printf(kAssertTagStr, strlen(kAssertTagStr)); // in CPU Specific code
}


//==============================================================================
// Function:
//		Warn
//
// Parameters:
//		tDebugSignature sig - the ID of the module requesting the Printf();
//		char * 	- format string
//		va_list	- the argument list that completes the format string
//
// Returns:
//		void
//
// Description:
//		Equivalent to DebugOut @ kDbgLvlCritical, with standard warning text prepended.
//		This is implemented as a separate function rather than just a macro for
//		VDebugOut, since will probably be adding additional warning feedback
//		(on screen, or audio) accompanying serial out message.  
//==============================================================================
static void __Warn(tDebugSignature sig, const char * formatString, va_list arguments)
{
	__VDebugOut(sig, kDbgLvlCritical, formatString, arguments, kDebugOutFormatWarn);
}

void 	CDebugMPI::DebugOut(tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, ...)
{
	va_list arguments;

	va_start(arguments, formatString);
	__VDebugOut(sig, lvl, formatString, arguments, kDebugOutFormatNormal);
	va_end(arguments);
}

void 	CDebugMPI::VDebugOut(tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments)
{
	__VDebugOut(sig, lvl, formatString, arguments, kDebugOutFormatNormal);
}

void 	CDebugMPI::DebugOutLiteral(tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, ...)
{
	va_list arguments;

	va_start(arguments, formatString);
	__VDebugOut(sig, lvl, formatString, arguments, kDebugOutFormatLiteral);
	va_end(arguments);
}

void 	CDebugMPI::VDebugOutLiteral(tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments)
{
	__VDebugOut(sig, lvl, formatString, arguments, kDebugOutFormatLiteral);
}


void 	CDebugMPI::Warn(tDebugSignature sig, const char * formatString, ...)
{
	va_list arguments;

	va_start(arguments, formatString);
	__Warn(sig, formatString, arguments);
	va_end(arguments);
}
						
void 	CDebugMPI::Assert(int testResult, const char * formatString, ...)
{
	// test w/in the inline--avoids overhead of fcn call if test o.k.
	if (!testResult)
	{
		va_list arguments;
		va_start(arguments, formatString);
		__assert(formatString, arguments);
	}

	// NOTE: we never return from calling an Assert, so save code space
	// by ditching the following...
	// va_end(arguments)
}

//==============================================================================
// Function:
//		DebugLevelSet
//
// Parameters:
//		char *pBuf -- contains the new master debug level in ascii form.
//
// Returns:
//		NA
//
// Description:
//		Implements the Debug Level Set command response.  Parses the passed
//		string for an integer that is less than or equal to the maximum debug 
//		level value (kMaxDebugLevel).  Sets the command task environment to
//		the parsed value.  Printf messages with debug levels greater than 
//		the set value will NOT be sent across the serial port.   
//==============================================================================
static void DebugLevelSet(char *pBuf)
{
	tDebugLevel newLevel;

	newLevel = (tDebugLevel)atoi(pBuf);

	if(newLevel <= kMaxDebugLevel)
	{
		gDebugEnvPtr->masterDebugLevel = newLevel;
	}	 
}

//==============================================================================
// Function:
//		SignatureDebugSet
//
// Parameters:
//		char *pBuf -- contains the module id and debug setting ascii form.
//
// Returns:
//		NA
//
// Description:
//		Implements the module Debug control.  Allows a user to turn Printf's 
//		On or Off for specific modules.   
//==============================================================================
static void SignatureDebugSet(char *pBuf)
{
	U16 newSetting, targetMod;

	targetMod = atoi(pBuf);

	while(*pBuf != ' ' && *pBuf != '\0')
	{
		pBuf++;
	}

	if(*pBuf == ' ')
	{
		newSetting = atoi(pBuf);

		if(targetMod <= kMaxDebugSig)
		{
			if(newSetting)
			{
				gDebugEnvPtr->sigDbgBitVecArray[targetMod/8] |= (1<<(targetMod%8));
			}
			else
			{
				gDebugEnvPtr->sigDbgBitVecArray[targetMod/8] &= ~(1<<(targetMod%8));
			}
		}
	}
}	
			
//==============================================================================
// Function:
//		DisableDebugOut
//
// Parameters:
//		tDebugSignature sig -- The target signature for which debug output
//			is to be disabled.
//
// Returns:
//		N/A
//
// Description:
//		Allows code to disable debug output for a specific signature so that
//		a user is not necessarily required to get the desired configuration
//		through a series of serial commands each time they power the system.   
//==============================================================================
void CDebugMPI::DisableDebugOut(tDebugSignature sig)
{
	if(sig <= kMaxDebugSig)
		gDebugEnvPtr->sigDbgBitVecArray[(sig>>3)] &= ~(1<<(sig&0x7));
}

//==============================================================================
// Function:
//		DisableDebugOut
//
// Parameters:
//		tDebugSignature sig -- The target signature for which debug output
//			is to be re-enabled.
//
// Returns:
//		N/A
//
// Description:
//		Allows code to re-enable debug output for a specific signature so that
//		a user is not necessarily required to get the desired configuration
//		through a series of serial commands each time they power the system.   
//==============================================================================
void CDebugMPI::EnableDebugOut(tDebugSignature sig)
{
	if(sig <= kMaxDebugSig)
		gDebugEnvPtr->sigDbgBitVecArray[(sig>>3)] |= (1<<(sig&0x7));
}

//==============================================================================
// Function:
//		DebugOutIsEnabled
//
// Parameters:
//		tDebugSignature sig -- The target signature
//		tDebugLevel	level	
//
// Returns:
//		Boolean 
//
// Description:
//		Checks if a DebugOut command with this level or higher is issued,
//		whether it will go out the serial port.   
//==============================================================================
Boolean CDebugMPI::DebugOutIsEnabled(tDebugSignature sig, tDebugLevel level)
{
	return CheckDebugOutIsEnabled(sig, level);
}

//==============================================================================
// Function:
//		SetDebugLevel
//		GetDebugLevel
//
// Parameters:
//		various
//
// Returns:
//		various
//
// Description:
//		Gets & sets the master debug level for all DebugOut.   
//==============================================================================

void CDebugMPI::SetDebugLevel(tDebugLevel newLevel)
{
	gDebugEnvPtr->masterDebugLevel = newLevel;
}

tDebugLevel CDebugMPI::GetDebugLevel()
{
	return gDebugEnvPtr->masterDebugLevel;
}

//==============================================================================
// Function:
//		EnableDebugOutTimestamp
//		DisableDebugOutTimestamp
//
// Parameters:
//		various
//
// Returns:
//		various
//
// Description:
//		Enable/disable adding timestamp to all debug output.   
//==============================================================================

void CDebugMPI::EnableDebugOutTimestamp()
{
	gDebugEnvPtr->timestampDebugOut = true;
}

void CDebugMPI::DisableDebugOutTimestamp()
{
	gDebugEnvPtr->timestampDebugOut = false;
}

// EOF
