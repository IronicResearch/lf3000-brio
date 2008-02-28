#ifndef LF_BRIO_AUDIOMPI_H
#define LF_BRIO_AUDIOMPI_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// AudioMPI.h
//
// Defines Module Public Interface (MPI) for Audio module
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <AudioTypes.h>

#include <CoreMPI.h>
#include <AudioEffectsProcessor.h>
LF_BEGIN_BRIO_NAMESPACE()

/// \class CAudioMPI
///
/// Module Public Interface (MPI) class for Audio module.
///
/// The audio system is composed of three basic parts: players, channels, and a
/// mixer.  When a user calls \ref StartAudio, this creates a player, connects
/// it to a channel, launches the player's audio stream.  As the audio is
/// rendered from the player, it is mixed with other channels by the mixer, and
/// finally output.  When the player's stream ends, an event is posted to the
/// listener if one has been supplied.  Subsequently, the player is destroyed
/// and the channel that it was using becomes free for another player.
///
/// Players are represented internally by a \ref tAudioID.  The \ref tAudioID is
/// returned when the player is created with a call to \ref StartAudio.
/// Whenever the user wishes to operate on a particular player's stream, he will
/// have to provide its \ref tAudioID.  The \ref tAudioID ceases to be valid
/// after a kAudioCompletedEvent or a kAudioTerminatedEvent.  If a listener is
/// provided for these events, that listener should assume that the tAudioID is
/// not valid in its Notify function.
///
/// Calling the various MPI functions with an invalid tAudioID will either
/// result in no action or an error.
///
/// The user can start a player with a given priority.  If no channel is
/// available, and a lower priority player is playing, that player will be
/// terminated in order to free up a channel.
///
/// Midi is supported.  For legacy reasons, midi players are handled with a
/// separate set of functions from the audio players.  Midi players have their
/// own channels and their priority is evaluated independently of the audio
/// players' channels.  They are also represented by a tMidiPlayerID, which is
/// analogous to the tAudioID.  In practice, any function that takes a tAudioID
/// can also operate on a midi channel by passing the tMidiPlayerID.  These
/// functions include \ref GetAudioTime, \ref IsAudioPlaying, \ref PauseAudio,
/// \ref ResumeAudio, \ref GetAudioVolume, \ref SetAudioVolume, \ref
/// GetAudioPriority, \ref SetAudioPriority, \ref SetPan, \ref GetPan, \ref
/// GetAudioEventListener, \ref SetAudioEventListener.  See \ref
/// MidiPlayerBasics for more details.
///
/// Please see the \ref KnownIssues section for important information about bugs
/// and unimplemented features.

/// \page KnownIssues Known Issues
///
/// \todo IsAudioPlaying on a particular channel returns true even if that channel
/// is paused.
///
/// \todo Priority is not implemented.
///
/// \todo Priority and listener args to AcquireMidiPlayer are ignored.
///
/// \todo Where applicable, the noDoneMessage arguments are ignored.  As of 28
/// Feb 08, fixing this functionality now actually breaks the AppManager
/// (sequencer?).  Specifically, when scrolling fast through the first wheel
/// after selecting a name, the audio from the previous icon is not pre-empted.
/// I'm not sure why.
///
///
/// \todo AudioEffectsProcessor features are unimplemented.
///
/// \todo Ensure that tMidiPlayerIDs can ALWAYS be used as tAudioIDs with no
/// special action by the user.  Unfortunately, because tMidiPlayerIDs are 8
/// bits and tAudioIDs are 32 bits the only sensible scheme to acheive this is
/// to reserve tAudioIDs 0-255 for tMidiPlayerIDs.  Sad.
///
/// \todo Ensure that GetAudioTime, IsAudioPlaying, PauseAudio, ResumeAudio,
/// GetAudioVolume, SetAudioVolume, GetAudioPriority, SetAudioPriority, SetPan,
/// GetPan, GetAudioEventListener, SetAudioEventListener operate properly on
/// midi streams.
///
/// \todo Low-level midi API is unimplemented.
///
/// \todo Ensure that MPI functions fail if kNoAudioID is passed in.
///
/// \todo Ensure that MPI functions fail if invalid (i.e., stale) tAudioID is
/// passed in.
///
/// \todo kAudioTerminatedEvent and kMidiTerminatedEvent are not implemented.
///
/// \todo Cue points are not implemented and may never be implemented.
///
/// \todo raw player (.raw .brio .aif .aiff and .wav) is implemented but not
/// thoroughly tested.
///
/// \todo infinite looping is not implemented.
///
/// \todo looping clicks.  It should be 100% continuous.

