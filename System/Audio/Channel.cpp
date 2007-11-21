//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Channel.cpp
//
// Description:
//		The class to manage the processing of audio data on an audio channel.
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

#define kPan_Default    0
#define kPan_Min    (-100)
#define kPan_Max      100

#define kVolume_Default  100
#define kVolume_Min        0
#define kVolume_Max      100

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
	pKernelMPI_ =  new CKernelMPI();
	result = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == result), "CChanel::ctor: Unable to create KernelMPI.\n");
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
	if (pKernelMPI_)    
        {
		delete pKernelMPI_;  
        pKernelMPI_ = NULL;
        }
}	// ---- end ~CChannel ----

// ==============================================================================
// SetPan :     Channel stereo position   Left .. Center .. Right
// =============================================================================
void CChannel::SetPan( S8 x )
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
void CChannel::SetVolume( U8 x )
{
//printf("CChannel::SetVolume : printf  %d\n", x);
volume_ = x; //BoundU8(&x, kVolume_Min, kVolume_Max);

// ChangeRangef(x, L1, H1, L2, H2)
// FIXX: move to decibels, but for now, linear volume
gainf = ChangeRangef((float)x, (float) kVolume_Min, (float)kVolume_Max, 0.0f, 1.0f);
gainf *= kDecibelToLinearf_m3dBf; // DecibelToLinearf(-3.0);
//gainf = ChangeRangef((float)x, 0.0f, 100.0f, -100.0f, 0.0f);
//gainf =  DecibelToLinearf(gainf);

//pDebugMPI_->DebugOut( kDbgLvlVerbose, 
//			"CChannel::SetVolume - %d -> %f\n", static_cast<int>(volume_) , static_cast<int>(gainf) );	
//printf( "CChannel::SetVolume - %d -> %g\n", volume_, gainf );	

//printf("%f dB -> %f \n", -3.01, DecibelToLinearf(-3.01));
//printf("%f dB -> %f \n", -6.02, DecibelToLinearf(-6.02));
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
// SendDoneMsg
// ==============================================================================
void CChannel::SendDoneMsg( void )
{
printf("CChannel::SendDoneMsg: pPlayer_=%p\n", (void*)pPlayer_);
if (pPlayer_)
    {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgAudioCompleted	data;
//printf("CChannel::SendDoneMsg audioID=%ld\n", pPlayer_->GetID());

	data.audioID = pPlayer_->GetID();	        
	data.payload = 0;
	data.count   = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pPlayer_->GetEventListener());
    }
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
// RenderBuffer
// ==============================================================================
    U32 
CChannel::RenderBuffer(S16 *pOut, int numFrames )
{
int numSamples = numFrames * 2;  // 2 channels
S32 y;
//{static long c=0; printf("CChannel::RenderBuffer: START %ld \n", c++); }

// Decide how to deal with player done i.e. playerFramesRendered comes back 
// less than numStereoFrames: does player send done, or channel.
	
// Compute effects processor
//	if (pFxChain_)
//		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOut );
ClearShorts(pOut, numSamples);

int framesRendered = 0;
if (pPlayer_)
    framesRendered = pPlayer_->RenderBuffer( pOut, numFrames );
if (numSamples > framesRendered*2)
    isDone_ = true;

//printf("Channel::RenderBuffer: levelsf <%f , %f > <%f, %f> dB \n", levelsf[kLeft], levelsf[kRight],
//        LinearToDecibelf(levelsf[kLeft]), LinearToDecibelf(levelsf[kRight]));
//printf("Channel::RenderBuffer: levelsi <%f , %f > \n", Q15ToFloat(levelsi[kLeft]), Q15ToFloat(levelsi[kRight]));

// ---- Render to out buffer
numSamples = framesRendered*2;
for (int i = 0; i < numSamples; i += 2)
	{
// Integer scaling for gain control
//	y = (S32)(levelsf[kLeft] * (float)pOut[i]);	// FLOAT
	y = (S32) MultQ15(levelsi[kLeft], pOut[i]);	// Q15  1.15 Fixed-point	
// Saturate to 16-bit range				
//	if      (y > kS16_Max) y = kS16_Max;
//	else if (y < kS16_Min) y = kS16_Min;				
	pOut[i] = (S16)y;

	y = (S32) MultQ15(levelsi[kRight], pOut[i+1]);				
// Saturate to 16-bit range				
//	if      (y > kS16_Max) y = kS16_Max;
//	else if (y < kS16_Min) y = kS16_Min;				
	pOut[i+1] = (S16)y;
	}

return (framesRendered);
}	// ---- end RenderBuffer ----

LF_END_BRIO_NAMESPACE()
// EOF	
