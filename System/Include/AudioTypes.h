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
	(kAudioLoopEndEvent)				\
	(kAudioMidiEvent)					\
	(kAudioTimeEvent)

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
typedef U8  	tAudioCuePoint;		///< Audio cue point (deprecated)
typedef U32		tAudioID;			///< Unique ID for the audio to allow tracking 
									///< of it through the audio subsystem 
typedef U32		tAudioPayload;		///< User payload to be sent with the done message
typedef U8		tAudioPriority;		///< Priority of the audio asset, 0-255. 
									///< 0 is lowest priority, 255 highest.
#define kNoAudioID	kU32Max	        ///< ID returned on failure

#define kAudioPanDefault		0	///< pan center left/right
#define kAudioPanMin		  (-100)
#define kAudioPanMax			100

#define kAudioVolumeDefault		100
#define kAudioVolumeMin			0
#define kAudioVolumeMax			100

#define kAudioDoneMsgBit 	    0x1 // Legacy definition.  KEEP.
#define kAudioDoneMsgBitMask 	0x1
#define kAudioLoopedBitMask 	0x2
#define kAudioLoopEndBitMask 	0x4
/// Audio options flags used for StartAudio and StartMidiFile
enum {
	kAudioOptionsNoDoneMsg				= 0x00,	///< No audio options specified
	kAudioOptionsDoneMsgAfterComplete	= 0x01,	///< Send after job (play + looping) is complete 
	kAudioOptionsLooped					= 0x02,	///< Repeat # times, specified in payload parameter
	kAudioOptionsLoopEndMsg         	= 0x04,	///< Send LoopEnd message at end of each loop
	kAudioOptionsMidiEvent				= 0x08,	///< Send MidiEvent message upon MIDI file meta events
	kAudioOptionsTimeEvent				= 0x10, ///< Send TimeEvent messages at periodic 'payload' intervals
};
#define kAudioOptionsNone 	0x0  ///< No options specified
typedef U32 tAudioOptionsFlags; 

#define kAudioRepeat_Infinite 	0x40000000  ///< Repeat forever.  Use for the payload parameter.

/// Prototype for function to get next chunk of stereo audio stream data
typedef Boolean (*tGetStereoAudioStreamFcn)(U16 numSamples, S16 *pStereoBuffer); 

//==============================================================================
// MIDI audio types 
//==============================================================================
typedef U8		tMidiPlayerID;		
typedef U32		tMidiPlayerInstrument;			
typedef U32		tMidiTrackBitMask;	///< A bit map of MIDI tracks (deprecated)
#define kAllTracksOfMIDI	(~0)	///< Indicates a "1" for all MIDI tracks
#define kNoMidiID	kU8Max	        ///< ID returned on failure

typedef U32		tMidiProgramList;	///< (deprecated)

//==============================================================================
// Defines for audio resources 
//==============================================================================

/// Standard header for raw Brio audio resources 
#define kAudioHeader_StereoBit 	0x1
struct tAudioHeader {
	U32				offsetToData;		///< Offset from start of header to
										///< start of data (12, which is size of this struct)
	U16				flags;				///< (Bit0: 0=mono, 1=stereo)
	U16				sampleRate;			///< Hz			
	U32				dataSize;			///< Bytes
};

//==============================================================================
// Audio priority policies
//==============================================================================