class IEventListener;

class CAudioMPI : public ICoreMPI {
public:
	//********************************
	// ICoreMPI functionality
	//********************************

	///Returns true if this instance of the AudioMPI is valid
	virtual	Boolean			IsValid() const;
	///Returns the name of this MPI
	virtual const CString*	GetMPIName() const;		
	///Returns the version of this MPI
	virtual tVersion		GetModuleVersion() const;
	///Returns the name of this module
	virtual const CString*	GetModuleName() const;	
	///Returns the origin of this module
	virtual const CURI*		GetModuleOrigin() const;
	
	//********************************
	// Class-specific functionality
	//********************************    

	//NOTE: These functions should not be here AT ALL.  They are going away.  Do
	//not use.
	void GAS( void *d ) ;   // LF Internal function.  Not for release
	void SAS( void *d ) ;   // LF Internal function.  Not for release
	
	/// The constructor for the CAudioMPI
	///
	/// \param pDefaultListener This is the default listener that will be
	/// notified of all audio events associated with this instance of the
	/// CAudioMPI. The user can override the default listener by explicitly
	/// providing a listener when calling StartAudio.  NULL is a valid value for
	/// the pDefaultListener.
	///
	/// The user can get and set the default audio listener by calling the
	/// functions \ref GetDefaultAudioEventListener and \ref
	/// SetDefaultAudioEventListener.
	CAudioMPI( const IEventListener* pDefaultListener = NULL );
	virtual ~CAudioMPI( void );
	
	/// Pause all audio output.
	///
	/// After calling this function, all audio output is paused.  IsAudioPlaying
	/// for any channel will return false.
	///
	/// \return This function always returns kNoErr
	tErrType	PauseAudioSystem( void );

	/// Resume all audio output.  Sensibly called after PauseAudioSystem.
	///
	/// The streams that were playing before the call to PauseAudioSystem are
	/// resumed.
	///
	/// \return This function always returns kNoErr
	tErrType	ResumeAudioSystem( void );

	/// Set output volume
	///
	/// \param volume This parameter ranges from kAudioVolumeMin and
	/// kAudioVolumeMax.  For legacy reasons, kAudioVolumeMin will always be 0
	/// and kAudioVolumeMax will always be 100.
	void 		SetMasterVolume( U8 volume );

	/// Get the current output volume
	///
	/// \return the current output volume
	U8			GetMasterVolume( void ) const;

	/// Get Speaker Equalizer value
	///
	/// Some systems require a special output stage depending on whether the
	/// output is going to a speaker or not.  Applications rarely have to worry
	/// about this.  Generally, the Brio system will call this function if
	/// necessary in response to heaphone connect/disconnect events.
	/// 
	/// \return true if the speaker equalizer is enabled, false otherwise.
	Boolean		GetSpeakerEqualizer(void) const;

	/// Enable or disable the speaker equalizer
	///
	/// See \ref GetSpeakerEqualizer for an explanation of what the speaker
	/// equalizer does.
	///
	/// \param enable true to enable the speaker equalizer, false to disable.
	void		SetSpeakerEqualizer( Boolean enable );
	
	/// Set the path where the audio system should look for audio resources.
	///
	/// \param path This is the path where the files referenced by calls to
	/// StartAudio should be.
	///
	/// \return Always returns kNoErr
	tErrType		SetAudioResourcePath( const CPath &path );

	/// Get the path from which audio resources are loaded.
	/// 
	/// See \ref SetAudioResourcePath for details
	const CPath* 	GetAudioResourcePath( void ) const;
	
	//********************************
	// Audio Playback. 
	//********************************    
	
