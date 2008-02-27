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

#define kMixer_Headphone_GainDB		   0 
#define kMixer_SoftClipper_PreGainDB   3 
#define kMixer_SoftClipper_PostGainDB  0

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

#define MIXER_LOCK pDebugMPI_->Assert((kNoErr == pKernelMPI_->LockMutex(mixerMutex_)),\
									  "Couldn't lock mixer mutex.\n")

#define MIXER_UNLOCK pDebugMPI_->Assert((kNoErr == pKernelMPI_->UnlockMutex(mixerMutex_)),\
										"Couldn't unlock  mixer mutex.\n")

//==============================================================================
// Global variables
//==============================================================================

// These should not be globals.	 They should be in the object as they were.	 But
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
		return true;
	}

	// open the keyboard driver
	fd = open(devname, O_RDONLY);
	if(fd < 0) {
		return true;
	}
	
	// ask for the state of the 'switches'
	if(ioctl(fd, EVIOCGSW(sizeof(int)), &sw) < 0) {
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
	const tMutexAttr	attr = {0};

	numInChannels_ = inChannels;
	if (numInChannels_ > kAudioMixer_MaxInChannels)
		printf("CAudioMixer::CAudioMixer: %d too many channels! Max=%d\n",
			   numInChannels_, kAudioMixer_MaxInChannels);
	pDebugMPI_->Assert((numInChannels_ <= kAudioMixer_MaxInChannels),
					   "CAudioMixer::CAudioMixer: %d is too many channels!\n",
					   numInChannels_ );
	
	samplingFrequency_ = kAudioSampleRate;

	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose);
	
	pEventMPI_ = new CEventMPI();

	pKernelMPI_ = new CKernelMPI();

	// Init mutex for serialization access to internal AudioMPI state.
	tErrType err = pKernelMPI_->InitMutex( mixerMutex_, attr );
	pDebugMPI_->Assert((kNoErr == err), "%s: Couldn't init mutex.\n", __FUNCTION__);
	
	// Allocate audio channels
	pChannels_ = new CChannel[ numInChannels_ ];
	pDebugMPI_->Assert((pChannels_ != kNull),
					   "CAudioMixer::CAudioMixer: couldn't allocate %d channels!\n",
					   numInChannels_);

	// Create MIDI player
	pMidiPlayer_ = NULL;

	// Configure sampling frequency conversion
	fsRack_[0] = (long)(samplingFrequency_*0.25f);
	fsRack_[1] = (long)(samplingFrequency_*0.5f );
	fsRack_[2] = (long)(samplingFrequency_);	   

	for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
		mixBinFilled_[i] = False;
	}

	for (i = 0; i < kAudioMixer_SRCCount; i++)
	{
		// All SRC units will be set for 2x interpolation: fs/4 -> fs/2 , fs/2
		// -> fs
		for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
		{
			// Initialize sampling rate converters	
			SRC *src = &src_[i][ch];
			DefaultSRC(src);
			// kSRC_Interpolation_Type : {Triangle, AddDrop, Linear, Triangle,
			// FIR, IIR, Box}
			src->type = kSRC_Interpolation_Type_FIR;
			// V6: no go.  Alias on 1000/8000Hz very  audible  
			// V7: ok:	   Alias on 1000/8000Hz quite audible
			// V8: good.   Alias on 1000/8000Hz inaudible 
			// 6=30dB_15 7=50dB_31 8=50dB_41
			src->filterVersion = kSRC_FilterVersion_8;
		}
	}

	for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
		ClearShorts(pTmpBufs_[i], kAudioMixer_TempBufferWords);
		pTmpBufOffsets_[i] = &pTmpBufs_[i][kSRC_Filter_MaxDelayElements];  
	}
	
	// numFrames = 256 for EMULATION, 512 for embedded
	int numFramesPerBuffer = kAudioFramesPerBuffer;
	
	// Headphone gain
	headphoneGainDB_	 = kMixer_Headphone_GainDB;
	headphoneGainF_		 = DecibelToLinearf(headphoneGainDB_);
	headphoneGainWholeI_ = (long)headphoneGainFracF_;
	headphoneGainFracF_	 = headphoneGainF_ - (float)headphoneGainWholeI_;
	headphoneGainFracI_	 = FloatToQ15(headphoneGainFracF_);

	// Set up Audio State struct (interfaces via AudioMPI)
	// NOTE : Keep at end of this routine
	{
		tAudioState *d = &audioState_;
		memset(d, 0, sizeof(tAudioState));
		if (sizeof(tAudioState) >= kAUDIO_MAX_MSG_SIZE)
			printf("UH OH CAudioMixer: sizeof(tAudioState)=%d kAUDIO_MAX_MSG_SIZE=%ld\n",
				   sizeof(tAudioState), kAUDIO_MAX_MSG_SIZE);
		
		d->useOutSoftClipper  = true;
		
		SetMasterVolume(100);
		
		d->systemSamplingFrequency = (long)samplingFrequency_;
		
		bool isSpeaker = IsSpeakerOn();
		EnableSpeaker(isSpeaker);
				
		d->softClipperPreGainDB	 = (S16) kMixer_SoftClipper_PreGainDB;
		d->softClipperPostGainDB = (S16) kMixer_SoftClipper_PostGainDB;
		
		d->srcType			= src_[0][kLeft].type;
		d->srcFilterVersion = src_[0][kLeft].filterVersion;
		d->channelGainDB = kChannel_HeadroomDB;
		
	} // end Set up Audio State struct

	// ---- Configure DSP engine
	for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
		// Configure output "soft" clipper
		DefaultWaveShaper(&outSoftClipper_[ch]);
		outSoftClipper_[ch].useFixedPoint = true;
	}
	SetSamplingFrequency( samplingFrequency_ );
	
	SetDSP();
	UpdateDSP();
	ResetDSP();
	
	// Init output driver and register callback.  We have to pass in a pointer
	// to the mixer object as a bit of "user data" so that when the callback
	// happens, the C call can get to the mixer's C++ member function for
	// rendering.
	err = InitAudioOutput( &CAudioMixer::WrapperToCallRender,
							   (void *)this);
	pDebugMPI_->Assert(kNoErr == err,
					   "Failed to initalize audio output\n" );
	err = StartAudioOutput();

	// See the documentation for GetNextAudioID and GetNextMidiID to understand
	// the audio id assignment scheme.
	nextAudioID = kNoMidiID + 1;
	nextMidiID = 0;

	pDebugMPI_->Assert(kNoErr == err,
					   "Failed to start audio output\n" );
	
	pDebugMPI_->Assert(FlatProfilerInit(0, 0) == 0, "Failed to init profiler.\n");
}

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
			//delete pChannels_[ch];
		}
	}
	
	// Deallocate MIDI player
	if (pMidiPlayer_)
		delete pMidiPlayer_;
	
	for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
		pTmpBufOffsets_[i] = NULL;
	}
	
	for (long i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	}
	
	MIXER_UNLOCK; 
	pKernelMPI_->DeInitMutex( mixerMutex_ );

	delete pKernelMPI_;

	delete pEventMPI_;

	delete pDebugMPI_;

	FlatProfilerDone();

}

