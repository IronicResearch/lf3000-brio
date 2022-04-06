#ifndef LF_BRIO_DEBUGTYPES_H
#define LF_BRIO_DEBUGTYPES_H
//==============================================================================
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DebugTypes.h
//
// Description:
//		Define types for the System Debug module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <exception>
#include <syslog.h>

LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Stack Monitoring
//==============================================================================

// A utility macro to determine if it's time to spew a warning over stack size.
// Warning is generated when stack has less than 12.5% free or 24 bytes
//#define DebugStackWarningThreshold(OriginalStackSize)		(((OriginalStackSize>>3)>24)?(OriginalStackSize>>3):24)

//==============================================================================
// tDebugSignature 
//
//		Defines a debug identifier for code modules or specific functionality
//		so serial output can be turned on or off @ runtime on a per-source basis.
//
//		All the System Debug signatures are explicitly defined here.  Groups 
//		are allocated for each Product and Cartridge to assign their own signatures.
//
//		The signature groups/ranges roughly match the allocation space definitions we 
//		use for asset handles, resource types, event types, & other enumerations 
//		in Chorus.
//==============================================================================

typedef U16	tDebugSignature;

#define kFirstDebugSig					0x0000
#define kMaxDebugSig					0x07FF

#define kFirstDebugSigGroup				0x0000

//------------------------------------------------------------------------------
// Groups
//
//		Each Debug signature is an 11-bit value.  The top 4-bits are used to segment 
//		the signatures into System, Product & Cartridge groups.
//------------------------------------------------------------------------------
#define kSystemDebugSigGroup			0x0000		// System-defined DebugSigs
// #define kSystem2DebugSigGroup		0x0080		// ...continuation of System DebugSigs		
#define kProductDebugSigGroup			0x0100		// product-specific DebugSigs
#define kProductSharedDebugSigGroup		0x0180		// product DebugSigs defined for multiple products
													//	 (e.g. shared applications, like a game engine)
#define kFlashLiteDebugSigGroup			0x0200
#define kAppManagerDebugSigGroup		0x0280
#define kLightningCoreDebugSigGroup		0x0300
#define kTestSuiteDebugSigGroup			0x0380

#define kCartridge1DebugSigGroup		0x0400
#define kCartridge2DebugSigGroup		0x0480
#define kCartridge3DebugSigGroup		0x0500
#define kCartridge4DebugSigGroup		0x0580
#define kCartridge5DebugSigGroup		0x0600
#define kCartridge6DebugSigGroup		0x0680
#define kCartridge7DebugSigGroup		0x0700
#define kCartridge8DebugSigGroup		0x0780

//------------------------------------------------------------------------------
// Group starting DebugSigs
//------------------------------------------------------------------------------
#define kFirstSystemDebugSig			(kSystemDebugSigGroup 			| kFirstDebugSig)
#define kFirstProductDebugSig			(kProductDebugSigGroup 			| kFirstDebugSig)
#define kFirstProductSharedDebugSig		(kProductSharedDebugSigGroup 	| kFirstDebugSig)
#define kFirstCartridge1DebugSig		(kCartridge1DebugSigGroup 		| kFirstDebugSig)
#define kFirstCartridge2DebugSig		(kCartridge2DebugSigGroup 		| kFirstDebugSig)
#define kFirstCartridge3DebugSig		(kCartridge3DebugSigGroup		| kFirstDebugSig)
#define kFirstCartridge4DebugSig		(kCartridge4DebugSigGroup 		| kFirstDebugSig)
#define kFirstCartridge5DebugSig		(kCartridge5DebugSigGroup 		| kFirstDebugSig)
#define kFirstCartridge6DebugSig		(kCartridge6DebugSigGroup 		| kFirstDebugSig)
#define kFirstCartridge7DebugSig		(kCartridge7DebugSigGroup 		| kFirstDebugSig)
#define kFirstCartridge8DebugSig		(kCartridge8DebugSigGroup 		| kFirstDebugSig)

#define kFlashLiteDebugSig			(kFlashLiteDebugSigGroup 		| kFirstDebugSig)
#define kAppManagerDebugSig 		(kAppManagerDebugSigGroup         | kFirstDebugSig)
#define kTestSuiteDebugSig			(kTestSuiteDebugSigGroup               |kFirstDebugSig)

//==============================================================================
// System-defined Debug Signatures
//==============================================================================


//==============================================================================
// tDebugLevel 
//
//		Enumerates all the possible debug levels that can exist in the system.
//		The most crucial Printf's should be given a value of kDbgLvlCritical.
//		The user can set the master debug level at run-time thus limiting 
//		by importance the amount of Printf's that cause data to be sent out
//		the serial port.
//==============================================================================
enum tDebugLevel
{
	kDbgLvlSilent,
	kDbgLvlCritical,
	kDbgLvlImportant,
	kDbgLvlValuable,
	kDbgLvlNoteable,
	kDbgLvlVerbose,

	kMaxDebugLevel = kDbgLvlVerbose
} ;

const int DebugLevel2LogLevel [ ] = {LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_NOTICE, LOG_INFO, LOG_DEBUG};

//------------------------------------------------------------------------------
// tLogPlayerLoadTypeOffset
//
//		Player load type offset used to identify the type of logging record.
//		For player load events, the logging event type is defined as a  
//		known logging event plus this offset.  (This is so the details of the
//		logging event types don't need to be made public.)  
//------------------------------------------------------------------------------
enum tLogPlayerLoadTypeOffset 
{
	kLogFlyPlayerLaunchApp = 0,
	kLogPegPlayerLaunchApp,
	kLogFlashPlayerLoadMovie,
	kLogFlashPlayerPlayMovie
};


//------------------------------------------------------------------------------
// UnitTestAssertException
//
// For unit tests, better to throw an excetpion than actually assert out.
//------------------------------------------------------------------------------
class UnitTestAssertException : public std::exception
{
public:
	virtual const char *what() const throw()
	{
		return "Brio DebugMPI::Assert";
	}
	virtual ~UnitTestAssertException( ) throw() {}
};
		   
LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DEBUGTYPES_H

// EOF