	/// Play an audio resource
	///
	/// Create a player and add its output stream to the system.  The stream
	/// will stop under one of the following circumstances:
	///
	/// 1. The end of the player's audio resource is reached and the
	/// kAudioOptionsLooped flag is not specified.  This is a
	/// kAudioCompletedEvent.
	///
	/// 2. The kAudioOptionsLooped option is passed and the stream has been
	/// repeated as many times as indicated by the payload parameter.  This is
	/// also a kAudioCompletedEvent.
	///
	/// 3. The user calls StopAudio with the player's tAudioID.  This is a
	/// kAudioTerminatedEvent.
	///
	/// 4. The priority policy terminates the player's stream because there are
	/// no channels available and somebody called StartAudio for a player of
	/// higher priority.  This is a kAudioTerminatedEvent.
	///
	/// 5. The AudioMPI destructor is called.  Any streams that are playing at
	/// this time will be terminated with a kAudioTerminatedEvent.
	///
	/// Numerous parameters affect the behavior of the player.  Specifically:
	///
	/// \param path This is the path to the audio resource to be played.  If it
	/// begins with a / it is considered absolute.  Otherwise, it is relative to
	/// the path returned by \ref GetAudioResourcePath.
	///
	/// \param volume The volume, between kAudioVolumeMin and kAudioVolumeMax at
	/// which the stream should be played.  This volume can be inspected and
	/// adjusted later by calling \ref GetAudioVolume and \ref SetAudioVolume
	/// respectively.
	///
	/// \param priority In the event that all channels are busy AND a player of
	/// lower priority is currently playing, a lower priority player will be
	/// stopped with a kAudioTerminatedEvent to free a channel.  Similarly, if a
	/// future call to StartAudio happens with a higher priority, then this
	/// player may be terminated with a kAudioTerminatedEvent.  priority is an
	/// unsigned value from 0 to 255 with 255 being the highest priority.
	///
	/// \param pan The pan is a signed value between kAudioPanMin and
	/// kAudioPanMax.  This pan can be inspected and adjusted later by calling
	/// \ref GetAudioPan and \ref SetAudioPan respectively.
	///
	/// \param pListener This is the listener to whom all audio events related
	/// to this player will be posted.  Passing NULL means that no events will
	/// ever be posted, even if the default listener is set to a non NULL value
	/// using SetDefaultAudioEventListener.
	///
	/// \param payload The payload parameter is a user-supplied argument that is
	/// passed to the listener when various events occur.  However, for legacy
	/// reasons, there is one overloaded use case.  When the kAudioOptionsLooped
	/// flag is set, the payload parameter represents the number of times the
	/// audio should be looped.  Pass kAudioRepeat_Infinite to repeat forever.
	/// In this legacy case, the payload parameter will still be passed to the
	/// listener.
	/// 
	/// \param flags The flags parameter is a collection of OR'd values that
	/// influence many of the player's behaviors.  See \ref AudioOptions for
	/// details on which flags are available and how they influence the behavior
	/// of the player.
	/// 
	/// \return This function returns kNoAudioID on failure and a valid tAudioID
	/// on success.
	tAudioID 	StartAudio( const CPath &path, 
					U8					volume, 
					tAudioPriority		priority,
					S8					pan, 
					const IEventListener *pListener = kNull,
					tAudioPayload		payload = 0,
					tAudioOptionsFlags	flags	= 0 );
	
	/// Play and audio resource using various defaults
	///
	/// This function plays the audio resource at path using the default volume,
	/// priority, pan, and listener.  See \ref StartAudio for details.
	tAudioID 	StartAudio( const CPath &path, 
		  					tAudioPayload		payload,
		  					tAudioOptionsFlags	flags );

	/// Pause a particular audio player.
	///
	/// \param id The tAudioID of the stream to pause
	///
	///  This function has no effect if the id is invalid.
	void 		PauseAudio(  tAudioID id );
	
	/// Resume a paused audio player.  Sensibly called after \ref PauseAudio.
	///
	/// \param id The tAudioID of the stream to resume.
	///
	/// This function has no effect if the id is invalid.
	void 		ResumeAudio( tAudioID id );
	
	/// Stop a player's audio stream.
	///
	/// After calling this function, the tAudioID is invalid.  An event may or
	/// may not be posted depending on the details below.
	///
	/// \param id The tAudioID of the player to stop.
	///
	/// \param noDoneMessage If this parameter is true, then the
	/// kAudioTerminatedEvent will not be posted to the listener associated with
	/// this audio player.  If this parameter is false, AND the player has an
	/// associated listerner, AND the StartAudio call that launched this player
	/// had the kAudioOptionsDoneMsgAfterComplete flag set, then the
	/// kAudioTerminatedEvent will be posted to the listener.
	///
	/// This function has no effect if id is invalid.
	void 		StopAudio(	tAudioID id, Boolean noDoneMessage ); 

