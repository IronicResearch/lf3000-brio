// ==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
// ==============================================================================
//
// Mixer.cpp
//
//          The class to manage summation of several audio channels
//
// ==============================================================================
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

#include <RawPlayer.h>
#include <VorbisPlayer.h>

#define kVolume_Default  100
#define kVolume_Min        0
#define kVolume_Max      100

#define kMixer_HeadroomBits_Default 1

// Debug input/output stuff

float outputDCValueDB;
float outputDCValuef;
Q15   outputDCValuei;

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define	BRIO_MIDI_PLAYER_ID		1	// FIXXX: Hard code player ID for now

//==============================================================================
// Global variables
//==============================================================================

// ==============================================================================
// CAudioMixer implementation
// ==============================================================================
CAudioMixer::CAudioMixer( int inChannels )
{
long i, j, ch;
//printf("CAudioMixer::CAudioMixer: inChannels=%d Max=%d\n", inChannels, kAudioMixer_MaxInChannels);

	numInChannels_ = inChannels;
if (numInChannels_ > kAudioMixer_MaxInChannels)
	printf("CAudioMixer::CAudioMixer: %d too many channels! Max=%d\n", numInChannels_, kAudioMixer_MaxInChannels);
	pDebugMPI_->Assert((numInChannels_ <= kAudioMixer_MaxInChannels), "CAudioMixer::CAudioMixer: %d is too many channels!\n", numInChannels_ );

#ifdef NEW_ADD_PLAYER
    playerToAdd_  = NULL;
    targetChannel_= NULL;
#endif

    SetMasterVolume(kVolume_Default);
    samplingFrequency_ = kAudioSampleRate;

	DefaultBrioMixer(&pDSP_);
	pDSP_.channelCount = numInChannels_;
	
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose); //kAudioDebugLevel );

// Allocate audio channels
	pChannels_ = new CChannel[ numInChannels_ ];
	pDebugMPI_->Assert((pChannels_ != kNull), "CAudioMixer::CAudioMixer: couldn't allocate %d channels!\n" , numInChannels_);
    pChannel_OutBuffer_ = new S16[ 2*kAudioOutBufSizeInWords ];  // Factor 2 (4?) to allow for 2x upsampling
    for (ch = 0; ch < numInChannels_; ch++)
        {
    	channel_tmpPtrs_[ch] = new S16[ 2*kAudioOutBufSizeInWords ];	
//        pChannels_[ch].outP = channel_tmpPtrs_[ch];
        }

	// Create MIDI player
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
//		src->type = kSRC_Interpolation_Type_Triangle; // AddDrop, Linear, Triangle, FIR, IIR, Box

        src->type = kSRC_Interpolation_Type_FIR;
        src->filterVersion = kSRC_FilterVersion_8; // 6=30dB_15 7=50dB_31 8=50dB_41

		SRC_SetInSamplingFrequency (src, (float)fsRack_[i]);
		SRC_SetOutSamplingFrequency(src, samplingFrequency_);

		UpdateSRC(src);
		ResetSRC(src);
		}
	mixBinFilled_[i] = False;
	}

// Input debug stuff
    inputIsDC      = false;
    inputDCValueDB = 0.0f; // 0.0f, -3.01f, -6.02f
    inputDCValuef  = DecibelToLinearf(inputDCValueDB);
    inputDCValuei  = FloatToQ15(inputDCValuef);
//printf("CAudioMixer::CAudioMixer inDC=%g dB -> %g -> $%X\n", inputDCValueDB, inputDCValuef, inputDCValuei);

#ifdef NEED_SAWTOOTH
    inputIsSawtoothWave_ = false;
    z_     = 0;
    delta_ = 0;
    SetUpSawtoothOscillator(&z_, &delta_, 4000.0f/(float)kAudioSampleRate, 0.0f);
#endif

// Configure DSP engine
    preGainDB  = 0.09f;
    postGainDB = 0.07f;

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
    SetWaveShaper_Parameters(d, kWaveShaper_Type_V4, preGainDB, postGainDB);
    SetWaveShaper_SamplingFrequency(d, samplingFrequency_);
    PrepareWaveShaper(d);
    }
    }
//for (ch = 0; ch < pDSP_.channelCount; ch++)
//	pChannels_[ch].SetMixerChannelDataPtr(&pDSP_.channels[ch]);

