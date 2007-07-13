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

// system includes
#include <SystemTypes.h>
//#include <DebugTypes.h>

// Module includes
#include <DebugPriv.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Function Prototypes
//==============================================================================	


// info on implementation of this module
const CURI	kModuleOriginURI = "Debug Module Origin URI";

//============================================================================
// Instance management interface for the Module Manager
//============================================================================

static CDebugModule*	sinst = NULL;

			  
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
CDebugModule::CDebugModule(void) : masterDebugLevel_(kDbgLvlValuable),
		timestampDebugOut_(false), throwOnAssert_(false)
{
	// initialize the sigDbgBitVecArray_ so that DebugOut for all modules is 
	// turned on by default.
	for(U32 i = 0 ; i < kSigBitVecArraySize ; i++)
		sigDbgBitVecArray_[i] = 0xff;

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

tVersion CDebugModule::GetModuleVersion() const
{
	return kDebugModuleVersion;
}
	
const CString* CDebugModule::GetModuleName() const
{
	return &kDebugModuleName;
}
 
const CURI* CDebugModule::GetModuleOrigin() const
{
	return &kModuleOriginURI;
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
		sigDbgBitVecArray_[(sig>>3)] &= ~(1<<(sig&0x7));
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
		sigDbgBitVecArray_[(sig>>3)] |= (1<<(sig&0x7));
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
Boolean CDebugModule::DebugOutIsEnabled( tDebugSignature sig, tDebugLevel lvl ) const
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
			(lvl > masterDebugLevel_) ||
			 sig > kMaxDebugSig ||
			 !(sigDbgBitVecArray_[(sig >> 3)] & (1 << (sig & 0x7)))) ? false : true;
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
	masterDebugLevel_ = newLevel;
}

tDebugLevel CDebugModule::GetDebugLevel() const
{
	return masterDebugLevel_;
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
	timestampDebugOut_ = true;
}

void CDebugModule::DisableDebugOutTimestamp()
{
	timestampDebugOut_ = false;
}

Boolean CDebugModule::TimestampIsEnabled() const
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

void CDebugModule::EnableThrowOnAssert()
{
	throwOnAssert_ = true;
}

void CDebugModule::DisableThrowOnAssert()
{
	throwOnAssert_ = false;
}

Boolean CDebugModule::ThrowOnAssertIsEnabled() const
{
	return throwOnAssert_;
}

//==============================================================================
const char* const CDebugModule::ErrorToString(tErrType err) const
{
	return ErrToStr(err);
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()
extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance( tVersion /*version*/ )
	{
		if (sinst == NULL)
			sinst = new CDebugModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance( ICoreModule* /*ptr*/ )
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG

// EOF
