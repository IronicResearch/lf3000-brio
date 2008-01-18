#ifndef LF_BRIO_AUDIOTYPES_H
#define LF_BRIO_AUDIOTYPES_H
//==============================================================================
// Copyright (c) 2LeapFrog Enterprises, Inc.
//==============================================================================
//
// AudioTypes.h
//
// Type definitions for the Brio audio system.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <GroupEnumeration.h>
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
	(kAudioCuePointEvent)               \
	(kAudioLoopEndEvent)		    

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
	(kAudioPlayerCreateErr)	    \
	(kAudioNoDataAvailErr)		\
	(kAudioNoMoreDataErr)		\
	(kAudioInvalid)				\
	(kAudioMidiErr)             \
	(kAudioMidiUnavailable)

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
#define kNoAudioID	kU32Max	        // ID returned on failure

#define kAudioDoneMsgBit 	    0x1 // Legacy definition.  KEEP.
#define kAudioDoneMsgBitMask 	0x1
#define kAudioLoopedBitMask 	0x2
#define kAudioLoopEndBitMask 	0x4
// Audio options flags used for StartAudio and StartMidiFile
enum {
	kAudioOptionsNoDoneMsg				= 0x00,	// No audio options specified
	kAudioOptionsDoneMsgAfterComplete	= 0x01,	// Send after job (play + looping) is complete 
	kAudioOptionsLooped					= 0x02,	// Repeat # times, specified in payload parameter
	kAudioOptionsLoopEndMsg         	= 0x04	// Send LoopEnd message at end of each loop  
};
#define kAudioOptionsNone 	0x0  // No options specified
typedef U32 tAudioOptionsFlags; 

#define kAudioRepeat_Infinite 	0x40000000  // Repeat forever.  Use for the payload parameter.

// Prototype for function to get next chunk of stereo audio stream data
typedef Boolean (*tGetStereoAudioStreamFcn)(U16 numSamples, S16 *pStereoBuffer); 

//==============================================================================
// MIDI audio types 
//==============================================================================
typedef U8		tMidiPlayerID;		
typedef U32		tMidiPlayerInstrument;			
typedef U32		tMidiTrackBitMask;	// A bit map of MIDI tracks
#define kAllTracksOfMIDI	(~0)	// Indicates a "1" for all MIDI tracks
#define kNoMidiID	kU8Max	        // ID returned on failure

typedef U32		tMidiProgramList;	// TODO: ?

//==============================================================================
// Defines for audio resources 
//==============================================================================

// Standard header for raw Brio audio resources 
#define kAudioHeader_StereoBit 	0x1
struct tAudioHeader {
	U32				offsetToData;		// Offset from start of header to
										// start of data (16, which is size of this struct)
	U16				flags;				// (Bit0: 0=mono, 1=stereo)
	U16				sampleRate;			// Hz			
	U32				dataSize;			// Bytes
};

//==============================================================================
// Audio message data payload types
//==============================================================================
struct tAudioMsgAudioCompleted {
	tAudioID			audioID;
	tAudioPayload		payload;
	U8					count;
};

struct tAudioMsgLoopEnd {
	tAudioID			audioID;
	tAudioPayload		payload;
	U8					count;
};

struct tAudioMsgMidiCompleted {
	tMidiPlayerID		midiPlayerID;
	tAudioPayload		payload;
	U8					count;
};

struct tAudioMsgCuePoint {
	tAudioID			audioID;
	tAudioPayload		payload;
	tAudioCuePoint		cuePoint;
};

union tAudioMsgData {
	tAudioMsgAudioCompleted		audioCompleted;
	tAudioMsgMidiCompleted		midiCompleted;
	tAudioMsgCuePoint			audioCuePoint;
	tAudioMsgLoopEnd            loopEnd;
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
	CAudioEventMessage( const tAudioMsgMidiCompleted&  data );
	CAudioEventMessage( const tAudioMsgCuePoint&       data );
	CAudioEventMessage( const tAudioMsgLoopEnd&        data );

	virtual U16	GetSizeInBytes() const;

	tAudioMsgData	audioMsgData;
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOTYPES_H

// EOF	
