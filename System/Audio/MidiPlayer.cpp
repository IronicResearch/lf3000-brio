//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// MidiPlayer.cpp
//
// Class to manage playing of MIDI data.
//
//==============================================================================

// System includes
#include <pthread.h>
#include <errno.h>
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <MidiPlayer.h>

#include <AudioMPI.h>
#include <EventMPI.h>

#include <EmulationConfig.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define kMIDI_SamplesPerFrame	2
#define kMIDI_BitsPerSample		(sizeof(S16)*8)
#define kMIDI_ChannelCount	16
#define kMIDI_MaxVoices		16

// MIDI Enginer operates at system fs/kMIDI_SamplingFrequencyDivisor
#define kMIDI_SamplingFrequencyDivisor (2)
#define kMIDI_SamplingFrequency		(kAudioSampleRate/kMIDI_SamplingFrequencyDivisor)
#define kMIDI_FramesPerIteration	256

//==============================================================================
// Global variables
//==============================================================================

//============================================================================
// Emulation setup
//============================================================================

//---------------------------------------------------------------------------
CPath GetAppRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetModuleSearchPath();
	return dir;
#else	
	return "/Didj/Base/Brio/Module/";
#endif	// EMULATION
}

// ==============================================================================
// CMidiPlayer implementation
// ==============================================================================
CMidiPlayer::CMidiPlayer( tMidiPlayerID id )
{
	tErrType			err;
	S16					midiErr;
	Boolean				ret;

	// Setup member variables
	id_					= id;
	framesPerIteration_ = kMIDI_FramesPerIteration;
	channels_			= kMIDI_SamplesPerFrame;
	bitsPerSample_		= kMIDI_BitsPerSample;

	pListener_ = kNull;

	SetPan(	  kPan_Default);
	SetVolume(kVolume_Default);
	
	bFilePaused_ = false;
	bFileActive_ = false;
	bActive_	 = false;

	// Get Debug MPI
	pDebugMPI_ =  new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	pDebugMPI_->Assert((true == ret),
					   "CMidiPlayer::ctor: Couldn't create DebugMPI.\n");

	// Set debug level from a constant
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	// Dynamically load the SPMIDI Library (*MUST* be prior to 1st SPMIDI_ call)
	ret = LoadMidiLibrary();
	pDebugMPI_->Assert((true == ret),
					   "CMidiPlayer::ctor: Couldn't load MIDI library.\n");
	
	// Initialize SPMIDI Library
	SPMIDI_Initialize();

	// Start SP-MIDI synthesis engine using the desired sample rate.
	midiErr = SPMIDI_CreateContext( &pContext_, kMIDI_SamplingFrequency);  
	pDebugMPI_->Assert((midiErr == kNoErr),
					   "CMidiPlayer::ctor: SPMIDI_CreateContext() failed.\n");

	// Double MIDI master volume as generally is it too quiet. This risks
	// clipping for multi-voice play.
	SPMIDI_SetMasterVolume( pContext_, SPMIDI_DEFAULT_MASTER_VOLUME );

	// For now only one MIDI player for whole system!  
	pFilePlayer_ = kNull;

	// Mobileer "spmidi" variables
	spMIDI_orchestra_ = NULL;

	// FIXXX: should set Mobileer sampling frequency here
	SPMIDI_SetMaxVoices( pContext_, kMIDI_MaxVoices );

}	// ---- end CMidiPlayer() ----

// ==============================================================================
// ~CMidiPlayer
// ==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	tErrType				result;
	tAudioStopMidiFileInfo	info;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose,
						 "CMidiPlayer::~ -- Cleaning up.\n");	

	// If a file is playing (the user didn't call stop first) , stop it for them.
	if (bFileActive_) 
	{
		//Note: StopMidiFile calls the listener.  The caller of this destructor
		//probably holds the mixer lock and/or the audio MPI lock.  If the
		//listener tries to operate on the mixer or audio mpi, we'll get a
		//deadlock!  Move all callback stuff to Mixer.cpp to avoid this.
		StopMidiFile( &info );
		pDebugMPI_->DebugOut(kDbgLvlVerbose,
							 "CMidiPlayer::~: set bFileActive_ to false\n");	
		bFileActive_ = false;
		info.noDoneMsg = true;
	}
	else
		info.noDoneMsg = !bSendDoneMessage_;

	if (pFilePlayer_)
	{
		MIDIFilePlayer_Delete( pFilePlayer_ );
		pFilePlayer_ = kNull;
	}

	SPMIDI_DeleteContext( pContext_ );
	SPMIDI_Terminate();
	UnloadMidiLibrary();
	
	// Free MPIs
	if (pDebugMPI_)
		delete pDebugMPI_;

}	// ---- end ~CMidiPlayer() ----

