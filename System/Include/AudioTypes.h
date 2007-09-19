#ifndef LF_BRIO_AUDIOTYPES_H
#define LF_BRIO_AUDIOTYPES_H
//==============================================================================
// Copyright (c) 2LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioTypes.h
//
// Description:
//		Type definitions for the Brio audio system.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <GroupEnumeration.h>
//#include <RsrcTypes.h>
//#include <RsrcMgrMPI.h>
//#include <EventListener.h>
#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Audio events
//==============================================================================
#define AUDIO_EVENTS					\
	(kAudioCompletedEvent)				\
	(kMidiCompletedEvent)				\
	(kAudioCuePointEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupAudio), AUDIO_EVENTS)

const tEventType kAllAudioEvents = AllEvents(kGroupAudio);


//==============================================================================	   
// Audio errors
//==============================================================================
#define AUDIO_ERRORS			\
	(kAudioCreateTaskErr)		\
	(kAudioCreateEventErr)		\
	(kAudioCreateQueueErr)		\
	(kAudioNullContextErr)		\
	(kAudioNoChannelAvailErr)	\
	(kAudioNoDataAvailErr)		\
	(kAudioNoMoreDataErr)		\
	(kAudioInvalid)				\
	(kAudioMidiErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupAudio), AUDIO_ERRORS)


//==============================================================================
// Basic audio types
//==============================================================================
typedef U8  	tAudioCuePoint;		// Audio cue point 
typedef U32		tAudioID;			// Unique ID for the audio to allow tracking 
									// of it through the audio subsystem 
typedef U32		tAudioPayload;		// User payload to be sent with the done message
typedef U8		tAudioPriority;		// Priority of the audio asset, 0-255. 
									// 0 is lowest priority, 255 highest.
#define kNoAudioID			kU32Max	// ID returned when the system is unable to
									// play the audio

#define kAudioDoneMsgBit	0x01
// Audio options flags
// Bits 0-1 refer to done messages 
enum {
	kAudioOptionsNoDoneMsg				= 0x00,	// No done message
	kAudioOptionsDoneMsgAfterComplete	= 0x01	// Done message after audio is complete 
};
typedef U32 tAudioOptionsFlags; 

// Prototype for the function to call to get the next chunk of stereo audio stream data
typedef Boolean (*tGetStereoAudioStreamFcn)(U16 numSamples, S16 *pStereoBuffer); 

//==============================================================================
// MIDI audio types 
//==============================================================================
typedef U8		tMidiPlayerID;		// MidiPlayer ID
typedef U32		tMidiInstr;			// MidiPlayer intrument
typedef U32		tMidiTrackBitMask;	// A bit map of Midi tracks
#define kAllTracksOfMIDI	(~0)	// Indicates a "1" for all Midi tracks

typedef U32		tMidiProgramList;	// TODO: ?


//==============================================================================
// Defines for audio resources 
//==============================================================================

/// Audio resource types
const tRsrcType kAudioRsrcMIDI = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupAudio, 1);
const tRsrcType kAudioRsrcOggVorbis = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupAudio, 2);
const tRsrcType kAudioRsrcRaw = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupAudio, 3);
const tRsrcType kAudioRsrcOggTheora = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupAudio, 4);

// Standard header for raw Brio audio resources 
struct tAudioHeader {
	U32				offsetToData;		// Offset from the start of the header to
										// the start of the data (std is 16)
	tRsrcType		type;				// AudioRsrcType
	U16				flags;				// Bit mask of audio flags
										// (Bit0: 0=mono, 1=stereo)
	U16				sampleRate;			// Sample rate in Hz			
	U32				dataSize;			// Data size in bytes
};


//==============================================================================
// Audio message data payload types
//==============================================================================
//	kAudioMsgCompleted
struct tAudioMsgAudioCompleted {
	tAudioID			audioID;
	tAudioPayload		payload;
	U8					count;
};

struct tAudioMsgMidiCompleted {
	tMidiPlayerID		midiPlayerID;
	tAudioPayload		payload;
	U8					count;
};

//	kAudioMsgCuePoint
struct tAudioMsgCuePoint {
	tAudioID			audioID;
	tAudioPayload		payload;
	tAudioCuePoint		cuePoint;
};


union tAudioMsgData {
	tAudioMsgAudioCompleted		audioCompleted;
	tAudioMsgMidiCompleted		midiCompleted;
	tAudioMsgCuePoint			audioCuePoint;
};



//==============================================================================
// Class:
//		CAudioEventMessage
//
// Description:
//		Class that describes the format of all Audio Event Messages. 
//==============================================================================

class CAudioEventMessage : public IEventMessage 
{
public:
	CAudioEventMessage( const tAudioMsgAudioCompleted& data );
	CAudioEventMessage( const tAudioMsgMidiCompleted& data );
	CAudioEventMessage( const tAudioMsgCuePoint& data );
	virtual U16	GetSizeInBytes() const;

	tAudioMsgData	audioMsgData;
};


LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOTYPES_H

// EOF	