	/// Is a given player playing?
	///
	/// \param id The tAudioID of the player in question.
	///
	/// \returns true if the player is not paused.  Returns false if audio has
	/// been paused with a call to PauseAudioSystem OR if the player has been
	/// paused with a call to PauseAudio, OR if the tAudioID is invalid.
	Boolean		IsAudioPlaying( tAudioID id );

	/// Is any player playing?
	///
	/// \returns true if any player is playing.  Returns false if audio has been
	/// paused with a call to PauseAudioSystem, OR if all players are paused, OR
	/// if no players exist.
	Boolean		IsAudioPlaying( void );

	/// Report a player's stream time in milliseconds 
	///
	/// \param id tAudioID of the player in question
	///
	/// \returns time in milliseconds since player creation.  Note that this
	/// time halts when the player is paused.  Returns 0 if id is not valid.
	U32 	GetAudioTime( tAudioID id ) const;
	
	/// Get volume of a player
	///
	/// \param id tAudioID of the player in question
	/// 
	/// \returns volume between kAudioVolumeMax and kAudioVolumeMin.  Returns
	/// kAudioVolumeMin if id is not valid.
	U8		GetAudioVolume( tAudioID id ) const;

	/// Set volume of a player
	///
	/// \param id tAudioID of the player in question
	/// 
	/// \param volume between kAudioVolumeMax and kAudioVolumeMin.
	///
	/// This function has no affect if id is invalid.  If volume is not within
	/// the bounds of kAudioVolumeMin and kAudioVolumeMax, it will be rounded to
	/// the nearest valid volume.
	void	SetAudioVolume( tAudioID id, U8 volume );

	/// Get pan of a player
	///
	/// \param id tAudioID of the player in question
	/// 
	/// \returns pan between kAudioPanMax and kAudioPanMin.  Returns
	/// 0 if id is not valid.
	S8		GetAudioPan( tAudioID id ) const;

	/// Set pan of a player
	///
	/// \param id tAudioID of the player in question
	/// 
	/// \param pan between kAudioPanMax and kAudioPanMin.
	///
	/// This function has no affect if id is invalid.  If volume is not within
	/// the bounds of kAudioPanMin and kAudioPanMax, it will be rounded to
	/// the nearest valid pan.
	void	SetAudioPan( tAudioID id, S8 pan );

	/// Get the priority of a particular player
	///
	/// The priority is used to decide what to do with a player when there are
	/// no free channels and somebody tries to play one.
	///
	/// \param id tAudioID of the player in question.
	/// 
	/// \return priority between 0 and 255.  255 is the highest priority.
	tAudioPriority	GetAudioPriority( tAudioID id ) const;

	/// Set the priority of a particular player
	///
	/// \param id tAudioID of the player in question
	/// 
	/// \param priority between 0 and 255 inclusive.  Note that 0 is the lowest
	/// priority and 255 is the highest priority.
	///
	/// This function has no effect if the id is invalid.
	void SetAudioPriority( tAudioID id, tAudioPriority priority );

	/// Get the event listener associated with a particular player
	///
	/// The event listener's Notify function is called when audio events occur.
	/// If the event listener is NULL, no events will be posted.
	///
	/// \param id tAudioID of the player in question.
	/// 
	/// \return a pointer to the current event listener associated with the
	/// player.  Returns NULL if the tAudioID is invalid.
	const IEventListener*	GetAudioEventListener( tAudioID id ) const;

	/// Set the event listener associated with of a particular player
	///
	/// \param id tAudioID of the player in question
	/// 
	/// \param pListener the new listener.
	///
	/// This function has no effect if the id is invalid.
	void	SetAudioEventListener( tAudioID id, IEventListener *pListener );

	/// Get the default audio volume
	///
	/// This is the volume at which players are launched for StartAudio variants
	/// that do not take a volume argument.
	///
	/// \return the volume between kAudioVolumeMax and kAudioVolumeMin
	U8		GetDefaultAudioVolume( void ) const;

	/// Set the default audio volume
	///
	/// This is the volume at which players are launched for StartAudio variants
	/// that do not take a volume argument.
	///
	/// \param volume the volume between kAudioVolumeMax and kAudioVolumeMin.
	/// If a value is outside of this range, it will be rounded to the nearest
	/// valid value.
	void	SetDefaultAudioVolume( U8 volume );

