#include <IAudioEffect.h>

// Note.  This is pseudo code.  It does not work yet.

// Level Meter data
S16 outLevels_ShortTime[2];
S16 outLevels_LongTime [2];
S16 temp_ShortTime	   [2];
S32 longTimeHoldCounter;
S32 longTimeHoldInterval;
S32 shortTimeCounter;
S32 shortTimeInterval;
float longTimeDecayF;
S16	  longTimeDecayI;

S16	   outLevels_ShortTime[2];
S16	   outLevels_LongTime [2];
S16	   outLevels_Max	  [2];
S16	   outLevels_MaxCount [2];
U8	   computeLevelMeters;



LevelMeter::LevelMeter():IAudioEffect()
{
	// Set up level meters
	for (ch = kLeft; ch <= kRight; ch++)
	{
		outLevels_ShortTime[ch] = 0;
		outLevels_LongTime [ch] = 0;
		temp_ShortTime	   [ch] = 0;
	}

	for (long ch = kLeft; ch <= kRight; ch++)
	{
		d->outLevels_Max	 [ch] = 0;
		d->outLevels_MaxCount[ch] = 0;
		
		d->outLevels_ShortTime[ch] = outLevels_ShortTime[ch];
		d->outLevels_LongTime [ch] = outLevels_LongTime [ch];
	}

	U32 shortTimeRateHz = 12;
	U32 longTimeRateHz	=  1;
	shortTimeCounter	= 0;
	longTimeHoldCounter = 0;
	shortTimeInterval	 = kAudioSampleRate/(shortTimeRateHz * numFramesPerBuffer);
	longTimeHoldInterval = shortTimeInterval/longTimeRateHz;
	
	float rateF = ((float)kAudioSampleRate)/(float)(shortTimeRateHz *numFramesPerBuffer);
	longTimeDecayF = (1.0f - 0.5f/rateF)/(float)shortTimeRateHz; 
	longTimeDecayF *= 6.0f;
	longTimeDecayI = FloatToQ15(longTimeDecayF);

}

LevelMeter::ProcessEffect(U32 numSamples, S16 *pStereoBuffer)
{

	// Audio level meters
	// Scan buffer for maximum value
	tAudioState *as = &audioState_;
	// Scan output buffer for max values
	for (long ch = kLeft; ch <= kRight; ch++)
	{
		// Grab from one channel, so use buffer offset
		S16 x = MaxAbsShorts(&pOut[ch], numFrames, 2);
		if (x > temp_ShortTime[ch])
			temp_ShortTime[ch] = x;
		
		as->outLevels_MaxCount[ch] += (x == as->outLevels_Max[ch]);
		if (x >= as->outLevels_Max[ch])
		{
			if (x > as->outLevels_Max[ch])
				as->outLevels_MaxCount[ch] = 1;
			as->outLevels_Max[ch] = x;
		}
	}
	shortTimeCounter++;
	if (shortTimeCounter >= shortTimeInterval)
	{
		for (long ch = kLeft; ch <= kRight; ch++)
		{ 
			S16 x = temp_ShortTime[ch];			 
			outLevels_ShortTime[ch] = x;
			if (x >= outLevels_LongTime[ch])
			{
				outLevels_LongTime[ch] = x;
				longTimeHoldCounter	   = 0;
			}
			else  
			{
				if (longTimeHoldCounter >= longTimeHoldInterval)
				{
					outLevels_LongTime[ch] = MultQ15(outLevels_LongTime[ch],
													 longTimeDecayI);
				}
				else
					longTimeHoldCounter++;
			}
			
			as->outLevels_LongTime [ch] = outLevels_LongTime [ch];
			as->outLevels_ShortTime[ch] = outLevels_ShortTime[ch];
			temp_ShortTime[ch] = 0;
		}
		shortTimeCounter = 0;
	}
}
