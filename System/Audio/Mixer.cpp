//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		Mixer.cpp
//
// Description:
//		The class to manage the processing of audio data on an audio channel.
//
//==============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sndfileutil.h"

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Mixer.h>
#include <AudioOutput.h>


// Debug : info for sound file input/output
long readInSoundFile   = false;
long writeOutSoundFile = false;
long inSoundFileDone   = false;

SNDFILE	*inSoundFile;
SF_INFO	inSoundFileInfo ;
char *inSoundFileName = "SINE/sine_db00_0500Hz_16k_c1.wav";
//"GoldenTestSet_16k/B_Quigley.wav";
//"SINE/sine_db00_0250Hz_16k_c1.wav";
//"GoldenTestSet_16k/B_Quigley.wav";
//"Music/Temptation_16k_st.wav";
// GoldenTestSet_16k
// B_Quigley.wav
// B_DOT.wav
// C_DJ2.wav

SNDFILE	*outSoundFile;
SF_INFO	outSoundFileInfo ;
char *outSoundFilePath = "BrioOut.wav";

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define	BRIO_MIDI_PLAYER_ID		1	// Hard code player ID for now...

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CAudioMixer implementation
//==============================================================================
CAudioMixer::CAudioMixer( int inChannels )
{
long i, j, ch;
//	printf("CAudioMixer::CAudioMixer: inChannels=%d Max=%d\n", inChannels, kAudioMixer_MaxInChannels);

	numInChannels_ = inChannels;
if (numInChannels_ > kAudioMixer_MaxInChannels)
	printf("CAudioMixer::CAudioMixer: %d too many channels! Max=%d\n", numInChannels_, kAudioMixer_MaxInChannels);
	pDebugMPI_->Assert((numInChannels_ <= kAudioMixer_MaxInChannels), "CAudioMixer::CAudioMixer: %d is too many channels!\n", numInChannels_ );

    SetMasterVolume(100);
    samplingFrequency_ = kAudioSampleRate;

	DefaultBrioMixer(&pDSP_);
	pDSP_.channelCount = numInChannels_;
	
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose); //kAudioDebugLevel );

//	printf("CAudioMixer::CAudioMixer: printf numInChannels_=%d \n", numInChannels_);

// Allocate audio channels
	pChannels_ = new CChannel[ numInChannels_ ];
	pDebugMPI_->Assert((pChannels_ != kNull), "CAudioMixer::CAudioMixer: Mixer couldn't allocate channels!\n" );
    pChannel_OutBuffer_ = new S16[ 2*kAudioOutBufSizeInWords ];  // Factor 2 to allow for 2x upsampling
    for (ch = 0; ch < numInChannels_; ch++)
        {
    	channel_tmpPtrs_[ch] = new S16[ 2*kAudioOutBufSizeInWords ];	
//        pChannels_[ch].outP = channel_tmpPtrs_[ch];
        }

	// Init midi player ptr
	pMidiPlayer_   = new CMidiPlayer( BRIO_MIDI_PLAYER_ID );

fsRack_[0] = (long)(samplingFrequency_*0.25f); // kAudioSampleRate_Div4;
fsRack_[1] = (long)(samplingFrequency_*0.5f ); // kAudioSampleRate_Div2;
fsRack_[2] = (long)(samplingFrequency_);       // kAudioSampleRate_Div1;
//for (i = 0; i < 3; i++)
//printf("fsRack_[%ld] = %ld\n", i, fsRack_[i]);

for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
		{

		mixBinBufferPtrs_[i][ch] = new S16[2*kAudioMixer_MaxOutChannels*kAudioOutBufSizeInWords];

// Initialize sampling rate converters  
        SRC *src = &src_[i][ch];
		DefaultSRC(src);
		src->type = kSRC_Interpolation_Type_Triangle; // Linear, Triangle, FIR, IIR, Box

//        src->type = kSRC_Interpolation_Type_FIR;
//        src->filterVersion = kSRC_FilterVersion_6; // 30dB_15
//        src->filterVersion = kSRC_FilterVersion_7; // 50dB_31
//        src->filterVersion = kSRC_FilterVersion_8; // 50dB_41

		SRC_SetInSamplingFrequency (src, (float)fsRack_[i]);
		SRC_SetOutSamplingFrequency(src, samplingFrequency_);

		UpdateSRC(src);
		ResetSRC(src);
		}
	mixBinFilled_[i] = False;
	}

