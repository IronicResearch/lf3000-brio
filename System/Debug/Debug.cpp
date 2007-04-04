//==============================================================================
//
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		Debug.cpp
//
// Description:
//		Implements the Module underlying the System Debug MPI.
//
//==============================================================================

//==============================================================================

// std includes

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>		// for vprintfs()
#include <sys/time.h>	// for gettimeofday()

// system includes
#include <SystemTypes.h>
#include <DebugMPI.h>

// Module includes
#include <DebugPriv.h>

//------------------------------------------------------------------------------
// tDebugOutFormats
//
//		Bit mask used internally by the MPI to indicate any special formatting
//		to use for the debug output.  Some are mutually exclusive, but others
//		might be used in combination.
//------------------------------------------------------------------------------
typedef enum {
	kDebugOutFormatNormal	= 0x0000,
	kDebugOutFormatLiteral 	= 0x0001,
	kDebugOutFormatWarn		= 0x0002
} tDebugOutFormats;


//==============================================================================
// Function Prototypes
//==============================================================================	


// info on implementation of this module
const CURI	kModuleOriginURI = "Debug Module Origin URI";

//==============================================================================
// Defines
//==============================================================================

const char kAssertTagStr[] 				= "\n!ASSERT: ";
const char kWarnTagStr[] 				= "!WARNING: ";
const char kSavedAssertTagStr[] 		= "\n!SAVED ASSERT: ";
const char kDebugOutSignatureFmt[] 		= "[%d] ";
const char kDebugOutTimestampFmt[]		= "<@%d.%03dms> ";

//============================================================================
// Instance management interface for the Module Manager
//============================================================================

static CDebugModule*	sinst = NULL;

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
Boolean CDebugModule::pCheckDebugOutIsEnabled(tDebugSignature sig, tDebugLevel lvl)
{
	// want to exit this as soon as possible for performance reasons so
	// n't take hit when debug switch off on shipping product, so we
	//  the checks in a very specific orderva_list arguments

	// If we're silencedva_list arguments
	// 		va_list argumentsor level is > master level
	// 		va_list argumentsor signature out of range
	// 		va_list argumentsor output disabled for signature
	// 		then debug not enabled.
	// Else debug enabled for this signature.
	return ((lvl == kDbgLvlSilent) || 
			(lvl > masterDebugLevel) ||
			 sig > kMaxDebugSig ||
			 !(sigDbgBitVecArray[(sig >> 3)] & (1 << (sig & 0x7)))) ? false : true;
}
	
	
			  
//==============================================================================
// Function:
//		ctor
//
// Parameters:
//		NA
//
// Returns:
//
// Description:

//==============================================================================
CDebugModule::CDebugModule(void)
{
	U32 i;

	masterDebugLevel = kDbgLvlValuable;
	timestampDebugOut = false;

	// initialize the sigDbgBitVecArray so that DebugOut for all modules is 
	// turned on by default.
	for(i = 0 ; i < kSigBitVecArraySize ; i++)
		sigDbgBitVecArray[i] = 0xff;

}

//==============================================================================
// Function:
//		dtor
//
// Parameters:
//		NA
//
// Returns:
//		void
//
// Description:
//==============================================================================
CDebugModule::~CDebugModule(void)
{
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

Boolean CDebugModule::IsValid() const
{
	return (sinst != NULL) ? true : false; 
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

tErrType	CDebugModule::GetModuleVersion(tVersion &version) const
{
	version = kDebugModuleVersion;
	return kNoErr; 
}
	
tErrType	CDebugModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kDebugModuleName;
	return kNoErr; 
}
 
tErrType	CDebugModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleOriginURI;
	return kNoErr; 
}	


