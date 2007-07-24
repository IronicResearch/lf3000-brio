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
//		from the AudioMPI.  The format of the message data are in AudioTypesPriv.h 
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <KernelTypes.h>
#include <AudioTypes.h>
#include <AudioTypesPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

const U8 kAudioMsgDefaultPriority = 0;

enum {
	kAudioCmdMsgTypeNOP=0,

	kAudioCmdMsgTypeRegisterGetStereoAudioStreamFcn,
	kAudioCmdMsgTypeRegisterAudioEffectsProcessor,
	kAudioCmdMsgTypeRegisterGlobalAudioEffectsProcessor,
	kAudioCmdMsgTypeChangeAudioEffectsProcessor,

	kAudioCmdMsgTypeSetMasterVolume,

	kAudioCmdMsgTypeStartAllAudio,
	kAudioCmdMsgTypePauseAllAudio,
	kAudioCmdMsgTypeResumeAllAudio,
	kAudioCmdMsgTypeStopAllAudio,

	kAudioCmdMsgTypeStartAudio,
	kAudioCmdMsgTypePauseAudio,
	kAudioCmdMsgTypeResumeAudio,
	kAudioCmdMsgTypeStopAudio,

	kAudioCmdMsgTypeGetAudioTime,
	
	kAudioCmdMsgTypeIsAudioPlaying,
	kAudioCmdMsgTypeIsAnyAudioPlaying,

	kAudioCmdMsgTypeSetAudioVolume,
	kAudioCmdMsgTypeSetAudioPriority,
	kAudioCmdMsgTypeSetAudioPan,
	kAudioCmdMsgTypeSetAudioEventHandler,

	kAudioCmdMsgTypeAcquireMidiPlayer,
	kAudioCmdMsgTypeReleaseMidiPlayer,

	kAudioCmdMsgTypeStartMidiFile,
	kAudioCmdMsgTypePauseMidiFile,
	kAudioCmdMsgTypeResumeMidiFile,
	kAudioCmdMsgTypeStopMidiFile,
	
	kAudioCmdMsgTypeIsMidiFilePlaying,
	kAudioCmdMsgTypeIsAnyMidiFilePlaying,
	
	kAudioCmdMsgTypeGetEnabledMidiTracks,
	kAudioCmdMsgTypeEnableMidiTracks,
	kAudioCmdMsgTypeTransposeMidiTracks,
	kAudioCmdMsgTypeChangeMidiInstrument,
	kAudioCmdMsgTypeChangeMidiTempo,
	
	kAudioCmdMsgTypeSendMidiCommand,
	kAudioCmdMsgTypeMidiNoteOn,
	kAudioCmdMsgTypeMidiNoteOff
};
typedef U8 tAudioCmdMsgType;

//************************************
//************************************

// All audio commands are derived from this...
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
	CAudioMsgSetMasterVolume( const U8 masterVolume );
	U8	GetData( void ) { return masterVolume_; }
	
private:
	U8	masterVolume_;
};

// kAudioCmdMsgTypeStartAudio
class CAudioMsgStartAudio : public CAudioCmdMsg {
public:    
	// Start the audio system running.
	CAudioMsgStartAudio( void );

	// Start playing an audio resource.
	CAudioMsgStartAudio( const tAudioStartAudioInfo& data );
	
	tAudioStartAudioInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioStartAudioInfo		data_;
};

// kAudioCmdMsgTypeGetAudioTime

class CAudioMsgGetAudioTime : public CAudioCmdMsg {
public:    
	// Pause ALL audio.
	CAudioMsgGetAudioTime( void );

	// Pause only audio indicated by ID.
	CAudioMsgGetAudioTime( const tAudioID id );

	tAudioID	GetData( void ) { return id_; }
	
private:
	tAudioID 	id_;
};

// kAudioCmdMsgTypePauseAudio

class CAudioMsgPauseAudio : public CAudioCmdMsg {
public:    
	// Pause ALL audio.
	CAudioMsgPauseAudio( void );

	// Pause only audio indicated by ID.
	CAudioMsgPauseAudio( const tAudioID id );

	tAudioID	GetData( void ) { return id_; }
	
private:
	tAudioID 	id_;
};

// kAudioCmdMsgTypeResumeAudio

class CAudioMsgResumeAudio : public CAudioCmdMsg {
public:    
	// Resume ALL audio.
	CAudioMsgResumeAudio( void );			
	
	// Resume only audio indicated by ID.
	CAudioMsgResumeAudio( const tAudioID id );
	
	tAudioID	GetData( void ) { return id_; }

private:
	tAudioID	 id_;
};