// Configure DSP engine

    preGainDB  = 0.0f;
    postGainDB = 0.0f;

    useOutEQ_          = False;
    useOutSoftClipper_ = False;
for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
// Configure output Equalizer
    outEQ_BandCount_ = 1;
    for (j = 0; j < kAudioMixer_MaxEQBands; j++)
        DefaultEQ(&outEQ_[ch][j]);

//#define EQ_WITH_LOWPASS_RESPONSE
//#define EQ_WITH_RESONANT_SPIKES
#define EQ_WITH_FLAT_RESPONSE
#ifdef EQ_WITH_LOWPASS_RESPONSE
    outEQ_BandCount_ = 1;
    SetEQ_Parameters(&outEQ_[ch][0], 200.0f, 20.0f, 10.0f, kEQ_Mode_LowPass);
    SetEQ_Parameters(&outEQ_[ch][1], 200.0f, 20.0f, 0.0f, kEQ_Mode_Parametric);
    SetEQ_Parameters(&outEQ_[ch][2], 400.0f, 20.0f, 0.0f, kEQ_Mode_LowShelf);
#endif 
#ifdef EQ_WITH_RESONANT_SPIKES
    {
    float q = 20.0f;
    float gainDB = 10.0f;
    outEQ_BandCount_ = 3;
    SetEQ_Parameters(&outEQ_[ch][0], 100.0f, q, gainDB, kEQ_Mode_Parametric);
    SetEQ_Parameters(&outEQ_[ch][1], 200.0f, q, gainDB, kEQ_Mode_Parametric);
    SetEQ_Parameters(&outEQ_[ch][2], 400.0f, q, gainDB, kEQ_Mode_Parametric);
    }
#endif 
#ifdef EQ_WITH_FLAT_RESPONSE
    outEQ_BandCount_ = 3;
    SetEQ_Parameters(&outEQ_[ch][0], 1000.0f, 1.0f, 0.0f, kEQ_Mode_Parametric);
    SetEQ_Parameters(&outEQ_[ch][1], 2000.0f, 1.0f, 0.0f, kEQ_Mode_Parametric);
    SetEQ_Parameters(&outEQ_[ch][2], 4000.0f, 1.0f, 0.0f, kEQ_Mode_Parametric);
#endif 
    for (j = 0; j < outEQ_BandCount_; j++)
        {
        SetEQ_SamplingFrequency(&outEQ_[ch][j], samplingFrequency_);
        PrepareEQ(&outEQ_[ch][j]);
        }

// Configure output "soft" clipper
    {
    WAVESHAPER *d = &outSoftClipper_[ch];
    DefaultWaveShaper(d);
    SetWaveShaper_Parameters(d, kWaveShaper_Type_V4, 0.0f, 0.0f);
    SetWaveShaper_SamplingFrequency(d, samplingFrequency_);
    PrepareWaveShaper(d);
    }
    }
for (ch = 0; ch < pDSP_.channelCount; ch++)
	pChannels_[ch].SetMixerChannelDataPtr(&pDSP_.channels[ch]);

for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
	long bufWords = 2*(kAudioOutBufSizeInWords + kSRC_Filter_MaxDelayElements);
	S16 *p = new S16[ bufWords * sizeof(S16)];
	tmpBufferPtrs_   [i] = p;
	tmpBufOffsetPtrs_[i] = &p[2*kSRC_Filter_MaxDelayElements];
	}

inSoundFile  = NULL;
outSoundFile = NULL;

