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
/// MIDI functions are no longer implemented.
///
/// Please see the \ref KnownIssues section for important information about bugs
/// and unimplemented features.

/// \page KnownIssues Known Issues
///
/// \todo IsAudioPlaying on a particular channel returns true even if that channel
/// is paused.
///
/// \todo Where applicable, the noDoneMessage arguments are ignored.  As of 28
/// Feb 08, fixing this functionality now actually breaks the AppManager
/// (sequencer?).  Specifically, when scrolling fast through the first wheel
/// after selecting a name, the audio from the previous icon is not pre-empted.
/// I'm not sure why.
///
/// \todo AudioEffectsProcessor features are unimplemented.
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
	/// influence many of the player's behaviors.  See \ref tAudioOptionsFlags for
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

	/// Play raw audio sample from memory buffer.
	///
	/// \param header The tAudioHeader describing the audio sample buffer size, 
	/// sample rate, and mono/stereo format. 
	///
	/// \param pBuffer The pointer to the audio sample data. The memory buffer
	/// must to fit the entire audio sample, as specified in the tAudioHeader.
	///
	/// \param pCallback Optional tGetStereoAudioStreamFcn rendering callback
	/// function if the caller intends to do its own audio streaming. 
	///
	/// This function plays a raw audio sample from a memory buffer with a valid 
	/// tAudioHeader signature. The audio sample data must be in an uncompressed 
	/// PCM format supported by the Brio Audio Mixer. (16-bit little-endian, 
	/// mono/stereo, 8/16/32 KHz sample rate.) Compatible with .brio raw audio
	/// file images, except the data can be located separately from the header.
	tAudioID 	StartAudio( tAudioHeader		&header,
							S16*				pBuffer,
							tGetStereoAudioStreamFcn pCallback,
							U8					volume, 
							tAudioPriority		priority,
							S8					pan, 
							const IEventListener *pListener = kNull,
							tAudioPayload		payload = 0,
							tAudioOptionsFlags	flags	= 0 );

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

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType AcquireMidiPlayer( tAudioPriority priority,
								IEventListener* pListener,
								tMidiPlayerID* pID );


	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType ReleaseMidiPlayer( tMidiPlayerID id ); 

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoAudioID -- This function is no longer implemented.
	tAudioID GetAudioIDForMidiID( tMidiPlayerID id );
	
	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType 	StartMidiFile( tMidiPlayerID		id,
							   const CPath			&path, 
							   U8					volume, 
							   tAudioPriority		priority,
							   IEventListener*		pListener,
							   tAudioPayload		payload,
							   tAudioOptionsFlags	flags );
	
	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType 	StartMidiFile( tMidiPlayerID		id,
							   const CPath			&path, 
							   tAudioPayload		payload,
							   tAudioOptionsFlags	flags );

	/// MIDI functions are no longer implemented.
	///
	/// \return false -- This function is no longer implemented.
	Boolean		IsMidiFilePlaying( tMidiPlayerID id );

	/// MIDI functions are no longer implemented.
	///
	/// \return false -- This function is no longer implemented.
	Boolean		IsMidiFilePlaying( void );

	/// MIDI functions are no longer implemented.
	///
	void 		PauseMidiFile( tMidiPlayerID id );
	
	/// MIDI functions are no longer implemented.
	///
	void 		ResumeMidiFile( tMidiPlayerID id );
	
	/// MIDI functions are no longer implemented.
	///
	void 		StopMidiFile( tMidiPlayerID id, Boolean noDoneMessage );
	
	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType 	SetEnableMidiTracks( tMidiPlayerID id,
									 tMidiTrackBitMask bitMask );

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType	TransposeMidiTracks( tMidiPlayerID id,
									 tMidiTrackBitMask bitMask,
									 S8 semitones );

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType	ChangeMidiTempo( tMidiPlayerID id, S8 tempo ); 
	
	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType	ChangeMidiInstrument( tMidiPlayerID id,
									  int channel,
									  tMidiPlayerInstrument instr );

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType 	ChangeMidiInstrument( tMidiPlayerID id,
									  tMidiTrackBitMask bits,
									  tMidiPlayerInstrument instr ); 
	
	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType MidiNoteOn( tMidiPlayerID id,
						 U8 channel,
						 U8 note,
						 U8 velocity,
						 tAudioOptionsFlags flags );

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType MidiNoteOn( tMidiPlayerID id, U8 channel, U8 note, U8 velocity);

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType MidiNoteOff( tMidiPlayerID id,
						  U8 channel,
						  U8 note,
						  U8 velocity,
						  tAudioOptionsFlags flags );

	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType MidiNoteOff(tMidiPlayerID id,
						 U8 channel,
						 U8 note);
	
	/// MIDI functions are no longer implemented.
	///
	/// \return kNoImplErr -- This function is no longer implemented.
	tErrType SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	
	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType CreateProgramList( tMidiProgramList **d );
	
	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType AddToProgramList( tMidiProgramList *d, U8 bank, U8 program );
	
	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType AddDrumToProgramList( tMidiProgramList *d, U8 bank, U8 program, int pitch );

	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType DeleteProgramList(	 tMidiProgramList *d );
	
	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType ScanForPrograms( tMidiPlayerID id, tMidiProgramList *d );
	
	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType LoadInstrumentFile( const CPath &path , tMidiProgramList *d );
	
	/// Low-level midi control
	///
	/// This function is not currently implemented.
	tErrType UnloadAllInstruments( void );

	/// Set the current priority policy
	///
	/// \param policy The desired priority policy.  See \ref PriorityPolicies
	/// for details regarding the options for this argument.
	///
	/// \returns kNoErr on success and kNoImplErr for unsupported policies.
	tErrType SetPriorityPolicy(tPriorityPolicy policy);
	
	/// Get the current priority policy
	///
	/// \returns	 the current priority policy.  See \ref PriorityPolicies for
	/// details on what the possible return values are.
	tPriorityPolicy GetPriorityPolicy(void);

 private:
	class CAudioModule*	pModule_;
	U32					mpiID_;
};

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

