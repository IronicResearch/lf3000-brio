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

//#include <stdlib.h>
//#include <stdio.h>
//#include <stdarg.h>		// for vprintfs
//#include <assert.h>
#include <sys/time.h>	// for gettimeofday()
#include <syslog.h>

// system includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <Module.h>

// Module includes
#include <DebugMPI.h>
#include <DebugPriv.h>
#include <KernelMPI.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Types and constants
//==============================================================================
//------------------------------------------------------------------------------
const CString kMPIName = "DebugMPI";

//------------------------------------------------------------------------------
const char kAssertTagStr[] 				= "!ASSERT: ";
const char kWarnTagStr[] 				= "!WARNING: ";
const char kErrorTagStr[] 				= "!ERROR: ";
const char kDebugOutSignatureFmt[] 		= "[%d] ";
const char kDebugOutTimestampFmt[]		= "<@%dms> ";

#define MAX_MSG_LEN	 512

//------------------------------------------------------------------------------
// tDebugOutFormats
//
//		Bit mask used internally by the MPI to indicate any special formatting
//		to use for the debug output.  Some are mutually exclusive, but others
//		might be used in combination.
//------------------------------------------------------------------------------
enum  {
	kDebugOutFormatNormal	= 0x0000,
	kDebugOutFormatLiteral 		= 0x0001,
	kDebugOutFormatWarn		= 0x0002,
	kDebugOutFormatErr		= 0x0004
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

	localDebugLevel_ = kDbgLvlValuable;

}

//----------------------------------------------------------------------------
CDebugMPI::~CDebugMPI( void )
{
	Module::Disconnect( pModule_ );
	closelog();
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
	//--------------------------------------------------------------------------
	void DebugOutPriv( const CDebugModule* pModule, tDebugSignature sig, 
						tDebugLevel lvl, const char* errString, const char * formatString, 
						va_list arguments, U16 debugOutFormat )
			__attribute__ ((format (printf, 5, 0)));
	
	//--------------------------------------------------------------------------
	void DebugOutPriv( const CDebugModule* pModule, tDebugSignature sig, 
						tDebugLevel lvl, const char* errString, const char * formatString, 
						va_list arguments, U16 debugOutFormat )
	{
		
		// Ready to output va_list arguments
		char outstr[MAX_MSG_LEN];
		int nc=0, n;

		if( ! pModule ) {
			printf("Error: Null pointer pModule_ at line %d ! \n", __LINE__);
			openlog("Emerald Base", LOG_CONS | LOG_NDELAY, LOG_LOCAL4);
			syslog(LOG_CRIT, "Error: Null pointer pModule_ at line %d ! \n", __LINE__);
			return;
		}
	

		// va_list argumentsif timestamping all output, add timestamp first
		if (!(debugOutFormat & kDebugOutFormatLiteral) && pModule->TimestampIsEnabled())
		{
			CKernelMPI	kernel;

			U32 mSec = kernel.GetElapsedTimeAsMSecs();
			nc = sprintf(outstr,kDebugOutTimestampFmt, mSec);
		}

		// va_list argumentsif not asking for literal output, show the module signature
		if (!(debugOutFormat & kDebugOutFormatLiteral)) {
			n = sprintf(&outstr[nc], kDebugOutSignatureFmt, sig);
			nc += n;
		}

		if (errString != NULL)
		{
			n = sprintf(&outstr[nc], errString);
			nc += n;
		}

		// va_list argumentsnow output the requested data.
		vsnprintf(&outstr[nc], MAX_MSG_LEN-nc, formatString, arguments);
		
		printf(outstr);
		openlog("Emerald App", LOG_CONS | LOG_NDELAY, LOG_LOCAL4);
		syslog(lvl, outstr);
	}

	//--------------------------------------------------------------------------
	void AssertPriv( const CDebugModule* pModule, tDebugSignature sig,
						 tDebugLevel flagDebugLevel,
						const char* errString, const char * formatString, 
						va_list arguments )
			__attribute__ ((format (printf, 5, 0)));
	
	//--------------------------------------------------------------------------
	void AssertPriv( const CDebugModule* pModule, tDebugSignature sig, 
						tDebugLevel flagDebugLevel,
						const char* errString, const char * formatString, 
						va_list arguments )
	{
		// For cleaner unit test output, if ThrowOnAssertIsEnabled and
		// the debug level is kDbgLvlSilent, we don't print any text.
		// In all other cases we do ouput the "!ASSERT: ..." text.
		//
		Boolean throwOnAssert;

		if( ! pModule ) {
			printf("Error: Null pointer pModule_ at line %d ! \n", __LINE__);
			openlog("Emerald Base", LOG_CONS | LOG_NDELAY, LOG_LOCAL4);
			syslog(LOG_CRIT, "Error: Null pointer pModule_ at line %d ! \n", __LINE__);
			return;
		}

		throwOnAssert = pModule->ThrowOnAssertIsEnabled();
		if ( !throwOnAssert )
		{
			// Ready to output va_list arguments
			char outstr[MAX_MSG_LEN];
			int nc=0, n;

			nc = sprintf(outstr, kAssertTagStr);
			n = sprintf(&outstr[nc], kDebugOutSignatureFmt, sig);
			nc += n;
						
			if (errString)
			{
				n = sprintf(&outstr[nc], "%s: ", errString );
				nc += n;
			}
			
			n = vsnprintf(&outstr[nc], MAX_MSG_LEN-nc, formatString, arguments );

			printf(outstr );
			openlog("Emerald App", LOG_CONS | LOG_NDELAY, LOG_LOCAL4);			
			syslog(kDbgLvlCritical, outstr);
		}
		
		if (throwOnAssert)
		{
			UnitTestAssertException e;
			throw e;
		}
		
		// In Emerald, Application doesn't power down the device
		// kernel.PowerDown();
	}
}


