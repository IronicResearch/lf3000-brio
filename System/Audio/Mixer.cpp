// ==============================================================================
// Copyright (c) 2002-2008 LeapFrog Enterprises, Inc.
// All Rights Reserved
// ==============================================================================
//
// Mixer.cpp
//
// The following is the basic structure of the audio system:
//
// +----------+   +-----------+    +---------+
// | Player A |---| Channel 1 |--->|         |
// +----------+   +-----------+    |         |
//                                 |         |
// +----------+   +-----------+    |         |                          /|
// | Player B |---| Channel 2 |--->|         |                         / |
// +----------+   +-----------+    |         |                      +-+  |
//                                 |         |     +-----------+    | |  |
//                +-----------+    |  Mixer  +---->| Audio Out |----| |  |   
//                | Channel 3 |--->|         |     +-----------+    | |  |  
//                +-----------+    |         |                      +-+  |
//                                 |         |                         \ |
//                   ....          |         |                          \|
//                                 |         |
//                +-----------+    |         |
//                | Channel N |--->|         |
//                +-----------+    |         |
//                                 +---------+
//
// The Mixer implemented in this file "owns" all of the objects show above.
// They are all allocated/deleted and otherwise managed right here.  The
// AudioMPI, which is implemented in Audio.cpp and specified in AudioMPI.h is
// the front end that controls the mixer.  However, it mostly just makes calls
// into this mixer to add players, change volume, etc.
//
// The Audio Out stage is of particular importance to the entire system.  It's
// essential component is a "render loop" that feeds the audio hardware.  Each
// iteration of the render loop populates a buffer whose length is generally
// about 20-40ms.  To ensure real time operation, the render loop should be free
// from all unnecessary calculations.  Note that the lowest level render loop is
// implemented by Port Audio.
//
// Generally, the AudioMPI changes the mixer state in some application thread.
// But the render loop is operating in its own thread.  To prevent the mixer
// from finding itself in an inconsistent state during a render iteration, the
// mixer uses a mutex.  This mutex should ONLY be accessed by the mixer.  It
// must be locked at the top of the render loop and unlocked at the end.  Any
// external calls that alter the mixer state in a way that affects the render
// MUST lock the mutex.
//
// By design, users of the AudioMPI launch an audio stream with the StartAudio
// function and get a call back via the Event subsystem when the stream is
// finished.  This call back MUST be initiated somehow from the render loop.
// Effort SHOULD be made to make this callback as quick as possible, because the
// render loop is so time sensitive.  Ultimately, the call backs should be
// queued and processed outside of the render loop.  This may require the
// addition of another thread.
// ==============================================================================
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#include "sndfileutil.h"

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Mixer.h>
#include <AudioOutput.h>
#include <Utility.h>

#include <RawPlayer.h>
#include <VorbisPlayer.h>

#undef ENABLE_PROFILING
#include <FlatProfiler.h>

#define kMixer_HeadroomBits_Default 2

#define kMixer_Headphone_GainDB        0 
#define kMixer_SoftClipper_PreGainDB   3 
#define kMixer_SoftClipper_PostGainDB  0

// Debug input/output stuff

float outputDCValueDB;
float outputDCValuef;
Q15   outputDCValuei;

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define	BRIO_MIDI_PLAYER_ID		1	// FIXXX: Hard code player ID for now

#define MIXER_LOCK pDebugMPI_->Assert((kNoErr == pKernelMPI_->LockMutex(mixerMutex_)),\
                                      "Couldn't lock mixer mutex.\n")

#define MIXER_UNLOCK pDebugMPI_->Assert((kNoErr == pKernelMPI_->UnlockMutex(mixerMutex_)),\
                                        "Couldn't unlock  mixer mutex.\n")

//==============================================================================
// Global variables
//==============================================================================

// These should not be globals.  They should be in the object as they were.  But
// somebody insisted on having static member functions that pass references to
// "this."
CDebugMPI *pDebugMPI_;
CEventMPI  *pEventMPI_;
CKernelMPI *pKernelMPI_;
tMutex mixerMutex_;

//------------------------------------------------------------------------------
//Returns speaker status from sysfs audio driver dump (embedded)
// 
// Not to be confused with IsSpeakerEnabled, which just returns the last known
// state.
//------------------------------------------------------------------------------
static bool IsSpeakerOn( void )
{
#ifdef EMULATION
	return true;
#else
	int fd;
	int sw = 0;
	char *devname = GetKeyboardName();

	if(devname == NULL) {
		//printf("error: no keyboard device\n");
		return true;
	}

	// open the keyboard driver
	fd = open(devname, O_RDONLY);
	if(fd < 0) {
		//printf("failed to open keyboard device\n");
		return true;
	}
	
	// ask for the state of the 'switches'
	if(ioctl(fd, EVIOCGSW(sizeof(int)), &sw) < 0) {
		//perror("failed to get switch state\n");
		close(fd);
		return true;
	}

	close(fd);

	if(sw & (1<<SW_HEADPHONE_INSERT))
		return false;
	return true;
#endif	// EMULATION
}

// ==============================================================================
// CAudioMixer implementation
// ==============================================================================
CAudioMixer::CAudioMixer( int inChannels )
{
long i, j, ch;
//printf("CAudioMixer::CAudioMixer: inChannels=%d Max=%d\n", inChannels, kAudioMixer_MaxInChannels);
    const tMutexAttr    attr = {0};

	numInChannels_ = inChannels;
if (numInChannels_ > kAudioMixer_MaxInChannels)
	printf("CAudioMixer::CAudioMixer: %d too many channels! Max=%d\n", numInChannels_, kAudioMixer_MaxInChannels);
	pDebugMPI_->Assert((numInChannels_ <= kAudioMixer_MaxInChannels), "CAudioMixer::CAudioMixer: %d is too many channels!\n", numInChannels_ );

#ifdef NEW_ADD_PLAYER
    playerToAdd_  = NULL;
    targetChannel_= NULL;
#endif

    samplingFrequency_ = kAudioSampleRate;

	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose); //kAudioDebugLevel );
    
	pEventMPI_ = new CEventMPI();

	pKernelMPI_ = new CKernelMPI();

    // Init mutex for serialization access to internal AudioMPI state.
    tErrType err = pKernelMPI_->InitMutex( mixerMutex_, attr );
    pDebugMPI_->Assert((kNoErr == err), "%s: Couldn't init mutex.\n", __FUNCTION__);
    
// Allocate audio channels
	pChannels_ = new CChannel[ numInChannels_ ];
	pDebugMPI_->Assert((pChannels_ != kNull), "CAudioMixer::CAudioMixer: couldn't allocate %d channels!\n" , numInChannels_);
//    pChannelBuf_ = new S16[ kAudioOutBufSizeInWords ];

// Create MIDI player
//	pMidiPlayer_ = new CMidiPlayer( BRIO_MIDI_PLAYER_ID );
    pMidiPlayer_ = NULL;

