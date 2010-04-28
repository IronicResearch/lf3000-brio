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
// | Player A |---| Stream 1  |--->|         |
// +----------+   +-----------+    |         |
//                                 |         |
// +----------+   +-----------+    |         |                          /|
// | Player B |---| Stream 2  |--->|         |                         / |
// +----------+   +-----------+    |         |                      +-+  |
//                                 |         |     +-----------+    | |  |
//                +-----------+    |  Mixer  +---->| Audio Out |----| |  |   
//                | Stream 3  |--->|         |     +-----------+    | |  |  
//                +-----------+    |         |                      +-+  |
//                                 |         |                         \ |
//                   ....          |         |                          \|
//                                 |         |
//                +-----------+    |         |
//                | Stream N  |--->|         |
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
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#undef __STRICT_ANSI__
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
#include <MemPlayer.h>
#include <VorbisPlayer.h>
#include <AVIPlayer.h>

#include <list>

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
CDebugMPI *pDebugMPI_ = NULL;
CEventMPI  *pEventMPI_ = NULL;
CKernelMPI *pKernelMPI_ = NULL;
tMutex mixerMutex_ = {0}; // real init via KernelMPI.InitMutex()

//------------------------------------------------------------------------------
// Helper thread to re-try enabling PortAudio output
//------------------------------------------------------------------------------
bool		bIsOutputEnabled_ = false;	// PortAudio output stream enabled?

