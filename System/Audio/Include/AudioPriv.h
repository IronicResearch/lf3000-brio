#ifndef LF_BRIO_AUDIOPRIVATE_H
#define LF_BRIO_AUDIOPRIVATE_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioPriv.h
//
// Description:
//		Defines the private, hidden data structures used by AudioMgrMPI. 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <CoreModule.h>
#include <KernelMPI.h>
#include <DebugMPI.h>

//#include <RsrcTypes.h>
//#include <AudioTypes.h>
//#include <AudioRsrcs.h>
//#include <AudioMixer.h>
//#include <EventHandler.h>

class IEventListener;

typedef U32 tRsrcHndl;

// Constants
const CString	kAudioModuleName	= "Audio";
const tVersion	kAudioModuleVersion	= MakeVersion(0,1);

//	Enum to indicate mono or stereo data
enum {
	kAudioSoundMono = 0,
	kAudioSoundStereo 
};
typedef U8  tAudioSound;

// These enums will be used when processing the commands within an ACS
enum {
	kAudioSeq0Mask	=0x0001,
	kAudioSeq1Mask	=0x0002,
	kAudioSeq2Mask	=0x0004,
	kAudioSeq3Mask	=0x0008,
	kAudioSeq4Mask	=0x0010,
	kAudioSeq5Mask	=0x0020,
	kAudioSeq6Mask	=0x0040,
	kAudioSeq7Mask	=0x0080,
	kAudioSeq8Mask	=0x0100,
	kAudioSeq9Mask	=0x0200,
	kAudioSeq10Mask	=0x0400,
	kAudioSeq11Mask	=0x0800,
	kAudioSeq12Mask	=0x1000,
	kAudioSeq13Mask	=0x2000,
	kAudioSeq14Mask	=0x4000,
	kAudioSeq15Mask	=0x8000
};
typedef U16  tAudioSeqMasks;

enum {
	kAudioSeqNone = 0,
	kAudioSeqReady,
	kAudioSeqActive,
	kAudioSeqSync,
	kAudioSeqDelayTimer,
	kAudioSeqWaitTimer,
	kAudioSeqDone
};
typedef U8  tAudioSeqStatus;

#define kAudioNumOutputChannels	2		// stereo output
#define	kAudioNumMixerChannels	4		// Number of mixer channels max.
#define kAudioBytesPerSample	2		// 16 bit
#define	kAudioSampleRate		44100	// obvious
#define kAudioTickInMS			20		// audio system interrupt rate (from size of DMA buffer)
#define kAudioBufSizeRatio		2		// ??

#define kNumAudioOutBuffer		2		// Number of audio output buffers

#define kAudioBytesPerFrame		(kAudioNumOutputChannels * kAudioBytesPerSample)
#define kAudioFramesPerMS		(kAudioSampleRate / 1000)
#define kAudioFramesPerBuffer	(kAudioFramesPerMS * kAudioTickInMS)

#define kAudioOutBufSizeInBytes	(kAudioFramesPerBuffer * kAudioBytesPerFrame)
#define kAudioOutBufSizeInWords	(kAudioOutBufSizeInBytes / kAudioBytesPerSample)

#define kAudioMgrTaskSize		0x1000
#define kAudioCodecTaskSize		0x1000
#define kAudioMgrTaskPriority	10
#define kAudioCodecTaskPriority	11

#define kNoChannelAvail			255		// Index returned when no channel is available

struct tAudioPlayerNode {
//	CAudioPlayer			*pPlayer;		// Pointer to the audio player
	struct tAudioPlayerNode	*pNext;			// Pointer to the next node in the list
//	tAudioPlayerNode(CAudioPlayer *player, struct tAudioPlayerNode *next) { pPlayer = player; pNext = next; }
};

//==============================================================================
// Bit Masks used within AudioEventBits
//==============================================================================
#define kAudioDoneBit			0x01
#define kAudioDecodeGoBit		0x02
#define kAudioXXXBit			0x04
#define kAllAudioEvents			0x07

//==============================================================================
class CAudioModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CAudioModule();
	virtual ~CAudioModule();
	virtual tErrType	SetDefaultListener( const IEventListener* pListener );
	

	// Overall Audio Control
	virtual tErrType	StartAudio( void );
	virtual tErrType	StopAudio( void );
	virtual tErrType	PauseAudio( void );
	virtual tErrType	ResumeAudio( void );

private:
	CKernelMPI			*KernelMPI;		// For kernel related functions
	CDebugMPI			*DebugMPI;			// For debug out, assert and friends.
//	tAudioRsrcConfig	config;				// Audio rsrc config
//	CAudioMixer			*pAudioMixer;		// Pointer to the global audio mixer
//	CAudioEffectsProcessor	*pAudioEffects;	// Pointer to the global audio effects processor
	tAudioPlayerNode	*pNode;				// Linked list of audio players
	S16					*pAudioOutBuffer;	// Pointer to the audio out buffers
	U16					outSizeInBytes;		// Size of one audio out buffer in bytes
	U16					outSizeInWords;		// Size of one audio out buffer in words
	volatile U8			audioOutIndex;		// Index of the current audio out buffer
	Boolean				audioOutputOn;		// Flag for whether audio out driver is on
	Boolean				audioCodecActive;	// Flag for whether an audio codec is active
	volatile Boolean	audioMgrTaskReady;	// Flag for whether audio manager task is ready
//	tAudioID			nextAudioID;		// Next audioID
	U8					masterVolume;		// Master volume
	U8					numMixerChannels;
	U32					sampleRate;
	const IEventListener*	mpDefaultListener;
	
//	NU_EVENT_GROUP		audioEvents;		// Audio event group
//	NU_QUEUE			audioCmdQueue;		// Audio command message queue
//	NU_TASK             audioMgrTask;		// Audio Mgr task
//	NU_TASK             audioCodecTask;		// Audio Codec Task
};

#endif		// LF_BRIO_AUDIOPRIVATE_H

// EOF	