// ==============================================================================
// NoteOn
// ==============================================================================
tErrType CMidiPlayer::NoteOn( U8 channel,
							  U8 note,
							  U8 velocity,
							  tAudioOptionsFlags /*flags*/ )
{
 
	SPMUtil_NoteOn( pContext_, (int) channel, (int) note, (int) velocity );

	return (kNoErr);
}	// ---- end NoteOn() ----

// ==============================================================================
// NoteOff
// ==============================================================================
tErrType CMidiPlayer::NoteOff( U8 channel,
							   U8 note,
							   U8 velocity,
							   tAudioOptionsFlags /*flags*/ )
{
 
	SPMUtil_NoteOff( pContext_, (int) channel, (int) note, (int) velocity );

	return (kNoErr);
}	// ---- end NoteOff() ----

// ==============================================================================
// SendCommand
// ==============================================================================
tErrType CMidiPlayer::SendCommand( U8 cmd, U8 data1, U8 data2 )
{
	SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, (int)data2 );

	return (kNoErr);
}	// ---- end SendCommand() ----

// ==============================================================================
// SendDoneMsg
// ==============================================================================
void CMidiPlayer::SendDoneMsg( void ) 
{
	if (!pListener_)
		return;

	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgMidiCompleted	data;
	data.midiPlayerID = id_;
	data.payload	  = loopCount_;	
	data.count		  = 1;
	
	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
	
	// fixme/rdg: need to find a better way to do done listeners support
	// multiple midi files playing at the same time.
}	// ---- end SendDoneMsg() ----

// ==============================================================================
// SendLoopEndMsg:	 Send message to Event listener each time the end of a loop is reached
// ==============================================================================
void CMidiPlayer::SendLoopEndMsg( void )
{
	if (!pListener_)
		return;

	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgLoopEnd		data;
	data.audioID = id_;			
	data.payload = loopCount_;
	data.count	 = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}	// ---- end SendLoopEndMsg() ----

// ==============================================================================
// StartMidiFile
// ==============================================================================
tErrType CMidiPlayer::StartMidiFile( tAudioStartMidiFileInfo *pInfo ) 
{
	tErrType result = 0;

	if (pFilePlayer_) 
	{
		return kAudioMidiUnavailable;
	}

	SetVolume(pInfo->volume);
	SetPan(0);

	if ( pInfo->pListener )
		pListener_ = pInfo->pListener;

	optionsFlags_ = pInfo->flags;
	shouldLoop_	  = (0 < pInfo->payload) && (0 != (pInfo->flags & kAudioOptionsLooped));
	loopCount_	  = pInfo->payload;
	loopCounter_  = 0;
	bSendDoneMessage_ = ((kNull != pListener_) &&
						 (0 != (pInfo->flags & kAudioOptionsDoneMsgAfterComplete)));
	bSendLoopEndMessage_ = ((kNull != pListener_) &&
							(0 != (pInfo->flags & kAudioOptionsLoopEndMsg)));

	// ----Selectively load instruments
	{
		int err;
		int preOrchestraCount = 0;

		StreamIO *sio = NULL;
	
		SPMIDI_ProgramList *programList =  NULL;
		CPath orchestraFileName;

		orchestraFileName = GetAppRsrcFolder() + "orchestra.mbis";

		err = SPMIDI_CreateProgramList( &programList );
		if( err < 0 ) 
			printf("CMidiPlayer::StartMidiFile: SPMIDI_CreateProgramList failed\n");

		// Scan the MIDIFile to see what instruments we should load.
		err = MIDIFile_ScanForPrograms( programList,
										pInfo->pMidiFileImage,
										pInfo->imageSize );
		if( err < 0 ) 
			printf("CMidiPlayer::StartMidiFile: MIDIFile_ScanForPrograms failed\n");

		// Load an Orchestra file into a memory stream and parse it.
		sio = Stream_OpenFile( (char *)orchestraFileName.c_str(), "rb");
		if ( NULL == sio )
			printf("CMidiPlayer::StartMidiFile: Stream_OpenFile failed on '%s'\n",
				   (char *)orchestraFileName.c_str());
		else
		{
			preOrchestraCount = SPMIDI_GetMemoryAllocationCount();
			// Scan MIDI file and load minimal set of the instruments 
			if ( SPMIDI_LoadOrchestra( sio, programList, &spMIDI_orchestra_ ) < 0 )
				printf( "CMidiPlayer::StartMidiFile: SPMIDI_LoadOrchestra failed on '%s'\n",
						(char *)orchestraFileName.c_str());
	
			// Close orchestra stream
			Stream_Close( sio );
		}
		SPMIDI_DeleteProgramList( programList );
	}

	// Create a player, parse MIDIFile image and setup tracks.
	result = MIDIFilePlayer_Create( &pFilePlayer_,
									(int)kMIDI_SamplingFrequency,
									pInfo->pMidiFileImage,
									pInfo->imageSize );
	bFileActive_ = true;
	
	return result;
}	// ---- end StartMidiFile() ----

