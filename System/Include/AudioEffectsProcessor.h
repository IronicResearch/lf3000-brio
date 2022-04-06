#ifndef LF_BRIO_AUDIO_EFFECTS_PROCESSOR_H
#define LF_BRIO_AUDIO_EFFECTS_PROCESSOR_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// 
// File:
//		AudioEffectsProcessor.h
//
// Description:
//		This class maintains a list of audio effects to be processed. 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <IAudioEffect.h>

LF_BEGIN_BRIO_NAMESPACE()

/// \cond UNIMPLEMENTED
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
/// \endcond

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_AUDIO_EFFECTS_PROCESSOR_H

// EOF	