// Open a stereo WAV file to write output of mixer
if (writeOutSoundFile)
	{
	outSoundFileInfo.frames     = 0;		
	outSoundFileInfo.samplerate = (long) samplingFrequency_;
	outSoundFileInfo.channels   = 2;
	outSoundFileInfo.format     = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
	outSoundFileInfo.sections = 1;
	outSoundFileInfo.seekable = 1;
printf("CAudioMixer::CAudioMixer: opened output file '%s' \n", outSoundFilePath);
	outSoundFile = OpenSoundFile( outSoundFilePath, &outSoundFileInfo, SFM_WRITE);
	}

// Open WAV file for input to mixer
if (readInSoundFile)
	{
    char inFilePath[1000];
#ifdef EMULATION
   strcpy(inFilePath, "/home/lfu/AudioFiles/");
#else
    strcpy(inFilePath, "/AudioFiles/");
#endif

// SINE/ : sine_db0_1000Hz_32k.wav sine_dbM3_1000Hz_32k sine_dbM6_1000Hz_32k
// at db {00, M3, M6} and fs={16k,32k} and f= { 125, 250, 500, 1000, 2000, 4000, 8000 }
// Representative Audio
//	char *inFileName = "GoldenTestSet_16k/A_AP.wav";
char *inFileName = inSoundFileName;

    strcat(inFilePath, inFileName);
	inSoundFile = OpenSoundFile(inFilePath, &inSoundFileInfo, SFM_READ);	if (!inSoundFile)
		{
		printf("CAudioMixer::CAudioMixer: Unable to open input file '%s'\n", inFilePath);
		readInSoundFile = False;
		}
	else
        {
		printf("CAudioMixer::CAudioMixer: opened input file '%s'\n", inFilePath);
// inSoundFileInfo.samplerate, frames, channels
//	printf("CAudioMixer::CAudioMixer: opened inFile: fs=%d frames=%d ch=%d \n", 
//		inSoundFileInfo.samplerate, (int)inSoundFileInfo.frames, inSoundFileInfo.channels);
    	}
    inSoundFileDone = false;
	}
}  // ---- end CAudioMixer::CAudioMixer() ----

//==============================================================================
// CAudioMixer::~CAudioMixer ::
//==============================================================================
CAudioMixer::~CAudioMixer()
{
long i;

	// Deallocate channels
 	if (pChannels_)
	{
    for (long ch = 0; ch < numInChannels_; ch++)
        {     	
		if (channel_tmpPtrs_[ch])
			free(channel_tmpPtrs_[ch]);	
//        delete pChannels_[ch];
        }
//fixme/dg: will this delete[] the array + the channels in the array too?  -> NO
	delete pChannels_;
	}
  	if (pChannel_OutBuffer_)
    		delete pChannel_OutBuffer_;
	
	// Deallocate MIDI player
	if (pMidiPlayer_)
		delete pMidiPlayer_;

	// Dellocate buffers 
	for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
		{
		if (tmpBufferPtrs_[i])
			free(tmpBufferPtrs_[i]);
		}
	
	for (long i = 0; i < kAudioMixer_MixBinCount; i++)
		{
        for (long j = 0; j < kAudioMixer_MaxOutChannels; j++)
            {
    		if (mixBinBufferPtrs_[i][j])
    			free(mixBinBufferPtrs_[i][j]);
            }
		}
	
	if (writeOutSoundFile)
		CloseSoundFile(&outSoundFile);

}  // ---- end ~CAudioMixer() ----

//==============================================================================
// FindChannelUsing
//==============================================================================
CChannel* CAudioMixer::FindChannelUsing( tAudioPriority /* priority */)
{
	// For now, just search for a channel not in use
	for (long i = 0; i < numInChannels_; i++)
	{
		if (!pChannels_[i].IsInUse()) 
			return &pChannels_[i];
	}
	
	// Reaching this point means all channels are currently in use
	return kNull;
}  // ---- end CAudioMixer::FindChannelUsing() ----