// ==============================================================================
// FindFreeChannel:	   Find a free channel using priority -> GKFIXX: add search by priority
// ==============================================================================
CChannel *CAudioMixer::FindFreeChannel( tAudioPriority /* priority */)
{
	CChannel *pCh = kNull, *pFreeCh = kNull;
	int active = 0;

	// Search for idle channel.  This is currently where we apply the
	// max-number-of-players policy.  It should be moved to CreatePlayer.
	for (long i = 0; i < numInChannels_; i++)
	{
		pCh = &pChannels_[i];
		if (pCh->GetPlayer())
			active++;
		else
			pFreeCh = pCh;
	}
	
	if(active >= kAudioMixer_MaxActiveAudioChannels)
		pFreeCh = kNull;
	
	return pFreeCh;
}

// ==============================================================================
// FindChannel:	 Look for channel with specified ID
//					Return ptr to channel
// ==============================================================================
CChannel *CAudioMixer::FindChannelInternal( tAudioID id )
{

	CChannel *pChannel = kNull;
	
	//Find channel with specified ID
	for (long i = 0; i < numInChannels_; i++)
	{
		CAudioPlayer *pPlayer = pChannels_[i].GetPlayer();
		if ( pPlayer && (pPlayer->GetID() == id))
			pChannel = &pChannels_[i];
	}
	return pChannel;
}

