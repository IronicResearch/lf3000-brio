//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Channel.cpp
//
// Description: manages processing of audio data on a mixer channel
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Channel.h>

#include <EventMPI.h>

#include <RawPlayer.h>
#include <VorbisPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CChannel implementation
//==============================================================================
CChannel::CChannel()
{
tErrType result;
// Initialize class variables
//{static int c=0; printf("CChannel::CChannel %d \n", c++);}

//	mpChain_ = kNull;
//	fOwnProcessor_ = false;
	pPlayer_ = kNull;

	fInUse_     = false;
	fPaused_    = false;
	fReleasing_ = false;
	isDone_     = false;

// Set DSP values
	pan_    = kPan_Default;
	volume_ = kVolume_Default;

	samplingFrequency_ = 0;

// Channel DSP Engine
//	pDSP_ = NULL;
//	DefaultMixerChannel(&pDSP_);
//	pDSP_->inGainDB = 0.0f;
//	pDSP_->EQ;
//	pDSP_->reverbSendDB = 0.0f;
//	pDSP_->pan         = kPanValue_Center;
//	pDSP_->postGainDB = 0.0f;

	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel ); // kDbgLvlVerbose, kAudioDebugLevel
//	printf("CChannel::CChannel: printf volume_=%d pan_=%d \n", volume_, pan_);

// Get Kernel MPI
//	pKernelMPI_ =  new CKernelMPI();
//	result = pKernelMPI_->IsValid();
//	pDebugMPI_->Assert((true == result), "CChanel::ctor: Unable to create KernelMPI.\n");
}	// ---- end CChannel ----

// ==============================================================================
// ~CChannel
// ==============================================================================
CChannel::~CChannel()
{
//{static int c=0; printf("CChannel::~CChannel %d \n", c++);}
	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;
//	if (pKernelMPI_)    
//       {
//		delete pKernelMPI_;  
//        pKernelMPI_ = NULL;
//        }
}	// ---- end ~CChannel ----

// ==============================================================================
// SetPan :     Channel stereo position   [Left .. Center .. Right]
// =============================================================================
    void 
CChannel::SetPan( S8 x )
{
pan_ = BoundS8(&x, kPan_Min, kPan_Max);

// Convert input range to [-100 .. 100]range [0 .. 1] suitable
// for the constant power calculation
// ChangeRangef(x, L1, H1, L2, H2)
float xf = ChangeRangef((float)pan_, (float) kPan_Min, (float)kPan_Max, 0.0f, 1.0f);

// PanValues(xf, panValuesf);
ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
//printf("CChannel::SetPan : printf %d -> <%f , %f> \n", x, panValuesf[kLeft], panValuesf[kRight]);
RecalculateLevels();
}	// ---- end SetPan ----

// ==============================================================================
// SetVolume : Convert range [0 .. 100] to [-100 ..0] dB
// ==============================================================================
    void 
CChannel::SetVolume( U8 x )
{
//printf("CChannel::SetVolume :  %d\n", x);
volume_ = BoundU8(&x, kVolume_Min, kVolume_Max);

// ChangeRangef(x, L1, H1, L2, H2)
// FIXX: move to decibels, but for now, linear volume
gainf  = ChangeRangef((float)x, (float) kVolume_Min, (float)kVolume_Max, 0.0f, 1.0f);
// Convert to square curve, which is milder than Decibels
gainf *= gainf;
gainf *= kChannel_Headroomf; // DecibelToLinearf(-Channel_HeadroomDB);
//gainf = ChangeRangef((float)x, 0.0f, 100.0f, -100.0f, 0.0f);
//gainf =  DecibelToLinearf(gainf);

//printf( "CChannel::SetVolume - %d -> %g\n", volume_, gainf );	

//printf("%f dB -> %f \n", -1.505, DecibelToLinearf(-1.505));
//printf("%f dB -> %f \n", -3.01, DecibelToLinearf(-3.01));
//printf("%f dB -> %f \n", -6.02, DecibelToLinearf(-6.02));
//printf("%f dB -> %f \n", 1.505, DecibelToLinearf(1.505));
//printf("%f dB -> %f \n", 3.01, DecibelToLinearf(3.01));
//printf("%f dB -> %f \n", 6.02, DecibelToLinearf(6.02));
//printf("sqrt(2)/2 = %g\n", 0.5f*sqrt(2.0));

RecalculateLevels();
}	// ---- end SetVolume ----