for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
	S16 *p = new S16[ 2*(kAudioOutBufSizeInWords + kSRC_Filter_MaxDelayElements) ];
	pTmpBufs_        [i] = p;
	tmpBufOffsetPtrs_[i] = &p[2*kSRC_Filter_MaxDelayElements];
	}

// DEBUG:  Wav File I/O
    readInSoundFile_   = false;
    writeOutSoundFile_ = false;
    inSoundFileDone_   = false;

    inSoundFilePath_  = "Saw_32k_m3dB_m.wav";
//"Saw_32k_m0dB_m.wav, Saw_32k_m3dB_m.wav, Saw_32k_m6dB_m.wav";

//"Sine_1kHz_0dB_15Sec_m_32k.wav";
//"SINE/sine_db00_0500Hz_16k_c1.wav";
    outSoundFilePath_ = "BrioOut.wav";

    inSoundFile_  = NULL;
    outSoundFile_ = NULL;

// Open a stereo WAV file to write output of mixer
if (writeOutSoundFile_)
	{
	outSoundFileInfo_.frames     = 0;		
	outSoundFileInfo_.samplerate = (long) samplingFrequency_;
	outSoundFileInfo_.channels   = 2;
	outSoundFileInfo_.format     = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
	outSoundFileInfo_.sections   = 1;
	outSoundFileInfo_.seekable   = 1;
printf("CAudioMixer::CAudioMixer: opened out file '%s' \n", outSoundFilePath_);
	outSoundFile_ = OpenSoundFile( outSoundFilePath_, &outSoundFileInfo_, SFM_WRITE);
	}

// Open WAV file for input to mixer
if (readInSoundFile_)
	{
#ifdef EMULATION
    char *inFilePath = ".\\"; // /home/lfu/AudioFiles/
#else
    char *inFilePath = "/AudioFiles/";
#endif

// SINE/ : sine_db0_1000Hz_32k.wav sine_dbM3_1000Hz_32k sine_dbM6_1000Hz_32k
// at db {00, M3, M6} and fs={16k,32k} and f= { 125, 250, 500, 1000, 2000, 4000, 8000 }
// Representative Audio
//	char *inFileName = "GoldenTestSet_16k/A_AP.wav";
char *inFileName = inSoundFilePath_;
printf("inFileName='%s'\n", inFileName);
//printf("inFilePath='%s'\n", inFilePath);

//    strcpy(inFilePath, inFileName);
	inSoundFile_ = OpenSoundFile(inFileName, &inSoundFileInfo_, SFM_READ);	if (!inSoundFile_)
		{
		printf("CAudioMixer::CAudioMixer: Unable to open input file '%s'\n", inFileName);
		readInSoundFile_ = False;
		}
	else
        {
		printf("CAudioMixer::CAudioMixer: opened input file '%s'\n", inFileName);
// inSoundFileInfo.samplerate, frames, channels
//	printf("CAudioMixer::CAudioMixer: opened inFile: fs=%d frames=%ld ch=%d \n", 
//		inSoundFileInfo.samplerate, inSoundFileInfo.frames, inSoundFileInfo.channels);
    	}
    inSoundFileDone_ = false;
	}

// Set up level meters
for (ch = kLeft; ch <= kRight; ch++)
    {
    outLevels_ShortTime[ch] = 0;
    outLevels_LongTime [ch] = 0;
    temp_ShortTime     [ch] = 0;
    }

// numFrames = 256 for EMULATION, 128? for embedded ??
// DUHHH is this frames or samples ?
#ifdef EMULATION
int numFramesPerBuffer = 256;
#else // EMBEDDED
int numFramesPerBuffer = 128;
#endif

U32 shortTimeRateHz   = 12;
shortTimeCounter  = 0;
shortTimeInterval = kAudioSampleRate/(shortTimeRateHz * numFramesPerBuffer);

U32 longTimeRateHz   = 1;
longTimeHoldCounter  = 0;
longTimeHoldInterval = shortTimeInterval/longTimeRateHz;

