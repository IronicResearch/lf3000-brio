#ifndef LF_BRIO_AUDIOMSG_H
#define LF_BRIO_AUDIOMSG_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
// AudioMsg.h
//
//		Defines the message format of messages accepted by the Audio task
//		from the AudioMPI.  The format of the message data are in AudioTypesPriv.h 
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
	kAudioCmdMsgTypeSetOutputEqualizer,

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
	kAudioCmdMsgTypeGetAudioVolume,
	kAudioCmdMsgTypeSetAudioPriority,
	kAudioCmdMsgTypeGetAudioPriority,
	kAudioCmdMsgTypeSetAudioPan,
	kAudioCmdMsgTypeGetAudioPan,
	kAudioCmdMsgTypeSetAudioListener,
	kAudioCmdMsgTypeGetAudioListener,
	
	kAudioCmdMsgTypeAcquireMidiPlayer,
	kAudioCmdMsgTypeReleaseMidiPlayer,

	kAudioCmdMsgTypeStartMidiFile,
	kAudioCmdMsgTypePauseMidiFile,
	kAudioCmdMsgTypeResumeMidiFile,
	kAudioCmdMsgTypeStopMidiFile,
	
	kAudioCmdMsgTypeIsMidiFilePlaying,
	kAudioCmdMsgTypeIsAnyMidiFilePlaying,
	
	kAudioCmdMsgTypeGetEnabledMidiTracks,
	kAudioCmdMsgTypeSetEnableMidiTracks,
	kAudioCmdMsgTypeTransposeMidiTracks,
	kAudioCmdMsgTypeChangeMidiInstrument,
	kAudioCmdMsgTypeChangeMidiTempo,
	
	kAudioCmdMsgTypeSendMidiCommand,
	kAudioCmdMsgTypeMidiNoteOn,
	kAudioCmdMsgTypeMidiNoteOff,
	kAudioCmdMsgExitThread,

	kAudioCmdMsgTypeSetAudioState,
	kAudioCmdMsgTypeGetAudioState
};
typedef U8 tAudioCmdMsgType;

//************************************
//************************************
// All audio commands are derived from this...
class CAudioCmdMsg: public CMessage {
public:    
	tAudioCmdMsgType	GetCmdType( void ) const { return type_; }

protected:
	tAudioCmdMsgType			type_;
};


// kAudioCmdMsgTypeSetMasterVolume
class CAudioMsgSetMasterVolume : public CAudioCmdMsg {
public:    
	CAudioMsgSetMasterVolume( const U8 x );
	U8	GetData( void ) { return d_; }
	
private:
	U8	d_;
};

// kAudioCmdMsgTypeSetOutputEqualizer
class CAudioMsgSetOutputEqualizer : public CAudioCmdMsg {
public:    
	CAudioMsgSetOutputEqualizer( const U8 x );
	U8	GetData( void ) { return d_; }
	
private:
	U8	d_;
};

// kAudioCmdMsgTypeSetAudioState
// kAudioCmdMsgTypeGetAudioState
class CAudioMsgSetAudioState : public CAudioCmdMsg {
public:    
	CAudioMsgSetAudioState( const tAudioState d );
	tAudioState	GetData( void ) { return d_; }
	
	tAudioState	d_;
private:
//	tAudioState	d_;
};

class CAudioMsgGetAudioState : public CAudioCmdMsg {
public:    
	CAudioMsgGetAudioState( void );
	tAudioState	GetData( void ) { return d_; }
	
	tAudioState	d_;
private:
//	tAudioState	d_;
};

// kAudioCmdMsgTypeSetAudioVolume
// kAudioCmdMsgTypeGetAudioVolume
class CAudioMsgSetAudioVolume : public CAudioCmdMsg {
public:    
	CAudioMsgSetAudioVolume( const tAudioVolumeInfo vi );
	tAudioVolumeInfo	GetData( void ) { return vi_; }
	
private:
	tAudioVolumeInfo	vi_;
};

class CAudioMsgGetAudioVolume : public CAudioCmdMsg {
public:    
	CAudioMsgGetAudioVolume( tAudioID id );
	tAudioVolumeInfo	GetData( void ) { return vi_; }
	
private:
	tAudioVolumeInfo	vi_;
};

//kAudioCmdMsgTypeSetAudioPriority,
//kAudioCmdMsgTypeGetAudioPriority,
class CAudioMsgSetAudioPriority : public CAudioCmdMsg {
public:    
	CAudioMsgSetAudioPriority( const tAudioPriorityInfo pi );
	tAudioPriorityInfo	GetData( void ) { return pi_; }
	
private:
	tAudioPriorityInfo	pi_;
};

