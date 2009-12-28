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

// Module includes
#include <DebugMPI.h>
#include <KernelMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

#define LF_LOG(ident, level, des) \
do { \
	openlog(ident, LOG_CONS | LOG_NDELAY, LOG_LOCAL4); \
	syslog(DebugLevel2LogLevel[level], "%s", des); \
}while(0)


//==============================================================================
// Types and constants
//==============================================================================
//------------------------------------------------------------------------------
const CString 		kMPIName = "DebugMPI";
const CURI     		kModuleOriginURI = "Debug Module Origin URI";
const CString		kDebugModuleName	= "Debug";
const tVersion 	kDebugModuleVersion	= 2;

//------------------------------------------------------------------------------
const char kAssertTagStr[] 				= "<ASSERT>: ";
const char kWarnTagStr[] 				= "<WARNING>: ";
const char kErrorTagStr[] 				= "<ERROR>: ";
const char kDebugOutSignatureFmt[] 		= "[0x%x] ";
const char kDebugOutTimestampFmt[]		= "<@%ums> ";

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

enum  {
	kOutputNone	= 0x0000,
	kOutputConsole = 0x0001,
	kOutputSyslog	 = 0x0002,
};


tDebugLevel	CDebugMPI::masterDebugLevel_ = kDbgLvlValuable;
Boolean		CDebugMPI::timestampDebugOut_ = false;
Boolean		CDebugMPI::throwOnAssert_ = false;

// info on implementation of this module


//==============================================================================
// ICoreMPI interface
//==============================================================================
//----------------------------------------------------------------------------
CDebugMPI::CDebugMPI( tDebugSignature sig ): sig_(sig)
{

	localDebugLevel_ = kDbgLvlValuable;

}

//----------------------------------------------------------------------------
CDebugMPI::~CDebugMPI( void )
{
}

//----------------------------------------------------------------------------
Boolean CDebugMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CDebugMPI::GetMPIName() const
{
	return &kMPIName;
}	

//----------------------------------------------------------------------------
tVersion CDebugMPI::GetModuleVersion() const
{
	return kDebugModuleVersion;
}
	
//----------------------------------------------------------------------------
const CString* CDebugMPI::GetModuleName() const
{
	return &kDebugModuleName;
}
 
//----------------------------------------------------------------------------
const CURI* CDebugMPI::GetModuleOrigin() const
{
	return &kModuleOriginURI;
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
	void DebugOutPriv(int outFlag, tDebugSignature sig, tDebugLevel lvl,
					const char* errString, Boolean timestampEnable,
					const char * formatString, va_list arguments)
			__attribute__ ((format (printf, 6, 0)));
	
	//--------------------------------------------------------------------------
	void DebugOutPriv(int outFlag, tDebugSignature sig, tDebugLevel lvl,
					const char* errString, Boolean timestampEnable,
					const char * formatString, va_list arguments )
	{
		
		// Ready to output va_list arguments
		char outstr[MAX_LOGGING_MSG_LEN];
		int nc=0, n;
		int priority;

		// show the module signature
		n = sprintf(&outstr[nc], kDebugOutSignatureFmt, sig);
		nc += n;
		
		// va_list argumentsif timestamping all output, add timestamp first
		if (timestampEnable)
		{
			CKernelMPI	kernel;

			U32 mSec = kernel.GetElapsedTimeAsMSecs();
			nc = sprintf(outstr,kDebugOutTimestampFmt, mSec);
		}

		if (errString != NULL)
		{
			n = sprintf(&outstr[nc], "%s: ", errString);
			nc += n;
		}

		// va_list argumentsnow output the requested data.
		vsnprintf(&outstr[nc], MAX_LOGGING_MSG_LEN-nc, formatString, arguments);
		
		if(outFlag & kOutputConsole) {
			printf("%s", outstr);
		}
		
		if(outFlag & kOutputSyslog) {
			LF_LOG("Emerald App", lvl, outstr);
		}
	}

	//--------------------------------------------------------------------------
	void AssertPriv(int outFlag, tDebugSignature sig,
						const char* errString, const char * formatString, 
						va_list arguments )
			__attribute__ ((format (printf, 4, 0)));
	
	//--------------------------------------------------------------------------
	void AssertPriv( int outFlag, tDebugSignature sig, 
						const char* errString, const char * formatString, 
						va_list arguments )
	{
		// For cleaner unit test output, if ThrowOnAssertIsEnabled and
		// the debug level is kDbgLvlSilent, we don't print any text.
		// In all other cases we do ouput the "!ASSERT: ..." text.
		//
		// Ready to output va_list arguments
		char outstr[MAX_LOGGING_MSG_LEN];
		int nc=0, n;

		nc = sprintf(outstr, "%s", kAssertTagStr);
		n = sprintf(&outstr[nc], kDebugOutSignatureFmt, sig);
		nc += n;
					
		if (errString)
		{
			n = sprintf(&outstr[nc], "%s: ", errString );
			nc += n;
		}
		
		n = vsnprintf(&outstr[nc], MAX_LOGGING_MSG_LEN-nc, formatString, arguments );

		if(outFlag & kOutputConsole) {
			printf("%s", outstr);
		}
		
		if(outFlag & kOutputSyslog) {
			LF_LOG("Emerald App", kDbgLvlCritical, outstr);
		}
		
		// In Emerald, Application doesn't power down the device, only in debug build.
		CKernelMPI	kernel;
		kernel.PowerDown();
	}
}


