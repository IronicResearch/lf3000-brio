#ifndef LF_BRIO_AUDIOCONFIG_H
#define LF_BRIO_AUDIOCONFIG_H

// Debug output level
#define kAudioDebugLevel		kDbgLvlImportant
//kDbgLvl Silent,Critical,Important, Valuable, Noteable,Verbose,

#define kPan_Default    0
#define kPan_Min    (-100)
#define kPan_Max      100

#define kVolume_Default  100
#define kVolume_Min        0
#define kVolume_Max      100

// kDecibelToLinearf_0dBf, m3dBf, m6dBf
#define kChannel_HeadroomDB -3.0f  // Decibels, GK FIXX Headroom should be positive number
#define kChannel_Headroomf (kDecibelToLinearf_m3dBf) // DecibelToLinearf(kChannel_HeadroomDB);

// Input streams.  Note each stream contains a left and right channel
#define kAudioMaxMidiStreams	1
#define	kAudioMinVorbisStreams	3
#define	kAudioMaxVorbisStreams	5
#define	kAudioMaxRawStreams		16
#define kAudioMaxMixerStreams	( kAudioMaxMidiStreams   + \
								  kAudioMaxVorbisStreams + \
								  kAudioMaxRawStreams )

// Output Channels, note each channel is monophonic to match portaudio terms
#define kAudioNumOutputChannels		2		// stereo output
#define kAudioBytesPerSample		(sizeof(short))	// 16 bit

#define	kAudioSampleRate			32000	
#define	kAudioSampleRate_Div1		kAudioSampleRate	
#define	kAudioSampleRate_Div2		(kAudioSampleRate/2)	
#define	kAudioSampleRate_Div4		(kAudioSampleRate/4)	

#define kAudioTickInMS				20		// audio system interrupt rate (obselete)

#define kAudioMgrTaskPriority		10
#define kAudioCodecTaskPriority		11

// Derived values
#define kAudioBytesPerMonoFrame		(kAudioBytesPerSample)
#define kAudioBytesPerStereoFrame	(kAudioBytesPerSample * 2)
#define kAudioFramesPerMS			(kAudioSampleRate / 1000)

// Emulation on Linux desktop likes smaller buffers or it glitches
// The ARM target likes larger buffers for some reason.
#if 0 //def EMULATION
#define kAudioFramesPerBuffer		256
#else
#define kAudioFramesPerBuffer		2048
#endif

#define kAudioSamplesPerStereoBuffer (kAudioFramesPerBuffer * kAudioNumOutputChannels  )
#define kAudioOutBufSizeInBytes		 (kAudioFramesPerBuffer * kAudioBytesPerStereoFrame)
#define kAudioOutBufSizeInWords		(kAudioOutBufSizeInBytes / kAudioBytesPerSample)

#endif /* LF_BRIO_AUDIOCONFIG_H */