//==============================================================================
// CAudioMixer::FindChannelUsing
//==============================================================================
CChannel* CAudioMixer::FindChannelUsing( tAudioID id )
{
//	U8 				iChan;
	tAudioID		idFromPlayer;
	CChannel*		pChan;
	CAudioPlayer* 	pPlayer;
	
	// Loop through mixer channels, look for one that's in use and then
	// test the player's ID against the requested id.
	for (long i = 0; i < numInChannels_; i++)
	{
		pChan = &pChannels_[i];
		if (pChan->IsInUse()) {
			pPlayer = pChan->GetPlayer();
			idFromPlayer = pPlayer->GetAudioID();
			if ( idFromPlayer == id )
				return &pChannels_[i];
		}
	}
	
	// Reaching this point means all no ID matched.
	return kNull;
}  // ---- end CAudioMixer::FindChannelUsing() ----

//==============================================================================
// IsAnyAudioActive
//==============================================================================
Boolean CAudioMixer::IsAnyAudioActive( void )
{
//	Boolean 	result = false;
//	U32 iChan = 0;
//	CChannel*	pChan = NULL;
	
	// Loop over the number of channels
	for (long i = 0; i < numInChannels_; i++)
		{
		if (pChannels_[i].IsInUse())
			return (true);
		}
	
	return false;
}  // ---- end CAudioMixer::IsAnyAudioActive() ----

//==============================================================================
// GetMixBinIndex : determine mix bin index from sampling frequency
//==============================================================================
long CAudioMixer::GetMixBinIndex( long samplingFrequency )
{
	long index =  kAudioMixer_MixBin_Index_FsDiv1;
	
	// FIXXX: currently matches numbers.  In the future, should assign mix bin
	// with closest sampling frequency and do conversion
	switch (samplingFrequency)
		{
		default:
		case kAudioSampleRate :
			index = kAudioMixer_MixBin_Index_FsDiv1;
		break;
		case kAudioSampleRate_Div2 :
			index = kAudioMixer_MixBin_Index_FsDiv2;
		break;
		case kAudioSampleRate_Div4 :
			index = kAudioMixer_MixBin_Index_FsDiv4;
		break;
		}
	
	return (index);
}  // ---- end CAudioMixer::GetMixBinIndex() ----

//==============================================================================
// GetSamplingRateDivisor : Determine divisor sampling frequency, which is your operating
//				sampling frequency
//==============================================================================
long CAudioMixer::GetSamplingRateDivisor( long samplingFrequency )
{
	long div =  1;
	
	// FIXXX: currently matches numbers.  In the future, should assign mix bin
	// with closest sampling frequency and do conversion
	switch (samplingFrequency)
		{
		default:
		case kAudioSampleRate :
			div = 1;
		break;
		case kAudioSampleRate_Div2 :
			div = 2;
		break;
		case kAudioSampleRate_Div4 :
			div = 4;
		break;
		}
	
	return (div);
}  // ---- end CAudioMixer::GetSamplingRateDivisor() ----

//==============================================================================
// CAudioMixer::RenderBuffer
//==============================================================================
int CAudioMixer::RenderBuffer( S16 *pOutBuff, U32 numFrames )
// numFrames  IS THIS FRAMES OR SAMPLES  !!!!!!!  THIS APPEARS TO BE SAMPLES
{
U32	i, ch;
U32 	playerFramesRendered;
long mixBinIndex;
long channelsPerFrame = kAudioMixer_MaxOutChannels;
// FIXXXX: mystery.  Offset Ptrs don't work !!!
short **tPtrs = tmpBufferPtrs_; // pTmpBuffers_, tmpBufOffsetPtrs_

//{static long c=0; printf("CAudioMixer::RenderBuffer : start %ld  numFrames=%ld channels=%ld\n", c++, numFrames, channelsPerFrame);}

//	printf("AudioMixer::RenderBuffer -- bufPtr: 0x%x, frameCount: %u \n", (unsigned int)pOutBuff, (int)numStereoFrames );
//	pDebugMPI_->Assert( ((numFrames * kAudioBytesPerStereoFrame) == kAudioOutBufSizeInBytes ),
//		"AudioMixer::RenderBuffer -- frameCount doesn't match buffer size!!!\n");
	
// Clear output and temporary buffers
//ClearShorts( pOutBuff, numFrames * channelsPerFrame );
//for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
//	ClearShorts( pTmpBuffers_[i], numFrames * channelsPerFrame );

// Clear stereo mix buffers
for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	ClearShorts( mixBinBufferPtrs_[i][0], numFrames*channelsPerFrame);
	ClearShorts( mixBinBufferPtrs_[i][1], numFrames*channelsPerFrame);
	mixBinFilled_[i] = False;
	}