	/// Get the default pan
	///
	/// This is the pan at which players are launched for StartAudio variants
	/// that do not take a pan argument.
	///
	/// \return the pan between kAudioPanMax and kAudioPanMin
	S8		GetDefaultAudioPan( void ) const;

	/// Set the default audio pan
	///
	/// This is the pan at which players are launched for StartAudio variants
	/// that do not take a pan argument.
	///
	/// \param pan the pan between kAudioPanMax and kAudioPanMin.  If a value is
	/// outside of this range, it will be rounded to the nearest valid value.
	void	SetDefaultAudioPan( S8 pan );

	/// Get the default priority
	///
	/// This is the priority at which players are launched for StartAudio
	/// variants that do not take a priority argument.
	///
	/// \return the priority between 0 and 255
	tAudioPriority	GetDefaultAudioPriority( void ) const;

	/// Set the default priority
	///
	/// This is the priority at which players are launched for StartAudio
	/// variants that do not take a priority argument.
	///
	/// \param priority the priority between 0 and 255.
	void			SetDefaultAudioPriority( tAudioPriority priority );

	/// Get the default listener
	///
	/// This is the listener with which players are launched for StartAudio
	/// variants that do not take a listener argument.
	///
	/// \return a pointer to the default listener
	const IEventListener	*GetDefaultAudioEventListener( void ) const;

	/// Set the default listener
	///
	/// This is the listener with which players are launched for StartAudio
	/// variants that do not take a listener argument.
	///
	/// \param pListener pointer to the new default listener
	void			SetDefaultAudioEventListener( IEventListener *pListener );
	
	//********************************
	// Audio FX functionality
	//********************************

	/// Audio effects processing is unimplemented.
	///
	/// \return this function will return kNoImplErr
	tErrType RegisterAudioEffectsProcessor(  CAudioEffectsProcessor *pChain ); 

	/// Audio effects processing is unimplemented.
	///
	/// \return this function will return kNoImplErr
	tErrType RegisterGlobalAudioEffectsProcessor( CAudioEffectsProcessor *pChain ); 

	/// Audio effects processing is unimplemented.
	///
	/// \return this function will return kNoImplErr
	tErrType ChangeAudioEffectsProcessor( tAudioID id, CAudioEffectsProcessor *pChain ); 


	//********************************
	// MIDI functionality
	//********************************	 

	/// \page MidiPlayerBasics "Midi Player Basics"
	///
	/// There are three basic midi use cases:
	///
	/// Use Case 1: The Legacy Use Case
	///
	/// 1. Call \ref AcquireMidiPlayer to create the player and retrieve a
	/// tMidiPlayerID.
	///
	/// 2. Call StartMidiFile to launch the midi stream.
	///
	/// 3. While the midi stream is playing, it can be manipulated using \ref
	/// IsMidiFilePlaying, \ref PauseMidiFile, \ref ResumeMidiFile, and \ref
	/// StopMidiFile.  It can also be manipulated using the Audio functions \ref
	/// GetAudioTime, \ref IsAudioPlaying, \ref PauseAudio, \ref ResumeAudio,
	/// \ref GetAudioVolume, \ref SetAudioVolume, \ref GetAudioPriority, \ref
	/// SetAudioPriority, \ref SetPan, \ref GetPan, \ref GetAudioEventListener,
	/// \ref SetAudioEventListener.  When using these audio functions, simply
	/// pass the tMidiPlayerID as the tAudioID.
	///
	/// 4. Upon receiving a Notify callback, or when IsMidiFilePlaying returns
	/// false, or after calling \ref StopMidiFile, destroy the midi player with
	/// ReleaseMidiPlayer.
	///
	/// Use Case 2: Normal Midi File Playback
	///
	/// Use the \ref StartAudio interface to launch a player in the same fashion
	/// that you would play any other audio file.  This use case is currently
	/// unsupported.
	///
	/// Use Case 3: Programatic generation of a midi stream
	///
	/// In addition to the high-level play-pause-resume-stop interface for midi
	/// players, a low-level midi API exists.  This low-level API can be used
	/// programatically generate a midi stream.  However, this API is not fully
	/// specified and not fully implemented.  So, at this time, programatic
	/// generation of midi is not supported.
	///