// ==============================================================================
// RecalculateLevels : Recalculate levels when either pan or volume changes
// ==============================================================================
void CChannel::RecalculateLevels()
{
//printf("CChannel::RecalculateLevels : \n");

levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
levelsf[kRight] =  panValuesf[kRight]*gainf;

// Convert 32-bit floating-point to Q15 fractional integer format 
levelsi[kLeft ] = FloatToQ15(levelsf[kLeft ]);
levelsi[kRight] = FloatToQ15(levelsf[kRight]);

//printf("CChannel::RecalculateLevels : gainf=%g Pan <L=%g R=%g>\n", 
//         gainf, panValuesf[kLeft], panValuesf[kRight]);
//printf("CChannel::RecalculateLevels : levelsf L=%g R=%g\n", levelsf[kLeft], levelsf[kRight]);
}	// ---- end RecalculateLevels ----

// ==============================================================================
// ShouldRender
// ==============================================================================
Boolean CChannel::ShouldRender( void ) 
{
Boolean shouldRender = (fInUse_ && !fPaused_ && !fReleasing_ && !isDone_); // && pPlayer_);
//printf("CChannel::ShouldRender : return (%d) = fInUse_=%d !fPaused_=%d && !fReleasing_=%d\n", shouldRender, fInUse_, !fPaused_, !fReleasing_);
	
return (shouldRender); 
}	// ---- end ShouldRender ----

// ==============================================================================
// SendDoneMsg:  Transmit Done message to listener
// ==============================================================================
void CChannel::SendDoneMsg( void )
{
//printf("CChannel::SendDoneMsg: pPlayer_=%p\n", (void*)pPlayer_);
if (!pPlayer_)
    return;

const tEventPriority	kPriorityTBD = 0;
tAudioMsgAudioCompleted	data;
//printf("CChannel::SendDoneMsg audioID=%ld\n", pPlayer_->GetID());

data.audioID = pPlayer_->GetID();	        
data.payload = 0;
data.count   = 1;

CEventMPI	event;
CAudioEventMessage	msg(data);
event.PostEvent(msg, kPriorityTBD, pPlayer_->GetEventListener());
}   // ---- end SendDoneMsg() ----

// ==============================================================================
// Release:
// ==============================================================================
    tErrType 
CChannel::Release( Boolean suppressPlayerDoneMsg )
{
//printf("CChannel::Release START $%p\n", (void *)pPlayer_);
	
if (!pPlayer_)
    return (kNoErr);

fReleasing_ = true;
fPaused_    = true;

// Deactivate "done" message if specified
if (suppressPlayerDoneMsg)
    {
    pPlayer_->SetEventListener(NULL);
    pPlayer_->ActivateSendDoneMessage(false);
    }

//#include <sys/time.h> sleep(1);
// Player's dtors are mutex-protected -  safe to be deleted.  
// The dtors wait until RenderBuffer() completes before deleting 
	delete pPlayer_;
	pPlayer_ = kNull;

// no longer in use
	fReleasing_ = false;
	fInUse_     = false;

//printf("CChannel::Release - END\n");
	return (kNoErr);
}	// ---- end Release ----

// ==============================================================================
// SetPlayer : 
// ==============================================================================
    void 
CChannel::SetPlayer( CAudioPlayer *pPlayer, long releaseExistingPlayer )
{
const IEventListener *pOldListener = NULL;
U8 bOldDoneMessage                 = false;
tAudioID oldId                     = 0;

//{static long c=0; printf("CChannel::SetPlayer: %ld START releaseExistingPlayer=%ld\n", c++, releaseExistingPlayer);}

// If we're pre-empting, release active player first.
if (pPlayer_)
    {
//{static long c=0; printf("CChannel::SetPlayer: %ld BEFO Release releaseExistingPlayer=%ld\n", c++, releaseExistingPlayer);}
    pOldListener    = pPlayer_->GetEventListener();
    bOldDoneMessage = pPlayer_->ShouldSendDoneMessage();
    oldId           = pPlayer_->GetID();

    if (releaseExistingPlayer)
    	Release( true );		// true = suppress done msg 
//{static long c=0; printf("CChannel::SetPlayer: %ld AFTA Release \n", c++);}
    }

pPlayer_ = pPlayer;
SetPan(    pPlayer->GetPan() );
SetVolume( pPlayer->GetVolume() );
SetSamplingFrequency(pPlayer->GetSampleRate());
//printf("CChannel::SetPlayer : pan=%d volume=%d \n", pan_, volume_);

// Send done message if it was setup 
if (pOldListener && bOldDoneMessage)
    {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgAudioCompleted	data;
//printf("CChannel::SetPlayer sending Done message from Audio ID =%ld\n", oldId);

	data.audioID = oldId;	        
	data.payload = 0;
	data.count   = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pOldListener);
    }