//
// ---- Render each channel to mix buffer with corresponding sampling frequency
//
if (readInSoundFile && !inSoundFileDone)
	{
    long mixBinIndex  = GetMixBinIndex(inSoundFileInfo.samplerate);
	S16 *mixBinP      = (S16 *) mixBinBufferPtrs_[mixBinIndex][0];
	long framesToRead = numFrames/GetSamplingRateDivisor(inSoundFileInfo.samplerate);
	long framesRead   = 0;

    ClearShorts(mixBinP, framesToRead*2);
// Copy stereo input to stereo mix buffer
	if (2 == inSoundFileInfo.channels)
 		framesRead = sf_readf_short(inSoundFile, mixBinP, framesToRead);
// Replicate mono input file to both channels of stereo mix buffer
	else
		{
 		framesRead = sf_readf_short(inSoundFile, tmpBufferPtrs_[0], framesToRead);
		InterleaveShorts(tmpBufferPtrs_[0], tmpBufferPtrs_[0], mixBinP, framesRead);
		}
    if (framesRead < framesToRead)
        {
	    inSoundFileDone = true;
        printf("inSoundFileDone ! %ld/%ld frames read \n", framesRead, framesToRead); 
        }
	mixBinFilled_[mixBinIndex] = True;
//printf("AudioMixer::RenderBuffer: read %ld samples from WAV file \n", framesRead);
//  inSoundFileInfo.samplerate, frames, channels
	}
else
{
for (ch = 0; ch < numInChannels_; ch++)
	{
	CChannel *pCh = &pChannels_[ch];

// Initialize output buffer to 0 - clears output if the player runs out of data.
	bzero( tmpBufferPtrs_[0], kAudioOutBufSizeInBytes );
	bzero( pChannel_OutBuffer_, kAudioOutBufSizeInBytes );
//printf("CAudioMixer::RenderBuffer :   pCh%ld->ShouldRender=%d \n", ch, pCh->ShouldRender());

	// Render if channel is in use and not paused
	if (pCh->ShouldRender())
		{
        long channelSamplingFrequency = pCh->GetSamplingFrequency();
		U32 framesToRender = (numFrames*channelSamplingFrequency)/(long)samplingFrequency_;
//printf("ch%ld: framesToRender=%ld for %ld Hz\n", ch, framesToRender, channelSamplingFrequency);

	// Have player render its data into our output buffer.  If the player
	// contains mono data, it will be rendered out as stereo data.
       playerFramesRendered = pCh->RenderBuffer( pChannel_OutBuffer_, tmpBufferPtrs_[0], framesToRender, False );

	// If player has finished, release channel.
		if ( playerFramesRendered < framesToRender ) 
			pCh->Release( false );	// Don't suppress done msg if requested
			
	// Add output to appropriate Mix "Bin" 
		long mixBinIndex = GetMixBinIndex(channelSamplingFrequency);
// FIXXX:  convert fs/4->fs/2 with gentler anti-aliasing filter and let fs/2 mix bin's conversion do fs/2->fs
		S16* pMixBin = mixBinBufferPtrs_[mixBinIndex][0];
		if (mixBinFilled_[mixBinIndex])
			Add2_Shortsi(pChannel_OutBuffer_, pMixBin, pMixBin, framesToRender*channelsPerFrame); 
		else
			CopyShorts(pChannel_OutBuffer_, pMixBin, framesToRender*channelsPerFrame);
		mixBinFilled_[mixBinIndex] = True;
		}
	}

// MIDI player renders to fs/2 output buffer 
if ( pMidiPlayer_->IsActive() )
	{
//	long rateDivisor = 2;
	long midiFrames  = (numFrames/2);
	long mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
	S16 *pMixBin = mixBinBufferPtrs_[mixBinIndex][0];

	playerFramesRendered = pMidiPlayer_->RenderBuffer( pMixBin, midiFrames, mixBinFilled_[mixBinIndex]); 
//{static long c=0; printf("CAudioMixer::RenderBuffer :   MIDI active %ld midiFrames=%ld\n", c++, midiFrames);}
	mixBinFilled_[mixBinIndex] = True;
	}
}