// ==============================================================================
// FindChannel:	 Look for channel with specified ID
//					Return ptr to channel
// ==============================================================================
CChannel *CAudioMixer::FindChannel( tAudioID id )
{
	CChannel *pChannel = kNull;
	
	MIXER_LOCK;
	pChannel = FindChannelInternal(id);
	MIXER_UNLOCK; 
	return pChannel;
}

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
		if (pChannels_[i].GetPlayer())
			ret = true;
	}
	MIXER_UNLOCK; 
	return ret;
}

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
}

// ==============================================================================
// GetSamplingRateDivisor : Determine divisor sampling frequency, which is your operating
//				sampling frequency
// ==============================================================================
long CAudioMixer::GetSamplingRateDivisor( long samplingFrequency )
{
	long div =	1;
	
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
}

// ==============================================================================
// CreatePlayer:   Allocate player based on file extension argument
// ==============================================================================
CAudioPlayer *CAudioMixer::CreatePlayer(tAudioStartAudioInfo *pInfo,
										char *sExt )
{
	CAudioPlayer *pPlayer = NULL;
	tAudioID newID;

	if (!strcmp(sExt, "raw")  || !strcmp( sExt, "RAW")	||
		!strcmp(sExt, "brio") || !strcmp( sExt, "BRIO") ||
		!strcmp(sExt, "aif")  || !strcmp( sExt, "AIF")	||
		!strcmp(sExt, "aiff") || !strcmp( sExt, "AIFF") ||
		!strcmp(sExt, "wav")  || !strcmp( sExt, "WAV") )
	{
		newID = GetNextAudioID();
		pPlayer = new CRawPlayer( pInfo, newID );
	}
	else if (!strcmp( sExt, "ogg" ) || !strcmp( sExt, "OGG") ||
			 !strcmp( sExt, "aogg") || !strcmp( sExt, "AOGG"))
	{
		newID = GetNextAudioID();
		pPlayer = new CVorbisPlayer( pInfo, newID );
	}

	return (pPlayer);
}

void CAudioMixer::DestroyPlayer(CAudioPlayer *pPlayer)
{
	delete pPlayer;
}

// ==============================================================================
// And now a word about tAudioIDs.  For legacy reasons, functions that operate
// on tAudioIDs also must operate on tMidiPlayerIDs.  Unfortunately,
// tMidiPlayerIDs are 8 bits, and tAudioIDs are 32 bits.  To ensure that we
// don't get into any trouble, we make the following internal constraint: values
// tAudioIDs 0-255 are reserved for midi so that they can be casted about from
// tMidiPlayerIDs to tAudioIDs and vice versa.  All of this muck is constrained
// to these two functions.
// ==============================================================================
tAudioID CAudioMixer::GetNextAudioID(void)
{
	tAudioID ret = nextAudioID;

	nextAudioID++;
	if (kNoAudioID == nextAudioID) {
		nextAudioID = kNoMidiID + 1;
	}
	return ret;
}

tAudioID CAudioMixer::GetNextMidiID(void)
{
	tAudioID ret = nextMidiID;

	nextMidiID++;
	if (kNoMidiID == nextMidiID) {
		nextMidiID = 0;
	}
	return ret;
}

// ==============================================================================
// AddPlayer:
// ==============================================================================
tAudioID CAudioMixer::AddPlayer( tAudioStartAudioInfo *pInfo, char *sExt )
{
	tAudioID id = kNoAudioID;
	CAudioPlayer *pPlayer = NULL;
	CChannel *pChannel;
	
	MIXER_LOCK;
	
	pChannel = FindFreeChannel( pInfo->priority );
	if (!pChannel)
	{
		pDebugMPI_->DebugOut(kDbgLvlImportant, "%s: No channel available\n",
							 __FUNCTION__);
		goto error;
	}

	pPlayer = CreatePlayer(pInfo, sExt);
	if (pPlayer)
	{
		id = pPlayer->GetID();
	}
	else
	{
		pDebugMPI_->DebugOut(kDbgLvlImportant,
							 "%s: failed to create Player with extension %s\n",
							 __FUNCTION__, sExt);
		goto error;
	} 

	pChannel->InitWithPlayer( pPlayer );
	
	pChannel->SetPan(	 pInfo->pan );
	pChannel->SetVolume( pInfo->volume );
	
	goto success;
 error:
	if(pPlayer)
		DestroyPlayer(pPlayer);
 success:
	MIXER_UNLOCK; 
	return id;
}

