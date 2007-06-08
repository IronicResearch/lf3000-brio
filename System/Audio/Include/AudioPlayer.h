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
//#include <RsrcTypes.h>
#include <AudioTypes.h>
#include <EventListener.h>
#include <AudioTypesPriv.h>

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
	CAudioPlayer( tAudioPlayAudioInfo* pData, tAudioID id  );
	virtual ~CAudioPlayer();
	
//	virtual void	Stop() = 0;
//	virtual void	Pause() = 0;
//	virtual void	Resume() = 0;
	
	// Reset internal data pointer back to start of audio data.
//	virtual void	Restart() = 0;

	// Process an audio tick for the class object
	virtual U32		RenderBuffer( S16 *pOutBuff, U32 numFrames ) = 0;

	// Return the requested status of the class object 
//	inline U8		IsPaused() { return bPaused_; }
	inline U8		IsComplete() { return bComplete_; }	
	inline U8		SendDoneMessage() { return bDoneMessage_; }
//	inline U8		IsStopping() { return bStopping_; }
	inline U8		HasAudioCodec() { return bHasCodec_; }

	// Get/Set the class member variables
	inline tAudioID			GetAudioID() { return id_; }
	inline tRsrcHndl		GetRsrcHandle() { return rsrc_; }
	inline tAudioPriority 	GetPriority() { return priority_; }
	inline void		 		SetPriority(tAudioPriority priority) { priority_ = priority; }
	inline tAudioPayload	GetPayload() { return payload_; }
	inline void		 		SetPayload(tAudioPayload value) { payload_ = value; }
	inline IEventListener*	GetEventListener() { return pListener_; }
	inline void		 		SetEventListener(IEventListener *pListener) { pListener_ = pListener; }
	inline U8				GetPan( void ) { return pan_; }
	inline U8				GetVolume( void ) { return volume_; }
//	inline CAudioPlayer*	GetParentPlayer() { return pParent_; }
//	inline void		 		SetParentPlayer(CAudioPlayer *pParent) { pParent_ = pParent; }
//	inline CChannel*		GetChannel() { return pChannel_; }
//	inline void		 		SetChannel(CChannel *pChannel) { pChannel_ = pChannel; }
	inline tAudioOptionsFlags	GetOptionsFlags() { return optionsFlags_; }
	inline void				SetOptionsFlags(tAudioOptionsFlags optionsFlags) 
								{ optionsFlags_ = optionsFlags;
								if (optionsFlags_ && kAudioDoneMsgBit) bDoneMessage_ = 1; }
	inline void		 		SetSendDoneMessage(Boolean doneMsg) { bDoneMessage_ = doneMsg; }
	inline U32				GetSampleRate( void ) { return dataSampleRate_; }
	
protected:
	U8			bPaused_:1;				// Player is paused
	U8			bComplete_:1;			// Reached the end of the audio// ???
	U8			bDoneMessage_:1;		// Caller requests done message 
	U8			bStopping_:1;			// Stop() has been called, but not stopped
	U8			bHasCodec_:1;			// Has audio codec
	U8			unused_:3;				// Unused

	tAudioID			id_;			// AudioID of the audio assigned to the player
	tRsrcHndl			rsrc_;			// Rsrc handle of the audio assigned to the player
	tAudioHeader*		pHeader_;		// Pointer to header of audio to play
	void*				pAudioData_;
	U32					audioDataSize_;	// bytes I think it should be...
	bool				hasStereoData_;
	U32					dataSampleRate_;	// sampling rate of the data associated with the player
	U8					volume_;		// Volume of the audio assigned to the player 
	S8					pan_;			// Pan of the audio assigned to the player 
	tAudioPriority		priority_;		// Priority of the audio assigned to the player
	tAudioPayload		payload_;		// User payload of the audio assigned to the player
	tAudioOptionsFlags	optionsFlags_;	// Options flags of the audio assigned to the player
	IEventListener 		*pListener_;	// Pointer to AudioEventHandler assigned to the player
//	CAudioPlayer		*pParent_;	// Pointer to the player's parent player
//	CChannel			*pChannel_;	// Pointer to the channel assigned to the player

};

#endif		// LF_BRIO_AUDIOPLAYER_H

// EOF	