	/// Create midi player
	///
	/// \param priority This parameter is the priority of the player.  It can be
	/// overridden when StartMidiFile is called with the tMidiPlayerID returned
	/// in pID.
	///
	/// \param pListener This parameter is the event listener of the player.  It
	/// can be overridden when StartMidiFile is called with the tMidiPlayerID
	/// returned in pID.
	///
	/// \param *pID This is a pointer to a tMidiPlayerID.  It will be set by
	/// this function to either a valid tMidiPlayerID or to kNoMidiID.
	///
	/// \return This function unconditionally returns kNoErr.  The caller must
	/// check the value of the pID argument to know if the call succeeded.
	///
	tErrType AcquireMidiPlayer( tAudioPriority priority,
								IEventListener* pListener,
								tMidiPlayerID* pID );


	/// Delete midi player
	///
	/// This function must be called after the midi player is no longer needed.
	/// After it is called, the tMidiPlayerID should be considered invalid.
	///
	/// \param id This is the id of the midi player that is no longer needed.
	///
	/// \return This function unconditionally returns kNoErr.
	tErrType ReleaseMidiPlayer( tMidiPlayerID id ); 

	/// Convert a tMidiPlayerID to a tAudioID
	///
	/// This function is not actually needed.  The tMidiPlayerID can be passed
	/// as a tAudioID to all functions that will take it.
	tAudioID GetAudioIDForMidiID( tMidiPlayerID id );
	
	/// Start playback of midi file.
	///
	/// This function is analogous to StartAudio.  The only difference is that
	/// it must be passed a valid tMidiPlayerID.
	tErrType 	StartMidiFile( tMidiPlayerID		id,
							   const CPath			&path, 
							   U8					volume, 
							   tAudioPriority		priority,
							   IEventListener*		pListener,
							   tAudioPayload		payload,
							   tAudioOptionsFlags	flags );
	
	/// Start playback of midi file with various defaults.
	///
	/// Uses defaults from MPI for volume, priority, and listener.  See \ref
	/// StartMidiFile for more details.
	tErrType 	StartMidiFile( tMidiPlayerID		id,
							   const CPath			&path, 
							   tAudioPayload		payload,
							   tAudioOptionsFlags	flags );

	/// Is a given midi player playing?
	///
	/// Returns true if id is playing.  Returns false otherwise.  Note that \ref
	/// IsAudioPlaying does the exact same thing as this function.
	///
	/// \param id the tMidiPlayerID of the midi player in question
	///
	/// \return true if id is playing, false otherwise.
	Boolean		IsMidiFilePlaying( tMidiPlayerID id );

	/// Is any midi player playing?
	///
	/// Returns true if any midi player is active.
	///
	/// \return true if any midi player is active, false if no midi players are
	/// active.
	Boolean		IsMidiFilePlaying( void );

	/// Pause a particular midi player.
	///
	/// \param id The tMidiPlayerID of the stream to pause
	///
	///  This function has no effect if the id is invalid.  Note that this does
	///  the same thing as \ref PauseAudio.
	void 		PauseMidiFile( tMidiPlayerID id );
	
	/// Resume a paused midi player.  Sensibly called after \ref PauseAudio or
	/// \ref PauseMidiFile.
	///
	/// \param id The tMidiPlayerID of the stream to resume.
	///
	/// This function has no effect if the id is invalid.  Note that this
	/// function does the same thing as \ref ResumeAudio.
	void 		ResumeMidiFile( tMidiPlayerID id );
	
