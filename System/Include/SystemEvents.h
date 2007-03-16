#ifndef LF_BRIO_SYSTEMEVENTS_H
#define LF_BRIO_SYSTEMEVENTS_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// File:
//		SystemEvents.h
//
// Description:
//		System-defined events.
//==============================================================================

#include <SystemTypes.h>

//==============================================================================	   
// System event groups
//==============================================================================

enum {  	 
	kSystemEventTypeGroupAudioMgr = kFirstNumSpaceGroup,
	kSystemEventTypeGroupDisplayMgr,
	kSystemEventTypeGroupEventMgr,
	kSystemEventTypeGroupRsrcMgr,
	kSystemEventTypeGroupUnitTests,
};

#define MakeFirstSystemGroupEventType(group) MakeEventType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag)
#define MakeSystemGroupAllEventsType(group)  MakeEventType(kSystemNumSpaceDomain, group, kU32LastNumSpaceTag)


//==============================================================================	   
// System events
//==============================================================================
//TODO/tp: Determine whethere to collect all here or distribute in "AudioTypes.h", etc.
enum {
	// AudioMgr events
	kAudioMgrEventTypeAudioCompleted = MakeFirstSystemGroupEventType(kSystemEventTypeGroupAudioMgr),
	kAudioMgrEventTypeAudioCuePoint,

	// DisplayMgr events
	// <first display mgr event> = MakeFirstSystemGroupEventType(kSystemEventTypeGroupDisplayMgr),
};

#endif // LF_BRIO_SYSTEMEVENTS_H

// EOF
