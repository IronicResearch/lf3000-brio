#ifndef LF_BRIO_DEBUGPRIVATE_H
#define LF_BRIO_DEBUGPRIVATE_H

//==============================================================================
//
// LeapFrog Confidential & Proprietary
// Copyright (c) 2004-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DebugPriv.h
//
// Description:
//		Internal defs for the System Debug module. 
//
//==============================================================================
// 
//==============================================================================

// System includes
#include <stdarg.h>	
#include <SystemTypes.h>
#include <CoreModule.h>

//==============================================================================
// Defines
//==============================================================================

// Assert definitions
#define kDebugAssertNVStringLength 	252

// define how many bytes need to represent the full range of possible
// Debug signatures (this scheme rounds-up on any partial bytes) 
#define kSigBitVecArraySize	(((kMaxDebugSig - kFirstDebugSig) / 8) + 1)

// Constants
const CString	kDebugModuleName	= "Debug";
const tVersion	kDebugModuleVersion	= MakeVersion(0,1);

class CDebugModule : public ICoreModule {
public:
		// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion( tVersion &version ) const;
	virtual tErrType	GetModuleName( ConstPtrCString &pName ) const;	
	virtual tErrType	GetModuleOrigin( ConstPtrCURI &pURI ) const;

	// class-specific functionality
	CDebugModule();
	virtual ~CDebugModule();

	virtual void DebugOut( tDebugSignature sig, tDebugLevel lvl, const char * formatString, va_list arguments );
	virtual void VDebugOut( tDebugSignature sig, tDebugLevel lvl, const char * formatString, va_list arguments );

	virtual void DebugOutLiteral( tDebugSignature sig, tDebugLevel lvl, const char * formatString, va_list arguments );
	virtual void VDebugOutLiteral( tDebugSignature sig, tDebugLevel lvl, const char * formatString, va_list arguments );

	virtual void Warn( tDebugSignature sig, const char * formatString, va_list arguments);

	virtual void Assert( const char * formatString, va_list arguments);
	
	virtual void DisableDebugOut( tDebugSignature sig );
	virtual void EnableDebugOut( tDebugSignature sig );
	virtual Boolean DebugOutIsEnabled( tDebugSignature sig, tDebugLevel level );

	virtual void SetDebugLevel( tDebugLevel newLevel );
	virtual tDebugLevel GetDebugLevel();
	
	virtual void EnableDebugOutTimestamp();
	virtual void DisableDebugOutTimestamp();
	
private:
	tDebugLevel		masterDebugLevel;
	Boolean			timestampDebugOut;
	U8				sigDbgBitVecArray[kSigBitVecArraySize];
	
	Boolean pCheckDebugOutIsEnabled(tDebugSignature sig, tDebugLevel lvl);	
	void pWarn( tDebugSignature sig, const char * formatString, va_list arguments );
	void pAssert( const char *formatString, va_list arguments );
	void pVDebugOut( tDebugSignature sig, tDebugLevel lvl, const char * formatString, 
						 va_list arguments, U16	debugOutFormat );
};

#endif		// LF_BRIO_DEBUGPRIVATE_H

// EOF
