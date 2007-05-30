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
#define kNoAudioID			(~0)	// ID returned when the system is unable to
									// play the audio

#define kAudioDoneMsgBit	0x01
// Audio options flags
// Bits 0-1 refer to done messages 
enum {
	kAudioOptionsNoDoneMsg				= 0x00,	// No done message
	kAudioOptionsDoneMsgAfterComplete	= 0x01,	// Done message after audio is complete 
	kAudioOptionsDoneMsgAfterEvery		= 0x03	// Done message after every audio in array
};
typedef U32 tAudioOptionsFlags; 

// Prototype for the function to call to get the next chunk of stereo audio stream data
typedef Boolean (*tGetStereoAudioStreamFcn)(U16 numSamples, S16 *pStereoBuffer); 

//==============================================================================
// MIDI audio types 
//==============================================================================
typedef U8		tMidiID;			// MidiPlayer ID
typedef U32		tMidiInstr;			// MidiPlayer intrument
typedef U32		tMidiTrackBitMask;	// A bit map of Midi tracks
#define kAllTracksOfMIDI	(~0)	// Indicates a "1" for all Midi tracks

//==============================================================================
// AudioRsrcType
//	
//==============================================================================
//#define kFirstAudioRsrcType MakeRsrcType(kSystemRsrcGroupAudio, kFirstRsrcTag)
#define kFirstAudioRsrcType		0x10001C01

enum {
	kAudioRsrcACS = kFirstAudioRsrcType,
	kAudioRsrcAdpcm,
	kAudioRsrcGen2,
	kAudioRsrcMIDI,
	kAudioRsrcRaw,
	kAudioRsrcFlashStream,
	kAudioRsrcSpeechStream,
	kAudioRsrcStreaming,
	kAudioRsrcWav
};

//==============================================================================
// Defines for audio resources 
//==============================================================================
// Standard header for all audio resources 
struct tAudioHeader {
	U32				offsetToData;		// Offset from the start of the header to
										// the start of the data (std is 16)
	tRsrcType		type;				// AudioRsrcType
	U16				flags;				// Bit mask of audio flags
										// (Bit0: 0=mono, 1=stereo)
	U16				sampleRate;			// Sample rate in Hz			
	U32				dataSize;			// Data size in bytes
};

// Extended header for those audio resources where the dataSize is less than the
// number of bytes that have been allocated to hold the data  
//struct tAudioHeaderDynamic : public tAudioHeader {
//	U32				allocatedSize;		// Number of bytes allocated for the data
//};

//==============================================================================
// Audio message data payload types
//==============================================================================
//	kAudioMsgCompleted
struct tAudioMsgDataCompleted {
	tAudioID			audioID;
	tAudioPayload		payload;
	U8					count;
};

//	kAudioMsgCuePoint
struct tAudioMsgDataCuePoint {
	tAudioID			audioID;
	tAudioPayload		payload;
	tAudioCuePoint		cuePoint;
};


union tAudioMsgData {
	tAudioMsgDataCompleted		audioCompleted;
	tAudioMsgDataCuePoint		audioCuePoint;
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
	CAudioEventMessage( const tAudioMsgDataCompleted& data );
	virtual U16	GetSizeInBytes() const;

	tAudioMsgData	audioMsgData;
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOTYPES_H

// EOF	