fInUse_     = true;
fPaused_    = false;
fReleasing_ = false;

//{static long c=0; printf("CChannel::SetPlayer: %ld END \n", c++);}
}	// ---- end SetPlayer ----

// ==============================================================================
// InitWithPlayer
// ==============================================================================
    tErrType 
CChannel::InitWithPlayer( CAudioPlayer* pPlayer )
{
//{static long c=0; printf("CChannel::InitWithPlayer: %ld start pPlayer=%p\n", c++, (void*)pPlayer);}

// Release inactive player first
if (pPlayer_)
	Release( true );		// true = suppress done msg if requested
//{static long c=0; printf("CChannel::InitWithPlayer: %ld AFTA Release \n", c++);}
	
// Convert interface parameters to DSP level data and reset channel
    pPlayer_    = pPlayer;
	SetPan(    pPlayer->GetPan() );
	SetVolume( pPlayer->GetVolume() );
	SetSamplingFrequency(pPlayer->GetSampleRate());
//printf("CChannel::InitWithPlayer : pan=%d volume=%d fs=%ld complete=%d\n", pan_, volume_, samplingFrequency_, pPlayer_->IsComplete());

// Convert high level parameters to values suitable for DSP
//	pDSP_->samplingFrequency = (float) samplingFrequency_;
//printf("CChannel::InitWithPlayer: samplingFrequency=%g \n", pDSP_->samplingFrequency);
//	MixerChannel_SetSamplingFrequency(pDSP_, samplingFrequency_);
//	UpdateMixerChannel(pDSP_);
//	ResetMixerChannel (pDSP_);
// FIXX: these buffers will migrate to briomixer.cpp/Mixer.cpp
//	MixerChannel_SetAllTempBuffers(pDSP_, tmpPs_, kChannel_MaxTempBuffers);

//(fInUse_ && !fPaused_ && !fReleasing_ && !isDone_);

fPaused_    = false;
fReleasing_ = false;
isDone_     = false;
// Finally, allow this channel to be added to Mixer
fInUse_     = true;

//{static long c=0; printf("CChannel::InitWithPlayer: %ld end \n", c++);}
return (kNoErr);
}	// ---- end InitWithPlayer ----

// ==============================================================================
// Render
// ==============================================================================
    U32 
CChannel::Render(S16 *pOut, int framesToRender )
// GK CHECK FIXX numFrames  IS THIS FRAMES OR SAMPLES  !!!!!!!  THIS APPEARS TO BE SAMPLES
{
int samplesToRender = framesToRender * 2;  // 2 channels
int samplesRendered = 0;
int framesRendered  = 0;

//{static long c=0; printf("CChannel::Render: START %ld framesToRender=%d\n", c++, framesToRender); }
// Decide how to deal with player done i.e. playerFramesRendered comes back 
// less than numStereoFrames: does player send done, or channel.
	
// Compute channel effects processor
//	if (pFxChain_)
//		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOut );
ClearShorts(pOut, samplesToRender);

// Call player to render a stereo buffer
if (pPlayer_)
    {
    framesRendered  = pPlayer_->Render( pOut, framesToRender );
    samplesRendered = framesRendered*2;
    isDone_ = (framesToRender > framesRendered);
    }

//printf("Channel::Render: levelsf <%f , %f > <%f, %f> dB \n", levelsf[kLeft], levelsf[kRight],
//        LinearToDecibelf(levelsf[kLeft]), LinearToDecibelf(levelsf[kRight]));
//printf("Channel::Render: levelsi <%f , %f > \n", Q15ToFloat(levelsi[kLeft]), Q15ToFloat(levelsi[kRight]));

// Scale stereo out buffer (Assumes all audio player output is two channel)
for (int i = 0; i < samplesRendered; i += 2)
	{
// Integer scaling : gain + stereo pan
//	y = (S32)(levelsf[kLeft] * (float)pOut[i]);	// FLOAT
	pOut[i  ] = (S16) MultQ15(levelsi[kLeft ], pOut[i  ]);	// Q15  1.15 Fixed-point	
	pOut[i+1] = (S16) MultQ15(levelsi[kRight], pOut[i+1]);				
	}

//{static long c=0; printf("Channel::Render%ld: END framesRendered=%d\n", c++, framesRendered);}
return (framesRendered);
}	// ---- end Render ----

LF_END_BRIO_NAMESPACE()
// EOF	
