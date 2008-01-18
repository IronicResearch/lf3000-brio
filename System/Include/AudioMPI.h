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

class IEventListener;


//==============================================================================
// Class:
//		CAudioMPI
//
// Description:
//		Module Public Interface (MPI) class for Audio module
//==============================================================================
class CAudioMPI : public ICoreMPI {
public:
	//********************************
	// ICoreMPI functionality
	//********************************

	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	//********************************
	// Class-specific functionality
	//********************************    

	// NOTE: Default listener is not currently used.
	CAudioMPI( const IEventListener* pDefaultListener = NULL );
	virtual ~CAudioMPI( void );

	//********************************
	// Audio output driver control

	//********************************    

   void GAS( void *d ) ;   // LF Internal function.  Not for release
   void SAS( void *d ) ;   // LF Internal function.  Not for release

	// Pauses audio output driver
	// While paused, audio system consumes no CPU.
	tErrType	PauseAudioSystem(  void );
	tErrType	ResumeAudioSystem( void );

	// Set output gain of mixer (Audio + MIDI)
	void 		SetMasterVolume( U8 volume );
	U8			GetMasterVolume( void ) const;

	// Get/Set speaker equalizer.  Speaker equalizer may be
	// set/cleared with kHeadphoneJackDetect message
	Boolean		GetSpeakerEqualizer(void) const;
	void		SetSpeakerEqualizer( Boolean enable );
	
	// Set/Get path for audio resource file
	tErrType		SetAudioResourcePath( const CPath &path );
	const CPath* 	GetAudioResourcePath( void ) const;
	
	//********************************
	// Audio Playback. 
	//********************************    
	
	// Plays an audio resource.
	// Audio done event will be posted to listener if provided.
	// Currently only volume and pListener are interpreted.
	tAudioID 	StartAudio( const CPath &path, 
					U8					volume, 
					tAudioPriority		priority,
					S8					pan, 
					const IEventListener *pListener = kNull,
					tAudioPayload		payload = 0,
					tAudioOptionsFlags	flags   = 0 );

// Same as above, but uses defaults for unspecified parameters
	tAudioID 	StartAudio( const CPath &path, 
        					tAudioPayload		payload,
        					tAudioOptionsFlags	flags );

// Playback controls
	void 		PauseAudio(  tAudioID id );
	void 		ResumeAudio( tAudioID id ); 
	void 		StopAudio(   tAudioID id, Boolean noDoneMessage ); 

	Boolean		IsAudioPlaying( tAudioID id );  // Is specific ID playing
    Boolean		IsAudioPlaying( void );         // Any audio playing?

	U32 	GetAudioTime( tAudioID id ) const; // Time (milliSeconds) since creation
	
// Get/Set channel parameters
	U8		GetAudioVolume( tAudioID id ) const;
	void	SetAudioVolume( tAudioID id, U8 x );
	S8		GetAudioPan(    tAudioID id ) const;
	void	SetAudioPan(    tAudioID id, S8 x );

	tAudioPriority	GetAudioPriority( tAudioID id ) const;
	void	        SetAudioPriority( tAudioID id, tAudioPriority priority );


	const IEventListener*	GetAudioEventListener( tAudioID id ) const;
	void	SetAudioEventListener( tAudioID id, IEventListener *pListener );

// Defaults to use when value is not specified in the Start() call.
	U8		GetDefaultAudioVolume( void ) const;
	void	SetDefaultAudioVolume( U8 x );
	S8		GetDefaultAudioPan( void ) const;
	void	SetDefaultAudioPan( S8 x );
// Not important for Didj.  Unimplemented
	tAudioPriority	GetDefaultAudioPriority( void ) const;
	void	        SetDefaultAudioPriority( tAudioPriority priority );

	const IEventListener *GetDefaultAudioEventListener( void ) const;
	void	              SetDefaultAudioEventListener( IEventListener *pListener );
	
	//********************************
	// Audio FX functionality
	//********************************   
//  GK FIXXXX Not implemented in Didj.  Should Hide.
	tErrType RegisterAudioEffectsProcessor(  CAudioEffectsProcessor *pChain ); 
	tErrType RegisterGlobalAudioEffectsProcessor( CAudioEffectsProcessor *pChain ); 
	tErrType ChangeAudioEffectsProcessor( tAudioID id, CAudioEffectsProcessor *pChain ); 


	//********************************
	// MIDI functionality
	//********************************    
	// NOTE: Currently, only one player

	// Activate/Deactivate MIDI engine 
	tErrType AcquireMidiPlayer( tAudioPriority priority, IEventListener* pListener, tMidiPlayerID* pID );
	tErrType ReleaseMidiPlayer( tMidiPlayerID id ); 
	
	// Get Audio ID associated with a currently playing MidiFile. 
	tAudioID GetAudioIDForMidiID( tMidiPlayerID id );
	
	// Start playback of MIDI file.
	// Currently only the volume and pListener options are used.
    tErrType 	StartMidiFile( tMidiPlayerID	id,
    					const CPath 		&path, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );

    // Uses defaults from MPI for volume, priority, and listener.
    tErrType 	StartMidiFile( tMidiPlayerID		id,
    							const CPath 		&path, 
    							tAudioPayload		payload,
    							tAudioOptionsFlags	flags );
    
	Boolean		IsMidiFilePlaying( tMidiPlayerID id );
	Boolean		IsMidiFilePlaying( void );  // Is MIDI system playing anything?

	void 		PauseMidiFile(  tMidiPlayerID id );
    void 		ResumeMidiFile( tMidiPlayerID id );
    void 		StopMidiFile(   tMidiPlayerID id, Boolean noDoneMessage );

// Properties of MIDI file play
    tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );
	tErrType 	SetEnableMidiTracks(  tMidiPlayerID id, tMidiTrackBitMask bitMask );
	tErrType 	TransposeMidiTracks(  tMidiPlayerID id, tMidiTrackBitMask bitMask, S8 semitones ); 
	tErrType 	ChangeMidiTempo(      tMidiPlayerID id, S8 tempo ); 

	// Send MIDI channel messages
	tErrType 	ChangeMidiInstrument( tMidiPlayerID id, int channel           , tMidiPlayerInstrument instr ); 
	tErrType 	ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask bits, tMidiPlayerInstrument instr ); 

    tErrType MidiNoteOn( tMidiPlayerID id, U8 channel, U8 note, U8 velocity, tAudioOptionsFlags flags );
    tErrType MidiNoteOn( tMidiPlayerID id, U8 channel, U8 note, U8 velocity);
    tErrType MidiNoteOff(tMidiPlayerID id, U8 channel, U8 note, U8 velocity, tAudioOptionsFlags flags );
    tErrType MidiNoteOff(tMidiPlayerID id, U8 channel, U8 note);

	tErrType SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	
	// ******** Loadable Instrument Support ********
	// Create an empty list of programs and drums. This can be used to keep 
	// track of which resources are needed to play a group of songs.
	tErrType CreateProgramList( tMidiProgramList **d );
	
	// Add a bank/program combination to the list of programs used. If the
	// bank/program has already been added then this will have no effect.  You can
	// use this to add sound effects of MIDI notes that are not in a MIDI File.
	tErrType AddToProgramList( tMidiProgramList *d, U8 bank, U8 program );

	// Add a bank/program/pitch combination to the list of drums used.
	tErrType AddDrumToProgramList( tMidiProgramList *d, U8 bank, U8 program, int pitch );
	tErrType DeleteProgramList(    tMidiProgramList *d );

	// Add all programs and drums used in this song to the list.
	tErrType ScanForPrograms( tMidiPlayerID id, tMidiProgramList *d );
	
	/* Load a set of instruments from a file or an in memory image using a stream.
	If programList is NULL then all instruments in the set will be loaded.
	Otherwise, only the instruments associated with programs and drums in the
	list will be loaded.

	You can call this multiple times. If there is a conflict with an instrument
	with the same bank and program number as a previous file then the most
	recently loaded instrument will be used. */	
	tErrType LoadInstrumentFile( const CPath &path , tMidiProgramList *d );
	
	// All instruments loaded dynamically by LoadInstrumentFile() will be
	// unloaded. Instruments that were compiled with the engine will not be affected.	
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
#define kMIDI_ChannelMessage_NoteOff          0x80
#define kMIDI_ChannelMessage_NoteOn           0x90
#define kMIDI_ChannelMessage_Aftertouch       0xA0
#define kMIDI_ChannelMessage_ControlChange    0xB0
#define kMIDI_ChannelMessage_ProgramChange    0xC0
#define kMIDI_ChannelMessage_ChannelPressure  0xD0
#define kMIDI_ChannelMessage_PitchWheel       0xE0

// Supported MIDI Controller list.  Other controllers are ignored
#define kMIDI_Controller_BankSelect           0x00
#define kMIDI_Controller_ModulationWheel      0x01
#define kMIDI_Controller_DataEntry            0x06
#define kMIDI_Controller_Volume               0x07
#define kMIDI_Controller_PanPosition          0x0A // # 10
#define kMIDI_Controller_Expression           0x0B // # 11
#define kMIDI_Controller_LSBOffset            0x20
#define kMIDI_Controller_Sustain              0x40 // # 64
#define kMIDI_Controller_RPN_LSB              0x64 // #100
#define kMIDI_Controller_RPN_Fine             0x64 // #100
#define kMIDI_Controller_RPN_MSB              0x65 // #101
#define kMIDI_Controller_RPN_Coarse           0x65 // #101

#define kMIDI_Controller_AllSoundOff            120
#define kMIDI_Controller_ResetAllControllers    121
#define kMIDI_Controller_AllNotesOff            123

#define kMIDI_RPN_PitchBendRange            0x0000
#define kMIDI_RPN_MasterFineTuning          0x0001
#define kMIDI_RPN_MasterCoarseTuning        0x0002

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

