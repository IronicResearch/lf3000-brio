//==============================================================================
// Copyright (c) 2002-2008 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Channel.cpp
//
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
CChannel::CChannel(tAudioID id)
{
//{static int c=0; printf("CChannel::CChannel %d  START \n", c++);}
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose); //kAudioDebugLevel );

    id_            = id;
	pRawPlayer_    = new CRawPlayer(NULL, id);
	pVorbisPlayer_ = new CVorbisPlayer(NULL, id);
//    pPlayer_ = NULL;
    useRawPlayer_ = false;
    pFileReadBuf_  = NULL;

	fInUse_     = false;
	fPaused_    = false;
	fReleasing_ = false;
	isDone_     = false;

// Set DSP values
	SetPan(kAudio_Pan_Default);
	SetVolume(kAudio_Volume_Default);
	samplingFrequency_ = 0;
}	// ---- end CChannel ----

// ==============================================================================
// ~CChannel
// ==============================================================================
CChannel::~CChannel()
{
}	// ---- end ~CChannel ----

// ==============================================================================
// SetPan :     Channel stereo position   [Left .. Center .. Right]
// =============================================================================
    void 
CChannel::SetPan( S8 x )
{
//printf("CChannel::SetPan: x= %d Range [%d .. %d]\n", x, kAudio_Pan_Min, kAudio_Pan_Max);
pan_ = BoundS8(&x, kAudio_Pan_Min, kAudio_Pan_Max);

RecalculateLevels();
}	// ---- end SetPan ----

// ==============================================================================
// SetVolume : Convert range [0 .. 127] to [-100 .. +4.15] dB
// ==============================================================================
    void 
CChannel::SetVolume( U8 x )
{
//printf("CChannel::SetVolume :x= %d Range[%d .. %d]\n", x, kAudio_Volume_Min, kAudio_Volume_Max);
volume_ = BoundU8(&x, kAudio_Volume_Min, kAudio_Volume_Max);

RecalculateLevels();
}	// ---- end SetVolume ----