float rateF = ((float)kAudioSampleRate)/(float)(shortTimeRateHz *numFramesPerBuffer);
longTimeDecayF = (1.0f - 0.5f/rateF)/(float)shortTimeRateHz; 
longTimeDecayF *= 6.0;
longTimeDecayI = FloatToQ15(longTimeDecayF);
//printf("rateF=%g longTimeDecayF=%g\n", rateF, longTimeDecayF);
//printf("shortTime: Hz=%ld frames=%d Interval=%ld longTimeDecayF=%g\n", 
//        shortTimeRateHz, numFramesPerBuffer, shortTimeInterval, longTimeDecayF);

//
// Keep at end of this routine, Set up Audio State struct (interfaces via AudioMPI)
//
{
tAudioState *d = &audioState_;
d->computeLevelMeters = false;
d->useOutEQ           = false;
d->useOutSoftClipper  = false;
d->useOutDSP          = (d->computeLevelMeters || d->useOutEQ || d->useOutSoftClipper);
if (d->useOutSoftClipper)
    d->headroomBits = 1+kMixer_HeadroomBits_Default;
else
    d->headroomBits =   kMixer_HeadroomBits_Default;

d->readInSoundFile   = readInSoundFile_;
d->writeOutSoundFile = writeOutSoundFile_;

d->softClipperPreGainDB  = (S16) preGainDB;
d->softClipperPostGainDB = (S16) postGainDB;

//kSRC_Interpolation_Type_Triangle; // Linear, Triangle, FIR, IIR, Box
d->srcType          = src_[0][kLeft].type;
d->srcFilterVersion = src_[0][kLeft].filterVersion;
for (long ch = kLeft; ch <= kRight; ch++)
    {
    d->outLevels_Max     [ch] = 0;
    d->outLevels_MaxCount[ch] = 0;

    d->outLevels_ShortTime[ch] = outLevels_ShortTime[ch];
    d->outLevels_LongTime [ch] = outLevels_LongTime [ch];
    }
d->channelGainDB = kChannel_HeadroomDB;
}

}  // ---- end CAudioMixer::CAudioMixer() ----

// ==============================================================================
// ~CAudioMixer :
// ==============================================================================
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
	}
	
// Deallocate MIDI player
	if (pMidiPlayer_)
		delete pMidiPlayer_;

// Dellocate buffers 
  	if (pChannel_OutBuffer_)
    		delete pChannel_OutBuffer_;
	for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
		{
		if (pTmpBufs_[i])
			free(pTmpBufs_[i]);
        pTmpBufs_[i] = NULL;
        tmpBufOffsetPtrs_[i] = NULL;
		}
	
	for (long i = 0; i < kAudioMixer_MixBinCount; i++)
		{
        for (long j = 0; j < kAudioMixer_MaxOutChannels; j++)
            {
    		if (mixBinBufferPtrs_[i][j])
    			free(mixBinBufferPtrs_[i][j]);
            }
		}
	
if (writeOutSoundFile_)
	CloseSoundFile(&outSoundFile_);

}  // ---- end ~CAudioMixer() ----

// ==============================================================================
// FindFreeChannel:    Find a free channel using priority -> GKFIXX: add search by priority
// ==============================================================================
    CChannel * 
CAudioMixer::FindFreeChannel( tAudioPriority /* priority */)
{
#ifdef KEEP_FOR_DEBUGGING
for (long i = 0; i < numInChannels_; i++)
	{
    CChannel *pCh = &pChannels_[i];
//printf("CAudioMixer::FindFreeChannel: %ld: IsInUse=%d isDone=%d player=%p\n", i, pCh->IsInUse(), pCh->isDone_, (void*)pCh->GetPlayerPtr());
    }
#endif

// For now, just search for a channel not in use
for (long i = 0; i < numInChannels_; i++)
	{
    CChannel *pCh = &pChannels_[i];
//    CAudioPlayer *pPlayer =pCh->GetPlayer();
		if (!pCh->IsInUse())// && pCh->isDone_) 
        {
//printf("CAudioMixer::FindFreeChannel: USING %ld IsInUse=%d isDone=%d Player=%p\n", i, pCh->IsInUse(), pCh->isDone_, (void*)pCh->GetPlayerPtr());
			return (pCh);
        }
	}
	
	// Reaching this point means all channels are currently in use
//printf("CAudioMixer::FindFreeChannel: all %d channels in use.\n", numInChannels_);
	return (kNull);
}  // ---- end FindFreeChannel() ----