// ==============================================================================
// PauseMidiFile
// ==============================================================================
tErrType CMidiPlayer::PauseMidiFile( void )
{
	if (bFileActive_)
	{
		bFilePaused_ = true;
		
		for (U8 ch = 0; ch < 16; ch++)
		{
			U8 cmd = 0xB0 | ch;
			SPMIDI_WriteCommand( pContext_, (int)cmd,
								 (int)kMIDI_Controller_AllSoundOff, (int)0 );
			SPMIDI_WriteCommand( pContext_, (int)cmd,
								 (int)kMIDI_Controller_AllNotesOff, (int)0 );
		}
	}
	
	return (0);
}	// ---- end PauseMidiFile() ----

// ==============================================================================
// ResumeMidiFile
// ==============================================================================
tErrType CMidiPlayer::ResumeMidiFile( void ) 
{
	if (bFileActive_)
		bFilePaused_ = false; 

	return (0); //result;
}	// ---- end ResumeMidiFile() ----

// ==============================================================================
// StopMidiFile
// ==============================================================================
tErrType CMidiPlayer::StopMidiFile( tAudioStopMidiFileInfo* pInfo ) 
{
	tErrType result = 0;
		
	bFileActive_ = false;
	bFilePaused_ = false;
	if (pListener_	&& !pInfo->noDoneMsg)
		SendDoneMsg();

	SPMUtil_Reset( pContext_ );
	if (pFilePlayer_)	
	{
		MIDIFilePlayer_Delete( pFilePlayer_ );
		pFilePlayer_ = kNull;
	}
	pListener_ = kNull;
	
	return result;
}	// ---- end StopMidiFile() ----

// ==============================================================================
// GetEnableTracks
// ==============================================================================
tErrType CMidiPlayer::GetEnableTracks( tMidiTrackBitMask *d ) 
{
	tMidiTrackBitMask	mask = 0;

	// Loop through channels and set bits
	for (U32 chan = 0; chan < kMIDI_ChannelCount; chan++) 
	{
		if (SPMIDI_GetChannelEnable( pContext_, chan ))
			mask |= 1 << chan;
	}
	*d = mask;

	return kNoErr;
}	// ---- end GetEnableTracks() ----

// ==============================================================================
// SetEnableTracks: 
// ==============================================================================
tErrType CMidiPlayer::SetEnableTracks( tMidiTrackBitMask d )
{

	// Loop through channels and set mask
	for (U32 ch = 0; ch < kMIDI_ChannelCount; ch++) 
		SPMIDI_SetChannelEnable( pContext_, ch, 0 != (d & (1 << ch)) );

	return kNoErr;
}	// ---- end SetEnableTracks() ----

// ==============================================================================
// TransposeTracks:	  
// ==============================================================================
tErrType CMidiPlayer::TransposeTracks( tMidiTrackBitMask trackBits, S8 semitones)
{
	// Global MIDI transpose: affects all MIDI channels

	// MIDI standard COARSE TUNING RPN = #2 to transpose by semitones by channel
	for (long ch = 0; ch < 16; ch++)
	{
		if (trackBits & (1 << ch))
		{
			SPMUtil_ControlChange(pContext_, ch, kMIDI_Controller_RPN_MSB, 0 ); 
			SPMUtil_ControlChange(pContext_, ch, kMIDI_Controller_RPN_LSB,
								  kMIDI_RPN_MasterCoarseTuning ); 
			
			// Set coarse tuning in Data Entry MSB, where A440 tuning = 0x40
			SPMUtil_ControlChange( pContext_, ch, kMIDI_Controller_DataEntry,
								   (semitones + 0x40)&0x7F ); 

			//	Fine tuning (Cents) currently ignored by Mobileer engine 
		}
	}
	
	return (kNoErr);
}	// ---- end TransposeTracks() ----