// Configure sampling frequency conversion
fsRack_[0] = (long)(samplingFrequency_*0.25f);
fsRack_[1] = (long)(samplingFrequency_*0.5f );
fsRack_[2] = (long)(samplingFrequency_);       

for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
//	pMixBinBufs_[i] = new S16[/* 2* */ kAudioMixer_MaxOutChannels*kAudioOutBufSizeInWords]; // GK FIXXX: 2x needed ??
	mixBinFilled_[i] = False;
	}

for (i = 0; i < kAudioMixer_SRCCount; i++)
	{
// All SRC units will be set for 2x interpolation:   fs/4 -> fs/2  , fs/2 -> fs
	for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
		{
// Initialize sampling rate converters  
        SRC *src = &src_[i][ch];
		DefaultSRC(src);
// kSRC_Interpolation_Type : {Triangle, AddDrop, Linear, Triangle, FIR, IIR, Box}
        src->type = kSRC_Interpolation_Type_FIR;
// V6: no go.  Alias on 1000/8000Hz very  audible  
// V7: ok:     Alias on 1000/8000Hz quite audible
// V8: good.   Alias on 1000/8000Hz inaudible 
        src->filterVersion = kSRC_FilterVersion_8; // 6=30dB_15 7=50dB_31 8=50dB_41

//		SRC_SetInSamplingFrequency (src, (float)fsRack_[i]);
//		SRC_SetOutSamplingFrequency(src, samplingFrequency_);
//		SRC_SetOutSamplingFrequency (src, (float)(2*fsRack_[i]));  // Don't worry.  Bin0 SRC not used.

//		UpdateSRC(src);
//		ResetSRC(src);
		}
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

for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
    ClearShorts(pTmpBufs_[i], kAudioMixer_TempBufferWords);
	pTmpBufOffsets_[i] = &pTmpBufs_[i][kSRC_Filter_MaxDelayElements];  
	}
// Set up level meters
for (ch = kLeft; ch <= kRight; ch++)
    {
    outLevels_ShortTime[ch] = 0;
    outLevels_LongTime [ch] = 0;
    temp_ShortTime     [ch] = 0;
    }

// numFrames = 256 for EMULATION, 512 for embedded
int numFramesPerBuffer = kAudioFramesPerBuffer;
//printf("CAudioMixer::CAudioMixer: numFramesPerBuffer=%d\n", numFramesPerBuffer);

U32 shortTimeRateHz = 12;
U32 longTimeRateHz  =  1;
shortTimeCounter    = 0;
longTimeHoldCounter = 0;
shortTimeInterval    = kAudioSampleRate/(shortTimeRateHz * numFramesPerBuffer);
longTimeHoldInterval = shortTimeInterval/longTimeRateHz;

float rateF = ((float)kAudioSampleRate)/(float)(shortTimeRateHz *numFramesPerBuffer);
longTimeDecayF = (1.0f - 0.5f/rateF)/(float)shortTimeRateHz; 
longTimeDecayF *= 6.0f;
longTimeDecayI = FloatToQ15(longTimeDecayF);
//printf("rateF=%g longTimeDecayF=%g\n", rateF, longTimeDecayF);
//printf("shortTime: Hz=%ld frames=%d Interval=%ld longTimeDecayF=%g\n", 
//        shortTimeRateHz, numFramesPerBuffer, shortTimeInterval, longTimeDecayF);

// Headphone gain
headphoneGainDB_     = kMixer_Headphone_GainDB;
headphoneGainF_      = DecibelToLinearf(headphoneGainDB_);
headphoneGainWholeI_ = (long)headphoneGainFracF_;
headphoneGainFracF_  = headphoneGainF_ - (float)headphoneGainWholeI_;
headphoneGainFracI_  = FloatToQ15(headphoneGainFracF_);
//printf("CAudioMixer::CAudioMixer: headphoneGainDB %g -> %g\n", headphoneGainDB_, headphoneGainF_);
//printf("CAudioMixer::CAudioMixer: headphoneGain WholeI=%d FracF=%g FracI=$%X\n", 
//        headphoneGainWholeI_, headphoneGainFracF_, (unsigned int) headphoneGainFracI_);
//
// Set up Audio State struct (interfaces via AudioMPI)
// NOTE : Keep at end of this routine
{
tAudioState *d = &audioState_;
memset(d, 0, sizeof(tAudioState));
if (sizeof(tAudioState) >= kAUDIO_MAX_MSG_SIZE)
    printf("UH OH CAudioMixer: sizeof(tAudioState)=%d kAUDIO_MAX_MSG_SIZE=%ld\n", sizeof(tAudioState), kAUDIO_MAX_MSG_SIZE);

d->computeLevelMeters = false;
d->useOutEQ           = false;
d->useOutSoftClipper  = true;
d->useOutDSP          = (d->computeLevelMeters || d->useOutEQ || d->useOutSoftClipper);

SetMasterVolume(100); //kVolume_Default);

d->systemSamplingFrequency = (long)samplingFrequency_;
d->outBufferLength = kAudioFramesPerBuffer;  // Dunno?!? Words? Frames?

	bool isSpeaker = IsSpeakerOn();
	EnableSpeaker(isSpeaker);

//if (d->useOutSoftClipper)
//    d->headroomBits = kMixer_HeadroomBits_Default;
//else
    d->headroomBits = kMixer_HeadroomBits_Default;

d->softClipperPreGainDB  = (S16) kMixer_SoftClipper_PreGainDB;
d->softClipperPostGainDB = (S16) kMixer_SoftClipper_PostGainDB;

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

// DEBUG:  Wav File I/O
d->readInSoundFile   = false;
d->writeOutSoundFile = false;
inSoundFileDone_     = false;
d->outFileBufferCount = 1000;

memset(d->inSoundFilePath, '\0', kAudioState_MaxFileNameLength);
strcpy(d->inSoundFilePath, "Saw_32k_m3dB_m.wav");
//"Saw_32k_m0dB_m.wav, Saw_32k_m3dB_m.wav, Saw_32k_m6dB_m.wav";

//"Sine_1kHz_0dB_15Sec_m_32k.wav";
//"SINE/sine_db00_0500Hz_16k_c1.wav";
memset(d->outSoundFilePath, '\0', kAudioState_MaxFileNameLength);
strcpy(d->outSoundFilePath, "BrioOut.wav");

inSoundFile_  = NULL;
outSoundFile_ = NULL;
} // end Set up Audio State struct

// 
// ---- Configure DSP engine
//
for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
// Configure output Equalizer
    outEQ_BandCount_ = 1;
    for (j = 0; j < kAudioMixer_MaxEQBands; j++)
        {
        DefaultEQ(&outEQ_[ch][j]);
//    SetEQ_SamplingFrequency(d, samplingFrequency_);
        }

