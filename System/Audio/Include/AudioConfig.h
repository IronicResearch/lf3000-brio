#ifndef LF_BRIO_AUDIOCONFIG_H
#define LF_BRIO_AUDIOCONFIG_H

// Fundamental assumptions
#define	kAudioNumMixerChannels		4		// Number of mixer channels max.

#define kAudioNumOutputChannels		2		// stereo output
#define kAudioBytesPerSample		2		// 16 bit
#define	kAudioSampleRate			44100	// obvious
#define kAudioTickInMS				20		// audio system interrupt rate 

#define kAudioMgrTaskPriority		10
#define kAudioCodecTaskPriority		11

// Derived values
#define kAudioBytesPerMonoFrame		(kAudioBytesPerSample)
#define kAudioBytesPerStereoFrame	(kAudioBytesPerSample * 2)
#define kAudioFramesPerMS			(kAudioSampleRate / 1000)
//#define kAudioFramesPerBuffer		(kAudioFramesPerMS * kAudioTickInMS)
#define kAudioFramesPerBuffer		256
#define kAudioSamplesPerStereoBuffer	(kAudioFramesPerBuffer * kAudioNumOutputChannels)
#define kAudioOutBufSizeInBytes		(kAudioFramesPerBuffer * kAudioBytesPerStereoFrame)
#define kAudioOutBufSizeInWords		(kAudioOutBufSizeInBytes / kAudioBytesPerSample)

#endif /* LF_BRIO_AUDIOCONFIG_H */