	/// Stop a midi player's stream.
	///
	/// After calling this function, the midi stream will stop.  An event may or
	/// may not be posted depending on the details below.
	///
	/// \param id The tMidiPlayerID of the player to stop.
	///
	/// \param noDoneMessage If this parameter is true, then the
	/// kMidiTerminatedEvent will not be posted to the listener associated with
	/// this audio player.  If this parameter is false, AND the player has an
	/// associated listerner, AND the StartMidiFile call that launched this
	/// player had the kAudioOptionsDoneMsgAfterComplete flag set, then the
	/// kMidiTerminatedEvent will be posted to the listener.
	///
	/// This function has no effect if id is invalid.  After calling this
	/// function, the user can call \ref ReleaseMidiPlayer.
	void 		StopMidiFile( tMidiPlayerID id, Boolean noDoneMessage );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType 	SetEnableMidiTracks( tMidiPlayerID id,
									 tMidiTrackBitMask bitMask );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType	TransposeMidiTracks( tMidiPlayerID id,
									 tMidiTrackBitMask bitMask,
									 S8 semitones );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType	ChangeMidiTempo( tMidiPlayerID id, S8 tempo ); 
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType	ChangeMidiInstrument( tMidiPlayerID id,
									  int channel,
									  tMidiPlayerInstrument instr );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType 	ChangeMidiInstrument( tMidiPlayerID id,
									  tMidiTrackBitMask bits,
									  tMidiPlayerInstrument instr ); 
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType MidiNoteOn( tMidiPlayerID id,
						 U8 channel,
						 U8 note,
						 U8 velocity,
						 tAudioOptionsFlags flags );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType MidiNoteOn( tMidiPlayerID id, U8 channel, U8 note, U8 velocity);

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType MidiNoteOff( tMidiPlayerID id,
						  U8 channel,
						  U8 note,
						  U8 velocity,
						  tAudioOptionsFlags flags );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType MidiNoteOff(tMidiPlayerID id,
						 U8 channel,
						 U8 note);
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType CreateProgramList( tMidiProgramList **d );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType AddToProgramList( tMidiProgramList *d, U8 bank, U8 program );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType AddDrumToProgramList( tMidiProgramList *d, U8 bank, U8 program, int pitch );

	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType DeleteProgramList(	 tMidiProgramList *d );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType ScanForPrograms( tMidiPlayerID id, tMidiProgramList *d );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType LoadInstrumentFile( const CPath &path , tMidiProgramList *d );
	
	/// Low-level midi control
	///
	/// Use of this function is not currently specified.
	tErrType UnloadAllInstruments( void );
	
 private:
	class CAudioModule*	pModule_;
	U32					mpiID_;
};

