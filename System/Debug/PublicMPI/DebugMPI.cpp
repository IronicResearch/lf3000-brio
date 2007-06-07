//==============================================================================
//
// Copyright (c) LeapFrog Enterprises, Inc.
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

// std includes

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>		// for vprintfs
#include <assert.h>
#include <stdarg.h>		// for vprintfs()
#include <sys/time.h>	// for gettimeofday()

// system includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <Module.h>

// Module includes
#include <DebugMPI.h>
#include <DebugPriv.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Types and constants
//==============================================================================
//------------------------------------------------------------------------------
const CString kMPIName = "DebugMPI";

//------------------------------------------------------------------------------
const char kAssertTagStr[] 				= "\n!ASSERT: ";
const char kWarnTagStr[] 				= "!WARNING: ";
const char kSavedAssertTagStr[] 		= "\n!SAVED ASSERT: ";
const char kDebugOutSignatureFmt[] 		= "[%d] ";
const char kDebugOutTimestampFmt[]		= "<@%d.%03dms> ";

//------------------------------------------------------------------------------
// tDebugOutFormats
//
//		Bit mask used internally by the MPI to indicate any special formatting
//		to use for the debug output.  Some are mutually exclusive, but others
//		might be used in combination.
//------------------------------------------------------------------------------
enum  {
	kDebugOutFormatNormal	= 0x0000,
	kDebugOutFormatLiteral 	= 0x0001,
	kDebugOutFormatWarn		= 0x0002
};


//==============================================================================
// ICoreMPI interface
//==============================================================================
//----------------------------------------------------------------------------
CDebugMPI::CDebugMPI( tDebugSignature sig ): pModule_(NULL), sig_(sig)
{
	ICoreModule*	pModule;
	Module::Connect( pModule, kDebugModuleName, kDebugModuleVersion );
	pModule_ = reinterpret_cast<CDebugModule*>(pModule);
}

//----------------------------------------------------------------------------
CDebugMPI::~CDebugMPI( void )
{
	Module::Disconnect( pModule_ );
}

//----------------------------------------------------------------------------
Boolean CDebugMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
const CString* CDebugMPI::GetMPIName() const
{
	return &kMPIName;
}	

//----------------------------------------------------------------------------
tVersion CDebugMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}
	
//----------------------------------------------------------------------------
const CString* CDebugMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}
 
//----------------------------------------------------------------------------
const CURI* CDebugMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}	


//==============================================================================
// Local utility functions
//==============================================================================
namespace
{
	//==========================================================================
	// Function:
	//		DebugOutPriv
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
	//==========================================================================
	void DebugOutPriv( const CDebugModule* pModule, tDebugSignature sig, 
						tDebugLevel lvl, const char * formatString, 
						va_list arguments, U16 debugOutFormat )
	{
		// NOTE: get out as fast as possible if not going to print out.  Using
		//		the helper fcn for this.
		// NOTE: If for some reason we are so broken that we were unable
		//		to connect to the underlying module, then print out everything.
		if (!pModule || pModule->DebugOutIsEnabled(sig, lvl))
		{
			// Ready to outputva_list arguments
	
			// va_list argumentsif timestamping all output, add timestamp first
			if (!(debugOutFormat & kDebugOutFormatLiteral) && pModule && pModule->TimestampIsEnabled())
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
}


//==============================================================================
// DebugOut functions
//==============================================================================
//----------------------------------------------------------------------------
void CDebugMPI::DebugOut( tDebugLevel lvl, const char * formatString, ... ) const
{
	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv( pModule_, sig_, lvl, formatString, arguments, kDebugOutFormatNormal );
	va_end( arguments );
}

//----------------------------------------------------------------------------
void CDebugMPI::VDebugOut( tDebugLevel lvl, const char * formatString, 
							va_list arguments ) const
{
	DebugOutPriv( pModule_, sig_, lvl, formatString, arguments, kDebugOutFormatNormal );
}

//----------------------------------------------------------------------------
void CDebugMPI::DebugOutErr( tDebugLevel lvl, tErrType err, 
							const char * formatString, ... ) const
{
	const char* errstr = NULL;
	char buf[16];
	if( pModule_ )
		errstr = pModule_->ErrorToString(err);
	else
	{
		sprintf(buf, "0x%x", (unsigned int)err);
		errstr = buf;
	}
		
	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv( pModule_, sig_, lvl, errstr, arguments, kDebugOutFormatNormal );
	DebugOutPriv( pModule_, sig_, lvl, ": ", arguments, kDebugOutFormatLiteral );
	DebugOutPriv( pModule_, sig_, lvl, formatString, arguments, kDebugOutFormatLiteral );
	va_end( arguments );
}
//----------------------------------------------------------------------------
void CDebugMPI::DebugOutLiteral( tDebugLevel lvl, const char * formatString, 
								... ) const
{
	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv( pModule_, sig_, lvl, formatString, arguments, kDebugOutFormatLiteral );
	va_end( arguments );
}

//----------------------------------------------------------------------------
void CDebugMPI::VDebugOutLiteral( tDebugLevel lvl, const char * formatString, 
									va_list arguments ) const
{
	DebugOutPriv( pModule_, sig_, lvl, formatString, arguments, kDebugOutFormatLiteral );
}


//==============================================================================
// Warn() & Assert()
//==============================================================================
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
void CDebugMPI::Warn( const char * formatString, ... ) const
{
	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv( pModule_, sig_, kDbgLvlCritical, formatString, arguments, kDebugOutFormatWarn );
	va_end( arguments );
}
						
//----------------------------------------------------------------------------
void CDebugMPI::Assert( int testResult, const char * formatString, ... ) const
{
	if (!testResult)
	{
		va_list arguments;
		va_start( arguments, formatString );
		printf( kAssertTagStr );
		printf( kDebugOutSignatureFmt, sig_ );
	    vprintf( formatString, arguments ); 
		va_end( arguments );
		assert(false);
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
void CDebugMPI::DisableDebugOut( tDebugSignature sig )
{
	if (pModule_)
		pModule_->DisableDebugOut( sig );
}

//==============================================================================
// Function:
//		EnableDebugOut
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
void CDebugMPI::EnableDebugOut( tDebugSignature sig )
{
	if (pModule_)
		pModule_->EnableDebugOut( sig );
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
Boolean CDebugMPI::DebugOutIsEnabled( tDebugSignature sig, tDebugLevel level ) const
{
	if (pModule_)
		return pModule_->DebugOutIsEnabled( sig, level );

	// if we got here, module not connected
	assert(false);
	
	return false; // get rid of compiler warning
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

void CDebugMPI::SetDebugLevel( tDebugLevel newLevel )
{
	if (pModule_)
		pModule_->SetDebugLevel( newLevel );
}

tDebugLevel CDebugMPI::GetDebugLevel() const
{
	if (pModule_)
		return pModule_->GetDebugLevel();

	// if we got here, module not connected
	assert(false);
	
	return kDbgLvlVerbose; // get rid of compiler warning
}

//==============================================================================
// Function:
//		EnableDebugOutTimestamp
//		DisableDebugOutTimestamp
//
// Parameters:
//		none
//
// Returns:
//		none
//
// Description:
//		Enable/disable adding timestamp to all debug output.   
//==============================================================================

void CDebugMPI::EnableDebugOutTimestamp()
{
	if (pModule_)
		pModule_->EnableDebugOutTimestamp();
}

void CDebugMPI::DisableDebugOutTimestamp()
{
	if (pModule_)
		pModule_->DisableDebugOutTimestamp();
}

LF_END_BRIO_NAMESPACE()
// EOF
