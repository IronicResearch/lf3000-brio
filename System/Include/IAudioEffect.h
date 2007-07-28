#ifndef LF_BRIO_AUDIO_EFFECT_H
#define LF_BRIO_AUDIO_EFFECT_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		IAudioEffect.h
//
// Description:
// Audio Effect interface class
// All Audio effects should be implemented with a class derived from this class 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


// Audio effect mode
enum {
	kAudioEffectPassThroughMode,		// By-pass the effect
	kAudioEffectProcessingMode			// Process the effect
};
typedef U8		tAudioEffectMode;

class IAudioEffect {
public:
	IAudioEffect() { };
	virtual ~IAudioEffect() { };

	// Get/Set the audio effect mode for a class object
	virtual tAudioEffectMode	GetEffectMode() = 0;
	virtual void			SetEffectMode(tAudioEffectMode mode) = 0;
	// Process the effect on the given stereo buffer
	virtual void			ProcessEffect(U32 numSamples, S16 *pStereoBuffer) = 0;

	// Each derived class will probably provide a function Configure()
	// to configure the parameters for the effect, but it is not a requirement.
};

LF_END_BRIO_NAMESPACE()
#endif /*LF_BRIO_AUDIO_EFFECT_H*/
// EOF	
