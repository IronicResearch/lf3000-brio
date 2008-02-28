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
	// numFrames to pOut.  The Render function returns the number of frames
	// rendered.  If the number of frames rendered is less than numFrames, this
	// implies that the player is done.  In this case, the player MUST set its
	// bIsDone_ flag to true before returning from Render.  The player should
	// not pad the output.  Note that it is the caller's responsibility to only
	// check if the player is paused prior to calling Render.
	virtual U32		Render( S16 *pOut, U32 numFrames ) = 0;
	virtual U32		GetAudioTime_mSec( void ) = 0; // Time since start of play

	// These functions are used to implement per-player policy.  They are
	// implemented as static class functions by convention.  A c++ programmer
	// wiser than I could probably do this in a much classier fashion, so to
	// speak.  Any class of players must maintain a static count of the number
	// of players, and a constant that is the max number of players allowed.  It
	// is up to the developer who creates the player to set this maximum.
	//static U32		GetNumPlayers(void);
	//static U32		GetMaxPlayers(void);


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