// Configure output "soft" clipper
    DefaultWaveShaper(&outSoftClipper_[ch]);
    outSoftClipper_[ch].useFixedPoint = true;
//    SetWaveShaper_SamplingFrequency(&outSoftClipper_[ch], samplingFrequency_);
    }
SetSamplingFrequency( samplingFrequency_ );

SetDSP();
UpdateDSP();
ResetDSP();

	// Init output driver and register callback.  We have to pass in a pointer
	// to the mixer object as a bit of "user data" so that when the callback happens,
	// the C call can get to the mixer's C++ member function for rendering.  
	err = InitAudioOutput( &CAudioMixer::WrapperToCallRender,
						   (void *)this);
	pDebugMPI_->Assert(kNoErr == err,
					   "Failed to initalize audio output\n" );
	err = StartAudioOutput();
	pDebugMPI_->Assert(kNoErr == err,
					   "Failed to start audio output\n" );

	pDebugMPI_->Assert(FlatProfilerInit(0, 0) == 0, "Failed to init profiler.\n");
//PrintMemoryUsage();
}  // ---- end CAudioMixer::CAudioMixer() ----

// ==============================================================================
// ~CAudioMixer :
// ==============================================================================
CAudioMixer::~CAudioMixer()
{
long i;

	StopAudioOutput();
    
	DeInitAudioOutput();

	MIXER_LOCK; 

// Deallocate channels
 if (pChannels_)
	{
    for (long ch = 0; ch < numInChannels_; ch++)
        {     	
//        delete pChannels_[ch];
        }
	}
	
// Deallocate MIDI player
	if (pMidiPlayer_)
		delete pMidiPlayer_;

	for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
		{
//		if (pTmpBufs_[i])
//			free(pTmpBufs_[i]);
//        pTmpBufs_      [i] = NULL;
        pTmpBufOffsets_[i] = NULL;
		}
	
	for (long i = 0; i < kAudioMixer_MixBinCount; i++)
		{
//		if (pMixBinBufs_[i])
//			free(pMixBinBufs_[i]);
		}
	
//if (audioState_.writeOutSoundFile)
// {
// }

if (outSoundFile_)
    {
    CloseSoundFile(&outSoundFile_);
    outSoundFile_ = NULL;
    }

 MIXER_UNLOCK; 
 pKernelMPI_->DeInitMutex( mixerMutex_ );

 delete pKernelMPI_;

 delete pEventMPI_;

 delete pDebugMPI_;

 FlatProfilerDone();

}  // ---- end ~CAudioMixer() ----

// ==============================================================================
// CAudioMixer::OpenInSoundFile :
//
//                              Return Boolean success
// ==============================================================================   
    int
CAudioMixer::OpenInSoundFile(char *path)
{
#ifdef EMULATION
//char *inFilePath = ".\\"; // /home/lfu/AudioFiles/
#else
//char *inFilePath = "/AudioFiles/";
#endif

// SINE/ : sine_db0_1000Hz_32k.wav sine_dbM3_1000Hz_32k sine_dbM6_1000Hz_32k
// at db {00, M3, M6} and fs={16k,32k} and f= { 125, 250, 500, 1000, 2000, 4000, 8000 }
// Representative Audio
//char *inFileName = inSoundFilePath_;
//printf("inFileName='%s'\n", path);

inSoundFile_ = OpenSoundFile(path, &inSoundFileInfo_, SFM_READ);if (!inSoundFile_)
	{
	printf("CAudioMixer::OpenInSoundFile: Unable to open file '%s'\n", path);
	audioState_.readInSoundFile = False;
    return (false);
	}
strcpy(audioState_.inSoundFilePath, path);

printf("CAudioMixer::OpenInSoundFile: opened file '%s'\n", audioState_.inSoundFilePath);
// inSoundFileInfo.samplerate, frames, channels
//	printf("CAudioMixer::OpenInSoundFile: opened inFile: fs=%d frames=%ld ch=%d \n", 
//		inSoundFileInfo.samplerate, inSoundFileInfo.frames, inSoundFileInfo.channels);

inSoundFileDone_ = false;
return (true);
}  // ---- end CAudioMixer::OpenInSoundFile() ----

// ==============================================================================
// CAudioMixer::OpenOutSoundFile :
//
//                              Return Boolean success
// ==============================================================================
    int
CAudioMixer::OpenOutSoundFile(char *path)
{
outSoundFileInfo_.frames     = 0;		
outSoundFileInfo_.samplerate = (long) samplingFrequency_;
outSoundFileInfo_.channels   = 2;
outSoundFileInfo_.format     = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
outSoundFileInfo_.sections   = 1;
outSoundFileInfo_.seekable   = 1;

outSoundFile_ = OpenSoundFile(path , &outSoundFileInfo_, SFM_WRITE);
if (!outSoundFile_)
    {
    printf("CAudioMixer::OpenOutSoundFile: unable to open '%s' \n", path);
    return (false);
    }
strcpy(audioState_.outSoundFilePath, path);
printf("CAudioMixer::OpenOutSoundFile: opened out file '%s'\n", audioState_.outSoundFilePath);

return (true);
}  // ---- end CAudioMixer::OpenOutSoundFile() ----

// ==============================================================================
// FindFreeChannel:    Find a free channel using priority -> GKFIXX: add search by priority
// ==============================================================================
    CChannel * 
