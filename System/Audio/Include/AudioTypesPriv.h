#ifndef LF_BRIO_AUDIOTYPESPRIV_H_
#define LF_BRIO_AUDIOTYPESPRIV_H_
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioTypesPriv.h
//
// Description:
//		Defines the private, hidden types used by AudioMgrMPI. 
//
//==============================================================================

#include <SystemTypes.h>
#include <AudioTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

// kAudioCmdMsgTypeStartAudio
struct tAudioStartAudioInfo { 
	const CPath* 		path;
	U8				 	volume;
	tAudioPriority	 	priority;
	S8				 	pan;
	const IEventListener* pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
	tAudioHeader*		pAudioHeader;		// For Brio Raw types, pointer to header and data, provided by AudioTask, returned from GetPtr()
	
	tAudioStartAudioInfo( const CPath* pa = NULL,
						U8 v = 0, 
						tAudioPriority p = 0, 
						S8 pn = 0, 
						const IEventListener* l = NULL, 
						tAudioPayload pl = 0, 
						tAudioOptionsFlags f = 0, 
						tAudioHeader* h = NULL )
		: path(pa), volume(v), priority(p), pan(pn), pListener(l),
		payload(pl), flags(f), pAudioHeader(h) {}
};

// kAudioCmdMsgTypeSetAudioVolume
// kAudioCmdMsgTypeGetAudioVolume
struct tAudioVolumeInfo { 
	tAudioID				id;
	U8				 		volume;
	
	tAudioVolumeInfo(   tAudioID				i = 0,
						U8 						v = 0 )
		: id(i), volume(v) 
		
		{ }
};

// kAudioCmdMsgTypeSetAudioPriority
// kAudioCmdMsgTypeGetAudioPriority
struct tAudioPriorityInfo { 
	tAudioID				id;
	tAudioPriority	 		priority;
	
	tAudioPriorityInfo(   tAudioID				i = 0, 
						tAudioPriority			p = 0 )
		: id(i), priority(p) 
		
		{ }
};

// kAudioCmdMsgTypeSetAudioPan
// kAudioCmdMsgTypeGetAudioPan
struct tAudioPanInfo { 
	tAudioID				id;
	S8				 		pan;
	
	tAudioPanInfo(   tAudioID				i = 0,
						S8 						pn = 0 )
		: id(i), pan(pn) 
		
		{ }
};

// kAudioCmdMsgTypeSetAudioListener
// kAudioCmdMsgTypeGetAudioListener
struct tAudioListenerInfo { 
	tAudioID				id;
	const IEventListener 	*pListener;
	
	tAudioListenerInfo(   tAudioID				i = 0,
						const IEventListener* 	l = NULL )
		: id(i), pListener(l) 
		
		{ }
};

// kAudioCmdMsgTypeStopAudio
struct tAudioStopAudioInfo {
	tAudioID			id;
	Boolean				suppressDoneMsg;

	tAudioStopAudioInfo( tAudioID i = -1, 
						 Boolean s = false )
		: id(i), suppressDoneMsg(s) {}
};

// kAudioCmdMsgTypeMidiNoteOn
struct tAudioMidiNoteInfo {
	U8					channel;
	U8					noteNum;
	U8				 	velocity;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;

	tAudioMidiNoteInfo( U8 c = 0, 
						U8 n = 0, 
						U8 v = 0, 
						tAudioPayload pl = 0, 
						tAudioOptionsFlags f = 0 )
		: channel(c), noteNum(n), velocity(v), 
		  payload(pl), flags(f) {}
};

// kAudioCmdMsgTypePlayMidiFile
struct tAudioStartMidiFileInfo {
	tMidiPlayerID		id;
	const CPath* 		path;
	U8*					pMidiFileImage;		// MIDI file loaded into RAM
	U32					imageSize;			// Size of MIDI file in RAM
	U8				 	volume;
	tAudioPriority		priority;
	const IEventListener* pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;

/*	
	tAudioStartMidiFileInfo( tMidiPlayerID	i = 0,
						U8 *pi = NULL,
						U32 is = 0,
						tAudioPriority p = 0,
						const IEventListener* l = NULL, 
						tAudioPayload pl = 0, 
						tAudioOptionsFlags f = 0 )
		: id(i), path(NULL), pMidiFileImage(pi), priority(p), pListener(l),
		payload(pl), flags(f) {}
*/
};

// kAudioCmdMsgTypeStopMidiFile
struct tAudioStopMidiFileInfo {
	tMidiPlayerID		id;
	Boolean				suppressDoneMsg;

	tAudioStopMidiFileInfo( tMidiPlayerID i = -1, Boolean s = false )
		: id(i), suppressDoneMsg(s) {}
};

//kAudioCmdMsgTypeGetEnabledMidiTracks
//kAudioCmdMsgTypeEnableMidiTracks
//kAudioCmdMsgTypeTransposeMidiTracks
//kAudioCmdMsgTypeChangeMidiInstrument
//kAudioCmdMsgTypeChangeMidiTempo

struct tAudioMidiFilePlaybackParams {
	tMidiPlayerID		id;						// ID of player to change
	tMidiTrackBitMask 	trackBitMask;
	S8 					transposeAmount;
	tMidiInstr 			instrument;
	S8 					tempo;

	tAudioMidiFilePlaybackParams( tMidiPlayerID	i = 0,
						tMidiTrackBitMask tbm = 0,
						S8 tsp = 0,
						tMidiInstr mi = 0,
						S8 t = 0 )
		: id(i),  trackBitMask(tbm), transposeAmount(tsp), instrument(mi),
		tempo(t) {}
};

LF_END_BRIO_NAMESPACE()
#endif /*LF_BRIO_AUDIOTYPESPRIV_H_*/