// 
// Combine Mix buffers to output, converting sampling rate if necessary
//
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv1;
if (mixBinFilled_[mixBinIndex])
    {
//printf("PRE 2*kAudioMixer_MaxOutChannels*kAudioOutBufSizeInWords=%d\n", 2*kAudioMixer_MaxOutChannels*kAudioOutBufSizeInWords);
//printf("PRE numFrames=%ld channelsPerFrame=%ld\n", numFrames, channelsPerFrame);
//printf("BEFO: mixBinP[%ld]=%X pOutBuff=%X len=%ld\n", mixBinIndex, (unsigned int) mixBinBufferPtrs_[mixBinIndex][0], (unsigned int) pOutBuff, numFrames*channelsPerFrame);
CopyShorts(mixBinBufferPtrs_[mixBinIndex][0], pOutBuff, numFrames); //*channelsPerFrame/2); //numFrames*channelsPerFrame);
//printf("AFTA: \n");
    }
else
	ClearShorts( pOutBuff, numFrames * channelsPerFrame );

#define MIX_THE_MIX_BINS
#ifdef MIX_THE_MIX_BINS
// Deinterleave and process each MixBuffer separately
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv4;
if (mixBinFilled_[mixBinIndex])
	{
	DeinterleaveShorts(mixBinBufferPtrs_[mixBinIndex][0], tPtrs[2], tPtrs[3], numFrames/4);
	RunSRC(tPtrs[2], tPtrs[0], numFrames/4, numFrames, &src_[mixBinIndex][0]);
	RunSRC(tPtrs[3], tPtrs[1], numFrames/4, numFrames, &src_[mixBinIndex][1]);
	InterleaveShorts(tPtrs[0], tPtrs[1], tPtrs[6], numFrames);
	Add2_Shortsi(tPtrs[6], pOutBuff, pOutBuff, numFrames*channelsPerFrame);
//CopyShorts(tPs[6], pOutBuff, numFrames*channelsPerFrame);
	}

mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
if (mixBinFilled_[mixBinIndex])
	{
	DeinterleaveShorts(mixBinBufferPtrs_[mixBinIndex][0], tPtrs[4], tPtrs[5], numFrames/2);
	RunSRC(tPtrs[4], tPtrs[0], numFrames/2, numFrames, &src_[mixBinIndex][0]);
	RunSRC(tPtrs[5], tPtrs[1], numFrames/2, numFrames, &src_[mixBinIndex][1]);
	InterleaveShorts(tPtrs[0], tPtrs[1], tPtrs[6], numFrames);
	Add2_Shortsi(tPtrs[6], pOutBuff, pOutBuff, numFrames*channelsPerFrame);
//CopyShorts(tPs[6], pOutBuff, numFrames*channelsPerFrame);
	}
		
// fixme/rdg:  needs CAudioEffectsProcessor
// If at least one audio channel is playing, Process global audio effects
//if (numPlaying && (gAudioContext->pAudioEffects != kNull))
//	gAudioContext->pAudioEffects->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuff );

// Scale stereo/interleaved buffer by master volume
/// NOTE: volume here is interpreted as a linear value
//	ORIG rdg for (i = 0; i < numFrames*channelsPerFrame; i++)
//		pOutBuff[i] = (S16)((pOutBuff[i] * (int)masterVolume_) >> 7); // fixme; 
//ScaleShortsf(pOutBuff, pOutBuff, numFrames*channelsPerFrame, masterGainf_[0]);
ScaleShortsi_Q15(pOutBuff, pOutBuff, numFrames*channelsPerFrame, masterGaini_[0]);
#endif // end WHOLE_THING

// ---- Output DSP block
long useOutDSP = False;
if (useOutDSP)
{
DeinterleaveShorts(pOutBuff, tPtrs[4], tPtrs[5], numFrames);

for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
    {
    short *pIn  = tPtrs[4+ch];
    short *pOut = tPtrs[4+ch];

// Compute Output Equalizer
// NOTE: should have 32-bit data path here for EQ before soft clipper
useOutEQ_ = False;
    if (useOutEQ_)
        {
{static long c=0; printf("ComputeEQ %ld : bands=%ld \n", c++, outEQ_BandCount_);}
        for (long j = 0; j < outEQ_BandCount_; j++)
            {
//            ComputeEQf(pIn, pOut, numFrames, &outEQ_[ch][j]);
            ComputeEQi(pIn, pOut, numFrames, &outEQ_[ch][j]);
            pOut = pIn;
            }
        }
// Compute Output Soft Clipper
useOutSoftClipper_ = False;
    if (useOutSoftClipper_)
        {
{static long c=0; if (!c) printf("ComputeWaveShaper %ld On=%ld: \n", c++, useOutSoftClipper_);}
        ComputeWaveShaper(pIn, pOut, numFrames, &outSoftClipper_[ch]);
        }
    }
InterleaveShorts(tPtrs[4], tPtrs[5], pOutBuff, numFrames);
}  // if (useOutDSP)

// Debug:  write output of mixer to sound file
if (writeOutSoundFile)
    {
    long framesWritten = sf_writef_short(outSoundFile, pOutBuff, numFrames);
//
    framesWritten = 0;
    if (inSoundFileDone)
        {
    	CloseSoundFile(&outSoundFile);
        outSoundFile = NULL;
        writeOutSoundFile = false;
printf("Closed outSoundFile\n");
        }
    }

//printf("CAudioMixer::RenderBuffer: END \n");
	return kNoErr;
} // ---- end CAudioMixer::RenderBuffer() ----

//==============================================================================
// WrapperToCallRenderBuffer
//==============================================================================
int CAudioMixer::WrapperToCallRenderBuffer( S16 *pOut,  unsigned long numStereoFrames, void* pToObject  )
{
//	CAudioMixer* mySelf = (CAudioMixer*)pToObject;
	
	// Call member function to get a buffer full of stereo data
	return ((CAudioMixer*)pToObject)->RenderBuffer( pOut, numStereoFrames );
} // ---- end CAudioMixer::WrapperToCallRenderBuffer() ----

// ==============================================================================
// SetMasterVolume :  output level for mixer
// ==============================================================================
void CAudioMixer::SetMasterVolume( U8 x )
{
masterVolume_ = x;

// FIXX: move to decibels, but for now, linear volume
// ChangeRangef(x, L1, H1, L2, H2)
masterGainf_[0] = ChangeRangef((float)x, 0.0f, 100.0f, 0.0f, 1.0f);
//masterGainf_[0] = DecibelToLinearf(ChangeRangef((float)x, 0.0f, 100.0f, -100.0f, 0.0f));
masterGainf_[1] = masterGainf_[0];

// Convert 32-bit floating-point to Q15 fractional integer format 
masterGaini_[0] = FloatToQ15(masterGainf_[0]);
masterGaini_[1] = masterGaini_[0];

//printf("CAudioMixer::SetMasterVolume %d -> %f ($%x)\n", masterVolume_ , masterGainf_[0], masterGaini_[0]);
} // ---- end CAudioMixer::SetMasterVolume() ----

