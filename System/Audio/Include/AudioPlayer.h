#ifndef LF_BRIO_AUDIOPLAYER_H
#define LF_BRIO_AUDIOPLAYER_H

//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// AudioPlayer.h
//
// Description:  Defines base class for all Audio Players
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <DebugMPI.h>

#include <AudioTypes.h>
#include <EventListener.h>
#include <AudioTypesPriv.h>
LF_BEGIN_BRIO_NAMESPACE()


class CChannel;
//==============================================================================
// Class:
//		CAudioPlayer
//
// Description:
//		The base class for all Audio Players. 
//==============================================================================
class CAudioPlayer {
public:
	CAudioPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  );
	virtual ~CAudioPlayer();
		
	virtual void	RewindFile() = 0;
	virtual U32		Render( S16 *pOut, U32 numFrames ) = 0;
	virtual U32     GetAudioTime_mSec( void ) = 0; // Time since start of play

	virtual void    SendDoneMsg   ( void );
	virtual void    SendLoopEndMsg( void );

// Return status
	inline U8 ShouldSendDoneMessage()    { return bSendDoneMessage_;   }
	inline U8 ShouldSendLoopEndMessage() { return bSendLoopEndMessage_;}
//	inline U8		HasAudioCodec()      { return bHasAudioCodec_; }
	inline U8		IsComplete()         { return bComplete_;      }
	inline U8		IsPaused()           { return bPaused_;        }

// Get/Set class member variables
	inline tAudioID			GetAudioID() { return id_; }   // GK FIXX:  used for AudioIDFromMIDIID()? 
	inline tAudioID			GetID()      { return id_; }
	inline void		 		ActivateSendDoneMessage   (Boolean x) { bSendDoneMessage_    = x; }
	inline void		 		ActivateSendLoopEndMessage(Boolean x) { bSendLoopEndMessage_ = x; }
	
//	void Set_WaitForRender( long x ) {waitForRender_ = x;}
//	long Get_WaitForRender( void   ) {return (waitForRender_);}

	inline tAudioPriority 	GetPriority()                 { return priority_; }
	inline void		 		SetPriority(tAudioPriority x) { priority_ = x; }

	inline tAudioPayload	GetPayload()                { return payload_; }
	inline void		 		SetPayload(tAudioPayload x) { payload_ = x; }

	inline const IEventListener *GetEventListener()                    { return pListener_;}
	inline void		 		SetEventListener( const IEventListener *x) { pListener_ = x;}

	inline U32				GetSampleRate( void ) { return samplingFrequency_; }

	inline tAudioOptionsFlags  GetOptionsFlags() { return optionsFlags_; }
	inline void				   SetOptionsFlags(tAudioOptionsFlags x) 
								{ optionsFlags_        = x;
								  bSendDoneMessage_    = ((x & kAudioDoneMsgBitMask) != 0) ? true : false; 
								  bSendLoopEndMessage_ = ((x & kAudioLoopEndBitMask) != 0) ? true : false; 
                                }
protected:
//#define USE_AUDIO_PLAYER_MUTEX
#ifdef USE_AUDIO_PLAYER_MUTEX
	CKernelMPI* 	pKernelMPI_;		
	tMutex			renderMutex_;		// To make Render() thread-safe
#endif

    S16             *pReadBuf_;

	U8			bPaused_;				
	U8			bComplete_;			 // Player has completed generating audio
	U8			bStopping_;			 // Stop() has been called, but not yet stopped
//	U8			bHasAudioCodec_;	
	U8			bSendDoneMessage_;	 // Caller requests done message 
	U8			bSendLoopEndMessage_;// Send each time the end of the loop has been reached

    S32             loopCount_;
    S32             loopCounter_;
	Boolean			shouldLoop_;	

	CDebugMPI*			pDebugMPI_;			
	tAudioID			id_;			
    long                waitForRender_;

	FILE*				fileH_;			// file struct of open file
	void*				pAudioData_;
	U32					audioDataBytes_;		
	
    long                channels_;
	U32					samplingFrequency_;

	tAudioPriority		priority_;		
	tAudioPayload		payload_;		
	tAudioOptionsFlags	optionsFlags_;	
	const IEventListener *pListener_;	// Pointer to AudioEventHandler 

#if	PROFILE_DECODE_LOOP
	S32				totalUsecs_;
	S32				totalBytes_;
	U32				minUsecs_;
	U32				maxUsecs_;
#endif
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_AUDIOPLAYER_H


