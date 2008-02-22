#ifndef LF_BRIO_AUDIOTYPESPRIV_H_
#define LF_BRIO_AUDIOTYPESPRIV_H_
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// AudioTypesPriv.h
//
//		Defines private types used by AudioMPI
//
//==============================================================================

#include <SystemTypes.h>
#include <AudioTypes.h>
#include <EventListener.h>
LF_BEGIN_BRIO_NAMESPACE()

#define kAudio_Pan_Max	  100
#define kAudio_Pan_Min	(-100)
#define kAudio_Pan_Default 0

#define kAudio_Volume_Max	 100
#define kAudio_Volume_Min	   0
#define kAudio_Volume_Default 100

struct tAudioStartAudioInfo { 
	const CPath*		path;
	U8					volume;
	tAudioPriority		priority;
	S8					pan;
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
	U8	   masterVolume;			

	U8	   speakerEnabled;
	U8	   useOutDSP;
	U8	   useOutEQ;
	U8	   srcType;
	U8	   srcFilterVersion;

	U8	   useOutSoftClipper;
	S16	   softClipperPreGainDB;
	S16	   softClipperPostGainDB;

	S16	   outLevels_ShortTime[2];
	S16	   outLevels_LongTime [2];
	S16	   outLevels_Max	  [2];
	S16	   outLevels_MaxCount [2];
	U8	   computeLevelMeters;

	U8	   readInSoundFile;
	U8	   writeOutSoundFile;
#define kAudioState_MaxFileNameLength 100
	char   inSoundFilePath [kAudioState_MaxFileNameLength];
	char   outSoundFilePath[kAudioState_MaxFileNameLength];

	long   systemSamplingFrequency;
	long   outFileBufferCount;
	long   outBufferLength;	 // Dunno?!? Words? Frames?

	U8	   headroomBits;
	float  channelGainDB;
	float  masterGainf[2];
} tAudioState; 

struct tAudioStartMidiFileInfo {
	tMidiPlayerID		id;
	const CPath*		path;
	U8*					pMidiFileImage;		// MIDI file loaded into RAM
	U32					imageSize;			// Size of MIDI file in RAM
	U8					volume;
	tAudioPriority		priority;
	const IEventListener* pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;

};

struct tAudioStopMidiFileInfo {
	tMidiPlayerID		id;
	Boolean				noDoneMsg;

	tAudioStopMidiFileInfo( tMidiPlayerID i = -1, Boolean s = false )
		: id(i), noDoneMsg(s) {}
};

LF_END_BRIO_NAMESPACE()
#endif /*LF_BRIO_AUDIOTYPESPRIV_H_*/
