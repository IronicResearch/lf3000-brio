#ifndef LF_BRIO_AUDIOCONFIG_H
#define LF_BRIO_AUDIOCONFIG_H

// Fundamental assumptions
#define	kAudioNumMixerChannels		4		// Number of mixer channels max.

#define kAudioNumOutputChannels		2		// stereo output
#define kAudioBytesPerSample		2		// 16 bit
#define	kAudioSampleRate			32000	// obvious
#define kAudioTickInMS				20		// audio system interrupt rate 

#define kAudioMgrTaskPriority		10
#define kAudioCodecTaskPriority		11

// Derived values
#define kAudioBytesPerMonoFrame		(kAudioBytesPerSample)
#define kAudioBytesPerStereoFrame	(kAudioBytesPerSample * 2)
#define kAudioFramesPerMS			(kAudioSampleRate / 1000)
//#define kAudioFramesPerBuffer		(kAudioFramesPerMS * kAudioTickInMS)

// Emulation on Linux desktop likes smaller buffers or it glitches
// The ARM target likes larger buffers for some reason.
#ifdef EMULATION
#define kAudioFramesPerBuffer		256
#else
#define kAudioFramesPerBuffer		512
#endif

#define kAudioSamplesPerStereoBuffer (kAudioFramesPerBuffer * kAudioNumOutputChannels)
#define kAudioOutBufSizeInBytes		(kAudioFramesPerBuffer * kAudioBytesPerStereoFrame)
#define kAudioOutBufSizeInWords		(kAudioOutBufSizeInBytes / kAudioBytesPerSample)

#endif /* LF_BRIO_AUDIOCONFIG_H */