// ==============================================================================
// ChangeProgram:		Change program (instrument) on specified MIDI channel
//
//				FIXXXX: this API is non-sensical.  Add another call that takes 
//				channel # as argument.
// ==============================================================================
tErrType CMidiPlayer::ChangeProgram( tMidiTrackBitMask trackBits,
									 tMidiPlayerInstrument number )
{
	for (long ch = 0; ch < 16; ch++)	
	{
		if (trackBits & (1 << ch))
		{
			// Send two byte MIDI program change
			U8 cmd	 = kMIDI_ChannelMessage_ProgramChange | (0x7f & ch);
			U8 data1 = (U8)(0x7f & number);
			SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, 0 );
		}
	}

	// GK FIXXXX: Need extra code as minimal instrument set is currently loaded,
	// so you have to load those not in memory

	return (kNoErr);
}	// ---- end ChangeProgram() ----

// ==============================================================================
// ChangeTempo:	   Scale rate of MIDI file play with S8 number
//					Current mapping is [-128 .. 127] to [1/16x .. 16x]
//		NOTE:  yes, this is not a great mapping of 256 values, but it's easy to
//		hit the key values of 1/16, 1/8, 1/4, 1/2, 2, 4, 8, 16 x
// ==============================================================================
tErrType CMidiPlayer::ChangeTempo( S8 tempoScale )
{
	if (tempoScale < -127)
		tempoScale = -127;

	// Map Brio tempo range to 16.16 tempo scaler
	// Change Range of [-128 .. 127] to [1/4 .. 4]
	float xf = ChangeRangef((float)tempoScale, -127.0f, 127.0f, -4.0f, 4.0f);
	float yf = powf(2.0f, xf);
	// Convert to 16.16 range
	U32 scaler_16d16 = (U32)(65536.0f*yf);

	if (pFilePlayer_)
		MIDIFilePlayer_SetTempoScaler( pFilePlayer_, scaler_16d16);

	return (kNoErr);
}	// ---- end ChangeTempo() ----