// ==============================================================================
// SetSamplingFrequency :  update fs for mixer and all DSP
//                  Does not reset or recalculate DSP parameters
// ==============================================================================
void CAudioMixer::SetSamplingFrequency( float x )
{
samplingFrequency_ = x;
// FIXX :  Add bounding code

fsRack_[0] = (long)(samplingFrequency_*0.25f); // kAudioSampleRate_Div4;
fsRack_[1] = (long)(samplingFrequency_*0.5f ); // kAudioSampleRate_Div2;
fsRack_[2] = (long)(samplingFrequency_);       // kAudioSampleRate_Div1;

for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
    long i;
    for (i = 0; i < outEQ_BandCount_; i++)
        SetEQ_SamplingFrequency(&outEQ_[ch][i], x);

    SetWaveShaper_SamplingFrequency(&outSoftClipper_[ch], x);

    for (i = 0; i < kAudioMixer_MixBinCount; i++)
        {
		SRC_SetInSamplingFrequency (&src_[i][ch], (float)fsRack_[i]);
		SRC_SetOutSamplingFrequency(&src_[i][ch], samplingFrequency_);
        }
    }
} // ---- end CAudioMixer::SetSamplingFrequency() ----

// ==============================================================================
// UpdateDSP :  Recalculate DSP parameters
// ==============================================================================
void CAudioMixer::UpdateDSP()
{
for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
    long i;
    for (i = 0; i < outEQ_BandCount_; i++)
        UpdateEQ(&outEQ_[ch][i]);

    UpdateWaveShaper(&outSoftClipper_[ch]);

    for (i = 0; i < kAudioMixer_MixBinCount; i++)
    	UpdateSRC(&src_[i][ch]);
    }
} // ---- end CAudioMixer::UpdateDSP() ----

// ==============================================================================
// ResetDSP :  Reset DSP state
// ==============================================================================
void CAudioMixer::ResetDSP()
{
long ch, i;

for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
    for (i = 0; i < outEQ_BandCount_; i++)
        ResetEQ(&outEQ_[ch][i]);

    ResetWaveShaper(&outSoftClipper_[ch]);

    for (i = 0; i < kAudioMixer_MixBinCount; i++)
    	{
    	ResetSRC(&src_[i][ch]);
    	mixBinFilled_[i] = False;
    	}
    }
} // ---- end CAudioMixer::ResetDSP() ----

// ==============================================================================
// PrepareDSP :  Update + Reset DSP state
// ==============================================================================
void CAudioMixer::PrepareDSP()
{
UpdateDSP();
ResetDSP();
} // ---- end CAudioMixer::PrepareDSP() ----

// ==============================================================================
// UpdateDebugGain :  Recalculate debug gain variables
// ==============================================================================
void CAudioMixer::UpdateDebugGain()
{
preGainf = DecibelToLinearf(preGainDB);
preGaini = FloatToQ15(preGainf);
//printf("CAudioMixer::UpdateDebugGain: preGainDB %g -> %g (%04X) \n", preGainDB, preGainf, preGaini);

postGainf = DecibelToLinearf(postGainDB);
postGaini = FloatToQ15(postGainf);
//printf("CAudioMixer::UpdateDebugGain: postGainDB %g -> %g (%04X) \n", postGainDB, postGainf, postGaini);

} // ---- end CAudioMixer::UpdateDebugGain() ----

// ==============================================================================
// SetOutputEqualizer :  Set output equalizer type.
//                          For now, just On or Off
// ==============================================================================
void CAudioMixer::SetOutputEqualizer(Boolean /* x */)
{
//printf("CAudioMixer::SetOutputEqualizer: useOutEQ_=%ld->%ld\n", (long)useOutEQ_, (long)x);
// Careful with reset if DSP is running in a separate thread.
//ResetDSP();
//useOutEQ_ = x;

//useOutSoftClipper_ = x;

} // ---- end CAudioMixer::SetOutputEqualizer() ----

LF_END_BRIO_NAMESPACE()
