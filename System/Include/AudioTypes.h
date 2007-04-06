#ifndef LF_BRIO_AUDIOTYPES_H
#define LF_BRIO_AUDIOTYPES_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
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
#include <CoreTypes.h>
//#include <RsrcTypes.h>
//#include <RsrcMgrMPI.h>
//#include <EventListener.h>
#include <EventMessage.h>

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
	kAudioRsrcStreaming
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
	U16				sampleRateInHz;		// Sample rate in Hz			
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
/*

//==============================================================================
// Class:
//		IAudioEffect
//
// Description:
//		Interface class for audio effects. 
//==============================================================================
// Audio effect mode
enum {
	kAudioEffectPassThroughMode,		// By-pass the effect
	kAudioEffectProcessingMode			// Process the effect
};
typedef U8		tAudioEffectMode;

// Audio Effect interface class
// All Audio effects should be implemented with a class derived from this class 
class IAudioEffect {
public:
	IAudioEffect() { };
	virtual ~IAudioEffect() { };

	// Get/Set the audio effect mode for a class object
	virtual tAudioEffectMode	GetEffectMode() = 0;
	virtual void				SetEffectMode(tAudioEffectMode mode) = 0;
	// Process the effect on the given stereo buffer
	virtual void				ProcessEffect(U32 numSamples, S16 *pStereoBuffer) = 0;

	// Each derived class will probably provide a function Configure()
	// to configure the parameters for the effect, but it is not a requirement.
};

//==============================================================================
// Class:
//		CAudioEffectsProcessor
//
// Description:
//		Class that maintains a list of audio effects to be processed. 
//==============================================================================
class CAudioEffectsProcessorImpl;
class CAudioEffectsProcessor {
public:
	CAudioEffectsProcessor();
	~CAudioEffectsProcessor();

	tErrType	Init();
	tErrType	DeInit();

	// Process the list of effects on the given stereo buffer
	tErrType	ProcessAudioEffects(U32 numSamples, S16 *pStereoBuffer);

	// Add/Remove audio effects from the list
	tErrType	AddAudioEffect(IAudioEffect *pEffect);
	tErrType	AddAudioEffectAfter(IAudioEffect *pToAdd, IAudioEffect *pAfter);
	tErrType	AddAudioEffectBefore(IAudioEffect *pToAdd, IAudioEffect *pBefore);
	tErrType	AddIthAudioEffect(IAudioEffect *pToAdd, U8 iEffect);
	tErrType	RemoveAudioEffect(IAudioEffect *pEffect);
	tErrType	RemoveIthAudioEffect(U8 iEffect);

	// Get a pointer to the ith audio effect in the list
	IAudioEffect *	GetIthAudioEffect(U8 iEffect);

private:
	class CAudioEffectsProcessorImpl	*mpImpl;
};

*/

#endif		// LF_BRIO_AUDIOTYPES_H

// EOF	