/// \page PriorityPolicies "About Priority Policies"
///
/// Channels and players are limited resources.  When all channels are full, or
/// the maximum number of players are active, calls to StartAudio will fail
/// unless suitable resources can be freed.  Some programmers may wish to set
/// priorities such that some audio can be halted to free resources for a new
/// audio.  The policy of when a player can be stopped to make room for a new
/// player is called the priority policy.  The priority policy can be changed on
/// the fly by calling SetPriorityPolicy.  The policies are described
/// below:
///
/// kAudioPriorityPolicyNone: This is the default priority policy that is used
/// if no call is made to SetPriorityPolicy.  It means that priority is totally
/// ignored.  Under this policy, if a user calls StartAudio and all channels are
/// full or the maximum number of players are in use, StartAudio will fail.
///
/// kAudioPriorityPolicySimple: This priority policy operates in the following
/// way:
///
/// Case 0: The system has a free channel and the max number of players of the
/// desired type has not been exceeded.  In this case, a call to StartAudio
/// succeeds and no players are halted.
///
/// Case 1: The system has a free channel, but the maximum number of players are
/// active.  In this case, the players will be ordered in a list by priority.
/// The system will search the list from lowest to highest priority.  The first
/// player that it encounters whose priority is LESS THAN OR EQUAL TO the new
/// player's be halted and the call to StartAudio will succeed.  If no player
/// has such a priority, StartAudio will fail.
///
/// Case 2: The system does not have a free channel, and the maximum number of
/// players of the desired type has not been exceeded.  In this case, the
/// players connected to the channels will be ordered in a list by priority.
/// The system will search the list from lowest to highest priority.  The first
/// player that it encounters whose priority is LESS THAN OR EQUAL TO the new
/// player's will be halted and the call to StartAudio will succeed.  If no
/// player has such a priority, StartAudio will fail.
/// 
/// Case 3: The system does not have a free channel, and the maximum number of
/// players of the desired type are active.  In this case, the priority will be
/// evaluated FIRST for the player as described in Case 1, then for the channel
/// as described in Case 2.
typedef U32 tPriorityPolicy;
#define kAudioPriorityPolicyNone 0
#define kAudioPriorityPolicySimple 1

//==============================================================================
// Audio message data payload types
//==============================================================================
/// Audio message posted for kAudioOptionsDoneMsgAfterComplete flag
struct tAudioMsgAudioCompleted {
	tAudioID			audioID;	///< player ID
	tAudioPayload		payload;	///< payload option
	U8					count;		///< playback count
};

/// Audio message posted for kAudioOptionsLooped flag
struct tAudioMsgLoopEnd {
	tAudioID			audioID;	///< player ID
	tAudioPayload		payload;	///< loop count option
	U8					count;		///< playback count
};

/// \deprecated
struct tAudioMsgMidiCompleted {
	tMidiPlayerID		midiPlayerID;
	tAudioPayload		payload;
	U8					count;
};

/// \deprecated
struct tAudioMsgCuePoint {
	tAudioID			audioID;
	tAudioPayload		payload;
	tAudioCuePoint		cuePoint;
};

/// \deprecated
struct tAudioMsgMidiEvent {
	tMidiPlayerID		midiPlayerID;
	tAudioPayload		payload;
	U8					trackIndex;
	U8					metaEventType;
	const char*			addrFileImage;
	int					numChars;
	void*				userData;
};

/// Audio message posted for kAudioOptionsTimeEvent flag
struct tAudioMsgTimeEvent {
	tAudioID			audioID;		///< player ID
	tAudioPayload		payload;		///< time interval (msec)
	U32					playtime;		///< playback time (msec)
	U32					realtime;		///< system time (msec)
};

/// Union of all possible Audio message types
union tAudioMsgData {
	tAudioMsgAudioCompleted		audioCompleted;	///< kAudioOptionsDoneMsgAfterComplete message
	tAudioMsgMidiCompleted		midiCompleted;	///< (deprecated)
	tAudioMsgCuePoint			audioCuePoint;	///< (deprecated)
	tAudioMsgLoopEnd            loopEnd;		///< kAudioOptionsLooped message
	tAudioMsgMidiEvent			midiEvent;		///< (deprecated)
	tAudioMsgTimeEvent			timeEvent;		///< kAudioOptionsTimeEvent message
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
	CAudioEventMessage( const tAudioMsgMidiEvent&      data );
	CAudioEventMessage( const tAudioMsgTimeEvent&      data );

	virtual U16	GetSizeInBytes() const;

	tAudioMsgData	audioMsgData;
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOTYPES_H

// EOF	