CAudioMixer::FindFreeChannel( tAudioPriority /* priority */)
{
CChannel *pCh;
//{static long c=0; printf("CAudioMixer::FindFreeChannel%d: searching for free channel ... \n", c++);}

#ifdef KEEP_FOR_DEBUGGING
for (long i = 0; i < numInChannels_; i++)
	{
    pCh = &pChannels_[i];
//printf("CAudioMixer::FindFreeChannel: %ld: IsInUse=%d isDone=%d player=%p\n", i, pCh->IsInUse(), pCh->isDone_, (void*)pCh->GetPlayerPtr());
    }
#endif

long active = 0;
long idle   = 0;
long paused = 0;
for (long i = 0; i < numInChannels_; i++)
    {
    active +=  pChannels_[i].IsInUse(); //(!pChannels_[i]->IsPaused());
    idle   += (!pChannels_[i].IsInUse());
    paused +=   pChannels_[i].IsPaused();
    }

//#define DEBUG_MIXER_FINDFREECHANNEL
#ifdef DEBUG_MIXER_FINDFREECHANNEL
{
for (long i = 0; i < numInChannels_; i++)
    {
    pCh = &pChannels_[i];
printf("CAudioMixer::FindFreeChannel: ch%ld idle=%d paused=%d\n", i, !pCh->IsInUse(), pCh->IsPaused());
    }
printf("CAudioMixer::FindFreeChannel: idle=%ld active=%ld paused=%ld total=%d\n", idle, active, paused, numInChannels_);
}
#endif // DEBUG_MIXER_FINDFREECHANNEL

// Although more channels are actually available, limit to
// preset active count
if (active >= kAudioMixer_MaxActiveAudioChannels)
    {
printf("CAudioMixer::FindFreeChannel: all %ld/%d active channels in use\n", active, kAudioMixer_MaxActiveAudioChannels);
	return (kNull);
    }

// Search for idle channel
for (long i = 0; i < numInChannels_; i++)
	{
    pCh = &pChannels_[i];
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
    CChannel * 
CAudioMixer::FindChannel( tAudioID id )
{

    CChannel *pChannel = kNull;

    MIXER_LOCK;
//  Find channel with specified ID
for (long i = 0; i < numInChannels_; i++)
	{
    CAudioPlayer *pPlayer = pChannels_[i].GetPlayer();
// GK FIXX CHECK  Excised IsInUse().  Hot code - check for functionality break
	if ( pPlayer && (pPlayer->GetID() == id)) // && pChannels_[i].IsInUse())
		pChannel = &pChannels_[i];
	}
 
    MIXER_UNLOCK; 
return pChannel;
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
// IsAnyAudioActive:  Gk FI
// ==============================================================================
Boolean CAudioMixer::IsAnyAudioActive( void )
{
    Boolean ret = false;

    MIXER_LOCK;
// Search for a channel that is in use
for (long i = 0; i < numInChannels_; i++)
	{
	if (pChannels_[i].IsInUse())
		ret = true;
	}
    MIXER_UNLOCK; 
return ret;
}  // ---- end IsAnyAudioActive() ----

// ==============================================================================
// GetMixBinIndex : Determine mix bin index from sampling frequency
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
CAudioMixer::CreatePlayer(tAudioStartAudioInfo *pInfo, char *sExt, tAudioID newID )
{
CAudioPlayer *pPlayer = NULL;

if (!strcmp(sExt, "raw")  || !strcmp( sExt, "RAW")  ||
    !strcmp(sExt, "brio") || !strcmp( sExt, "BRIO") ||
    !strcmp(sExt, "aif")  || !strcmp( sExt, "AIF")  ||
    !strcmp(sExt, "aiff") || !strcmp( sExt, "AIFF") ||
    !strcmp(sExt, "wav")  || !strcmp( sExt, "WAV") )
	{
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartAudio: Create RawPlayer\n");
	pPlayer = new CRawPlayer( pInfo, newID );
    }
else if (!strcmp( sExt, "ogg" ) || !strcmp( sExt, "OGG") ||
         !strcmp( sExt, "aogg") || !strcmp( sExt, "AOGG"))
    {
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartAudio: Create VorbisPlayer\n");
	pPlayer = new CVorbisPlayer( pInfo, newID );
    } 
else
    {
	pDebugMPI_->DebugOut( kDbgLvlImportant,
		"CAudioMixer::CreatePlayer: Create *NO* Player: invalid file extension ='%s' for file '%s'\n", sExt, (char *) pInfo->path->c_str());
    }

return (pPlayer);
}  // ---- end CreatePlayer() ----

void CAudioMixer::DestroyPlayer(CAudioPlayer *pPlayer)
{
    delete pPlayer;
}

// ==============================================================================
// AddPlayer:
// ==============================================================================
    tErrType 
CAudioMixer::AddPlayer( tAudioStartAudioInfo *pInfo, char *sExt, tAudioID newID )
{
    tErrType ret = kNoErr;
    CAudioPlayer *pPlayer = NULL;
    CChannel *pChannel;

    MIXER_LOCK;
    
    pChannel = FindFreeChannel( pInfo->priority );
    if (!pChannel)
    {
        printf("CAudioMixer::AddPlayer: no channel available\n");
        ret = kAudioNoChannelAvailErr;
        goto error;
    }
    pPlayer = CreatePlayer(pInfo, sExt, newID);
    if (!pPlayer)
    {
        printf("CAudioMixer::AddPlayer: failed to create Player with file extension='%s'\n", sExt);
        ret = kAudioPlayerCreateErr;  // GK FIXXX: add error for unsupported file extension
        goto error;
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
    
    pChannel->SetPan(    pInfo->pan );
    pChannel->SetVolume( pInfo->volume );
    
    goto success;
 error:
    if(pPlayer)
        DestroyPlayer(pPlayer);
 success:
    MIXER_UNLOCK; 
    return ret;
}  // ---- end AddPlayer() ----

// ==============================================================================
// Render:  Main mixer render routine
//
//                          Return # of frames rendered.  
//    (GX FIXXX:  incorrect return value.  Could be Max of channel returns values.)
// ==============================================================================
    int 
CAudioMixer::Render( S16 *pOut, U32 numFrames )
// GK CHECK FIXX numFrames  IS THIS FRAMES OR SAMPLES  !!!!!!!  THIS APPEARS TO BE SAMPLES
{
U32 	i, ch;
U32 	framesRendered;
U32     numFramesDiv2 = numFrames>>1;
U32     numFramesDiv4 = numFrames>>2;
long    mixBinIndex;
long    channels = kAudioMixer_MaxOutChannels;
S16 **tPtrs = pTmpBufOffsets_; // pTmpBufs_, pTmpBufOffsets_

	TimeStampOn(0);
//{static long c=0; printf("CAudioMixer::Render : start %ld  numFrames=%ld channels=%ld\n", c++, numFrames, channels);}

// Update parameters from AudioState
// GK FIXXX:  Mutex-protect this
//if (audioState_.useOutSoftClipper)
//    audioState_.headroomBits = kMixer_HeadroomBits_Default;
//else
    audioState_.headroomBits = kMixer_HeadroomBits_Default;

#ifdef NEW_ADD_PLAYER
// GK FIXXXX: HACK  need to mutex-protect this
if (playerToAdd_)
    {
    CAudioPlayer *pPlayer  = playerToAdd_;
    CChannel     *pChannel = targetChannel_;
    playerToAdd_  = NULL;
    targetChannel_= NULL;
printf("CAudioMixer::Render: Adding player\n");
    pChannel->SetPlayer(pPlayer, true); 
    pChannel->SetInUse(true);
    pChannel->isDone_ = false;
printf("CAudioMixer::Render: Added player\n");
    }
#endif // NEW_ADD_PLAYER

//	printf("AudioMixer::Render: bufPtr: 0x%x, frameCount: %u \n", (unsigned int)pOut, (int)numStereoFrames );
//	pDebugMPI_->Assert( ((numFrames * kAudioBytesPerStereoFrame) == kAudioOutBufSizeInBytes ),
//		"AudioMixer::Render -- frameCount doesn't match buffer size!!!\n");
	
// Clear output and temporary buffers
//ClearShorts( pOut, numFrames * channels );
//for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
//	ClearShorts( pTmpBuffers_[i], numFrames * channels );

// Clear stereo mix buffers
for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	ClearShorts( pMixBinBufs_[i], numFrames*channels);  // GK NOTE: can optimize to clear less for fs/4 and fs/2
	mixBinFilled_[i] = False;
	}

// Open WAV file to write output of mixer
if (audioState_.writeOutSoundFile && !outSoundFile_)
    OpenOutSoundFile(audioState_.outSoundFilePath);

// Open WAV file for input to mixer
if (audioState_.readInSoundFile && !inSoundFile_)
    OpenInSoundFile(audioState_.inSoundFilePath);

//
// ---- Render each channel to mix bin with corresponding sampling frequency
//
if (audioState_.readInSoundFile && !inSoundFileDone_)
	{
    long mixBinIndex  = GetMixBinIndex(inSoundFileInfo_.samplerate);
	S16 *mixBinP      = (S16 *) pMixBinBufs_[mixBinIndex];
	long framesToRead = numFrames/GetSamplingRateDivisor(inSoundFileInfo_.samplerate);
	long framesRead   = 0;

    ClearShorts(mixBinP, framesToRead*2);
// Copy stereo input to stereo mix buffer
	if (2 == inSoundFileInfo_.channels)
 		framesRead = sf_readf_short(inSoundFile_, mixBinP, framesToRead);
// Replicate mono input file to both channels of stereo mix buffer
	else
		{
 		framesRead = sf_readf_short(inSoundFile_, tPtrs[0], framesToRead);
		InterleaveShorts(tPtrs[0], tPtrs[0], mixBinP, framesRead);
		}
    if (framesRead < framesToRead)
        {
	    inSoundFileDone_ = true;
        printf("inSoundFileDone ! %ld/%ld frames read \n", framesRead, framesToRead); 
        }
	mixBinFilled_[mixBinIndex] = True;
printf("AudioMixer::Render: read %ld samples from WAV file \n", framesRead); 
//inSoundFileInfo.samplerate, frames, channels
	}
// 
// ---- Render all active channels (not including MIDI)
//
else
{
for (ch = 0; ch < numInChannels_; ch++)
	{
	CChannel *pCh = &pChannels_[ch];
//printf("CAudioMixer::Render: pCh%ld/%ld ->ShouldRender=%d \n", ch, numInChannels_, pCh->ShouldRender());
// Render if channel is in use and not paused
	if (pCh->ShouldRender())
		{
        ClearShorts(pChannelBuf_, numFrames*channels); // GK FIXXX: may not need this clear or may need keep in loop

// GK FIXXX: eliminate divide
        long channelSamplingFrequency = pCh->GetSamplingFrequency();
		U32  framesToRender = (numFrames*channelSamplingFrequency)/(long)samplingFrequency_;
        U32  sampleCount    = framesToRender*channels;
//printf("ch%ld: framesToRender=%ld for %ld Hz\n", ch, framesToRender, channelSamplingFrequency);

	// Player renders data into channel's stereo output buffer
        framesRendered = pCh->Render( pChannelBuf_, framesToRender );
    // NOTE: SendDoneMsg() deferred to next buffer when framtesToRender is multiple of total frames
	    if ( framesRendered < framesToRender ) 
//printf("frames %ld/%ld\n", framesRendered, framesToRender);
//	    if ( 0 == framesRendered ) 
            {
//			pCh->Release( true );	// false = Don't suppress done msg 
                pCh->isDone_ = true;
				#if 0 // Defer done message until Render() returns to caller
                pCh->fInUse_ = false;
                if (pCh->GetPlayerPtr()->ShouldSendDoneMessage()) {
                    MIXER_UNLOCK;
                    pCh->SendDoneMsg();
                    MIXER_LOCK;
                }
                #endif
            }
        if (inputIsDC) 
            SetShorts(pChannelBuf_, sampleCount, inputDCValuei);
#ifdef NEED_SAWTOOTH
        else if (inputIsSawtoothWave_)
            {
            SawtoothOscillator_S16(pChannelBuf_, sampleCount, &z_, delta_);
printf("Sawtooth z=%ld delta=%ld out[0]=%d\n", z_, delta_, pChannelBuf_[0]);
printf("Sawtooth z=$%08X delta=$%08X out[0]=$%04X\n", (unsigned int)z_, (unsigned int)delta_, (unsigned short) pChannelBuf_[0]);
            }
#endif
	// Add output to appropriate Mix "Bin" 
		long mixBinIndex = GetMixBinIndex(channelSamplingFrequency);
// GK FIXX: use fs/4->fs/2 with gentler anti-aliasing filter and let fs/2 mix bin's conversion do fs/2->fs
		S16 *pMixBin = pMixBinBufs_[mixBinIndex];
        ShiftRight_S16(pChannelBuf_, pChannelBuf_, sampleCount, audioState_.headroomBits);
        AccS16toS16(pMixBin, pChannelBuf_, sampleCount, mixBinFilled_[mixBinIndex]);

		mixBinFilled_[mixBinIndex] = True;
		}   // end ---- if (pCh->ShouldRender())
	}

// MIDI player renders to fs/2 output buffer 
// Even if fewer frames rendered
// {static long c=0; printf("CAudioMixer::Render%ld: pMidiPlayer_->IsActive=%d\n", c++, pMidiPlayer_->IsActive());}
if ( pMidiPlayer_ && pMidiPlayer_->IsActive() )
	{
	U32 mixBinIndex    = kAudioMixer_MixBin_Index_FsDiv2;
	U32 framesToRender = numFramesDiv2;  // fs/2
    U32 sampleCount    = framesToRender*channels;

    ClearShorts(pChannelBuf_, framesToRender); // GK FIXXX: may not need this clear or may need keep in loop
	framesRendered = pMidiPlayer_->Render( pChannelBuf_, framesToRender); 
// GK FIXXX: need to zero fill MIDI buffer ??
//{static long c=0; printf("CAudioMixer::Render%ld: MIDI framesToRender=%ld\n", c++, framesToRender);}
    ShiftRight_S16(pChannelBuf_, pChannelBuf_, sampleCount, audioState_.headroomBits);
    AccS16toS16(pMixBinBufs_[mixBinIndex], pChannelBuf_, sampleCount, mixBinFilled_[mixBinIndex]);

	mixBinFilled_[mixBinIndex] = True;
	}
}

// 
// Combine Mix buffers to output, converting sampling frequency if necessary
//
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv1;
if (mixBinFilled_[mixBinIndex])
    CopyShorts(pMixBinBufs_[mixBinIndex], pOut, numFrames); // GK FIXXXX: CRASH frames vs. channels problem !!
else
	ClearShorts( pOut, numFrames * channels );

//
// Combine Mix bins: sum multi-rate (fs/4, fs/2, fs) buffers with sampling frequency conversion
//
// Deinterleave and process each Mix Buffer separately to OutBuf
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv4;
if (mixBinFilled_[mixBinIndex])
	{
// Preserve tmp buffers between iterations as they hold past state used by SRC
	DeinterleaveShorts(pMixBinBufs_[mixBinIndex], tPtrs[2], tPtrs[3], numFramesDiv4);
	RunSRC(tPtrs[2], tPtrs[0], numFramesDiv4, numFramesDiv2, &src_[mixBinIndex][kLeft ]);
	RunSRC(tPtrs[3], tPtrs[1], numFramesDiv4, numFramesDiv2, &src_[mixBinIndex][kRight]);
	InterleaveShorts(tPtrs[0], tPtrs[1], tPtrs[6], numFramesDiv2);

// Add output of fs/4->fs/2 to the fs/2 bin
// NOTE:  need headroom to avoid overflow in Add2_Shortsi()
    S16 *pBin_Fs2 = pMixBinBufs_[kAudioMixer_MixBin_Index_FsDiv2];
    if (mixBinFilled_[kAudioMixer_MixBin_Index_FsDiv2])
    	Add2_Shortsi(tPtrs[6], pBin_Fs2, pBin_Fs2, (numFramesDiv2)*channels, false);
    else
        {
        CopyShorts(tPtrs[6], pBin_Fs2, numFrames*channels);
        mixBinFilled_[kAudioMixer_MixBin_Index_FsDiv2] = true;
        }
	}
// Clear portion used for SRC filter delay elements
// GK FIXX: don't do this every time.  Add "buffer cleared" flag
else
    {
	ClearShorts(&tPtrs[2][-kSRC_Filter_MaxDelayElements], numFramesDiv4);
	ClearShorts(&tPtrs[3][-kSRC_Filter_MaxDelayElements], numFramesDiv4);
    }

// Convert fs/2  to fs and add to output bin
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
if (mixBinFilled_[mixBinIndex])
	{
// Preserve tmp buffers between iterations as they hold past state used by SRC
	DeinterleaveShorts(pMixBinBufs_[mixBinIndex], tPtrs[4], tPtrs[5], numFramesDiv2);
	RunSRC(tPtrs[4], tPtrs[0], numFramesDiv2, numFrames, &src_[mixBinIndex][kLeft ]);
	RunSRC(tPtrs[5], tPtrs[1], numFramesDiv2, numFrames, &src_[mixBinIndex][kRight]);
	InterleaveShorts(tPtrs[0], tPtrs[1], tPtrs[6], numFrames);
// NOTE:  need headroom to avoid overflow in Add2_Shortsi()
	Add2_Shortsi(tPtrs[6], pOut, pOut, numFrames*channels, false);
	}
// Clear portion used for SRC filter delay elements
// GK FIXX: don't do this every time.  Add "buffer cleared" flag
else
    {
	ClearShorts(&tPtrs[4][-kSRC_Filter_MaxDelayElements], numFramesDiv2);
	ClearShorts(&tPtrs[5][-kSRC_Filter_MaxDelayElements], numFramesDiv2);
    }
		
// Process global audio effects
//if (numPlaying && gAudioContext->pAudioEffects)
//	gAudioContext->pAudioEffects->ProcessAudioEffects( kAudioOutBufSizeInWords, pOut );

// Scale stereo/interleaved buffer by master volume
// GKFIXX: should probably move earlier in signal chain to reduce clipping in those stages
//ScaleShortsf(pOut, pOut, numFrames*channels, masterGainf_[0]);
ScaleShortsi_Q15(pOut, pOut, numFrames*channels, masterGaini_[0]);

// Test with DC input
if (inputIsDC)
    {
    outputDCValuei  = pOut[0];
    outputDCValuef  = Q15ToFloat(outputDCValuei);
    outputDCValueDB = LinearToDecibelf(outputDCValuef);
printf("CAudioMixer: in DC %g dB -> %g |out %g dB -> %g (%d)\n", 
        inputDCValueDB, inputDCValuef, outputDCValueDB, outputDCValuef, outputDCValuei);
    }

// ---- Output DSP block
if (audioState_.useOutEQ || audioState_.useOutSoftClipper || audioState_.computeLevelMeters)
    audioState_.useOutDSP  = true;
#ifdef NEEDED
{static long c=0; if (!(c++)) { tAudioState *d = &audioState_;
printf("CAudioMixer: useOutDSP=%d computeLevelMeters=%d useOutEQ=%d useOutSoftClipper=%d\n", 
d->useOutDSP, d->computeLevelMeters, d->useOutEQ , d->useOutSoftClipper); }
}
#endif

if (audioState_.useOutDSP)
    {
    #define kTmpIndex 0
    DeinterleaveShorts(pOut, tPtrs[kTmpIndex], tPtrs[kTmpIndex+1], numFrames);
    for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
        { 
        S16 *pIn  = tPtrs[kTmpIndex+ch];
        S16 *pOut = tPtrs[kTmpIndex+ch]; 
    // Compute Output Equalizer
        if (audioState_.useOutEQ)
            {
    //{static long c=0; if (!c) printf("ComputeEQ %ld : bands=%ld \n", c++, outEQ_BandCount_);}
            for (long j = 0; j < outEQ_BandCount_; j++)
                {
                ComputeEQi(pIn, pOut, numFrames, &outEQ_[ch][j]);
                pOut = pIn;
                }
            }
    // test boost for single channel wave
//        for (U32 ii = 0; ii < numFrames; ii++) pIn[ii] *= 3;

    // Compute Output Soft Clipper
//    {static long c=0; printf("ComputeWaveShaper %ld On=%d: \n", c++, audioState_.useOutSoftClipper);}
        if (audioState_.useOutSoftClipper)
            {
//        for (U32 ii = 0; ii < numFrames; ii++) pIn[ii] *= 3;
            outSoftClipper_[ch].headroomBits = audioState_.headroomBits;  // GK_FIXX: move to setup code
    //{static long c=0; if (!c) printf("ComputeWaveShaper %ld On=%d: \n", c++, audioState_.useOutSoftClipper);}
            ComputeWaveShaper(pIn, pOut, numFrames, &outSoftClipper_[ch]);
            }
// Level control for headphones 
        else
            {
            }
        }
    InterleaveShorts(tPtrs[kTmpIndex], tPtrs[kTmpIndex+1], pOut, numFrames);
    }  // end if (useOutDSP)
ShiftLeft_S16(pOut, pOut, numFrames*channels, audioState_.headroomBits);

// Audio level meters
//  Scan buffer for maximum value
if (audioState_.computeLevelMeters)
    {
    tAudioState *as = &audioState_;
// Scan output buffer for max values
    for (long ch = kLeft; ch <= kRight; ch++)
        {
        S16 x = MaxAbsShorts(&pOut[ch], numFrames, 2); // Grab from one channel, so use buffer offset
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
                longTimeHoldCounter    = 0;
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
//printf("Mixer::Render Short=%5d Long=%5d \n", outLevels_ShortTime[0], outLevels_LongTime[0]);
    } // end computeLevelMeters

// Debug:  write output of mixer to sound file
static long outBufferCounter = 0; 
if (audioState_.writeOutSoundFile)
    {
    if (inSoundFileDone_ || outBufferCounter++ >= audioState_.outFileBufferCount)
        {
printf("Closing outSoundFile '%s'\n", audioState_.outSoundFilePath);
    	CloseSoundFile(&outSoundFile_);
        outSoundFile_      = NULL;
        audioState_.writeOutSoundFile = false;
        }
    else if (outSoundFile_)
        {
static long totalFramesWritten = 0;
    long framesWritten = sf_writef_short(outSoundFile_, pOut, numFrames);
    totalFramesWritten += framesWritten;
//{static long c=0; printf("CAudioMixer::Render %ld: outFile wrote %ld frames total=%ld\n", c++, framesWritten, totalFramesWritten);}	
        }
    }

	TimeStampOff(0);

//{static long c=0; printf("CAudioMixer::Render%ld: END numFrames=%ld\n", c++, numFrames);}
return (kNoErr);  // GK FIXX: should be # frames rendered
} // ---- end Render() ----

// ==============================================================================
// WrapperToCallRender: Call member function Render() to fit PortAudio callback format
//                              Return # of frames rendered
// ==============================================================================
    int 
CAudioMixer::WrapperToCallRender( S16 *pOut,  unsigned long numStereoFrames, void *pToObject  )
{	
//{static long c=0; printf("WrapperToCallRender%ld: IsPaused()=%d\n", c++, ((CAudioMixer*)pToObject)->IsPaused());}
    int error = kNoErr;
    
	U32 numInChannels = ((CAudioMixer*)pToObject)->numInChannels_;
	const IEventListener* 	pListeners[numInChannels];
	CAudioEventMessage* 	pEvtMsgs[numInChannels];
	memset(pListeners, 0, sizeof(pListeners));
	memset(pEvtMsgs, 0, sizeof(pEvtMsgs));
    
    MIXER_LOCK;

    if (((CAudioMixer*)pToObject)->IsPaused())
    {
        ClearShorts(pOut, numStereoFrames*kAudioMixer_MaxOutChannels);
        error = kNoErr;
    } else {
        error = ((CAudioMixer*)pToObject)->Render( pOut, numStereoFrames );
    }

	// Now that rendering is complete, check for any done messages to be sent
	for (U32 ch = 0; ch < numInChannels; ch++)
	{
		CChannel *pCh = &(((CAudioMixer*)pToObject)->pChannels_)[ch];
		if (pCh->isDone_ && pCh->fInUse_)
		{
			// Cache done messages while mixer is locked
			CAudioPlayer* pPlayer = pCh->GetPlayerPtr();
			if (pPlayer && pPlayer->ShouldSendDoneMessage()) 
			{
				pListeners[ch] = pPlayer->GetEventListener();
				pEvtMsgs[ch] = pPlayer->GetAudioEventMsg();
				pEvtMsgs[ch]->audioMsgData.audioCompleted.count++;
			}
			pCh->fInUse_ = false;
		}
	}
    
    MIXER_UNLOCK;

	// Now post any pending done messages which were cached while mixer was locked 
	for (U32 ch = 0; ch < numInChannels; ch++)
	{
		if (pListeners[ch] != kNull)
			pEventMPI_->PostEvent(*pEvtMsgs[ch], 128, pListeners[ch]);
	}

    return error;
} // ---- end WrapperToCallRender() ----

// ==============================================================================
// Pause: Stop all audio processing and I/O but do not alter state
//
//          FIXXXX: add code for fade out
// ==============================================================================
    void 
CAudioMixer::Pause()
{	
    MIXER_LOCK;
    isPaused_ = true;
    MIXER_UNLOCK;

} // ---- end WrapperToCallRender() ----

// ==============================================================================
// Resume: Restart all audio processing and I/O from current state
//
//          FIXXXX: add code for fade in
// ==============================================================================
    void 
CAudioMixer::Resume()
{	
    MIXER_LOCK;
    isPaused_ = false;
    MIXER_UNLOCK;
} // ---- end Resume() ----

// ==============================================================================
// SetMasterVolume :  Range [0..100] maps to [0.0 to 1.0]
// ==============================================================================
    void 
CAudioMixer::SetMasterVolume( U8 x )
{
    MIXER_LOCK;
S16 x16 = (S16) x;
audioState_.masterVolume = (U8) BoundS16(&x16, 0, 100);

// ChangeRangef(x, L1, H1, L2, H2)
//masterGainf_[0] = ChangeRangef((float)x, 0.0f, 100.0f, 0.0f, 1.0f);
masterGainf_[kLeft] = 0.01f*(float)x;

// Convert to square curve, which is milder than Decibels
//masterGainf_[0] = DecibelToLinearf(ChangeRangef((float)x, 0.0f, 100.0f, -100.0f, 0.0f));
masterGainf_[kLeft ] *= masterGainf_[kLeft];
masterGainf_[kRight] =  masterGainf_[kLeft];

// Convert 32-bit floating-point to Q15 fractional integer format 
masterGaini_[kLeft ] = FloatToQ15(masterGainf_[kLeft]);
masterGaini_[kRight] = masterGaini_[kLeft];

audioState_.masterGainf[kLeft ] = masterGainf_[kLeft ];
audioState_.masterGainf[kRight] = masterGainf_[kRight];
//printf("CAudioMixer::SetMasterVolume %d -> %f ($%0X)\n", audioState_.masterVolume , masterGainf_[0], masterGaini_[0]);
 MIXER_UNLOCK; 
} // ---- end SetMasterVolume() ----

// ==============================================================================
// SetSamplingFrequency :  update fs for mixer and all DSP
//                  Does not reset or recalculate DSP parameters
// ==============================================================================
    void 
CAudioMixer::SetSamplingFrequency( float x )
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

    for (i = 0; i < kAudioMixer_SRCCount; i++)
        {
        SRC *d = &src_[i][ch];
		SRC_SetInSamplingFrequency (d, (float)fsRack_[i]);
//		SRC_SetOutSamplingFrequency(d, samplingFrequency_);
		SRC_SetOutSamplingFrequency (d, (float)(2*fsRack_[i]));  // Don't worry.  Bin0 SRC not used.
        }
    }
} // ---- end SetSamplingFrequency() ----

// ==============================================================================
// SetDSP :  Transfer high level DSP parameters from audio state to DSP modules
// ==============================================================================
    void 
CAudioMixer::SetDSP()
{
//{static long c=0; printf("CAudioMixer::SetDSP %ld: START\n", c++);}
tAudioState *d = &audioState_;

U8     srcType;
U8     srcFilterVersion;

for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
    {
    WAVESHAPER *ws = &outSoftClipper_[ch];
    SetWaveShaper_SamplingFrequency(ws, samplingFrequency_);
    SetWaveShaper_Parameters(ws, kWaveShaper_Type_V4, (float)d->softClipperPreGainDB, (float)d->softClipperPostGainDB);

    {
//#define EQ_WITH_LOWPASS_RESPONSE
//#define EQ_WITH_RESONANT_SPIKES
#define EQ_WITH_FLAT_RESPONSE
#ifdef EQ_WITH_LOWPASS_RESPONSE
    outEQ_BandCount_ = 1;
    SetEQ_Parameters(&outEQ_[ch][0], 200.0f, 20.0f, 10.0f, kEQ_Mode_LowPass);
    SetEQ_Parameters(&outEQ_[ch][1], 200.0f, 20.0f,  0.0f, kEQ_Mode_Parametric);
    SetEQ_Parameters(&outEQ_[ch][2], 400.0f, 20.0f,  0.0f, kEQ_Mode_LowShelf);
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
    for (long j = 0; j < outEQ_BandCount_; j++)
        {
        SetEQ_SamplingFrequency(&outEQ_[ch][j], samplingFrequency_);
        PrepareEQ(&outEQ_[ch][j]);    
        }
    }

    }
} // ---- end SetDSP() ----

// ==============================================================================
// UpdateDSP :  Recalculate DSP parameters
// ==============================================================================
void CAudioMixer::UpdateDSP()
{
long i;
//{static long c=0; printf("CAudioMixer::UpdateDSP %ld: START\n", c++);}

for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
    for (i = 0; i < outEQ_BandCount_; i++)
        UpdateEQ(&outEQ_[ch][i]);

    UpdateWaveShaper(&outSoftClipper_[ch]);

    for (i = 0; i < kAudioMixer_SRCCount; i++)
    	UpdateSRC(&src_[i][ch]);
    }
} // ---- end UpdateDSP() ----

// ==============================================================================
// ResetDSP :  Reset DSP state
// ==============================================================================
void CAudioMixer::ResetDSP()
{
long ch, i;
//{static long c=0; printf("CAudioMixer::ResetDSP %ld: START\n", c++);}
for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
    for (i = 0; i < outEQ_BandCount_; i++)
        ResetEQ(&outEQ_[ch][i]);

    ResetWaveShaper(&outSoftClipper_[ch]);

    for (i = 0; i < kAudioMixer_MixBinCount; i++)
    	mixBinFilled_[i] = False;
 
    for (i = 0; i < kAudioMixer_SRCCount; i++)
    	ResetSRC(&src_[i][ch]);
    }
} // ---- end ResetDSP() ----

