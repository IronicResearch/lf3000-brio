#ifndef DEBUGPRIVATE_H
#define DEBUGPRIVATE_H

//==============================================================================
// $Source: X:/SW/Concord/System/Debug/Include/rcs/DebugPrivate.h $
//
// LeapFrog Confidential & Proprietary
// Copyright (c) 2004-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DebugPrivate.h
//
// Description:
//		Internal type defs for the System Debug module. 
//
//==============================================================================
// 
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <DebugMPI.h>

//==============================================================================
// Defines
//==============================================================================

// Assert definitions
#define kDebugAssertNVStringLength 	252

typedef struct
{
	U8	assertString[kDebugAssertNVStringLength];
	U32 assertChecksum;
} tAssertNVData;

#define kDebugAssertNVDataSize	(sizeof(tAssertNVData))

// The maximum number of commands and the size of the command task command array.
#define kMaxCommands 			(128)				

// define how many bytes need to represent the full range of possible
// Debug signatures (this scheme rounds-up on any partial bytes) 
#define kSigBitVecArraySize	(((kMaxDebugSig - kFirstDebugSig) / 8) + 1)


typedef struct {
	S8 						bValid;	   		// valid boolean
//	tDebugCallbackRoutine 	CallBack;		// callback routine to use when the
											//  command task receives this command. 
	const char 				*pHelpStr;		// Explains the command.
} tDebugCommand;


//------------------------------------------------------------------------------
// tDebugEnvironment
//		The form of	Debug command task global environment data. This struct 
//		should be modified whenever it is desired to change the contents of 
//		the task's global data.
//------------------------------------------------------------------------------
typedef struct
{
	U32				taskStackSize;
	U32				assertPriority;
	Boolean			commandTaskEnabled;
	tDebugCommand 	*commandArray;
	tDebugLevel		masterDebugLevel;
	Boolean			bContinue;
	Boolean			timestampDebugOut;
//	tTaskID			debugTaskID;
	U8				sigDbgBitVecArray[kSigBitVecArraySize];

} tDebugEnvironment;   


//extern tDebugEnvironment* gDebugEnvPtr;

//------------------------------------------------------------------------------
// DebugOut strings
//------------------------------------------------------------------------------
extern const char kWarnTagStr[];
extern const char kDebugOutSignatureFmt[];
extern const char kDebugOutTimestampFmt[];

#endif		// DEBUGPRIVATE_H

// EOF
