#ifndef LF_BRIO_AUDIOTYPESPRIV_H_
#define LF_BRIO_AUDIOTYPESPRIV_H_
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// AudioTypesPriv.h
//
//      Defines private types used by AudioMPI
//
//==============================================================================

#include <SystemTypes.h>
#include <AudioTypes.h>
#include <EventListener.h>
LF_BEGIN_BRIO_NAMESPACE()

#define kAudio_Pan_Max    100
#define kAudio_Pan_Min  (-100)
#define kAudio_Pan_Default 0

#define kAudio_Volume_Max    100
#define kAudio_Volume_Min      0
#define kAudio_Volume_Default 100

// kAudioCmdMsgTypeStartAudio
struct tAudioStartAudioInfo { 
	const CPath* 		path;
	U8				 	volume;
	tAudioPriority	 	priority;
	S8				 	pan;
	const IEventListener* pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
	tAudioHeader*		pRawHeader;		// For Brio Raw audio header 

	
	tAudioStartAudioInfo( const CPath* pa = NULL,
						U8 v = 0, 
						tAudioPriority p = 0, 
						S8 pn = 0, 
						const IEventListener* l = NULL, 
						tAudioPayload pl = 0, 
						tAudioOptionsFlags f = 0, 
						tAudioHeader* h = NULL )
		: path(pa), volume(v), priority(p), pan(pn), pListener(l),
		payload(pl), flags(f), pRawHeader(h) {}
};

typedef struct taudiostate {
	U8     masterVolume;			

    U8     speakerEnabled;
    U8     useOutDSP;
    U8     useOutEQ;
    U8     srcType;
    U8     srcFilterVersion;

    U8     useOutSoftClipper;
    S16    softClipperPreGainDB;
    S16    softClipperPostGainDB;

    S16    outLevels_ShortTime[2];
    S16    outLevels_LongTime [2];
    S16    outLevels_Max      [2];
    S16    outLevels_MaxCount [2];
    U8     computeLevelMeters;

    U8     readInSoundFile;
    U8     writeOutSoundFile;
#define kAudioState_MaxFileNameLength 100
    char   inSoundFilePath [kAudioState_MaxFileNameLength];
    char   outSoundFilePath[kAudioState_MaxFileNameLength];

    long   systemSamplingFrequency;
    long   outFileBufferCount;
    long   outBufferLength;  // Dunno?!? Words? Frames?

    U8     headroomBits;
    float  channelGainDB;
    float  masterGainf[2];
} tAudioState; 

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
	
	tAudioPanInfo(   tAudioID				i  = 0,
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
	Boolean				noDoneMsg;

	tAudioStopAudioInfo( tAudioID i = -1, 
						 Boolean s = false )
		: id(i), noDoneMsg(s) {}
};

// kAudioCmdMsgTypeMidiCommand
struct tAudioMidiCommandInfo {
	U8					cmd;
	U8					data1;
	U8				 	data2;

	tAudioMidiCommandInfo(U8 c  = 0, 
						  U8 d1 = 0, 
						  U8 d2 = 0 )
		: cmd(c), data1(d1), data2(d2) {}
};

// kAudioCmdMsgTypeMidiNoteOn
struct tAudioMidiNoteInfo {
	U8					channel;
	U8					note;
	U8				 	velocity;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;

	tAudioMidiNoteInfo( U8 c = 0, 
						U8 n = 0, 
						U8 v = 0, 
						tAudioPayload pl = 0, 
						tAudioOptionsFlags f = 0 )
		: channel(c), note(n), velocity(v), 
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
	Boolean				noDoneMsg;

	tAudioStopMidiFileInfo( tMidiPlayerID i = -1, Boolean s = false )
		: id(i), noDoneMsg(s) {}
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
	tMidiPlayerInstrument instrument;
	S8 					tempo;

	tAudioMidiFilePlaybackParams( tMidiPlayerID	i = 0,
						tMidiTrackBitMask tbm = 0,
						S8 tsp = 0,
						tMidiPlayerInstrument mi = 0,
						S8 t = 0 )
		: id(i),  trackBitMask(tbm), transposeAmount(tsp), instrument(mi),
		tempo(t) {}
};

LF_END_BRIO_NAMESPACE()
#endif /*LF_BRIO_AUDIOTYPESPRIV_H_*/