// ==============================================================================
// RecalculateLevels : Recalculate levels when either pan or volume changes
// ==============================================================================
void CChannel::RecalculateLevels()
{
float panValuesf[kAudioMixerChannel_MaxOutChannels];
float levelsf   [kAudioMixerChannel_MaxOutChannels]; // gain * panValue

// Convert input range to [-100 .. 100] range [0 .. 1] suitable for constant power calculation
// ChangeRangef(x, L1, H1, L2, H2)
float xf = ChangeRangef((float)pan_, (float) kAudio_Pan_Min, (float)kAudio_Pan_Max, 0.0f, 1.0f);
ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
//printf("CChannel::SetPan: %g -> <%f , %f>\n", xf, panValuesf[kLeft], panValuesf[kRight]);

// ChangeRangef(x, L1, H1, L2, H2)
//gainf = ChangeRangef((float)x, (float) kAudio_Volume_Min, (float)kAudio_Volume_Max, 0.0f, 1.0f);
float gainf = 0.01f*(float)volume_;
// Convert to square curve, which is milder than Decibels
gainf *= gainf;
//printf( "CChannel::SetVolume: PRE HEADROOM %d -> %g\n", volume_, gainf );	
gainf *= kChannel_Headroomf; // DecibelToLinearf(-Channel_HeadroomDB);

//float xf = 1.27f;
//printf("%f    -> %f dB\n", xf, LinearToDecibelf(xf));
//printf("%f    -> %f dB\n", xf*xf, LinearToDecibelf(xf*xf));
//1.27     = 2.08 dB
//(1.27)^2 = 4.15 dB
//printf("sqrt(2)/2 = %g\n", 0.5f*sqrt(2.0));

levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
levelsf[kRight] =  panValuesf[kRight]*gainf;

// Convert 32-bit floating-point to Q15 fractional integer format 
levelsi_[kLeft ] = FloatToQ15(levelsf[kLeft ]);
levelsi_[kRight] = FloatToQ15(levelsf[kRight]);

//printf("CChannel::RecalculateLevels : gainf=%g Pan <%g, %g>\n", 
//         gainf, panValuesf[kLeft], panValuesf[kRight]);
//printf("CChannel::RecalculateLevels : levelsf <%g, %g>\n", levelsf[kLeft], levelsf[kRight]);
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
// Release:
// ==============================================================================
    tErrType 
CChannel::Release( Boolean noPlayerDoneMsg )
{
//printf("CChannel::Release START $%p\n", (void *)pPlayer_);
	
fReleasing_ = true;
fPaused_    = true;

// Deactivate "done" message if specified
if (noPlayerDoneMsg)
    {
    if (useRawPlayer_)
        {
        pRawPlayer_->SetEventListener(NULL);
        pRawPlayer_->ActivateSendDoneMessage(false);
        }
    else
        {
        pVorbisPlayer_->SetEventListener(NULL);
        pVorbisPlayer_->ActivateSendDoneMessage(false);
        }
    }

// No longer in use
	fReleasing_ = false;
	fInUse_     = false;

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
tAudioID      oldId      = 0;
tAudioPayload oldPayload = 0;

//{static long c=0; printf("CChannel::SetPlayer: %ld START releaseExistingPlayer=%ld\n", c++, releaseExistingPlayer);}

// If we're pre-empting, release active player first.
//if (pPlayer_)
    {
//{static long c=0; printf("CChannel::SetPlayer: %ld BEFO Release releaseExistingPlayer=%ld\n", c++, releaseExistingPlayer);}
// Save variables before destroying player
//    pOldListener    = pPlayer_->GetEventListener();
//    bOldDoneMessage = pPlayer_->ShouldSendDoneMessage();

// GK FIXXX: should 
    if (releaseExistingPlayer)
    	Release( true );		// true = no done msg 
//{static long c=0; printf("CChannel::SetPlayer: %ld AFTA Release \n", c++);}
    }

//SetSamplingFrequency(pPlayer_->GetSampleRate());

// Send done message if it was setup 
if (pOldListener && bOldDoneMessage)
    {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgAudioCompleted	data;
//printf("CChannel::SetPlayer sending Done message from Audio ID =%ld\n", oldId);

	data.audioID = id_;	        
	data.payload = payload_;
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
if (pRawPlayer_ || pVorbisPlayer_)
	Release( true );		// true = no done msg if requested
//{static long c=0; printf("CChannel::InitWithPlayer: %ld AFTA Release \n", c++);}
	
// Convert interface parameters to DSP level data and reset channel
	SetSamplingFrequency(pPlayer->GetSampleRate());
//printf("CChannel::InitWithPlayer : pan=%d volume=%d fs=%ld complete=%d\n", pan_, volume_, samplingFrequency_, pPlayer_->IsComplete());

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
// StartPlayer : 
// ==============================================================================
    void 
CChannel::StartPlayer(tAudioStartAudioInfo* pInfo, char *sExt )
{
//{static long c=0; printf("CAudioMixer::StartChannel%d: START\n", c++);}

// "Release" existing player
fReleasing_ = true;
fPaused_    = true;

// GK FIXXX: should we send done message 
//if (noPlayerDoneMsg)
//    {
//    pPlayer_->SetEventListener(NULL);
//    pPlayer_->ActivateSendDoneMessage(false);
//    }
pRawPlayer_->CloseFile();
pVorbisPlayer_->CloseFile();
fReleasing_ = false;
fInUse_     = false;

// Select Player
if (!strcmp(sExt, "raw")  || !strcmp( sExt, "RAW")  ||
    !strcmp(sExt, "brio") || !strcmp( sExt, "BRIO") ||
    !strcmp(sExt, "aif")  || !strcmp( sExt, "AIF")  ||
    !strcmp(sExt, "aiff") || !strcmp( sExt, "AIFF") ||
    !strcmp(sExt, "wav")  || !strcmp( sExt, "WAV") )
	{
    pRawPlayer_->CloseFile();
//	printf("AudioTask::DoStartAudio: Create RawPlayer\n");
    pRawPlayer_->OpenFile((char *) pInfo->path->c_str());
    pRawPlayer_->SetAudioInfo(pInfo, id_);
    pRawPlayer_->SetFileReadBuf(pFileReadBuf_);
	SetSamplingFrequency(pRawPlayer_->GetSampleRate());

//    pPlayer_ = pRawPlayer_;
    useRawPlayer_ = true;
//printf("CChannel::StartPlayer: sizeof(CRawPlayer)=%d\n", sizeof(CRawPlayer));
    }
else if (!strcmp( sExt, "ogg" ) || !strcmp( sExt, "OGG") ||
         !strcmp( sExt, "aogg") || !strcmp( sExt, "AOGG"))
    {
    pVorbisPlayer_->CloseFile();
//	printf("AudioTask::DoStartAudio: Create VorbisPlayer\n");
    pVorbisPlayer_->OpenFile((char *) pInfo->path->c_str());
    pVorbisPlayer_->SetAudioInfo(pInfo, id_);
    pVorbisPlayer_->SetFileReadBuf(pFileReadBuf_);
	SetSamplingFrequency(pVorbisPlayer_->GetSampleRate());
//    pPlayer_ = pVorbisPlayer_;
    useRawPlayer_ = false;
//printf("CAudioMixer::CreatePlayer:: sizeof(CVorbisPlayer)=%d\n", sizeof(CVorbisPlayer));
    }
else
    {
	pDebugMPI_->DebugOut( kDbgLvlImportant,
		"CAudioMixer::CreatePlayer: invalid file extension ='%s' for '%s'\n", sExt, (char *) pInfo->path->c_str());
    return;  // GK FIXX: add return value later
    }

priority_     = pInfo->priority;
pListener_    = pInfo->pListener;
payload_      = pInfo->payload;
optionsFlags_ = pInfo->flags;

// Init with player
SetPan(    pInfo->pan );
SetVolume( pInfo->volume );

fPaused_    = false;
fReleasing_ = false;
isDone_     = false;
// Finally, allow this channel to be added to Mixer
fInUse_     = true;

}	// ---- end StartPlayer ----

// ==============================================================================
// StopPlayer : 
// ==============================================================================
    void 
CChannel::StopPlayer()
{
printf("CAudioMixer::StopPlayer() id=%d\n", (int)id_);	
fReleasing_ = true;
fPaused_    = true;

// Deactivate "done" message if specified
//if (noPlayerDoneMsg)
//    {
//    pPlayer_->SetEventListener(NULL);
//    pPlayer_->ActivateSendDoneMessage(false);
//    }

//pVorbisPlayer_->CloseFile();
//pRawPlayer_->CloseFile();

// No longer in use
	fReleasing_ = false;
	fInUse_     = false;
}	// ---- end StopPlayer ----

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

//{static long c=0; printf("CChannel::Render: START %ld id=%ld framesToRender=%d\n", c++, id_, framesToRender); }
ClearShorts(pOut, samplesToRender);

// Call player to render a stereo buffer
if (pRawPlayer_ || pVorbisPlayer_)
    {   
    if (useRawPlayer_)
        framesRendered  = pRawPlayer_->Render( pOut, framesToRender );
    else
        framesRendered  = pVorbisPlayer_->Render( pOut, framesToRender );

    samplesRendered = framesRendered*2;
    isDone_ = (framesToRender > framesRendered);
    }

// Scale stereo out buffer (Assumes all audio player output is two channel)
for (int i = 0; i < samplesRendered; i += 2)
	{
// Integer scaling : gain + stereo pan
	pOut[i  ] = (S16) MultQ15(levelsi_[kLeft ], pOut[i  ]);	// Q15  1.15 Fixed-point	
	pOut[i+1] = (S16) MultQ15(levelsi_[kRight], pOut[i+1]);				
	}

return (framesRendered);
}	// ---- end Render ----

// ==============================================================================
// SendDoneMsg:   Send message to Event listener when audio job completed, which
//                  includes the last iteration of a loop
// ==============================================================================
    void 
CChannel::SendDoneMsg( void )
{
if (!pListener_)
    return;
if (useRawPlayer_)
    {
    if (pRawPlayer_ && pRawPlayer_->ShouldSendDoneMessage())
        {
//printf("Channel::Render: BEFO SendDoneMsg() player ID=%ld\n", (long) pRawPlayer->GetID());
        pRawPlayer_->SendDoneMsg();
        }
    }
else
    {
    if (pVorbisPlayer_ && pVorbisPlayer_->ShouldSendDoneMessage())
        {
//printf("Channel::Render: BEFO SendDoneMsg() player ID=%ld\n", (long) pVorbisPlayer_->GetID());
        pVorbisPlayer_->SendDoneMsg();
        }
    }

//	const tEventPriority	kPriorityTBD = 0;
//	tAudioMsgAudioCompleted	data;
//printf("CChannel::SendDoneMsg id=%ld\n", id_);
//
//	data.audioID = id_;	        
//	data.payload = payload_;
//	data.count   = 1;
//
//	CEventMPI	event;
//	CAudioEventMessage	msg(data);
//	event.PostEvent(msg, kPriorityTBD, pListener_);
}   // ---- end SendDoneMsg() ----

// ==============================================================================
// GetTime_mSec : 
// ==============================================================================
    U32 
CChannel::GetTime_mSec()
{
if (useRawPlayer_)
    return (pRawPlayer_->GetTime_mSec());
else
    return (pVorbisPlayer_->GetTime_mSec());
}	// ---- end GetTime_mSec ----


LF_END_BRIO_NAMESPACE()
// EOF	
