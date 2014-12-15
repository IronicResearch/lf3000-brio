#include <IAudioEffect.h>

// Note.  This is pseudo code.  It does not work yet.

//	Output EQ parameters
#define kAudioMixer_MaxEQBands	3
long outEQ_BandCount_;
EQ outEQ_[kAudioMixer_MaxOutChannels][kAudioMixer_MaxEQBands];

OutputEqualizer::OutputEqualizer():IAudioEffect()
{
	// Configure output Equalizer
	outEQ_BandCount_ = 1;
	for (j = 0; j < kAudioMixer_MaxEQBands; j++)
	{
		DefaultEQ(&outEQ_[ch][j]);
	}
	
	// Apply equalizer
	outEQ_BandCount_ = 3;
	SetEQ_Parameters(&outEQ_[ch][0], 1000.0f, 1.0f, 0.0f, kEQ_Mode_Parametric);
	SetEQ_Parameters(&outEQ_[ch][1], 2000.0f, 1.0f, 0.0f, kEQ_Mode_Parametric);
	SetEQ_Parameters(&outEQ_[ch][2], 4000.0f, 1.0f, 0.0f, kEQ_Mode_Parametric);
	
	for (long j = 0; j < outEQ_BandCount_; j++)
	{
		SetEQ_SamplingFrequency(&outEQ_[ch][j], samplingFrequency_);
		PrepareEQ(&outEQ_[ch][j]);	  
	}

	// update the equalizer.  This would have to be called whenever the sample
	// rate was changed, etc.
	for (i = 0; i < outEQ_BandCount_; i++)
		UpdateEQ(&outEQ_[ch][i]);

	// Reset equalizer.  Not sure why this is being called after update.
	for (i = 0; i < outEQ_BandCount_; i++)
		ResetEQ(&outEQ_[ch][i]);

}

OutputEqualizer::ProcessEffect(U32 numSamples, S16 *pStereoBuffer)
{
	// Compute Output Equalizer
	for (long j = 0; j < outEQ_BandCount_; j++)
	{
		ComputeEQi(pIn, pOut, numFrames, &outEQ_[ch][j]);
		pOut = pIn;
	}
}