// ==============================================================================
// RemovePlayer:
// ==============================================================================
void CAudioMixer::RemovePlayer( tAudioID id, Boolean noDoneMessage )
{
	CChannel *pCh;
	CAudioPlayer *pPlayer;

	MIXER_LOCK;
	pCh = FindChannelInternal(id);
	if (pCh && pCh->GetPlayer()) {
		//perhaps we should pass noDoneMessage?
		pPlayer = pCh->GetPlayer();
		if(pPlayer)
			delete pPlayer;
		pCh->Release(true);
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// PausePlayer:
// ==============================================================================
void CAudioMixer::PausePlayer( tAudioID id )
{
	CChannel *pCh;

	MIXER_LOCK;
	pCh = FindChannelInternal(id);
	if (pCh && pCh->GetPlayer())
	{
		pCh->GetPlayer()->Pause();
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// ResumePlayer:
// ==============================================================================
void CAudioMixer::ResumePlayer( tAudioID id )
{
	CChannel *pCh;

	MIXER_LOCK;
	pCh = FindChannelInternal(id);
	if (pCh && pCh->GetPlayer())
	{
		pCh->GetPlayer()->Resume();
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// IsPlayerPlaying:
// ==============================================================================
Boolean CAudioMixer::IsPlayerPlaying( tAudioID id )
{
	CChannel *pCh;
	Boolean playing = false;

	MIXER_LOCK;
	pCh = FindChannelInternal(id);
	if (pCh && pCh->GetPlayer())
	{
		//We need to check pause also
		playing = true;
	}
	MIXER_UNLOCK;
	return playing;
}

// ==============================================================================
// Render:	Main mixer render routine
//
// Clear all of the mixer bin buffers.  There's one of these for each supported
// sample rate (i.e., fs, fs/2, and fs/4).
//
// For each channel, render into the global temp buffer pChannelBuf_, scale the
// rendered output down by kMixer_HeadroomBits_Default, add the output from
// pChannelBuf_ to the appropriate frequency bin.
//
// Render midi output to pChannelBuf_, scale the output down by
// kMixer_HeadroomBits_Default, add the output from pChannelBuf_ to the
// appropriate frequency bin.
//
// Copy samples from the frequency bin corresponding to the hardware's sample
// rate to the output buffer.
//
// Deinterleave the contents of the fs/4 frequency bin, convert the
// deinterleaved samples from fs/4 to fs/2, re-interleave the samples, and add
// them to the fs/2 frequency bin.
//
// Deinterleave the contents of the fs/2 frequency bin, convert the
// deinterleaved samples from fs/2 to fs, re-interleave the samples, and add
// them to the output buffer.
//
// Apply master gain to output buffer.  This is a real multiply, not a shift.
// 
// Deinterleave the output samples into temp buffers, apply the soft clipper to
// each channel, then reinterleave.
//
// Scale the output back up by kMixer_HeadroomBits_Default.
//
// Comments:
//
// By using 32 bit samples for the output of the sample rate conversion, the
// shifts by kMixer_HeadroomBits_Default can be eliminated.  This will improve
// performance by eliminating several linear passes over the buffers and
// preserve two bits of precision.
//
// Staged sample rate conversion is not saving any performance, but is difficult
// to maintain and extend.  Frequency bins should each be independently
// resampled and added to the output buffer.
//
//							Return # of frames rendered.  
//	  (GX FIXXX:  incorrect return value.  Could be Max of channel returns values.)
// ==============================================================================
int CAudioMixer::Render( S16 *pOut, U32 numFrames )
{
	// GK CHECK FIXX numFrames IS THIS FRAMES OR SAMPLES !!!!!!! THIS APPEARS TO
	// BE SAMPLES
	U32		i, ch;
	U32		framesRendered;
	U32		numFramesDiv2 = numFrames>>1;
	U32		numFramesDiv4 = numFrames>>2;
	long	mixBinIndex;
	long	channels = kAudioMixer_MaxOutChannels;
	S16 **tPtrs = pTmpBufOffsets_;

	TimeStampOn(0);

	// Clear stereo mix buffers
	for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
		ClearShorts( pMixBinBufs_[i], numFrames*channels);
		mixBinFilled_[i] = False;
	}

	// ---- Render all active channels (not including MIDI)
	for (ch = 0; ch < numInChannels_; ch++)
	{
		CChannel *pCh = &pChannels_[ch];
		// Render if channel is in use and not paused
		if (pCh->GetPlayer() && !pCh->GetPlayer()->IsPaused())
		{
			ClearShorts(pChannelBuf_, numFrames*channels);
			long channelSamplingFrequency = pCh->GetSamplingFrequency();
			U32	 framesToRender =
				(numFrames*channelSamplingFrequency)/(long)samplingFrequency_;
			U32	 sampleCount	= framesToRender*channels;
			// Player renders data into channel's stereo output buffer
			framesRendered = pCh->Render( pChannelBuf_, framesToRender );
			// NOTE: SendDoneMsg() deferred to next buffer when
			// framtesToRender is multiple of total frames
			if ( framesRendered < framesToRender ) 
			{
				pCh->isDone_ = true;
				//Done message is sent later
			}

			// Add output to appropriate Mix "Bin" 
			long mixBinIndex = GetMixBinIndex(channelSamplingFrequency);
			S16 *pMixBin = pMixBinBufs_[mixBinIndex];
			ShiftRight_S16(pChannelBuf_, pChannelBuf_, sampleCount,
						   kMixer_HeadroomBits_Default);
			AccS16toS16(pMixBin, pChannelBuf_, sampleCount,
						mixBinFilled_[mixBinIndex]);
			mixBinFilled_[mixBinIndex] = True;
		}
	}

	// MIDI player renders to fs/2 output buffer Even if fewer frames
	// rendered
	if ( pMidiPlayer_ )
	{
		U32 mixBinIndex	   = kAudioMixer_MixBin_Index_FsDiv2;
		U32 framesToRender = numFramesDiv2;	 // fs/2
		U32 sampleCount	   = framesToRender*channels;
		
		ClearShorts(pChannelBuf_, framesToRender);
		framesRendered = pMidiPlayer_->Render( pChannelBuf_, framesToRender); 
		ShiftRight_S16(pChannelBuf_, pChannelBuf_,
					   sampleCount, kMixer_HeadroomBits_Default);
		AccS16toS16(pMixBinBufs_[mixBinIndex], pChannelBuf_, sampleCount,
						mixBinFilled_[mixBinIndex]);
		mixBinFilled_[mixBinIndex] = True;
	}

	// Combine Mix buffers to output, converting sampling frequency if necessary
	mixBinIndex = kAudioMixer_MixBin_Index_FsDiv1;
	if (mixBinFilled_[mixBinIndex])
		CopyShorts(pMixBinBufs_[mixBinIndex], pOut, numFrames);
	else
		ClearShorts( pOut, numFrames * channels );

	// Combine Mix bins: sum multi-rate (fs/4, fs/2, fs) buffers with sampling
	// frequency conversion
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
	else
	{
		// Clear portion used for SRC filter delay elements
		ClearShorts(&tPtrs[2][-kSRC_Filter_MaxDelayElements], numFramesDiv4);
		ClearShorts(&tPtrs[3][-kSRC_Filter_MaxDelayElements], numFramesDiv4);
	}

	// Convert fs/2	 to fs and add to output bin
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
	else
	{
		// Clear portion used for SRC filter delay elements
		ClearShorts(&tPtrs[4][-kSRC_Filter_MaxDelayElements], numFramesDiv2);
		ClearShorts(&tPtrs[5][-kSRC_Filter_MaxDelayElements], numFramesDiv2);
	}

	// Process global audio effects

	// Scale stereo/interleaved buffer by master volume GKFIXX: should probably
	// move earlier in signal chain to reduce clipping in those stages
	// //ScaleShortsf(pOut, pOut, numFrames*channels, masterGainf_[0]);
	ScaleShortsi_Q15(pOut, pOut, numFrames*channels, masterGaini_[0]);

	// Apply soft clipper
	if (audioState_.useOutSoftClipper)
	{
		DeinterleaveShorts(pOut, tPtrs[0], tPtrs[1], numFrames);
		for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
		{ 
			S16 *pIn  = tPtrs[ch];
			S16 *pOut = tPtrs[ch]; 
			
			// GK_FIXX: move to setup code
			outSoftClipper_[ch].headroomBits = kMixer_HeadroomBits_Default;
			ComputeWaveShaper(pIn, pOut, numFrames, &outSoftClipper_[ch]);
		}
		InterleaveShorts(tPtrs[0], tPtrs[1], pOut, numFrames);
	}
	ShiftLeft_S16(pOut, pOut, numFrames*channels, kMixer_HeadroomBits_Default);
	
	TimeStampOff(0);

	return (kNoErr);
}

// ==============================================================================
// WrapperToCallRender: Call member function Render() to fit PortAudio callback format
//								Return # of frames rendered
// ==============================================================================
int CAudioMixer::WrapperToCallRender( S16 *pOut,
									  unsigned long numStereoFrames,
									  void *pToObject  )
{	
	int error = kNoErr;
	
	U32 numInChannels = ((CAudioMixer*)pToObject)->numInChannels_;
	const IEventListener*	pListeners[numInChannels];
	CAudioEventMessage*		pEvtMsgs[numInChannels];
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
		if (pCh->isDone_ && pCh->GetPlayer())
		{
			// Cache done messages while mixer is locked
			CAudioPlayer* pPlayer = pCh->GetPlayer();
			if (pPlayer && pPlayer->ShouldSendDoneMessage()) 
			{
				pListeners[ch] = pPlayer->GetEventListener();
				pEvtMsgs[ch] = pPlayer->GetAudioEventMsg();
				pEvtMsgs[ch]->audioMsgData.audioCompleted.count++;
				if (pListeners[ch] != kNull)
					pEventMPI_->PostEvent(*pEvtMsgs[ch], 128, pListeners[ch]);
				delete pPlayer;
			}
			pCh->Release(true);
		}
	}
	
	MIXER_UNLOCK;
	
	return error;
}

// ==============================================================================
// Pause: Stop all audio processing and I/O but do not alter state
//
//			FIXXXX: add code for fade out
// ==============================================================================
void CAudioMixer::Pause()
{	
	MIXER_LOCK;
	isPaused_ = true;
	MIXER_UNLOCK;

}

// ==============================================================================
// Resume: Restart all audio processing and I/O from current state
//
//			FIXXXX: add code for fade in
// ==============================================================================
void CAudioMixer::Resume()
{	
	MIXER_LOCK;
	isPaused_ = false;
	MIXER_UNLOCK;
}

// ==============================================================================
// SetMasterVolume :  Range [0..100] maps to [0.0 to 1.0]
// ==============================================================================
void CAudioMixer::SetMasterVolume( U8 x )
{
	MIXER_LOCK;
	S16 x16 = (S16) x;
	audioState_.masterVolume = (U8) BoundS16(&x16, 0, 100);

	masterGainf_[kLeft] = 0.01f*(float)x;

	// Convert to square curve, which is milder than Decibels
	masterGainf_[kLeft ] *= masterGainf_[kLeft];
	masterGainf_[kRight] =	masterGainf_[kLeft];

	// Convert 32-bit floating-point to Q15 fractional integer format 
	masterGaini_[kLeft ] = FloatToQ15(masterGainf_[kLeft]);
	masterGaini_[kRight] = masterGaini_[kLeft];

	audioState_.masterGainf[kLeft ] = masterGainf_[kLeft ];
	audioState_.masterGainf[kRight] = masterGainf_[kRight];
	MIXER_UNLOCK; 
}

// ==============================================================================
// SetSamplingFrequency :  update fs for mixer and all DSP
//					Does not reset or recalculate DSP parameters
// ==============================================================================
void CAudioMixer::SetSamplingFrequency( float x )
{

	samplingFrequency_ = x;
	// FIXX :  Add bounding code

	fsRack_[0] = (long)(samplingFrequency_*0.25f); // kAudioSampleRate_Div4;
	fsRack_[1] = (long)(samplingFrequency_*0.5f ); // kAudioSampleRate_Div2;
	fsRack_[2] = (long)(samplingFrequency_);	   // kAudioSampleRate_Div1;

	for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
		long i;

		SetWaveShaper_SamplingFrequency(&outSoftClipper_[ch], x);

		for (i = 0; i < kAudioMixer_SRCCount; i++)
		{
			SRC *d = &src_[i][ch];
			SRC_SetInSamplingFrequency (d, (float)fsRack_[i]);
			SRC_SetOutSamplingFrequency (d, (float)(2*fsRack_[i]));
		}
	}
}

// ==============================================================================
// SetDSP :	 Transfer high level DSP parameters from audio state to DSP modules
// ==============================================================================
void CAudioMixer::SetDSP()
{
	tAudioState *d = &audioState_;

	U8	   srcType;
	U8	   srcFilterVersion;

	for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{
		WAVESHAPER *ws = &outSoftClipper_[ch];
		SetWaveShaper_SamplingFrequency(ws, samplingFrequency_);
		SetWaveShaper_Parameters(ws, kWaveShaper_Type_V4,
								 (float)d->softClipperPreGainDB,
								 (float)d->softClipperPostGainDB);		
	}
}

// ==============================================================================
// UpdateDSP :	Recalculate DSP parameters
// ==============================================================================
void CAudioMixer::UpdateDSP()
{
	long i;

	for (long ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{

		UpdateWaveShaper(&outSoftClipper_[ch]);
		
		for (i = 0; i < kAudioMixer_SRCCount; i++)
			UpdateSRC(&src_[i][ch]);
	}
}

// ==============================================================================
// ResetDSP :  Reset DSP state
// ==============================================================================
void CAudioMixer::ResetDSP()
{
	long ch, i;
	for (ch = 0; ch < kAudioMixer_MaxOutChannels; ch++)
	{

		ResetWaveShaper(&outSoftClipper_[ch]);

		for (i = 0; i < kAudioMixer_MixBinCount; i++)
			mixBinFilled_[i] = False;
 
		for (i = 0; i < kAudioMixer_SRCCount; i++)
			ResetSRC(&src_[i][ch]);
	}
}

// ==============================================================================
// SetAudioState :	Set "secret" interface parameters for Mixer
// ==============================================================================
void CAudioMixer::SetAudioState(tAudioState *d)
{
	MIXER_LOCK;
	bcopy(d, &audioState_, sizeof(tAudioState));
	SetDSP();
	UpdateDSP();
	MIXER_UNLOCK; 
}

// ==============================================================================
// GetAudioState :	Get "secret" interface parameters for Mixer
// ==============================================================================
void CAudioMixer::GetAudioState(tAudioState *d)
{
	MIXER_LOCK;
	bcopy(&audioState_, d, sizeof(tAudioState));
	MIXER_UNLOCK; 
}

// ==============================================================================
// EnableSpeaker :	Setup DSP that is different for Line and Speaker outputs
// ==============================================================================
void CAudioMixer::EnableSpeaker(Boolean x)
{
	audioState_.useOutSoftClipper = x;
	audioState_.speakerEnabled	  = x;
}

// ==============================================================================
// CreateMIDIPlayer :  
// ==============================================================================
CMidiPlayer *CAudioMixer::CreateMIDIPlayer()
{
	MIXER_LOCK;
	if (pMidiPlayer_)
		delete pMidiPlayer_;

	pMidiPlayer_ = new CMidiPlayer( NULL, GetNextMidiID() );
	MIXER_UNLOCK; 
	return (pMidiPlayer_);
}

// ==============================================================================
// DestroyMIDIPlayer :	
// ==============================================================================
void CAudioMixer::DestroyMIDIPlayer()
{
	MIXER_LOCK;
	if (pMidiPlayer_)
	{
		delete pMidiPlayer_;
		pMidiPlayer_ = NULL;
	}
	MIXER_UNLOCK; 
}

LF_END_BRIO_NAMESPACE()
