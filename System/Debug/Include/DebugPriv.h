#ifndef LF_BRIO_DEBUGPRIVATE_H
#define LF_BRIO_DEBUGPRIVATE_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
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
#include <StringTypes.h>
#include <DebugTypes.h>
#include <CoreModule.h>
LF_BEGIN_BRIO_NAMESPACE()


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
const tVersion	kDebugModuleVersion	= 2;

//==============================================================================
const char* const ErrToStr( tErrType error );


//==============================================================================
class CDebugModule : public ICoreModule {
public:
		// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT void DisableDebugOut( tDebugSignature sig );
	VTABLE_EXPORT void EnableDebugOut( tDebugSignature sig );
	VTABLE_EXPORT Boolean DebugOutIsEnabled( tDebugSignature sig, 
											tDebugLevel level ) const;

	VTABLE_EXPORT void SetMasterDebugLevel( tDebugLevel newLevel );
	VTABLE_EXPORT tDebugLevel GetMasterDebugLevel( ) const;
	
	VTABLE_EXPORT void EnableDebugOutTimestamp( );
	VTABLE_EXPORT void DisableDebugOutTimestamp( );
	VTABLE_EXPORT Boolean TimestampIsEnabled( ) const;
	
	VTABLE_EXPORT void 	EnableThrowOnAssert();
	VTABLE_EXPORT void 	DisableThrowOnAssert();
	VTABLE_EXPORT Boolean ThrowOnAssertIsEnabled( ) const;	
	
	VTABLE_EXPORT const char* const ErrorToString( tErrType error ) const;
	
private:
	tDebugLevel		masterDebugLevel_;
	Boolean			timestampDebugOut_;
	Boolean			throwOnAssert_;
	U8				sigDbgBitVecArray_[kSigBitVecArraySize];

	// Limit object creation to the Module Manager interface functions
	CDebugModule( );
	virtual ~CDebugModule( );
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_DEBUGPRIVATE_H

// EOF