static void* HelperThread(void* pctx)
{
	while (true)
	{
		// Re-try enabling PortAudio output
		if (kNoErr == InitAudioOutput( &CAudioMixer::WrapperToCallRender, pctx) )
		{
			StartAudioOutput();
			bIsOutputEnabled_ = true;
			break;
		}
		pKernelMPI_->TaskSleep(100);
	}
	return NULL;
}

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
CAudioMixer::CAudioMixer( int inStreams ):
	IEventListener(kMixerTypes, ArrayCount(kMixerTypes))
{
	long i, j, ch;
	const tMutexAttr	attr = {0};

	numInStreams_ = inStreams;
	
	samplingFrequency_ = kAudioSampleRate;

	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose);
	
	pEventMPI_ = new CEventMPI();

	pKernelMPI_ = new CKernelMPI();

	pDebugMPI_->Assert((numInStreams_ <= kAudioMaxMixerStreams),
					   "CAudioMixer::CAudioMixer: %d is too many streams!\n",
					   numInStreams_ );
	
	// Init mutex for serialization access to internal AudioMPI state.
	tErrType err = pKernelMPI_->InitMutex( mixerMutex_, attr );
	pDebugMPI_->Assert((kNoErr == err), "%s: Couldn't init mutex.\n", __FUNCTION__);
	
	// Allocate audio streams
	pStreams_ = new CStream[ numInStreams_ ];
	pDebugMPI_->Assert((pStreams_ != kNull),
					   "CAudioMixer::CAudioMixer: couldn't allocate %d streams!\n",
					   numInStreams_);

	// Configure sampling frequency conversion
	fsRack_[0] = (long)(samplingFrequency_*0.25f);
	fsRack_[1] = (long)(samplingFrequency_*0.5f );
	fsRack_[2] = (long)(samplingFrequency_);	   

	for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
		mixBinFilled_[i] = false;
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
	
	// Headphone gain	// FIXME: dead code?
	headphoneGainDB_	 = kMixer_Headphone_GainDB;
	headphoneGainF_		 = DecibelToLinearf(headphoneGainDB_);
	headphoneGainWholeI_ = (long)headphoneGainF_; // valgrind suspect
	headphoneGainFracF_	 = headphoneGainF_ - (float)headphoneGainWholeI_;
	headphoneGainFracI_	 = FloatToQ15(headphoneGainFracF_);

	// Set up Audio State struct (interfaces via AudioMPI)
	// NOTE : Keep at end of this routine
	{
		tAudioState *d = &audioState_;
		memset(d, 0, sizeof(tAudioState));
		pDebugMPI_->Assert((sizeof(tAudioState) < kAUDIO_MAX_MSG_SIZE),
			"%s.%d: sizeof(tAudioState)=%d kAUDIO_MAX_MSG_SIZE=%ld\n",
			__FUNCTION__, __LINE__, sizeof(tAudioState), kAUDIO_MAX_MSG_SIZE);
		
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
	
	// Init mixer state vars prior to 1st PortAudio callback (valgrind)
	isPaused_ = false;

	// Init output driver and register callback.  We have to pass in a pointer
	// to the mixer object as a bit of "user data" so that when the callback
	// happens, the C call can get to the mixer's C++ member function for
	// rendering.
	err = InitAudioOutput( &CAudioMixer::WrapperToCallRender,
							   (void *)this);
	// Relax assert to critical warning since re-tries are now possible.
	bIsOutputEnabled_ = (kNoErr == err);
	if (!bIsOutputEnabled_)
		pDebugMPI_->DebugOut(kDbgLvlCritical, "%s: Failed to initalize audio output\n", __FUNCTION__ );
	// Thanks to the miracle of NOP asserts, we can keep trying to
	// enable PortAudio output in case /dev/dsp is in use.
	if (!bIsOutputEnabled_)
	{
		tTaskHndl hndl;
		tTaskProperties props;
		props.TaskMainFcn = &HelperThread;
		props.taskMainArgCount = 1;
		props.pTaskMainArgValues = this;
		pKernelMPI_->CreateTask(hndl, props, NULL);
	}
	else
		StartAudioOutput();

#ifdef USE_RENDER_THREAD	
	// Create separate thread for rendering
	tTaskProperties props;
	props.TaskMainFcn = &CAudioMixer::RenderThread;
	props.taskMainArgCount = 1;
	props.pTaskMainArgValues = this;
	pKernelMPI_->CreateTask(hRenderThread_, props, NULL);
#endif
	
	// See the documentation for GetNextAudioID and GetNextMidiID to understand
	// the audio id assignment scheme.
	nextAudioID = kNoMidiID + 1;
	nextMidiID = 0;
	curMidiId_ = kNoMidiID;
	curMidiAudioId_ = kNoAudioID;

	currentPolicy = kAudioPriorityPolicyNone;

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
	
#ifdef USE_RENDER_THREAD
	// Terminate rendering thread
	void* retval;
	pKernelMPI_->CancelTask(hRenderThread_);
	pKernelMPI_->TaskSleep(10);
	pKernelMPI_->JoinTask(hRenderThread_, retval);
#endif
	
	// Deallocate input streams
	if (pStreams_)
	{
		for (long ch = 0; ch < numInStreams_; ch++)
		{
			CAudioPlayer *pPlayer = pStreams_[ch].GetPlayer();
			if ( pPlayer )
				delete pPlayer;
			pStreams_[ch].Release(true);
		}
		delete[] pStreams_;
	}
	
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

tEventStatus CAudioMixer::Notify( const IEventMessage &msgIn )
{
	tEventStatus status = kEventStatusOK;
	
	const CMixerMessage& msg = dynamic_cast<const CMixerMessage&>(msgIn);
	if( msg.GetEventType() == kAudioCompletedEvent )
	{
		status = kEventStatusOKConsumed;
		DestroyPlayer(msg.pPlayer_);
	}
	return status;
}

// ==============================================================================
// FindFreeStream: Find a free Stream using priority
// ==============================================================================
CStream *CAudioMixer::FindFreeStream( tAudioPriority /* priority */)
{
	CStream *pStream = kNull;

	// Search for idle stream.
	for (long i = 0; i < numInStreams_; i++)
	{
		pStream = &pStreams_[i];
		if (!pStream->GetPlayer())
			return pStream;
		// Stream may contain player which is yet to be destroyed
		if (pStream->IsDone())
			return pStream;
	}
	
	return pStream;
}

// ==============================================================================
// FindStream:	Look for stream with specified ID
//				Return ptr to stream
// ==============================================================================
CStream *CAudioMixer::FindStreamInternal( tAudioID id )
{

	CStream *pStream = kNull;
	
	//Find stream with specified ID
	for (long i = 0; i < numInStreams_; i++)
	{
		CAudioPlayer *pPlayer = pStreams_[i].GetPlayer();
		if ( pPlayer && (pPlayer->GetID() == id))
			pStream = &pStreams_[i];
	}
	return pStream;
}

// ==============================================================================
// FindStream:	Look for stream with specified ID
//				Return ptr to stream
// ==============================================================================
CStream *CAudioMixer::FindStream( tAudioID id )
{
	CStream *pStream = kNull;
	
	MIXER_LOCK;
	pStream = FindStreamInternal(id);
	MIXER_UNLOCK; 
	return pStream;
}

// ==============================================================================
// HandlePlayerEvent: This function decides what to do with the player at
// certain events.  In response to some events, the player will be deleted.  It
// is the caller's responsibility to ensure that the player is not bound to a
// stream under these circumstances.
// ==============================================================================
void CAudioMixer::HandlePlayerEvent( CAudioPlayer *pPlayer, tEventType type )
{
	const IEventListener *listener;
	tAudioOptionsFlags flags;

	if(!pPlayer)
		return;

	flags = pPlayer->GetOptionsFlags();
	listener = pPlayer->GetEventListener();

	if(type == kAudioCompletedEvent)
	{
		// In this case, we delete the player because the player is all done.
		if(listener && (flags & kAudioOptionsDoneMsgAfterComplete))
		{
			// Unfortunately, the MPI implies that midi players are somehow
			// different than other players, even if we're just playing back a file.
			// Specifically, we have to send different done messages than for other
			// players (of course the option flag is the same).
			if (pPlayer->GetID() == curMidiAudioId_)
			{
				tAudioMsgMidiCompleted	msg;
				msg.midiPlayerID = curMidiId_; //pPlayer->GetID();
				msg.payload = pPlayer->GetPayload();
				msg.count = 1;
				CAudioEventMessage event(msg);
				pEventMPI_->PostEvent(event, 128, listener);
			}
			else
			{
				tAudioMsgAudioCompleted msg;
				msg.audioID = pPlayer->GetID();
				msg.payload = pPlayer->GetPayload();
				msg.count = 1;
				CAudioEventMessage event(msg);
				pEventMPI_->PostEvent(event, 128, listener);
			}
		}
#if 0	// FIXME/dm: Player destruction at this point breaks Nicktoons functionality with GM Brio 3011
		// Defer player destruction.
		CMixerMessage event(pPlayer, kAudioCompletedEvent);
		pEventMPI_->PostEvent(event, 128, this);
#endif
	}
	else if(type == kAudioLoopEndEvent)
	{
		if(listener && (flags & kAudioOptionsLoopEndMsg))
		{
			tAudioMsgLoopEnd msg;
			msg.audioID = pPlayer->GetID();
			msg.payload = pPlayer->GetNumLoops();
			msg.count = pPlayer->GetLoopCount();
			CAudioEventMessage event(msg);
			pEventMPI_->PostEvent(event, 128, listener);
		}
	}
	else if(type == kAudioTimeEvent)
	{
		if(listener && (flags & kAudioOptionsTimeEvent))
		{
			tAudioMsgTimeEvent msg;
			msg.audioID = pPlayer->GetID();
			msg.payload = pPlayer->GetPayload();
			msg.playtime = pPlayer->GetAudioTime_mSec();
			msg.realtime = pKernelMPI_->GetElapsedTimeAsMSecs();
			CAudioEventMessage event(msg);
			pEventMPI_->PostEvent(event, 128, listener);
		}
	}
	else
	{
		pDebugMPI_->DebugOut(kDbgLvlImportant,
							 "%s: Unrecognized audio event 0x%lx\n",
							 __FUNCTION__, type);
	}
}

// ==============================================================================
// HandlePlayerLooping: This function inspects the player state and decides what
// to do with respect to looping.  It returns true if all looping is complete
// and the player can be freed.  It returns false if the player still has loops
// to play.  pPlayer MUST be non-null.
// ==============================================================================
Boolean CAudioMixer::HandlePlayerLooping(CAudioPlayer *pPlayer)
{

	tAudioOptionsFlags flags = pPlayer->GetOptionsFlags();

	if(!pPlayer->IsDone())
		return false;

	if(!(flags & kAudioOptionsLooped))
		return true;

	if(pPlayer->GetLoopCount() < pPlayer->GetNumLoops())
	{
		pPlayer->IncLoopCount();
		pPlayer->RewindFile();
		return false;
	}

	return true;
}

// ==============================================================================
// IsAnyAudioActive:  Gk FI
// ==============================================================================
Boolean CAudioMixer::IsAnyAudioActive( void )
{
	Boolean ret = false;

	MIXER_LOCK;
	// Search for a stream that is in use
	for (long i = 0; i < numInStreams_; i++)
	{
		if (pStreams_[i].GetPlayer() && !pStreams_[i].IsDone())
			// Additionally qualify stream state
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
// FindKillableStream: When all players of a certain type are playing, or all
// streams are full, this function can be called to return a stream with a
// player that can be halted, given the priority and the current priority
// policy.  The caller can optionally supply a condition that the player must
// meet to further filter the results.
// ==============================================================================

// But first, a helper function for the list sort.
bool comparePriority(CStream *c1, CStream *c2)
{
	tAudioPriority p1 = c1->GetPlayer()->GetPriority();
	tAudioPriority p2 = c2->GetPlayer()->GetPriority();
	if(p1 < p2)
		return true;
	return false;
}

CStream *CAudioMixer::FindKillableStream(ConditionFunction *cond,
										   tAudioPriority priority)
{
	CStream *pStream = NULL;

	if(currentPolicy == kAudioPriorityPolicySimple)
	{
		list<CStream *> sortList;
		//Probably should have used a list for the streams from the beginning
		//so we wouldn't have to populate it manually.
		for(U32 i=0; i<numInStreams_; i++)
		{
			pStream = &pStreams_[i];
			CAudioPlayer *pPlayer = pStream->GetPlayer();
			// 1st candidate for killable stream is dead player
			if (pPlayer && pPlayer->IsDone())
				return pStream;
			if(pPlayer)
				// Only add non-null players that meet the condition, if there
				// is a condition.
				if((!cond) ||
				   (cond && cond(pPlayer)) )
					sortList.push_front(pStream);
		}
		sortList.sort(comparePriority);
		// Now the list is in increasing order of priority.  Step through it
		// until we find one of lesser or equal priority to kill
		pStream=NULL;
		list<CStream *>::iterator it;
		for (it = sortList.begin(); it != sortList.end(); it++) {
			if((*it)->GetPlayer()->GetPriority() <= priority) {
				pStream = *it;
#if 1			// BUGFIX/dm: Use most preferable choice per spec (TTP #1958)
				break;
#endif
			}
		}
	}
	return pStream;
}

tPriorityPolicy CAudioMixer::GetPriorityPolicy(void)
{
	tPriorityPolicy policy;
	MIXER_LOCK;
	policy = currentPolicy;
	MIXER_UNLOCK;
	return policy;
}

tErrType CAudioMixer::SetPriorityPolicy(tPriorityPolicy policy)
{
	tErrType ret = kNoImplErr;
	MIXER_LOCK;
	if((policy == kAudioPriorityPolicyNone) ||
	   (policy == kAudioPriorityPolicySimple))
	{
		currentPolicy = policy;
		ret = kNoErr;
	}
	MIXER_UNLOCK;
	return ret;
}

// ==============================================================================
// CreatePlayer:   Allocate player based on file extension argument.  This
// function is rife with player-specific code that could be generalized.
// Somebody with fancier C++ skills than I will have to do it.
// ==============================================================================
CAudioPlayer *CAudioMixer::CreatePlayer(tAudioStartAudioInfo *pInfo,
										char *sExt )
{
	CAudioPlayer *pPlayer = NULL;
	tAudioID newID;

	if (sExt == NULL)
	{
		if (CMemPlayer::GetNumPlayers() < CMemPlayer::GetMaxPlayers())
		{
			newID = GetNextAudioID();
			pPlayer = new CMemPlayer( pInfo, newID );
		}
		else
		{
			pDebugMPI_->DebugOut(kDbgLvlImportant,
				"%s: Max number of mem players exceeded.\n", __FUNCTION__);
		}
	}
	else if (!strcmp(sExt, "raw")  || !strcmp( sExt, "RAW")	||
		!strcmp(sExt, "brio") || !strcmp( sExt, "BRIO") ||
		!strcmp(sExt, "wav")  || !strcmp( sExt, "WAV") || 
		!strcmp(sExt, "avi"))
	{
		if(CRawPlayer::GetNumPlayers() < CRawPlayer::GetMaxPlayers())
		{
			newID = GetNextAudioID();
			if (!strcmp(sExt, "avi"))
				pPlayer = new CAVIPlayer( pInfo, newID );
			else
				pPlayer = new CRawPlayer( pInfo, newID );
		}
		else
		{
			//We must decide whether to stop and destroy a player depending on
			//priority.
			CStream *ch = FindKillableStream(CRawPlayer::IsRawPlayer,
											 pInfo->priority);
			if(ch)
			{
				RemovePlayerInternal(ch->GetPlayer()->GetID(), false);
				newID = GetNextAudioID();
				pPlayer = new CRawPlayer( pInfo, newID );
			} else {
				pDebugMPI_->DebugOut(kDbgLvlImportant,
									 "%s: Max number of raw players exceeded.\n",
									 __FUNCTION__);
			}
		}
	}
	else if (!strcmp( sExt, "ogg" ) || !strcmp( sExt, "OGG") ||
			 !strcmp( sExt, "aogg") || !strcmp( sExt, "AOGG"))
	{
		if(CVorbisPlayer::GetNumPlayers() < CVorbisPlayer::GetMaxPlayers())
		{
			newID = GetNextAudioID();
			pPlayer = new CVorbisPlayer( pInfo, newID );
		}
		else
		{
			//We must decide whether to stop and destroy a player depending on
			//priority.
			CStream *ch = FindKillableStream(CVorbisPlayer::IsVorbisPlayer,
											   pInfo->priority);
			if(ch)
			{
				RemovePlayerInternal(ch->GetPlayer()->GetID(), false);
				newID = GetNextAudioID();
				pPlayer = new CVorbisPlayer( pInfo, newID );
			} else {
				pDebugMPI_->DebugOut(kDbgLvlImportant,
									 "%s: Max number of vorbis players exceeded.\n",
									 __FUNCTION__);
			}
		}
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
	CStream *pStream;
	
	MIXER_LOCK;
	pStream = FindFreeStream( pInfo->priority );
	if (!pStream)
	{
		pDebugMPI_->DebugOut(kDbgLvlImportant, "%s: No stream available\n",
							 __FUNCTION__);
		goto error;
	}

	if (pStream->GetPlayer())
		RemovePlayerInternal(pStream->GetPlayer()->GetID(), false);

	// Turns out, CreatePlayer is very slow (at least for ogg)
	// and does not need the mixer lock, so we release it.
	MIXER_UNLOCK;
	pPlayer = CreatePlayer(pInfo, sExt);
	MIXER_LOCK;

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

	pStream->InitWithPlayer( pPlayer );

	// Warn if player sample rate was clamped to fit Brio mixer stream
	if (pStream->GetSamplingFrequency() != pPlayer->GetSampleRate()) {
		pDebugMPI_->DebugOut(kDbgLvlImportant,
				"%s: player %d sample rate clamped from %d to %d\n",
				__FUNCTION__, (int)id, (int)pPlayer->GetSampleRate(), (int)pStream->GetSamplingFrequency());
	}
	
	pStream->SetPan(	 pInfo->pan );
	pStream->SetVolume( pInfo->volume );
	
	goto success;
 error:
	if(pPlayer)
		DestroyPlayer(pPlayer);
 success:
	MIXER_UNLOCK; 
	return id;
}

// ==============================================================================
// RemovePlayerInternal:
// ==============================================================================
void CAudioMixer::RemovePlayerInternal( tAudioID id, Boolean noDoneMessage )
{
	CStream *pStream;
	CAudioPlayer *pPlayer;
	pStream = FindStreamInternal(id);
	if (pStream && pStream->GetPlayer()) {
		//Q: perhaps we should pass noDoneMessage?  A: Actually, this will break
		//the app manager and perhaps other apps.  Because this flag has been
		//ignored for so long, people have not consistently set it.
		//Unfortunately, it's a bit late to fix.
		pPlayer = pStream->GetPlayer();
		if(pPlayer)
			delete pPlayer;
		pStream->Release(true);
	}
}

// ==============================================================================
// RemovePlayer:
// ==============================================================================
void CAudioMixer::RemovePlayer( tAudioID id, Boolean noDoneMessage )
{

	MIXER_LOCK;
	RemovePlayerInternal(id, noDoneMessage);
	MIXER_UNLOCK; 
}

// ==============================================================================
// RemoveAllPlayer: 
// ==============================================================================
void CAudioMixer::RemoveAllPlayer( void )
{
	Boolean ret = false;

	MIXER_LOCK;
	// Search for a stream that is in use
	for (long i = 0; i < numInStreams_; i++)
	{
		CAudioPlayer *pPlayer = pStreams_[i].GetPlayer();
		if ( pPlayer )
			delete pPlayer;
		pStreams_[i].Release(true);
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// PauseAllPlayer: 
// ==============================================================================
void CAudioMixer::PauseAllPlayer( void )
{
	MIXER_LOCK;
	// Pause all active player streams
	for (long i = 0; i < numInStreams_; i++)
	{
		CAudioPlayer *pPlayer = pStreams_[i].GetPlayer();
		if ( pPlayer && !pPlayer->IsDone() )
			pPlayer->Pause();
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// ResumeAllPlayer: 
// ==============================================================================
void CAudioMixer::ResumeAllPlayer( void )
{
	MIXER_LOCK;
	// Resume all active paused player streams
	for (long i = 0; i < numInStreams_; i++)
	{
		CAudioPlayer *pPlayer = pStreams_[i].GetPlayer();
		if ( pPlayer && !pPlayer->IsDone() && pPlayer->IsPaused() )
			pPlayer->Resume();
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// PausePlayer:
// ==============================================================================
void CAudioMixer::PausePlayer( tAudioID id )
{
	CStream *pStream;

	MIXER_LOCK;
	pStream = FindStreamInternal(id);
	if (pStream && pStream->GetPlayer() && !pStream->GetPlayer()->IsDone())
	{
		pStream->GetPlayer()->Pause();
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// ResumePlayer:
// ==============================================================================
void CAudioMixer::ResumePlayer( tAudioID id )
{
	CStream *pStream;

	MIXER_LOCK;
	pStream = FindStreamInternal(id);
	if (pStream && pStream->GetPlayer() && !pStream->GetPlayer()->IsDone())
	{
		pStream->GetPlayer()->Resume();
	}
	MIXER_UNLOCK; 
}

// ==============================================================================
// IsPlayerPlaying:
// ==============================================================================
Boolean CAudioMixer::IsPlayerPlaying( tAudioID id )
{
	CStream *pStream;
	Boolean playing = false;

	MIXER_LOCK;
	pStream = FindStreamInternal(id);
	if (pStream && pStream->GetPlayer() && !pStream->IsDone())
	{
		// Additionally qualify stream state
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
// For each input stream, render into the global temp buffer pStreamBuf_, scale the
// rendered output down by kMixer_HeadroomBits_Default, add the output from
// pStreamBuf_ to the appropriate frequency bin.
//
// Render midi output to pStreamBuf_, scale the output down by
// kMixer_HeadroomBits_Default, add the output from pStreamBuf_ to the
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
//	  (GX FIXXX:  incorrect return value.  Could be Max of stream returns values.)
// ==============================================================================
int CAudioMixer::Render( S16 *pOut, U32 numFrames )
{
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
		mixBinFilled_[i] = false;
	}

	// ---- Render all active streams (not including MIDI)
	for (ch = 0; ch < numInStreams_; ch++)
	{
		CStream *pStream = &pStreams_[ch];
		// Render if stream is in use and not paused
		if ( pStream->GetPlayer() &&
		    !pStream->GetPlayer()->IsPaused() &&
			!pStream->IsDone())
		{
			ClearShorts(pStreamBuf_, numFrames*channels);
			long streamSamplingFrequency = pStream->GetSamplingFrequency();
			U32	 framesToRender = pStream->GetFramesToRender();
			U32	 sampleCount	= framesToRender*channels;

			// Player renders data into stream's stereo output buffer
#ifdef USE_RENDER_THREAD
			framesRendered = pStream->PostRender( pStreamBuf_, framesToRender );
#else
			framesRendered = pStream->Render( pStreamBuf_, framesToRender );
#endif
			// Now that rendering is complete, send done messages and delete
			// players.  Note that this is going to be migrated to a Notify
			// function to off-load the render loop.
			CAudioPlayer *pPlayer = pStream->GetPlayer();
			// Incomplete Render callback?
			if (framesRendered < framesToRender && !pPlayer->IsDone())
				return kAudioNoDataAvailErr;
			if (pStream->IsDone())
			{
				// find the number of samples that the player did not render
				U32 zeroSamples = (framesToRender - framesRendered)*channels;
				U32 zeroOffset = framesRendered*channels;
				// Now we either play the clip again or free the stream
				tAudioOptionsFlags flags = pPlayer->GetOptionsFlags();
				Boolean freeStream = HandlePlayerLooping(pPlayer);
				
				if(freeStream)
				{
					//If we're freeing the stream, pad the output and send the
					//done message
					memset(pStreamBuf_ + zeroOffset, 0, zeroSamples*sizeof(S16));
					HandlePlayerEvent(pPlayer, kAudioCompletedEvent);
#if 0 				// FIXME/dm: Stream cannot be released because player is not destroyed above
					pStream->Release(true);
#endif
				}
				else
				{
					//If we're not freeing the stream, we must be looping.  To
					//ensure continuity, we need to fill in the rest of the
					//stream buffer by rendering again
					if(zeroSamples)
					{
						// FIXME: remainder at pStreamBuf_[framesRendered]
						pStream->Render(pStreamBuf_ + framesRendered, framesToRender - framesRendered);
					}
					HandlePlayerEvent(pPlayer, kAudioLoopEndEvent);
				}
			}
			else if (pPlayer->IsTimeEvent())
			{
				// Handle time event callback at elapsed time intervals
				if (pPlayer->IsTimeElapsed())
					HandlePlayerEvent(pPlayer, kAudioTimeEvent);
			}

			// Add output to appropriate Mix "Bin" 
			long mixBinIndex = GetMixBinIndex(streamSamplingFrequency);
			S16 *pMixBin = pMixBinBufs_[mixBinIndex];
			ShiftRight_S16(pStreamBuf_, pStreamBuf_, sampleCount,
						   kMixer_HeadroomBits_Default);
			AccS16toS16(pMixBin, pStreamBuf_, sampleCount,
						mixBinFilled_[mixBinIndex]);
			mixBinFilled_[mixBinIndex] = true;

		}
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
		
	MIXER_LOCK;

	if (((CAudioMixer*)pToObject)->IsPaused())
	{
		ClearShorts(pOut, numStereoFrames*kAudioMixer_MaxOutChannels);
		error = kNoErr;
	} else {
		error = ((CAudioMixer*)pToObject)->Render( pOut, numStereoFrames );
	}

	MIXER_UNLOCK;
	
	return error;
}

// ==============================================================================
// RenderThread: Does rendering in separate thread from mixer callback
// ==============================================================================
#ifdef USE_RENDER_THREAD
void* CAudioMixer::RenderThread(void* pCtx)
{
	CAudioMixer*	pMixer = (CAudioMixer*)pCtx;
	
	while (true)
	{
		bool busy = false;
		MIXER_LOCK;
		// Render all active streams to buffers
		for (int ch = 0; ch < pMixer->numInStreams_; ch++)
		{
			CStream*		pStream = &pMixer->pStreams_[ch];
			CAudioPlayer*	pPlayer = pStream->GetPlayer();
			// Render if stream is in use and not paused
			if ( pPlayer &&
			    !pPlayer->IsPaused() &&
				!pPlayer->IsDone())
			{
				// Rendering is buffered into ring buffer 
				// to be extracted via PostRender()
				S16* pBuf = pStream->GetRenderBuf();
				U32 frames = pStream->GetFramesToRender();
				U32 rendered = pStream->PreRender(pBuf, frames);
				if (rendered == frames)
					busy = true;
			}
		}
		MIXER_UNLOCK;
		if (!busy)
			pKernelMPI_->TaskSleep(10);
	}
	
	return NULL;
}
#endif

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
			mixBinFilled_[i] = false;
 
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
	// soft clipper on for speaker, off for headphones (TTP #1532)
	audioState_.useOutSoftClipper = x;
	audioState_.speakerEnabled	  = x;
	pDebugMPI_->DebugOut(kDbgLvlValuable, "%s: %d\n", __FUNCTION__, (int)x);
}

// ==============================================================================
// GetMidiID :	Get next MIDI player ID
// ==============================================================================
tMidiPlayerID CAudioMixer::GetMidiID(void)
{
	curMidiId_ = (tMidiPlayerID)GetNextMidiID();
	return curMidiId_;
}

// ==============================================================================
// SetMidiID :	Set MIDI audio player ID
// ==============================================================================
void CAudioMixer::SetMidiAudioID(tMidiPlayerID midiId, tAudioID audioId)
{
	curMidiId_ = midiId;
	curMidiAudioId_ = audioId;
}

// ==============================================================================
// SeekAudioTime :
// ==============================================================================
Boolean CAudioMixer::SeekAudioTime(tAudioID id, U32 timeMilliSeconds)
{
	Boolean found = false;
	MIXER_LOCK;
	CStream *pStream = FindStreamInternal(id);
	if (pStream)
	{
		CAudioPlayer *pPlayer = pStream->GetPlayer();
		if (pPlayer)
			found = pPlayer->SeekAudioTime( timeMilliSeconds );
	}
	MIXER_UNLOCK;
	return found;
}

LF_END_BRIO_NAMESPACE()