// ==============================================================================
// FindChannel:  Look for channel with specified ID
//                  Return ptr to channel
// ==============================================================================
CChannel* CAudioMixer::FindChannel( tAudioID id )
{
//  Find channel with specified ID
for (long i = 0; i < numInChannels_; i++)
	{
    CAudioPlayer *pPlayer = pChannels_[i].GetPlayer();
//    GKGK  Excised IsInUse().  Hot code - check for functionaliy break
	if ( pPlayer && (pPlayer->GetID() == id)); // && pChannels_[i].IsInUse())
		return (&pChannels_[i]);
	}
	
// Reaching this point means all no ID matched.
return ( kNull );
}  // ---- end FindChannel() ----

// ==============================================================================
// FindFreeChannelIndex:    Find specified channel by ID.  Must be "in use"
//                              Return index in channel array
// ==============================================================================
    long
CAudioMixer::FindFreeChannelIndex( tAudioID id )
{
for (long i = 0; i < numInChannels_; i++)
	{
    CAudioPlayer *pPlayer = pChannels_[i].GetPlayer();
	if ( pPlayer && (pPlayer->GetID() == id) && pChannels_[i].IsInUse())
		return (i);
	}
	
// Unable to find
return ( -1 );
}  // ---- end FindFreeChannelIndex() ----

// ==============================================================================
// IsAnyAudioActive
// ==============================================================================
Boolean CAudioMixer::IsAnyAudioActive( void )
{
// Search for a channel that is in use
for (long i = 0; i < numInChannels_; i++)
	{
	if (pChannels_[i].IsInUse())
		return (true);
	}

return (false);
}  // ---- end IsAnyAudioActive() ----

// ==============================================================================
// GetMixBinIndex : determine mix bin index from sampling frequency
// ==============================================================================
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
}  // ---- end GetMixBinIndex() ----

// ==============================================================================
// GetSamplingRateDivisor : Determine divisor sampling frequency, which is your operating
//				sampling frequency
// ==============================================================================
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
}  // ---- end GetSamplingRateDivisor() ----

// ==============================================================================
// CreatePlayer:   Allocate player based on file extension argument
// ==============================================================================
    CAudioPlayer *
CAudioMixer::CreatePlayer(tAudioStartAudioInfo *pAudioInfo, char *sExt, tAudioID newID )
{
CAudioPlayer *pPlayer = NULL;

if (!strcmp(sExt, "raw")  || !strcmp( sExt, "RAW")  ||
    !strcmp(sExt, "brio") || !strcmp( sExt, "BRIO") ||
    !strcmp(sExt, "aif")  || !strcmp( sExt, "AIF")  ||
    !strcmp(sExt, "aiff") || !strcmp( sExt, "AIFF") ||
    !strcmp(sExt, "wav")  || !strcmp( sExt, "WAV") )
	{
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartAudio: Create RawPlayer\n");
	pPlayer = new CRawPlayer( pAudioInfo, newID );
    }
else if (!strcmp( sExt, "ogg" ) || !strcmp( sExt, "OGG") ||
         !strcmp( sExt, "aogg") || !strcmp( sExt, "AOGG"))
    {
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartAudio: Create VorbisPlayer\n");
	pPlayer = new CVorbisPlayer( pAudioInfo, newID );
    } 
else
    {
	pDebugMPI_->DebugOut( kDbgLvlImportant,
		"AudioTask::DoStartAudio: Create *NO* Player: unhandled audio type='%s'\n", sExt);
    }

return (pPlayer);
}  // ---- end CreatePlayer() ----

// ==============================================================================
// AddPlayer:
// ==============================================================================
    tErrType 
CAudioMixer::AddPlayer( tAudioStartAudioInfo *pAudioInfo, char *sExt, tAudioID newID )
{
CChannel *pChannel = FindFreeChannel( pAudioInfo->priority );
if (!pChannel)
    {
    printf("CAudioMixer::AddPlayer: no channel available\n");
    return (kAudioNoChannelAvailErr);
    }
CAudioPlayer *pPlayer = CreatePlayer(pAudioInfo, sExt, newID);
if (!pPlayer)
    {
    printf("CAudioMixer::AddPlayer: failed to create Player with '%s'\n", sExt);
    return (kAudioNoChannelAvailErr);  // GK FIXXX: add error for failed player allocation
    }

#ifdef NEW_ADD_PLAYER
// Trigger flags to call SetPlayer() in Mixer render loop
// GK NOTE:  this is crap due to race condition
targetChannel_ = pChannel;  // Do this one first
playerToAdd_   = pPlayer;
//pChannel->SetPlayer(pPlayer, true);
//pChannel->SetInUse(true);
#endif // NEW_ADD_PLAYER

#ifdef OLD_ADD_PLAYER
pChannel->InitWithPlayer( pPlayer );
#endif // OLD_ADD_PLAYER

return (kNoErr);
}  // ---- end AddPlayer() ----