// MIDI note definitions
#define kMIDI_Cm1	0
#define kMIDI_Dbm1	1
#define kMIDI_Dm1	2
#define kMIDI_Ebm1	3
#define kMIDI_Em1	4
#define kMIDI_Fm1	5
#define kMIDI_Gbm1	6
#define kMIDI_Gm1	7
#define kMIDI_Abm1	8
#define kMIDI_Am1	9
#define kMIDI_Bbm1	10
#define kMIDI_Bm1	11
#define kMIDI_C0	12
#define kMIDI_Db0	13
#define kMIDI_D0	14
#define kMIDI_Eb0	15
#define kMIDI_E0	16
#define kMIDI_F0	17
#define kMIDI_Gb0	18
#define kMIDI_G0	19
#define kMIDI_Ab0	20
#define kMIDI_A0	21
#define kMIDI_Bb0	22
#define kMIDI_B0	23
#define kMIDI_C1	24
#define kMIDI_Db1	25
#define kMIDI_D1	26
#define kMIDI_Eb1	27
#define kMIDI_E1	28
#define kMIDI_F1	29
#define kMIDI_Gb1	30
#define kMIDI_G1	31
#define kMIDI_Ab1	32
#define kMIDI_A1	33
#define kMIDI_Bb1	34
#define kMIDI_B1	35
#define kMIDI_C2	36
#define kMIDI_Db2	37
#define kMIDI_D2	38
#define kMIDI_Eb2	39
#define kMIDI_E2	40
#define kMIDI_F2	41
#define kMIDI_Gb2	42
#define kMIDI_G2	43
#define kMIDI_Ab2	44
#define kMIDI_A2	45
#define kMIDI_Bb2	46
#define kMIDI_B2	47
#define kMIDI_C3	48
#define kMIDI_Db3	49
#define kMIDI_D3	50
#define kMIDI_Eb3	51
#define kMIDI_E3	52
#define kMIDI_F3	53
#define kMIDI_Gb3	54
#define kMIDI_G3	55
#define kMIDI_Ab3	56
#define kMIDI_A3	57
#define kMIDI_Bb3	58
#define kMIDI_B3	59
#define kMIDI_C4	60
#define kMIDI_Db4	61
#define kMIDI_D4	62
#define kMIDI_Eb4	63
#define kMIDI_E4	64
#define kMIDI_F4	65
#define kMIDI_Gb4	66
#define kMIDI_G4	67
#define kMIDI_Ab4	68
#define kMIDI_A4	69
#define kMIDI_Bb4	70
#define kMIDI_B4	71
#define kMIDI_C5	72
#define kMIDI_Db5	73
#define kMIDI_D5	74
#define kMIDI_Eb5	75
#define kMIDI_E5	76
#define kMIDI_F5	77
#define kMIDI_Gb5	78
#define kMIDI_G5	79
#define kMIDI_Ab5	80
#define kMIDI_A5	81
#define kMIDI_Bb5	82
#define kMIDI_B5	83
#define kMIDI_C6	84
#define kMIDI_Db6	85
#define kMIDI_D6	86
#define kMIDI_Eb6	87
#define kMIDI_E6	88
#define kMIDI_F6	89
#define kMIDI_Gb6	90
#define kMIDI_G6	91
#define kMIDI_Ab6	92
#define kMIDI_A6	93
#define kMIDI_Bb6	94
#define kMIDI_B6	95
#define kMIDI_C7	96
#define kMIDI_Db7	97
#define kMIDI_D7	98
#define kMIDI_Eb7	99
#define kMIDI_E7	100
#define kMIDI_F7	101
#define kMIDI_Gb7	102
#define kMIDI_G7	103
#define kMIDI_Ab7	104
#define kMIDI_A7	105
#define kMIDI_Bb7	106
#define kMIDI_B7	107
#define kMIDI_C8	108
#define kMIDI_Db8	109
#define kMIDI_D8	110
#define kMIDI_Eb8	111
#define kMIDI_E8	112
#define kMIDI_F8	113
#define kMIDI_Gb8	114
#define kMIDI_G8	115
#define kMIDI_Ab8	116
#define kMIDI_A8	117
#define kMIDI_Bb8	118
#define kMIDI_B8	119
#define kMIDI_C9	120
#define kMIDI_Db9	121
#define kMIDI_D9	122
#define kMIDI_Eb9	123
#define kMIDI_E9	124
#define kMIDI_F9	125
#define kMIDI_Gb9	126
#define kMIDI_G9	127
// Above MIDI range 
#define kMIDI_Ab9	128
#define kMIDI_A9	129
#define kMIDI_Bb9	130
#define kMIDI_B9	131
#define kMIDI_C10	132
#define kMIDI_Db10	133
#define kMIDI_D10	134
#define kMIDI_Eb10	135
#define kMIDI_E10	136
#define kMIDI_F10	137
#define kMIDI_Gb10	138
#define kMIDI_G10	139

// MIDI Channel Message
// Upper Nibble: Message ID  
// Lower Nibble: Channel # [0..15]
#define kMIDI_ChannelMessage_NoteOff			0x80
#define kMIDI_ChannelMessage_NoteOn				0x90
#define kMIDI_ChannelMessage_Aftertouch			0xA0
#define kMIDI_ChannelMessage_ControlChange		0xB0
#define kMIDI_ChannelMessage_ProgramChange		0xC0
#define kMIDI_ChannelMessage_ChannelPressure	0xD0
#define kMIDI_ChannelMessage_PitchWheel			0xE0

// Supported MIDI Controller list.  Other controllers are ignored
#define kMIDI_Controller_BankSelect				0x00
#define kMIDI_Controller_ModulationWheel		0x01
#define kMIDI_Controller_DataEntry				0x06
#define kMIDI_Controller_Volume					0x07
#define kMIDI_Controller_PanPosition			0x0A
#define kMIDI_Controller_Expression				0x0B
#define kMIDI_Controller_LSBOffset				0x20
#define kMIDI_Controller_Sustain				0x40
#define kMIDI_Controller_RPN_LSB				0x64
#define kMIDI_Controller_RPN_Fine				0x64
#define kMIDI_Controller_RPN_MSB				0x65
#define kMIDI_Controller_RPN_Coarse				0x65

#define kMIDI_Controller_AllSoundOff			120
#define kMIDI_Controller_ResetAllControllers	121
#define kMIDI_Controller_AllNotesOff			123

#define kMIDI_RPN_PitchBendRange				0x0000
#define kMIDI_RPN_MasterFineTuning				0x0001
#define kMIDI_RPN_MasterCoarseTuning			0x0002

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

