#ifndef LF_BRIO_AUDIOPLAYER_H
#define LF_BRIO_AUDIOPLAYER_H

//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// AudioPlayer.h
//
// Description:	 Defines base class for all Audio Players
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
	CAudioPlayer( tAudioStartAudioInfo* pInfo, tAudioID id	);
	virtual ~CAudioPlayer();
		
	virtual void	RewindFile() = 0;

	// Render is the heart of the player.  When called, it renders the next
	// numFrames to pOut.  Note that it is the caller's responsibility to only
	// check if the player is paused prior to calling Render.
	virtual U32		Render( S16 *pOut, U32 numFrames ) = 0;
	virtual U32		GetAudioTime_mSec( void ) = 0; // Time since start of play

	// Return status
	inline U8		IsPaused()			 { return bPaused_;		   }
	inline Boolean	IsDone()			 { return bIsDone_; }

	// Get/Set class member variables
	inline tAudioID			GetAudioID() { return id_; }
	inline tAudioID			GetID()		 { return id_; }

	// Loop state.  The player doesn't use this internally.  It's just
	// convenient to store this info with the player.
	inline U32				GetLoopCount() { return loopCounter_; }
	inline U32				IncLoopCount() { return loopCounter_++; }
	inline U32				GetNumLoops() { return numLoops_; }

	inline tAudioPriority	GetPriority()				  { return priority_; }
	inline void				SetPriority(tAudioPriority x) { priority_ = x; }

	inline tAudioPayload	GetPayload()				{ return payload_; }
	inline void				SetPayload(tAudioPayload x) { payload_ = x; }

	inline const IEventListener *GetEventListener()					   { return pListener_;}
	inline void				SetEventListener( const IEventListener *x) { pListener_ = x;}

	inline U32				GetSampleRate( void ) { return samplingFrequency_; }

	inline tAudioOptionsFlags  GetOptionsFlags() { return optionsFlags_; }
	inline void				   SetOptionsFlags(tAudioOptionsFlags x) { optionsFlags_ = x;}

	inline void					Pause(void) 		{ bPaused_ = true; }
	inline void					Resume(void) 		{ bPaused_ = false; }
	
protected:
	S16				*pReadBuf_;

	U8			bPaused_;				
	Boolean		bIsDone_;			 // Player has completed generating audio
	U8			bStopping_;			 // Stop() has been called, but not yet stopped

	S32				numLoops_;
	S32				loopCounter_;

	CDebugMPI*			pDebugMPI_;			
	tAudioID			id_;			
	long				waitForRender_;

	FILE*				fileH_;			// file struct of open file
	void*				pAudioData_;
	U32					audioDataBytes_;		
	
	long				channels_;
	U32					samplingFrequency_;

	tAudioPriority		priority_;		
	tAudioPayload		payload_;		
	tAudioOptionsFlags	optionsFlags_;	
	const IEventListener *pListener_;	// Pointer to AudioEventHandler 
	
	tAudioMsgData		msgData_;		// union of all audio message types
	CAudioEventMessage*	pEvtMsg_;		// audio event message to be posted 

};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_AUDIOPLAYER_H