// ==============================================================================
// RenderBuffer
// ==============================================================================
    int 
CAudioMixer::RenderBuffer( S16 *pOutBuf, U32 numFrames )
// numFrames  IS THIS FRAMES OR SAMPLES  !!!!!!!  THIS APPEARS TO BE SAMPLES
{
U32 	i, ch;
U32 	playerFramesRendered;
long    mixBinIndex;
long    channelsPerFrame = kAudioMixer_MaxOutChannels;
// FIXXXX: mystery.  Offset Ptrs don't work !!!
short **tPtrs = pTmpBufs_; // pTmpBufs_, tmpBufOffsetPtrs_

// numFrames = 256 for EMULATION, 128? for embedded ??
//{static long c=0; printf("CAudioMixer::RenderBuffer : start %ld  numFrames=%ld channels=%ld\n", c++, numFrames, channelsPerFrame);}

// Update parameters from AudioState
// GK FIXXX:  should wrap this in a Mutex
if (audioState_.useOutSoftClipper)
    audioState_.headroomBits = 1+kMixer_HeadroomBits_Default;
else
    audioState_.headroomBits = kMixer_HeadroomBits_Default;

// GK FIXXXX: HACK  need to mutex-protect this
#ifdef NEW_ADD_PLAYER
if (playerToAdd_)
    {
    CAudioPlayer *pPlayer  = playerToAdd_;
    CChannel     *pChannel = targetChannel_;
    playerToAdd_  = NULL;
    targetChannel_= NULL;
printf("CAudioMixer::RenderBuffer: Adding player\n");
    pChannel->SetPlayer(pPlayer, true); 
    pChannel->SetInUse(true);
    pChannel->isDone_ = false;
printf("CAudioMixer::RenderBuffer: Added player\n");
    }
#endif // NEW_ADD_PLAYER

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
if (readInSoundFile_ && !inSoundFileDone_)
	{
    long mixBinIndex  = GetMixBinIndex(inSoundFileInfo_.samplerate);
	S16 *mixBinP      = (S16 *) mixBinBufferPtrs_[mixBinIndex][0];
	long framesToRead = numFrames/GetSamplingRateDivisor(inSoundFileInfo_.samplerate);
	long framesRead   = 0;

    ClearShorts(mixBinP, framesToRead*2);
// Copy stereo input to stereo mix buffer
	if (2 == inSoundFileInfo_.channels)
 		framesRead = sf_readf_short(inSoundFile_, mixBinP, framesToRead);
// Replicate mono input file to both channels of stereo mix buffer
	else
		{
 		framesRead = sf_readf_short(inSoundFile_, pTmpBufs_[0], framesToRead);
		InterleaveShorts(pTmpBufs_[0], pTmpBufs_[0], mixBinP, framesRead);
		}
    if (framesRead < framesToRead)
        {
	    inSoundFileDone_ = true;
        printf("inSoundFileDone ! %ld/%ld frames read \n", framesRead, framesToRead); 
        }
	mixBinFilled_[mixBinIndex] = True;
//printf("AudioMixer::RenderBuffer: read %ld samples from WAV file \n", framesRead);
//  inSoundFileInfo.samplerate, frames, channels
	}
else
{
ClearShorts(pChannel_OutBuffer_, numFrames*channelsPerFrame);
// Render if channel is in use and not paused
for (ch = 0; ch < numInChannels_; ch++)
	{
	CChannel *pCh = &pChannels_[ch];
//printf("CAudioMixer::RenderBuffer : pCh%ld->ShouldRender=%d \n", ch, pCh->ShouldRender());
	if (pCh->ShouldRender())
		{
        long channelSamplingFrequency = pCh->GetSamplingFrequency();
		U32 framesToRender = (numFrames*channelSamplingFrequency)/(long)samplingFrequency_;
        U32 sampleCount    = framesToRender*channelsPerFrame;
//printf("ch%ld: framesToRender=%ld for %ld Hz\n", ch, framesToRender, channelSamplingFrequency);

	// Player renders data into channel's stereo output buffer.  
        playerFramesRendered = pCh->RenderBuffer( pChannel_OutBuffer_, framesToRender );
	    if ( playerFramesRendered < framesToRender ) 
//printf("frames %ld/%ld\n", playerFramesRendered, framesToRender);
//	    if ( 0 == playerFramesRendered ) 
            {
//            ClearShorts(pChannel_OutBuffer_, framesToRender);
//			pCh->Release( true );	// false = Don't suppress done msg 
 
            pCh->isDone_ = true;
            pCh->fInUse_ = false;
            if (pCh->GetPlayerPtr()->ShouldSendDoneMessage())
                pCh->SendDoneMsg();
            }

        if (inputIsDC) 
            SetShorts(pChannel_OutBuffer_, sampleCount, inputDCValuei);
#ifdef NEED_SAWTOOTH
        else if (inputIsSawtoothWave_)
            {
            SawtoothOscillator_S16(pChannel_OutBuffer_, sampleCount, &z_, delta_);
printf("Sawtooth z=%ld delta=%ld out[0]=%d\n", z_, delta_, pChannel_OutBuffer_[0]);
printf("Sawtooth z=$%08X delta=$%08X out[0]=$%04X\n", (unsigned int)z_, (unsigned int)delta_, (unsigned short) pChannel_OutBuffer_[0]);
            }
#endif
	// Add output to appropriate Mix "Bin" 
		long mixBinIndex = GetMixBinIndex(channelSamplingFrequency);
// FIXXX:  convert fs/4->fs/2 with gentler anti-aliasing filter and let fs/2 mix bin's conversion do fs/2->fs
		S16 *pMixBin = mixBinBufferPtrs_[mixBinIndex][0];
        ShiftRight_S16(pChannel_OutBuffer_, pChannel_OutBuffer_, sampleCount, audioState_.headroomBits);
        AccS16toS16(pMixBin, pChannel_OutBuffer_, sampleCount, mixBinFilled_[mixBinIndex]);

		mixBinFilled_[mixBinIndex] = True;
		}
	}

// MIDI player renders to fs/2 output buffer 
if ( pMidiPlayer_->IsActive() )
	{
	long mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
	S16 *pMixBin = mixBinBufferPtrs_[mixBinIndex][0];
	long framesToRender  = (numFrames/2);
    U32 sampleCount = framesToRender*channelsPerFrame;

	playerFramesRendered = pMidiPlayer_->RenderBuffer( pChannel_OutBuffer_, framesToRender); 
//{static long c=0; printf("CAudioMixer::RenderBuffer: MIDI active %ld framesToRender=%ld\n", c++, framesToRender);}
    ShiftRight_S16(pChannel_OutBuffer_, pChannel_OutBuffer_, sampleCount, audioState_.headroomBits);
    AccS16toS16(pMixBin, pChannel_OutBuffer_, sampleCount, mixBinFilled_[mixBinIndex]);

	mixBinFilled_[mixBinIndex] = True;
	}
}

// 
// Combine Mix buffers to output, converting sampling frequency if necessary
//
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv1;
if (mixBinFilled_[mixBinIndex])
    CopyShorts(mixBinBufferPtrs_[mixBinIndex][0], pOutBuf, numFrames); 
else
	ClearShorts( pOutBuf, numFrames * channelsPerFrame );

#define MIX_THE_MIX_BINS
#ifdef MIX_THE_MIX_BINS
// Deinterleave and process each Mix Buffer separately to OutBuf
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv4;
if (mixBinFilled_[mixBinIndex])
	{
	DeinterleaveShorts(mixBinBufferPtrs_[mixBinIndex][0], tPtrs[2], tPtrs[3], numFrames/4);
	RunSRC(tPtrs[2], tPtrs[0], numFrames/4, numFrames, &src_[mixBinIndex][0]);
	RunSRC(tPtrs[3], tPtrs[1], numFrames/4, numFrames, &src_[mixBinIndex][1]);
	InterleaveShorts(tPtrs[0], tPtrs[1], tPtrs[6], numFrames);
	Add2_Shortsi(tPtrs[6], pOutBuf, pOutBuf, numFrames*channelsPerFrame);
//CopyShorts(tPs[6], pOutBuf, numFrames*channelsPerFrame);
	}

mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
if (mixBinFilled_[mixBinIndex])
	{
	DeinterleaveShorts(mixBinBufferPtrs_[mixBinIndex][0], tPtrs[4], tPtrs[5], numFrames/2);
	RunSRC(tPtrs[4], tPtrs[0], numFrames/2, numFrames, &src_[mixBinIndex][0]);
	RunSRC(tPtrs[5], tPtrs[1], numFrames/2, numFrames, &src_[mixBinIndex][1]);
	InterleaveShorts(tPtrs[0], tPtrs[1], tPtrs[6], numFrames);
	Add2_Shortsi(tPtrs[6], pOutBuf, pOutBuf, numFrames*channelsPerFrame);
//CopyShorts(tPs[6], pOutBuf, numFrames*channelsPerFrame);
	}
		
// fixme/rdg:  needs CAudioEffectsProcessor
// If at least one audio channel is playing, Process global audio effects
//if (numPlaying && (gAudioContext->pAudioEffects != kNull))
//	gAudioContext->pAudioEffects->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuf );


// Scale stereo/interleaved buffer by master volume
/// NOTE: volume here is interpreted as a linear value
//ScaleShortsf(pOutBuf, pOutBuf, numFrames*channelsPerFrame, masterGainf_[0]);
ScaleShortsi_Q15(pOutBuf, pOutBuf, numFrames*channelsPerFrame, masterGaini_[0]);

// Test with DC input
if (inputIsDC)
    {
    outputDCValuei  = pOutBuf[0];
    outputDCValuef  = Q15ToFloat(outputDCValuei);
    outputDCValueDB = LinearToDecibelf(outputDCValuef);

printf("CAudioMixer: in DC %g dB -> %g |out %g dB -> %g (%d)\n", 
        inputDCValueDB, inputDCValuef, outputDCValueDB, outputDCValuef, outputDCValuei);
    }
#endif // end MIX_THE_MIX_BINS

// ---- Output DSP block

//long computeLevelMeters = audioState_.computeLevelMeters;
//useOutEQ_               = audioState_.useOutEQ;
//useOutSoftClipper_      = audioState_.useOutSoftClipper;
if (audioState_.useOutEQ || audioState_.useOutSoftClipper || audioState_.computeLevelMeters)
    audioState_.useOutDSP  = true;

{static long c=0; if (! (c++)) {
printf("CAudioMixer: useOutDSP=%d computeLevelMeters=%d useOutEQ=%d useOutSoftClipper=%d\n", audioState_.useOutDSP, audioState_.computeLevelMeters, audioState_.useOutEQ , audioState_.useOutSoftClipper); 

printf("CAudioMixer: readInSoundFile  =%ld Path='%s'\n", readInSoundFile_, inSoundFilePath_);
printf("CAudioMixer: writeOutSoundFile=%ld Path='%s'\n", writeOutSoundFile_, outSoundFilePath_);
}
}

if (audioState_.useOutDSP)
{
DeinterleaveShorts(pOutBuf, tPtrs[4], tPtrs[5], numFrames);

for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
    {
    short *pIn  = tPtrs[4+ch];
    short *pOut = tPtrs[4+ch];

// Compute Output Equalizer
// NOTE: should have 32-bit data path here for EQ before soft clipper
//audioState_.useOutEQ = False;
    if (audioState_.useOutEQ)
        {
//{static long c=0; if (!c) printf("ComputeEQ %ld : bands=%ld \n", c++, outEQ_BandCount_);}
        for (long j = 0; j < outEQ_BandCount_; j++)
            {
//            ComputeEQf(pIn, pOut, numFrames, &outEQ_[ch][j]);
            ComputeEQi(pIn, pOut, numFrames, &outEQ_[ch][j]);
            pOut = pIn;
            }
        }
// Compute Output Soft Clipper
//{static long c=0; printf("ComputeWaveShaper %ld On=%d: \n", c++, audioState_.useOutSoftClipper);}
    if (audioState_.useOutSoftClipper)
        {
//{static long c=0; if (!c) printf("ComputeWaveShaper %ld On=%d: \n", c++, audioState_.useOutSoftClipper);}
        ComputeWaveShaper(pIn, pOut, numFrames, &outSoftClipper_[ch]);
        }
    }
InterleaveShorts(tPtrs[4], tPtrs[5], pOutBuf, numFrames);
ShiftLeft_S16(pOutBuf, pOutBuf, numFrames*channelsPerFrame, audioState_.headroomBits);

// Audio level meters
//  Scan buffer for maximum value
if (audioState_.computeLevelMeters)
    {
    tAudioState *as = &audioState_;
// Scan output buffer for max values
    for (long ch = kLeft; ch <= kRight; ch++)
        {
//        S16 x = MaxAbsShorts(tPtrs[4+ch], numFrames);
        S16 x = MaxAbsShorts(&pOutBuf[ch], numFrames, 2);
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
                longTimeHoldCounter  = 0;
                }
            else  
                {
                if (longTimeHoldCounter >= longTimeHoldInterval)
                    {
//                outLevels_LongTime[ch] = (S16) (longTimeDecayF * (float) outLevels_LongTime[ch]);
                 outLevels_LongTime[ch] = MultQ15(outLevels_LongTime[ch], longTimeDecayI);
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
//printf("Short=%5d Long=%5d \n", outLevels_ShortTime[0], outLevels_LongTime[0]);
    } // end computeLevelMeters

}  // end if (useOutDSP)

// Debug:  write output of mixer to sound file
static long outBufferCount = 0; outBufferCount++;
if (writeOutSoundFile_)
    {
static long totalFramesWritten = 0;
    long framesWritten = sf_writef_short(outSoundFile_, pOutBuf, numFrames);
    totalFramesWritten += framesWritten;
//{static long c=0; printf("CAudioMixer::RenderBuffer %ld: outCount=%ld wrote %ld frames total=%ld\n", c++, outBufferCount, framesWritten, totalFramesWritten);}

//    framesWritten = 0;
    if (inSoundFileDone_ || outBufferCount >= 1000)
        {
printf("Closing outSoundFile '%s'\n", outSoundFilePath_);
    	CloseSoundFile(&outSoundFile_);
        outSoundFile_      = NULL;
        writeOutSoundFile_ = false;
        }
    }

//printf("CAudioMixer::RenderBuffer: END \n");
	return (kNoErr);
} // ---- end RenderBuffer() ----

// ==============================================================================
// WrapperToCallRenderBuffer
// ==============================================================================
int CAudioMixer::WrapperToCallRenderBuffer( S16 *pOut,  unsigned long numStereoFrames, void* pToObject  )
{
//	CAudioMixer* mySelf = (CAudioMixer*)pToObject;
	
	// Call member function to get a buffer full of stereo data
	return ((CAudioMixer*)pToObject)->RenderBuffer( pOut, numStereoFrames );
} // ---- end WrapperToCallRenderBuffer() ----

// ==============================================================================
// SetMasterVolume :  Set master output level for mixer
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
} // ---- end SetMasterVolume() ----

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
} // ---- end SetSamplingFrequency() ----

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
} // ---- end UpdateDSP() ----

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
} // ---- end ResetDSP() ----

// ==============================================================================
// PrepareDSP :  Update + Reset DSP state
// ==============================================================================
void CAudioMixer::PrepareDSP()
{
UpdateDSP();
ResetDSP();
} // ---- end PrepareDSP() ----

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

} // ---- end UpdateDebugGain() ----

// ==============================================================================
// SetAudioState :  Set audio state
// ==============================================================================
void CAudioMixer::SetAudioState(tAudioState *d)
{
//printf("CAudioMixer::SetAudioState: start srcType=%d\n", d->srcType);

bcopy(d, &audioState_, sizeof(tAudioState));
//printf("CAudioMixer::SetAudioState: audioState_.srcType=%d\n", audioState_.srcType);
} // ---- end SetAudioState() ----

// ==============================================================================
// GetAudioState :  Get audio state
// ==============================================================================
void CAudioMixer::GetAudioState(tAudioState *d)
{
//printf("CAudioMixer::GetAudioState: start srcType=%d\n", d->srcType);

bcopy(&audioState_, d, sizeof(tAudioState));
} // ---- end GetAudioState() ----

LF_END_BRIO_NAMESPACE()