// ==============================================================================
// SetAudioState :  Set "secret" interface parameters for Mixer
// ==============================================================================
    void 
CAudioMixer::SetAudioState(tAudioState *d)
{
//printf("CAudioMixer::SetAudioState: start srcType=%d\n", d->srcType);
    MIXER_LOCK;
bcopy(d, &audioState_, sizeof(tAudioState));
//printf("CAudioMixer::SetAudioState: audioState_.srcType=%d\n", audioState_.srcType);
SetDSP();
UpdateDSP();
//ResetDSP();
 MIXER_UNLOCK; 
} // ---- end SetAudioState() ----

// ==============================================================================
// GetAudioState :  Get "secret" interface parameters for Mixer
// ==============================================================================
    void 
CAudioMixer::GetAudioState(tAudioState *d)
{
    MIXER_LOCK;
//printf("CAudioMixer::GetAudioState: start srcType=%d\n", d->srcType);
bcopy(&audioState_, d, sizeof(tAudioState));
 MIXER_UNLOCK; 
} // ---- end GetAudioState() ----

// ==============================================================================
// PrintMemoryUsage :  
// ==============================================================================
    void 
CAudioMixer::PrintMemoryUsage()
{
long bytes = 0;
#ifdef EMULATION
long framesPerBuffer = 256;
#else
long framesPerBuffer = 512;
#endif

bytes += sizeof(CAudioMixer);
printf("CAudioMixer::PrintMemoryUsage: sizeof(CAudioMixer)=%d\n", sizeof(CAudioMixer));

long channelMemory = numInChannels_*sizeof(CChannel);
channelMemory += sizeof(pChannelBuf_);	
printf("CAudioMixer::PrintMemoryUsage: channelMemory=%ld\n", channelMemory);
bytes += channelMemory;

long mixBinMemory = sizeof(pMixBinBufs_);
printf("CAudioMixer::PrintMemoryUsage: mixBinMemory=%ld\n", mixBinMemory);
bytes += mixBinMemory;

long tmpBufMemory = sizeof(pTmpBufs_);
printf("CAudioMixer::PrintMemoryUsage: tmpBufMemory=%ld\n", tmpBufMemory);
bytes += tmpBufMemory;

#ifdef EMULATION
printf("CAudioMixer::PrintMemoryUsage: totalBytes= EMULATION=%ld EMBEDDED=%ld\n", bytes, 2*bytes);
#else
printf("CAudioMixer::PrintMemoryUsage: totalBytes= EMULATION=%ld EMBEDDED=%ld\n", bytes/2, bytes);
#endif
} // ---- end PrintMemoryUsage() ----

