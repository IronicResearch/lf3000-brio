#ifndef LF_BRIO_AUDIOMSG_H
#define LF_BRIO_AUDIOMSG_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
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
#include <AudioTypesPriv.h>

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
	kAudioCmdMsgTypeMidiNoteOn,
	kAudioCmdMsgTypeMidiNoteOff,
	kAudioCmdMsgTypePlayMidiFile
};
typedef U8 tAudioCmdMsgType;

//************************************
//************************************

class CAudioCmdMsg: public CMessage {
public:    
	tAudioCmdMsgType	GetCmdType( void ) const { return type_; }
	void				SetCmdType( tAudioCmdMsgType type ) { type_ = type; }

protected:
	tAudioCmdMsgType			type_;
};

// kAudioCmdMsgTypeSetMasterVolume

class CAudioMsgSetMasterVolume : public CAudioCmdMsg {
public:    
	CAudioMsgSetMasterVolume( const tAudioMasterVolume& data );
	tAudioMasterVolume*		GetData( void ) { return &data_; }
	
private:
	tAudioMasterVolume			data_;
};


// kAudioCmdMsgTypePlayAudio
class CAudioMsgPlayAudio : public CAudioCmdMsg {
public:    
	CAudioMsgPlayAudio( const tAudioPlayAudioInfo& data );
	tAudioPlayAudioInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioPlayAudioInfo			data_;
};

class CAudioMsgStartAudio : public CAudioCmdMsg {
public:    
	// Start the audio system running.
	CAudioMsgStartAudio( void );
	tAudioCmdMsgType	GetMessageCmdType( void ) const { return type_; }
	
private:
};

// kAudioCmdMsgTypeStopAudio

class CAudioMsgStopAudio : public CAudioCmdMsg {
public:    
	// Pause ALL audio.
	CAudioMsgStopAudio( void );
	
	// Pause only audio indicated by ID.
	CAudioMsgStopAudio( const tAudioStopAudioInfo& data );

	tAudioStopAudioInfo	GetData( void ) const { return data_; }

private:
	tAudioStopAudioInfo			data_;
};

// kAudioCmdMsgTypePauseAudio

class CAudioMsgPauseAudio : public CAudioCmdMsg {
public:    
	// Pause ALL audio.
	CAudioMsgPauseAudio( void );

	// Pause only audio indicated by ID.
	CAudioMsgPauseAudio( const tAudioPauseAudioInfo& data );

	tAudioPauseAudioInfo	GetData( void ) const { return data_; }
	
private:
	tAudioPauseAudioInfo		data_;

};

// kAudioCmdMsgTypeResumeAudio

class CAudioMsgResumeAudio : public CAudioCmdMsg {
public:    
	// Resume ALL audio.
	CAudioMsgResumeAudio( void );			
	
	// Resume only audio indicated by ID.
	CAudioMsgResumeAudio( const tAudioResumeAudioInfo& data );
	
	tAudioResumeAudioInfo	GetData( void ) const { return data_; }

private:
	tAudioResumeAudioInfo		data_;
};

// ---------------------------------------------------
// MIDI messages
class CAudioMsgAcquireMidiPlayer : public CAudioCmdMsg {
public:    
	// Start the audio system running.
	CAudioMsgAcquireMidiPlayer( void );
	tAudioCmdMsgType	GetMessageCmdType( void ) const { return type_; }
	
private:
};

// kAudioCmdMsgTypeMidiNoteOn
class CAudioMsgMidiNoteOn : public CAudioCmdMsg {
public:    
	CAudioMsgMidiNoteOn( const tAudioMidiNoteInfo& data );
	tAudioMidiNoteInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioMidiNoteInfo			data_;
};

// kAudioCmdMsgTypeMidiNoteOff
class CAudioMsgMidiNoteOff : public CAudioCmdMsg {
public:    
	CAudioMsgMidiNoteOff( const tAudioMidiNoteInfo& data );
	tAudioMidiNoteInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioMidiNoteInfo			data_;
};

// kAudioCmdMsgTypePlayMidiFile
class CAudioMsgPlayMidiFile : public CAudioCmdMsg {
public:    
	CAudioMsgPlayMidiFile( const tAudioMidiFileInfo& data );
	tAudioMidiFileInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioMidiFileInfo			data_;
};

//-----------------------------------------------------------------------
// return messages
class CAudioReturnMessage : public CMessage 
{
public:    	
	CAudioReturnMessage( void );

	void SetAudioErr( tErrType err ) { err_ = err; };
	void SetAudioID( tAudioID audioID ) { audioID_ = audioID; };
	void SetMidiID( tMidiID midiID ) { midiID_ = midiID; };
	
	tMidiID GetMidiID( void ) { return midiID_; }
	tAudioID GetAudioID( void ) { return audioID_; }
	tErrType GetAudioErr( void ) { return err_; }
	
private:
	tErrType		err_;
	tAudioID		audioID_;
	tMidiID			midiID_;
};

const U32	kMAX_AUDIO_MSG_SIZE	=	(sizeof(CAudioMsgPlayMidiFile) + 4);


#endif	// LF_BRIO_AUDIOMSG_H

// EOF	