//==============================================================================
// DebugOut functions
//==============================================================================
//----------------------------------------------------------------------------
void CDebugMPI::DebugOut( tDebugLevel level, const char * formatString, ... ) const
{
	int flag = kOutputNone;
	
	// check local debug level affect console output
	if (level <= localDebugLevel_) {
		flag |= kOutputConsole;
	}
	
	// check master debug level, affect syslog
	if(level <= masterDebugLevel_) {
		flag |= kOutputSyslog;
	}
	
	if(flag != kOutputNone) {
		va_list arguments;
		va_start( arguments, formatString );
		DebugOutPriv(flag, sig_, level, NULL, TimestampIsEnabled(), formatString, arguments);
		va_end( arguments );		
	}
}

//----------------------------------------------------------------------------
void CDebugMPI::VDebugOut( tDebugLevel level, const char * formatString, 
							va_list arguments ) const
{
	int flag = kOutputNone;
	
	// check local debug level affect console output
	if (level <= localDebugLevel_) {
		flag |= kOutputConsole;
	}
	
	// check master debug level, affect syslog
	if(level <= masterDebugLevel_) {
		flag |= kOutputSyslog;
	}
	
	if(flag != kOutputNone) {
		DebugOutPriv(flag, sig_, level, NULL, TimestampIsEnabled(), formatString, arguments );
	}
}

//----------------------------------------------------------------------------
void CDebugMPI::DebugOutErr( tDebugLevel level, tErrType err, 
							const char * formatString, ... ) const
{
	int flag;

	const char *errstr;
	errstr = ErrToStr(err);
	
	flag = kOutputConsole | kOutputSyslog;	
	
	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv(flag, sig_, kDbgLvlCritical, errstr, TimestampIsEnabled(), formatString, arguments );
	va_end( arguments );
}

//----------------This function is the same as DebugOut, provided for backward compatibility
void CDebugMPI::DebugOutLiteral( tDebugLevel level, const char * formatString, 
								... ) const
{
	int flag = kOutputNone;
	
	// check local debug level affect console output
	if (level <= localDebugLevel_) {
		flag |= kOutputConsole;
	}
	
	// check master debug level, affect syslog
	if(level <= masterDebugLevel_) {
		flag |= kOutputSyslog;
	}

	if(flag != kOutputNone) {
		va_list arguments;
		va_start( arguments, formatString );
		DebugOutPriv(flag, sig_, level, NULL, TimestampIsEnabled(), formatString, arguments);
		va_end( arguments );		
	}
	
}

//----------------This function is the same as VDebugOut, provided for backward compatibility
void CDebugMPI::VDebugOutLiteral( tDebugLevel level, const char * formatString, 
									va_list arguments ) const
{
	int flag = kOutputNone;
	
	// check local debug level affect console output
	if (level <= localDebugLevel_) {
		flag |= kOutputConsole;
	}
	
	// check master debug level, affect syslog
	if(level <= masterDebugLevel_) {
		flag |= kOutputSyslog;
	}
	
	if(flag != kOutputNone) {
		DebugOutPriv(flag, sig_, level, NULL, TimestampIsEnabled(), formatString, arguments );
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
	int flag = kOutputConsole | kOutputSyslog;

	va_list arguments;
	va_start( arguments, formatString );
	DebugOutPriv(flag, sig_, kDbgLvlImportant, NULL, TimestampIsEnabled(), formatString, arguments);
	va_end( arguments );
}

//----------------------------------------------------------------------------
void CDebugMPI::Assert( int testResult, const char * formatString, ... ) const
{
	int flag = kOutputConsole | kOutputSyslog;
	
	if (!testResult)
	{
		if(ThrowOnAssertIsEnabled()) {
			UnitTestAssertException e;
			throw e;
		} else {
			va_list arguments;
			va_start( arguments, formatString );
			AssertPriv(flag, sig_, NULL, formatString, arguments );
			va_end( arguments );
		}
	}
}


//----------------------------------------------------------------------------
void CDebugMPI::AssertNoErr( tErrType err, const char * formatString, ... ) const
{
	int flag = kOutputConsole | kOutputSyslog;
	
	if (err != kNoErr)
	{
		if(ThrowOnAssertIsEnabled()) {
			UnitTestAssertException e;
			throw e;
		} else {
			const char* errstr = NULL;

			errstr = ErrToStr(err);
			
			va_list arguments;
			va_start( arguments, formatString );
			AssertPriv(flag, sig_, errstr, formatString, arguments );
			va_end( arguments );
		}
			
	}
}



//==============================================================================
// Function:
// 	The following two functions are used for backward compatibility only
//==============================================================================
void CDebugMPI::DisableDebugOut( tDebugSignature sig )
{
	return;
}

void CDebugMPI::EnableDebugOut( tDebugSignature sig )
{
	return;
}

Boolean CDebugMPI::DebugOutIsEnabled( tDebugSignature sig, tDebugLevel level ) const
{
	if (level > localDebugLevel_)
		return false;
	else
		return true;

}

//==============================================================================
// Function:
//		SetDebugLevel
//		GetDebugLevel
// 		SetMasterDebugLevel
//		GetMasterDebugLevel
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
	masterDebugLevel_ = newLevel;
}

tDebugLevel CDebugMPI::GetMasterDebugLevel() const
{
	return masterDebugLevel_;
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
	timestampDebugOut_ = true;
}

void CDebugMPI::DisableDebugOutTimestamp()
{
	timestampDebugOut_ = false;
}

Boolean CDebugMPI::TimestampIsEnabled() const
{
	return timestampDebugOut_;
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
	throwOnAssert_ = true;
}

void CDebugMPI::DisableThrowOnAssert()
{
	throwOnAssert_ = false;
}

Boolean CDebugMPI::ThrowOnAssertIsEnabled() const
{
	return throwOnAssert_;
}

LF_END_BRIO_NAMESPACE()
// EOF