// kAudioCmdMsgTypeStopAudio

class CAudioMsgStopAudio : public CAudioCmdMsg {
public:    
	// Pause ALL audio.
	CAudioMsgStopAudio( void );
	
	// Pause only audio indicated by ID.
	CAudioMsgStopAudio( const tAudioStopAudioInfo& data );

	tAudioStopAudioInfo*	GetData( void ) { return &data_; }

private:
	tAudioStopAudioInfo			data_;
};

// kAudioCmdMsgTypeIsAudioPlaying

class CAudioMsgIsAudioPlaying : public CAudioCmdMsg {
public:    
	// Is any audio playing?
	CAudioMsgIsAudioPlaying( void );
	
	// Is the audio represted by this id playing?
	CAudioMsgIsAudioPlaying( const tAudioID id );

	tAudioID	GetData( void ) { return id_; }

private:
	tAudioID	id_;
};

// ---------------------------------------------------
// MIDI messages
class CAudioMsgAcquireMidiPlayer : public CAudioCmdMsg {
public:    
	// Start the audio system running.
	CAudioMsgAcquireMidiPlayer( void );
	
private:
};

class CAudioMsgReleaseMidiPlayer : public CAudioCmdMsg {
public:    
	// Start the audio system running.
	CAudioMsgReleaseMidiPlayer( void );
	
private:
};

// kAudioCmdMsgTypeStartMidiFile
class CAudioMsgStartMidiFile : public CAudioCmdMsg {
public:    
	CAudioMsgStartMidiFile( const tAudioStartMidiFileInfo& data );
	tAudioStartMidiFileInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioStartMidiFileInfo		data_;
};

// kAudioCmdMsgTypePauseMidiFile
class CAudioMsgPauseMidiFile : public CAudioCmdMsg {
public:    
	CAudioMsgPauseMidiFile( const tMidiPlayerID id );
	tMidiPlayerID	GetData( void ) { return id_; }
	
private:
	tMidiPlayerID		id_;
};

// kAudioCmdMsgTypeResumeMidiFile
class CAudioMsgResumeMidiFile : public CAudioCmdMsg {
public:    
	CAudioMsgResumeMidiFile( const tMidiPlayerID id );
	tMidiPlayerID	GetData( void ) { return id_; }
	
private:
	tMidiPlayerID		id_;
};

// kAudioCmdMsgTypeStopMidiFile
class CAudioMsgStopMidiFile : public CAudioCmdMsg {
public:    
	CAudioMsgStopMidiFile( const tAudioStopMidiFileInfo& data );
	tAudioStopMidiFileInfo*	GetData( void ) { return &data_; }
	
private:
	tAudioStopMidiFileInfo		data_;
};

// kAudioCmdMsgTypeIsMidiFilePlaying

class CAudioMsgIsMidiFilePlaying : public CAudioCmdMsg {
public:    
	// Is any MIDi file playing?
	CAudioMsgIsMidiFilePlaying( void );
	
	// Is the MIDI file represted by this id playing?
	CAudioMsgIsMidiFilePlaying( const tMidiPlayerID id );

	tMidiPlayerID	GetData( void ) { return id_; }

private:
	tMidiPlayerID	id_;
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


//-----------------------------------------------------------------------
// return message
class CAudioReturnMessage : public CMessage 
{
public:    	
	CAudioReturnMessage( void );

	void SetAudioErr( tErrType err ) { err_ = err; };
	void SetAudioID( tAudioID id ) { audioID_ = id; };
	void SetMidiID( tMidiPlayerID id ) { midiPlayerID_ = id; };
	Boolean SetBooleanResult( Boolean val ) { booleanValue_ = val; }
	U32 SetU32Result( U32 val ) { u32Value_ = val; }
	
	tMidiPlayerID GetMidiID( void ) { return midiPlayerID_; }
	tAudioID GetAudioID( void ) { return audioID_; }
	tErrType GetAudioErr( void ) { return err_; }
	Boolean GetBooleanResult( void ) { return booleanValue_; }
	U32 GetU32Result( void ) { return u32Value_; }
	
private:
	tErrType		err_;
	tAudioID		audioID_;
	tMidiPlayerID	midiPlayerID_;
	Boolean			booleanValue_;
	U32				u32Value_;
};

const U32	kMAX_AUDIO_MSG_SIZE	=	(sizeof(CAudioMsgStartMidiFile) * 2);


LF_END_BRIO_NAMESPACE()	
#endif	// LF_BRIO_AUDIOMSG_H

// EOF	
