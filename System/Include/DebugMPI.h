#ifndef DEBUGMPI_H
#define DEBUGMPI_H

//==============================================================================
//
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DebugMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Debug module. 
//
//==============================================================================
//
//==============================================================================

// std includes
#include <stdarg.h>		// for vprinfs

// System includes
#include <CoreTypes.h>
#include <StringTypes.h>
#include <CoreMPI.h>

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
//#define kReserved2DebugSigGroup		0x0200
//#define kReserved3DebugSigGroup		0x0280
//#define kReserved4DebugSigGroup		0x0300
//#define kReserved5DebugSigGroup		0x0380

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


//==============================================================================
// System-defined Debug Signatures
//==============================================================================

typedef enum
{
	kDebugSig = kFirstSystemDebugSig,
	kUIMgrSig,
	kAudioMgrSig,
	kRtcMPISig,
	kSysMgrSig,
	kFlashPlayerSig,
	kLcdMPISig,
	kButtonMPISig,
	kDbmSig,
	kKernelMPISig,
	kFlashPlayerASTraceSig,
	kCffsMPISig,
	kFastUARTMPISig,
	kRegionMgrSig,
	kTestMPISig,
	kFlashDeviceDriverSig,
	kVideoMPISig,
	kTvMPISig,
	kLPPlayerSig,
	kHwrXESig,
	kProfilerSig,
	kUsbMPISig,
    kDotUsbMPISig,
 	kGCtrlMPISig,
	kCommMgrMPISig,
	kTestCommProviderMPISig,
	kUsbCommProviderMPISig
} tSystemDebugSigs;


//==============================================================================
// tDebugLevel 
//
//		Enumerates all the possible debug levels that can exist in the system.
//		The most crucial Printf's should be given a value of kDbgLvlCritical.
//		The user can set the master debug level at run-time thus limiting 
//		by importance the amount of Printf's that cause data to be sent out
//		the serial port.
//==============================================================================
typedef enum
{
	kDbgLvlSilent = 0,
	kDbgLvlCritical = 1,
	kDbgLvlImportant,
	kDbgLvlValuable,
	kDbgLvlNoteable,
	kDbgLvlVerbose,

	kMaxDebugLevel = kDbgLvlVerbose
} tDebugLevel;

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

//------------------------------------------------------------------------------
// tLogPlayerLoadTypeOffset
//
//		Player load type offset used to identify the type of logging record.
//		For player load events, the logging event type is defined as a  
//		known logging event plus this offset.  (This is so the details of the
//		logging event types don't need to be made public.)  
//------------------------------------------------------------------------------
typedef enum {
	kLogFlyPlayerLaunchApp = 0,
	kLogPegPlayerLaunchApp,
	kLogFlashPlayerLoadMovie,
	kLogFlashPlayerPlayMovie
} tLogPlayerLoadTypeOffset;

//------------------------------------------------------------------------------
// tLogEventMask
//
//		Bit mask used to enable/disable the types of events that can be logged.
//		The bits will be set by a parameter in the logging configuration resource. 
//------------------------------------------------------------------------------
typedef U8 tLogEventMask;

#define kLogNoneMask			0x00
#define kLogPowerOnMask			0x01
#define kLogPowerOffMask		0x02
#define kLogArrayMask			0x04
#define kLogStringMask			0x08
#define kLogLaunchAppMask		0x10
#define kLogPlayMovieMask		0x20
#define kLogDebugOutMask		0x40
#define kLogAllMask				0xFF

//------------------------------------------------------------------------------
// tDebugCallbackRoutine
//
// 	The callback routine type for command registration
//------------------------------------------------------------------------------
//typedef  void (*tDebugCallbackRoutine)(char *pStr);

// this flag is originally defined to 0 inside mpi.h
// it is redefined to 1 here to support InitDependents()
//#ifdef DebugMpiInitDepend
//#undef DebugMpiInitDepend
//#endif
//#define DebugMpiInitDepend() ((CCoreMPI::ModuleIsInstalled(kDebugMpiID)) ? ((CCoreMPI::ModuleIsInited(kDebugMpiID)) ? (true) : (CCoreMPI::InitModule(kDebugMpiID))) : (true))


//==============================================================================
// Class:
//		CDebugMPI
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Debug module.
//==============================================================================

class CDebugMPI : public ICoreMPI {
public:
	// MPI core functions
	Boolean		IsValid() const;	
	
	tErrType	GetMPIVersion(tVersion &version) const;		   
	tErrType	GetMPIName(ConstPtrCString &pName) const;		

	tErrType	GetModuleVersion(tVersion &version) const;
	tErrType	GetModuleName(ConstPtrCString &pName) const;	
	tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;
	
	CDebugMPI();  
	~CDebugMPI();
	
	// printf-like serial output (suppressible based on tDebugSignature). The signature
	// value is prepended in the output to allow easy debug identification.  These are
	// the primary calls to use for output.
	// Example: "This is my text output" -> [3] This is my text output 
	void 	DebugOut(tDebugSignature sig, tDebugLevel lvl, const char * formatString, ...);
	void	VDebugOut(tDebugSignature sig, tDebugLevel lvl, const char * formatString, va_list arguments);

	// same as above, but without anything prepended or postfixed in the output.  This is useful
	// when printing a sequence of output that needs to be cleanly formatted (tables, etc).
	// Example:  "This is my text output" -> This is my text output
	void	DebugOutLiteral(tDebugSignature sig, tDebugLevel lvl, const char * formatString, ...);
	void	VDebugOutLiteral(tDebugSignature sig, tDebugLevel lvl, const char * formatString, va_list arguments);

	// issue a warning that a critical, but recoverable, error occurred.  This
	// is just one step below Asserting.  The warning message is printed
	// on the screen, &/or out the serial port, and could be accompanied by
	// an audio alert to indicate a critical error occurred.
	// (suppressible based on tDebugSignature)
	void	Warn(tDebugSignature sig, const char * formatString, ...);

	// system execution cannot continue beyond a failed assert
	void 	Assert(int testResult, const char * formatString, ...);
								
	// turning on & off debug-out on a debug-signature basis
	static void 	DisableDebugOut(tDebugSignature sig);
	static void 	EnableDebugOut(tDebugSignature sig);

	//------------------------------------------------------------------------------
	// Function:	 	EnableDebugOutTimestamp
	//					DisableDebugOutTimestamp
	// Availability:	MPI v1.1+
	// Description:		if enabled, adds a timestamp to all debug output.  Time
	//					shown is elapsed time since system boot.
	//------------------------------------------------------------------------------
	static void 	EnableDebugOutTimestamp();
	static void 	DisableDebugOutTimestamp();

	// checks if debug out is enabled for a specific signature & level.  Helpful for
	// checking in advance if anything will go out the serial port before issuing a
	// series of DebugOuts.  'atLeastLevel' indicates output enabled for levels
	// >= that specified--default checks if any output enabled for the 'sig'.
	static Boolean		DebugOutIsEnabled(tDebugSignature sig, tDebugLevel atLeastLevel=kDbgLvlCritical);

	// accessing the master DebugOut level
	static void 		SetDebugLevel(tDebugLevel newLevel);
	static tDebugLevel	GetDebugLevel();


private:

};
		   
#endif // DEBUGMPI_H

// EOF
