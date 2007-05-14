#ifndef LF_BRIO_AUDIOCONFIG_H
#define LF_BRIO_AUDIOCONFIG_H

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

#endif /* LF_BRIO_AUDIOCONFIG_H */