// ==============================================================================
// EnableSpeaker :  Setup DSP that is different for Line and Speaker outputs
// ==============================================================================
    void 
CAudioMixer::EnableSpeaker(Boolean x)
{
//{static long c=0; printf("CAudioMixer::EnableSpeaker%ld: enabled=%d -> %d\n", c++, audioState_.speakerEnabled, x);}
//audioState_.useOutEQ = x;  // Don't use EQ for now
    // This access should be locked, but the abstraction barrier is MAD broken.
    audioState_.useOutSoftClipper = x;
    audioState_.speakerEnabled    = x;
} // ---- end EnableSpeaker() ----

// ==============================================================================
// CreateMIDIPlayer :  
// ==============================================================================
    CMidiPlayer * 
CAudioMixer::CreateMIDIPlayer()
{
    MIXER_LOCK;
//{static long c=0; printf("CAudioMixer::CreateMIDIPlayer%ld: \n", c++);}
if (pMidiPlayer_)
	delete pMidiPlayer_;

pMidiPlayer_ = new CMidiPlayer( BRIO_MIDI_PLAYER_ID );
 MIXER_UNLOCK; 
return (pMidiPlayer_);
} // ---- end CreateMIDIPlayer() ----

// ==============================================================================
// DestroyMIDIPlayer :  
// ==============================================================================
    void 
CAudioMixer::DestroyMIDIPlayer()
{
    MIXER_LOCK;
//{static long c=0; printf("CAudioMixer::DestroyMIDIPlayer%ld: \n", c++);}
if (pMidiPlayer_)
    {
	delete pMidiPlayer_;
	pMidiPlayer_ = NULL;
    }
 MIXER_UNLOCK; 
} // ---- end DestroyMIDIPlayer() ----

LF_END_BRIO_NAMESPACE()
