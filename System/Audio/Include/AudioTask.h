#ifndef LF_BRIO_AUDIOTASK_H
#define LF_BRIO_AUDIOTASK_H

#include <SystemTypes.h>
#include <StringTypes.h>
#include <KernelTypes.h>
#include <KernelMPI.h>
#include <DebugMPI.h>
#include <EventListener.h>

#include <AudioConfig.h>
#include <AudioPlayer.h>

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

#define kNoChannelAvail			255		// Index returned when no channel is available

struct tAudioPlayerNode {
	CAudioPlayer			*pPlayer;		// Pointer to the audio player
	struct tAudioPlayerNode	*pNext;			// Pointer to the next node in the list
	tAudioPlayerNode(CAudioPlayer *player, struct tAudioPlayerNode *next) { pPlayer = player; pNext = next; }
};

//==============================================================================
// Bit Masks used within AudioEventBits
//==============================================================================
#define kAudioDoneBit			0x01
#define kAudioDecodeGoBit		0x02
#define kAudioXXXBit			0x04
#define kAllAudioEvents			0x07

tErrType InitAudioTask( void );
void DeInitAudioTask( void );

#endif /* LF_BRIO_AUDIOTASK_H */
