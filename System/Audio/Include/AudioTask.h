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
LF_BEGIN_BRIO_NAMESPACE()

//	Enum to indicate mono or stereo data
enum {
	kAudioSoundMono = 0,
	kAudioSoundStereo 
};
typedef U8  tAudioSound;


#define kNoChannelAvail			255		// Index returned when no channel is available

tErrType InitAudioTask( void );
void DeInitAudioTask( void );

LF_END_BRIO_NAMESPACE()
#endif /* LF_BRIO_AUDIOTASK_H */