class CAudioMsgGetAudioPriority : public CAudioCmdMsg {
public:    
	CAudioMsgGetAudioPriority( tAudioID id );
	tAudioPriorityInfo	GetData( void ) { return pi_; }
	
private:
	tAudioPriorityInfo	pi_;
};

//kAudioCmdMsgTypeSetAudioPan,
//kAudioCmdMsgTypeGetAudioPan,
class CAudioMsgSetAudioPan : public CAudioCmdMsg {
public:    
	CAudioMsgSetAudioPan( const tAudioPanInfo pi );
	tAudioPanInfo	GetData( void ) { return pi_; }
	
private:
	tAudioPanInfo	pi_;
};

class CAudioMsgGetAudioPan : public CAudioCmdMsg {
public:    
	CAudioMsgGetAudioPan( tAudioID id );
	tAudioPanInfo	GetData( void ) { return pi_; }
	
private:
	tAudioPanInfo	pi_;
};

//kAudioCmdMsgTypeSetAudioListener,
//kAudioCmdMsgTypeGetAudioListener,
class CAudioMsgSetAudioListener : public CAudioCmdMsg {
public:    
	CAudioMsgSetAudioListener( const tAudioListenerInfo li );
	tAudioListenerInfo	GetData( void ) { return li_; }
	
private:
	tAudioListenerInfo	li_;
};

class CAudioMsgGetAudioListener : public CAudioCmdMsg {
public:    
	CAudioMsgGetAudioListener( tAudioID id );
	tAudioListenerInfo	GetData( void ) { return li_; }
	
private:
	tAudioListenerInfo	li_;
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
	
	CAudioMsgIsMidiFilePlaying( void );  // Is any MIDi file playing?
	CAudioMsgIsMidiFilePlaying( const tMidiPlayerID id );

	tMidiPlayerID	GetData( void ) { return id_; }

private:
	tMidiPlayerID	id_;
};

//kAudioCmdMsgTypeGetEnabledMidiTracks
//kAudioCmdMsgTypeSetEnableMidiTracks
//kAudioCmdMsgTypeTransposeMidiTracks
//kAudioCmdMsgTypeChangeMidiInstrument
//kAudioCmdMsgTypeChangeMidiTempo
class CAudioMsgMidiFilePlaybackParams : public CAudioCmdMsg {
public:
	CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id );
	CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const tMidiTrackBitMask trackBitMask );
	CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const tMidiTrackBitMask trackBitMask, const S8 transposeAmount );
	CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const tMidiTrackBitMask trackBitMask, const tMidiPlayerInstrument instr );
	CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const S8 tempo );

	tAudioMidiFilePlaybackParams *GetData( void ) { return &data_; }
	
private:
	tAudioMidiFilePlaybackParams		data_;
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

// kAudioCmdMsgExitThread
class CAudioCmdMsgExitThread : public CAudioCmdMsg {
public:    
	CAudioCmdMsgExitThread( void );
};

//-----------------------------------------------------------------------
// return message
class CAudioReturnMessage : public CMessage 
{
public:    	
	CAudioReturnMessage( void );

	void SetAudioErr( tErrType err )          { err_          = err; }
	void SetAudioID( tAudioID id )            { audioID_      = id;  }
	void SetMidiID( tMidiPlayerID id )        { midiPlayerID_ = id;  }
	void SetBooleanResult( Boolean val )      { booleanValue_ = val; }
	void SetU32Result( U32 val )              { u32Value_     = val; }
	void SetAudioStateResult( tAudioState d ) { audioState_   = d;   }
	
	tMidiPlayerID GetMidiID(           void ) { return midiPlayerID_; }
	tAudioID      GetAudioID(          void ) { return audioID_; }
	tErrType      GetAudioErr(         void ) { return err_; }
	Boolean       GetBooleanResult(    void ) { return booleanValue_; }
	U32           GetU32Result(        void ) { return u32Value_; }
	tAudioState   GetAudioStateResult( void ) { return audioState_; }
	
private:
	tErrType		err_;
	tAudioID		audioID_;
	tMidiPlayerID	midiPlayerID_;
	Boolean			booleanValue_;
	U32				u32Value_;
	tAudioState		audioState_;
};

const U32	kAUDIO_MAX_MSG_SIZE	=	256;
const U32	kAUDIO_MAX_NUM_MSGS	=	8;

LF_END_BRIO_NAMESPACE()	
#endif	// LF_BRIO_AUDIOMSG_H

// EOF	