//==============================================================================
// Function:
//		pVDebugOut
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
void CDebugModule::pVDebugOut( tDebugSignature sig, tDebugLevel lvl, const char * formatString, 
						 va_list arguments, U16	debugOutFormat )
{
	// NOTE: get out as fast as possible if not going to print out.  Using
	//		 the helper fcn for this.
	if (pCheckDebugOutIsEnabled(sig, lvl))
	{
		// Ready to outputva_list arguments

		// va_list argumentsif timestamping all output, add timestamp first
		if (timestampDebugOut)
		{
			U32 mSec, uSec;
            timeval time;

//			mSec = CKernelMPI::GetElapsedPrecisionTime(&uSec);
			gettimeofday( &time, NULL );
			printf( kDebugOutTimestampFmt, (time.tv_sec / 1000), time.tv_usec );
		}

		// va_list argumentsif not asking for literal output, show the module signature
		if (!(debugOutFormat & kDebugOutFormatLiteral))
			printf(kDebugOutSignatureFmt, sig);

		// va_list argumentsif this is a warning, output warning string
		if (debugOutFormat & kDebugOutFormatWarn)
			printf(kWarnTagStr);

		// va_list argumentsnow output the requested data.
		vprintf(formatString, arguments);
	}
}


//==============================================================================
// Function:
//		passert
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
void CDebugModule::pAssert(const char *formatString, va_list arguments)
{
	printf( kAssertTagStr );
    vprintf( formatString, arguments ); 
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
void CDebugModule::pWarn( tDebugSignature sig, const char * formatString, va_list arguments )
{
	pVDebugOut( sig, kDbgLvlCritical, formatString, arguments, kDebugOutFormatWarn );
}

void 	CDebugModule::DebugOut( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments )
{
	pVDebugOut( sig, lvl, formatString, arguments, kDebugOutFormatNormal );
}

void 	CDebugModule::VDebugOut( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments )
{
	pVDebugOut( sig, lvl, formatString, arguments, kDebugOutFormatNormal );
}

void 	CDebugModule::DebugOutLiteral( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments )
{
	pVDebugOut( sig, lvl, formatString, arguments, kDebugOutFormatLiteral );
}

void 	CDebugModule::VDebugOutLiteral( tDebugSignature sig, tDebugLevel lvl, 
						const char * formatString, va_list arguments )
{
	pVDebugOut( sig, lvl, formatString, arguments, kDebugOutFormatLiteral );
}


void 	CDebugModule::Warn( tDebugSignature sig, const char * formatString, va_list arguments )
{
	pWarn( sig, formatString, arguments );
}
						
void 	CDebugModule::Assert( const char * formatString, va_list arguments )
{
	pAssert( formatString, arguments );
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
void CDebugModule::DisableDebugOut( tDebugSignature sig )
{
	if (sig <= kMaxDebugSig)
		sigDbgBitVecArray[(sig>>3)] &= ~(1<<(sig&0x7));
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
void CDebugModule::EnableDebugOut( tDebugSignature sig )
{
	if (sig <= kMaxDebugSig)
		sigDbgBitVecArray[(sig>>3)] |= (1<<(sig&0x7));
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
Boolean CDebugModule::DebugOutIsEnabled( tDebugSignature sig, tDebugLevel level )
{
	return pCheckDebugOutIsEnabled(sig, level);
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

void CDebugModule::SetDebugLevel( tDebugLevel newLevel )
{
	masterDebugLevel = newLevel;
}

tDebugLevel CDebugModule::GetDebugLevel()
{
	return masterDebugLevel;
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

void CDebugModule::EnableDebugOutTimestamp()
{
	timestampDebugOut = true;
}

void CDebugModule::DisableDebugOutTimestamp()
{
	timestampDebugOut = false;
}

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
//------------------------------------------------------------------------
extern "C" tVersion ReportVersion()
{
	// TBD: ask modules for this or incorporate verion into so file name
	return kDebugModuleVersion;
}
	
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance( tVersion version )
{
	if (sinst == NULL)
		sinst = new CDebugModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance( ICoreModule* ptr )
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}

// EOF