//==============================================================================
// DebugOut functions
//==============================================================================
//----------------------------------------------------------------------------
void CDebugMPI::DebugOut( tDebugLevel lvl, const char * formatString, ... ) const
{
	// check both local debug level, master debug level, signature
	if ((lvl <= localDebugLevel_) && DebugOutIsEnabled(sig_, lvl)) {
		va_list arguments;
		va_start( arguments, formatString );
		DebugOutPriv( pModule_, sig_, lvl, NULL, formatString, arguments, kDebugOutFormatNormal );
		va_end( arguments );
	}
}

//----------------------------------------------------------------------------
void CDebugMPI::VDebugOut( tDebugLevel lvl, const char * formatString, 
							va_list arguments ) const
{
	if ((lvl <= localDebugLevel_)  && DebugOutIsEnabled(sig_, lvl)) {
		DebugOutPriv( pModule_, sig_, lvl, NULL, formatString, arguments, kDebugOutFormatNormal );
	}
}

//----------------------------------------------------------------------------
void CDebugMPI::DebugOutErr( tDebugLevel lvl, tErrType err, 
							const char * formatString, ... ) const
{
	char errstr[MAX_MSG_LEN];	

	if( ! pModule_ ) {
		printf("Error: Null pointer pModule_ at line %d ! \n", __LINE__);
		openlog("Emerald Base", LOG_CONS | LOG_NDELAY, LOG_LOCAL4);
		syslog(LOG_CRIT, "Error: Null pointer pModule_ at line %d ! \n", __LINE__);
		return;
	}
	
	vsnprintf(errstr, MAX_MSG_LEN, "%s: ", (void *)pModule_->ErrorToString(err));
	
	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv( pModule_, sig_, kDbgLvlCritical, errstr, formatString, arguments, kDebugOutFormatNormal );
	va_end( arguments );
}
//----------------------------------------------------------------------------
void CDebugMPI::DebugOutLiteral( tDebugLevel lvl, const char * formatString, 
								... ) const
{
	if ((lvl <= localDebugLevel_)  && DebugOutIsEnabled(sig_, lvl)) {	
		va_list arguments;
		va_start( arguments, formatString );
		DebugOutPriv( pModule_, sig_, lvl, NULL, formatString, arguments, kDebugOutFormatLiteral );
		va_end( arguments );
	}
}

//----------------------------------------------------------------------------
void CDebugMPI::VDebugOutLiteral( tDebugLevel lvl, const char * formatString, 
									va_list arguments ) const
{
	if ((lvl <= localDebugLevel_)  && DebugOutIsEnabled(sig_, lvl)) {
		DebugOutPriv( pModule_, sig_, lvl, NULL, formatString, arguments, kDebugOutFormatLiteral );
	}
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
	DebugOutPriv( pModule_, sig_, kDbgLvlImportant, NULL, formatString, arguments, kDebugOutFormatWarn );
	va_end( arguments );
}

//----------------------------------------------------------------------------
void CDebugMPI::Assert( int testResult, const char * formatString, ... ) const
{
	if (!testResult)
	{
		va_list arguments;
		va_start( arguments, formatString );
		AssertPriv( pModule_, sig_, kDbgLvlCritical, NULL, formatString, arguments );
		va_end( arguments );
	}
}


//----------------------------------------------------------------------------
void CDebugMPI::AssertNoErr( tErrType err, const char * formatString, ... ) const
{
	if (err != kNoErr)
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
		AssertPriv( pModule_, sig_, kDbgLvlCritical, errstr, formatString, arguments );
		va_end( arguments );
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
	CKernelMPI	kernel;
	kernel.PowerDown();
	
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
	localDebugLevel_ = newLevel;
}

tDebugLevel CDebugMPI::GetDebugLevel() const
{
	return localDebugLevel_;
}

void CDebugMPI::SetMasterDebugLevel( tDebugLevel newLevel )
{
	if (pModule_)
		pModule_->SetMasterDebugLevel( newLevel );
}

tDebugLevel CDebugMPI::GetMasterDebugLevel() const
{
	if (pModule_)
		return pModule_->GetMasterDebugLevel();

	// if we got here, module not connected
	CKernelMPI	kernel;
	kernel.PowerDown();
	
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

//==============================================================================
// Function:
//		EnableThrowOnAssert
//		DisableThrowOnAssert
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

void CDebugMPI::EnableThrowOnAssert()
{
	if (pModule_)
		pModule_->EnableThrowOnAssert();
}

void CDebugMPI::DisableThrowOnAssert()
{
	if (pModule_)
		pModule_->DisableThrowOnAssert();
}

LF_END_BRIO_NAMESPACE()
// EOF
