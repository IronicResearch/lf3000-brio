#ifndef LF_BRIO_CHANNEL_H
#define LF_BRIO_CHANNEL_H

//==============================================================================
// Copyright (c) 2002-2008 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Channel.h
//
// Defines class to manage processing of audio data on an audio channel.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

#include <RawPlayer.h> 
#include <VorbisPlayer.h> 

#include <DebugMPI.h>

#include <Dsputil.h>

LF_BEGIN_BRIO_NAMESPACE()

class CAudioPlayer;
//==============================================================================
// Class:
//		CChannel
//                  NOTE:  this original class is quickly becoming fluff
//
// Description:
//		Class to manage the processing of audio data on an audio channel. 
//==============================================================================
class CChannel {
public:
	CChannel(tAudioID id);
	~CChannel();

	tErrType	InitWithPlayer( CAudioPlayer* pPlayer );
	tErrType	Release( Boolean noPlayerDoneMsg );

// Set pause state
	inline void	Pause()  { /*if (fInUse_) */ fPaused_ = true;  }
	inline void	Resume() { /*if (fInUse_) */ fPaused_ = false; }

	inline void	SetInUse(Boolean  x) { fInUse_ = x; }
	inline void	SetIsDone(Boolean x) { isDone_ = x; }

	// If less than numFrames is available
	// the remainder of the channel's buffer will be zero padded.
	// The channel also assumes that the mix buffer is stereo 
	// so mono data is copied to both stereo channel on mix out.
	U32			Render( S16 *pOut, int numStereoFrames );

	inline U8	GetVolume()		{ return volume_; }
	void		SetVolume(U8 x);

	inline S8	GetPan()		{ return pan_; }
	void		SetPan(S8 x);

	inline S8	GetID()		{ return id_; }
	void		SetID(tAudioID id )  { id_ = id; }

	U32         GetTime_mSec( void );

    IEventListener* GetEventListener(void);
	void            SetEventListener(const IEventListener *p);

	inline U32	GetSamplingFrequency()		    { return samplingFrequency_; }
	void		SetSamplingFrequency(U32 x)     { samplingFrequency_ = x;}

	void		StartPlayer(tAudioStartAudioInfo* pInfo, char *sExt);
	void		StopPlayer ();
	void		SetPlayer(CAudioPlayer *pPlayer, long releaseExistingPlayer);
//	inline CAudioPlayer *GetPlayer() { return pPlayer_; }

	void		SetFileReadBuf(void *p)     { pFileReadBuf_ = p;}
	
	inline tAudioPriority	GetPriority()            				{ return priority_; }
	inline void				SetPriority( tAudioPriority priority ) 	{ priority_ = priority; }

//	inline float	GetEQ_Frequency()   { return eq_frequency_; }
//	inline float	GetEQ_Q()           { return eq_q_; }
//	inline float	GetEQ_GainDB()      { return eq_gainDB_; }

//	inline void		SetEQ_Parameters(float frequency, float q, float gainDB) { eq_frequency_ = frequency; eq_q_ = q; eq_gainDB_ = gainDB;}

// Return status
	Boolean		ShouldRender( void );
	inline Boolean			IsInUse()  { return fInUse_; }
	inline Boolean			IsPaused() { return fPaused_; }

    void SendDoneMsg( void );  // GK FIXXX: will move to this

private:
	CDebugMPI 		*pDebugMPI_;

    Boolean useRawPlayer_;
    Boolean isDone_;
	Boolean fInUse_;		

	tAudioID 	    id_;	

	U8			volume_;	
	S8			pan_   ;		 

//	float		eq_frequency_;
//	float		eq_q_;
//	float		eq_gainDB_;

#define kAudioMixerChannel_MaxOutChannels 2
    Q15         levelsi_[kAudioMixerChannel_MaxOutChannels];
	U32			samplingFrequency_;	

    void       *pFileReadBuf_; 

//	CAudioPlayer			*pPlayer_;	// pActivePlayer	 
	CRawPlayer				*pRawPlayer_;		 
	CVorbisPlayer			*pVorbisPlayer_;		 
	
	Boolean fPaused_;		
	Boolean fReleasing_;	// Channel is in process of being reset

	tAudioPriority		priority_;		
	tAudioPayload		payload_;		
	tAudioOptionsFlags	optionsFlags_;	
	const IEventListener *pListener_;	// Pointer to AudioEventHandler 
	
    void   RecalculateLevels();
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_CHANNEL_H

