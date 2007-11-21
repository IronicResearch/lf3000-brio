#ifndef LF_BRIO_AUDIOPLAYER_H
#define LF_BRIO_AUDIOPLAYER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioPlayer.h
//
// Description:
//		Defines the base class for all Audio Players.
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
	CAudioPlayer( tAudioStartAudioInfo* pAudioInfo, tAudioID id  );
	virtual ~CAudioPlayer();
		
	virtual void	Rewind() = 0;
	virtual U32		RenderBuffer( S16 *pOutBuff, U32 numFrames ) = 0;
	virtual U32 GetAudioTime_mSec( void ) = 0; // Time since start of audio play

	// Return status
	inline U8 ShouldSendDoneMessage() { return bDoneMessage_; }
	inline U8		HasAudioCodec()   { return bHasAudioCodec_; }
	inline U8		IsComplete()      { return bComplete_; }
	inline U8		IsPaused()        { return bPaused_; }

	// Get/Set class member variables
	inline tAudioID			GetAudioID() { return id_; }
	inline tAudioID			GetID()      { return id_; }
	inline void		 		ActivateSendDoneMessage(Boolean x) { bDoneMessage_ = x; }
	
	void Set_WaitForRender( long x ) {waitForRender_ = x;}
	long Get_WaitForRender( void   ) {return (waitForRender_);}

	inline tAudioPriority 	GetPriority() { return priority_; }
	inline void		 		SetPriority(tAudioPriority x) { priority_ = x; }

	inline tAudioPayload	GetPayload() { return payload_; }
	inline void		 		SetPayload(tAudioPayload x) { payload_ = x; }

	inline const IEventListener*	GetEventListener() { return pListener_; }
	inline void		 		SetEventListener( const IEventListener *x) { pListener_ = x; }

	inline U8				GetPan(        void ) { return pan_; }
	inline U8				GetVolume(     void ) { return volume_; }
	inline U32				GetSampleRate( void ) { return samplingFrequency_; }


	inline tAudioOptionsFlags	GetOptionsFlags() { return optionsFlags_; }
	inline void				SetOptionsFlags(tAudioOptionsFlags x) 
								{ optionsFlags_ = x;
								  bDoneMessage_ = ((x & kAudioDoneMsgBit) != 0) ? true : false; }
protected:
	U8			bPaused_;				
	U8			bComplete_;			// Player has completed generating audio
	U8			bDoneMessage_;		// Caller requests done message 
	U8			bStopping_;			// Stop() has been called, but not stopped
	U8			bHasAudioCodec_;	
//	U8			unused_:3;				

	CDebugMPI*			pDebugMPI_;		// Debug output access.	
	tAudioID			id_;			
    long            waitForRender_;

	FILE*				fileH_;			// file struct of open file
	void*				pAudioData_;
	U32					audioDataBytes_;		
	
    long                channels_;
	U32					samplingFrequency_;

	Boolean				shouldLoopFile_;	

	S8					pan_;			 
	U8					volume_;		 

	tAudioPriority		priority_;		
	tAudioPayload		payload_;		
	tAudioOptionsFlags	optionsFlags_;	
	const IEventListener *pListener_;	// Pointer to AudioEventHandler assigned to player
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_AUDIOPLAYER_H

// EOF	
