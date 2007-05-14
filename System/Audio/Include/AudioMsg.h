#ifndef LF_BRIO_AUDIOMSG_H
#define LF_BRIO_AUDIOMSG_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioMsg.h
//
// Description:
//		Defines the message format of messages accepted by the AudioMgr task
//		from the AudioMPI 
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <KernelTypes.h>
#include <AudioTypes.h>

const U8 kAudioMsgDefaultPriority = 0;

enum {
	kAudioCmdMsgTypeNOP=0,

	kAudioCmdMsgTypeRegisterGetStereoAudioStreamFcn,
	kAudioCmdMsgTypeRegisterAudioEffectsProcessor,
	kAudioCmdMsgTypeRegisterGlobalAudioEffectsProcessor,
	kAudioCmdMsgTypeChangeAudioEffectsProcessor,

	kAudioCmdMsgTypeSetMasterVolume,

	kAudioCmdMsgTypeStartAllAudio,
	kAudioCmdMsgTypeStopAllAudio,
	kAudioCmdMsgTypePauseAllAudio,
	kAudioCmdMsgTypeResumeAllAudio,

	kAudioCmdMsgTypePlayAudio,
	kAudioCmdMsgTypePlayAudioArray,
	kAudioCmdMsgTypeAppendAudioArray,

	kAudioCmdMsgTypeStartAudio,
	kAudioCmdMsgTypeStopAudio,
	kAudioCmdMsgTypePauseAudio,
	kAudioCmdMsgTypeResumeAudio,

	kAudioCmdMsgTypeSetAudioVolume,
	kAudioCmdMsgTypeSetAudioPriority,
	kAudioCmdMsgTypeSetAudioPan,
	kAudioCmdMsgTypeSetAudioEventHandler,

	kAudioCmdMsgTypeAcquireMidiPlayer,
	kAudioCmdMsgTypeReleaseMidiPlayer,
	kAudioCmdMsgTypeGetMidiIDForAudioID,
	kAudioCmdMsgTypeGetAudioIDForMidiID,
	kAudioCmdMsgTypeStopMidiPlayer,
	kAudioCmdMsgTypePauseMidiPlayer,
	kAudioCmdMsgTypeResumeMidiPlayer,

	kAudioCmdMsgTypeGetEnabledMidiTracks,
	kAudioCmdMsgTypeEnableMidiTracks,
	kAudioCmdMsgTypeTransposeMidiTracks,
	kAudioCmdMsgTypeChangeMidiInstrument,
	kAudioCmdMsgTypeChangeMidiTempo,
	kAudioCmdMsgTypeSendMidiCommand,
	kAudioCmdMsgTypePlayMidiNote
};
typedef U8 tAudioCmdMsgType;

//************************************
//************************************

class CAudioMsg: public CMessage {
public:    
	tAudioCmdMsgType			mType_;

	tAudioCmdMsgType	GetMessageCmdType( void ) const { return mType_; }
	void				SetMessageCmdType( tAudioCmdMsgType type ) { mType_ = type; }

private:
};

// kAudioCmdMsgTypeSetMasterVolume
struct tAudioMasterVolume {
	U8				 	masterVolume;
};

class CAudioMsgSetMasterVolume : public CAudioMsg {
public:    
	CAudioMsgSetMasterVolume( const tAudioMasterVolume& data );
	tAudioMasterVolume	GetMessageData( void ) const { return mData_; }
	
private:
	tAudioMasterVolume			mData_;
};


// kAudioCmdMsgTypePlayAudio
struct tAudioPlayAudioData {
	tRsrcHndl			hRsrc;
	U8				 	volume;
	tAudioPriority	 	priority;
	S8				 	pan;
	IEventListener		*pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
};

class CAudioMsgPlayAudio : public CAudioMsg {
public:    
	CAudioMsgPlayAudio( const tAudioPlayAudioData& data );
	tAudioPlayAudioData	GetMessageData( void ) const { return mData_; }
	
private:

	tAudioPlayAudioData			mData_;

};

class CAudioMsgStartAudio : public CAudioMsg {
public:    
	// Start the audio system running.
	CAudioMsgStartAudio( void );
	tAudioCmdMsgType	GetMessageCmdType( void ) const { return mType_; }
	
private:
};

// kAudioCmdMsgTypeStopAudio
struct tAudioStopAudioData {
	tAudioID			audioID;
	U8				 	suppressDoneMsg;
};

class CAudioMsgStopAudio : public CAudioMsg {
public:    
	// Pause ALL audio.
	CAudioMsgStopAudio( void );
	
	// Pause only audio indicated by ID.
	CAudioMsgStopAudio( const tAudioStopAudioData& data );

	tAudioStopAudioData	GetMessageData( void ) const { return mData_; }

private:
	tAudioStopAudioData			mData_;

};

// kAudioCmdMsgTypePauseAudio
struct tAudioPauseAudioData {
	tAudioID			audioID;
};

class CAudioMsgPauseAudio : public CAudioMsg {
public:    
	// Pause ALL audio.
	CAudioMsgPauseAudio( void );

	// Pause only audio indicated by ID.
	CAudioMsgPauseAudio( const tAudioPauseAudioData& data );

	tAudioPauseAudioData	GetMessageData( void ) const { return mData_; }
	
private:
	tAudioPauseAudioData		mData_;

};

// kAudioCmdMsgTypeResumeAudio
struct tAudioResumeAudioData {
	tAudioID			audioID;
};

class CAudioMsgResumeAudio : public CAudioMsg {
public:    
	// Resume ALL audio.
	CAudioMsgResumeAudio( void );			
	
	// Resume only audio indicated by ID.
	CAudioMsgResumeAudio( const tAudioResumeAudioData& data );
	
	tAudioResumeAudioData	GetMessageData( void ) const { return mData_; }

private:
	tAudioResumeAudioData		mData_;
};


class CAudioReturnMessage : public CAudioMsg 
{
public:    	
	CAudioReturnMessage( tErrType err, tAudioID audioID );
	
	tAudioID GetMessageAudioID( void ) { return mAudioID_; }
	tAudioID GetMessageAudioErr( void ) { return mErr_; }
	
private:
	tErrType			mErr_;
	tAudioID			mAudioID_;
};

const U32	kMAX_AUDIO_MSG_SIZE	=	sizeof(CAudioMsgPlayAudio);


#endif	// LF_BRIO_AUDIOMSG_H

// EOF	