// ==============================================================================
// Render:	  Render MIDI stream and/or MIDI file to audio samples
//
//			FIXXXX: Return something.  Number of frames rendered from MIDI file,
//					not total frames rendered, which can be zero-padded
// ==============================================================================
U32 CMidiPlayer::Render( S16* pOut, U32 numStereoFrames )
{
	tErrType result;
	U32		i, midiLoopCount;
	int		mfp_result = 0;
	U32		framesToRead;
	U32		framesRead = 0;
	U32		numStereoSamples = numStereoFrames * 2;
	
	// Clear output buffer
	// FIXXXX: move to DSP loop and clear only if needed
	bzero( pOut, kAudioOutBufSizeInBytes );
	
	//
	// -------- Render MIDI file
	//
	S16 *pBuf = pOut;
	if ( bFileActive_ && !bFilePaused_ ) 
	{
		// TODO: make this bulletproof.  Right now no check for sizes.  Figure
		// out how many calls to spmidi we need to make to get a full output
		// buffer
		framesToRead = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / framesToRead;			
		long fileEndReached = False;
		for (i = 0; i < midiLoopCount && !fileEndReached; i++) 
		{
			if (pFilePlayer_)
			{
				mfp_result = MIDIFilePlayer_PlayFrames( pFilePlayer_,
														pContext_,
														framesToRead );
				if (mfp_result < 0)
					printf("!!!!! CMidiPlayer::Render: MIDIFilePlayer_PlayFrames failed!!!\n");

				// fixme/dg: rationalize numStereoFrames and numFrames_!!
				framesRead = SPMIDI_ReadFrames( pContext_,
												pBuf,
												framesToRead,
												channels_,
												bitsPerSample_ );
	
				pBuf += framesRead * kMIDI_SamplesPerFrame;
			}
			
			// Returning 1 means MIDI file done.
			fileEndReached = (mfp_result > 0);
			if (fileEndReached)
			{
				bFileActive_ = false;
				if (shouldLoop_) 
				{
					if (loopCounter_++ < loopCount_)
					{
						MIDIFilePlayer_Rewind( pFilePlayer_ ); 
						bFileActive_   = true;
						fileEndReached = false;
						// Send loop end message
						if (bSendLoopEndMessage_)
						{
							SendLoopEndMsg();
						}
					} 
				} 
				// File done, delete player and reset engine			  
				if (!bFileActive_)
				{
					if (pFilePlayer_)
					{
						MIDIFilePlayer_Delete( pFilePlayer_ );
						pFilePlayer_ = kNull;
					}
					if (bSendDoneMessage_)
						SendDoneMsg();
					SPMUtil_Reset( pContext_ );
				}
			}
		}
	} 
#define INCLUDE_PROGRAMMATIC_MIDI
#ifdef INCLUDE_PROGRAMMATIC_MIDI
	// 
	// Add MIDI output from programmatic interface to output
	//	GK FIXXXX: note currently not added but mutually exclusive
	else 
	{
		// Calculate # of calls to get full output buffer
		framesToRead = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / framesToRead;
		for (i = 0; i < midiLoopCount; i++) 
		{
			framesRead = SPMIDI_ReadFrames(pContext_, pBuf, framesToRead,
										   channels_, bitsPerSample_);
			pBuf += framesRead * kMIDI_SamplesPerFrame;
		}
	}
#endif // INCLUDE_PROGRAMMATIC_MIDI

	// Pan and scale MIDI output and mix into output buffer
	// Mobileer engine output is stereo, but we may want mono for stereo panning
	U32 numFrames = numStereoSamples;
	for ( i = 0; i < numFrames; i += 2)
	{
		// Apply gain and Saturate to 16-bit range
		S32 y = (S32) MultQ15(levelsi[kLeft], pOut[i]);	// Q15	1.15 Fixed-point
		// Convert mono midi render to stereo output  GK: ?
		pOut[i] = (S16) y;

		// Apply gain and Saturate to 16-bit range
		y = (S32) MultQ15(levelsi[kRight], pOut[i+1]);	// Q15	1.15 Fixed-point		
		// Convert mono midi render to stereo output  GK: ?
		pOut[i+1] = (S16) y;
	}

	return (framesRead);
}	// ---- end Render() ----

// ==============================================================================
// SetPan :		Channel stereo position	  Left .. Center .. Right
// ==============================================================================
void CMidiPlayer::SetPan( S8 x )
{
	pan_ = BoundS8(&x, kPan_Min, kPan_Max);

	// Convert input range range to [0 .. 1] suitable
	float xf = ChangeRangef((float)pan_, (float) kPan_Min, (float)kPan_Max, 0.0f, 1.0f);
	ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
	if (panValuesf[kLeft] > -1e-06 && panValuesf[kLeft] < 1e-06)
		panValuesf[kLeft] = 0.0f;
	if (panValuesf[kRight] > -1e-06 && panValuesf[kRight] < 1e-06)
		panValuesf[kRight] = 0.0f;
	
	RecalculateLevels();
}	// ---- end SetPan ----

// ==============================================================================
// SetVolume : Convert Brio volume range to DSP values
// ==============================================================================
void CMidiPlayer::SetVolume( U8 x )
{
	volume_ = BoundU8(&x, kVolume_Min, kVolume_Max);

	// FIXX: move to decibels, but for now, linear volume
	gainf = ChangeRangef((float)x, (float) kVolume_Min, (float)kVolume_Max, 0.0f, 1.0f);
	gainf *= kChannel_Headroomf; // DecibelToLinearf(-Channel_HeadroomDB);

	RecalculateLevels();
}	// ---- end SetVolume ----

// ==============================================================================
// RecalculateLevels : Recalculate levels when either pan or volume changes
// ==============================================================================
void CMidiPlayer::RecalculateLevels()
{
	levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
	levelsf[kRight] =  panValuesf[kRight]*gainf;

	// Convert 32-bit floating-point to Q15 fractional integer format 
	levelsi[kLeft ] = FloatToQ15(levelsf[kLeft ]);
	levelsi[kRight] = FloatToQ15(levelsf[kRight]);

}	// ---- end RecalculateLevels ----

LF_END_BRIO_NAMESPACE()
