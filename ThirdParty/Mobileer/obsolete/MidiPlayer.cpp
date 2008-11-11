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
#include <sys/stat.h>
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

#define kMIDI_EventPriority			128			// asynchronous

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// Local functions
//==============================================================================

namespace
{
	// Local Mobileer callback function to avoid C++ namespace warnings
	void  LocalMetaEventMsg( int trackIndex, 
								int metaEventType, 
								const char* addr,
								int numChars,
								void* userData )
	{
		CMidiPlayer*			pMidiPlayer = reinterpret_cast<CMidiPlayer*>(userData);
		pMidiPlayer->SendMetaEventMsg(trackIndex, metaEventType, addr, numChars, userData);
	}
}

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
// 
// If pInfo is NULL, we simply create the midi player.  Later, we expect
// somebody will call StartMidiFile with a proper pInfo.  If pInfo is not null,
// we call StartMidiFile now.
// ==============================================================================
CMidiPlayer::CMidiPlayer( tAudioStartAudioInfo *pInfo, tAudioID id ) :
	CAudioPlayer( pInfo, id	 )
{
	tErrType			err;
	S16					midiErr;
	Boolean				ret;

	// Setup member variables
	id_					= id;
	framesPerIteration_ = kMIDI_FramesPerIteration;
	channels_			= kMIDI_SamplesPerFrame;
	bitsPerSample_		= kMIDI_BitsPerSample;

	SetPan(	  kPan_Default);
	SetVolume(kVolume_Default);
	
	// Get Debug MPI
	pDebugMPI_ =  new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	pDebugMPI_->Assert((true == ret),
					   "CMidiPlayer::ctor: Couldn't create DebugMPI.\n");

	// Get Debug MPI
	pKernelMPI_ = new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret),
					   "CMidiPlayer::ctor: Couldn't create KernelMPI.\n");

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
	pMidiFileImage = NULL;

	// Mobileer "spmidi" variables
	spMIDI_orchestra_ = NULL;

	// FIXXX: should set Mobileer sampling frequency here
	SPMIDI_SetMaxVoices( pContext_, kMIDI_MaxVoices );

	//Shall we play the file now?
	if(pInfo != NULL) {
		// BC: What if this fails?
		StartMidiFile( pInfo );
	}
}	// ---- end CMidiPlayer() ----

// ==============================================================================
// ~CMidiPlayer
// ==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	tErrType				result;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose,
						 "CMidiPlayer::~ -- Cleaning up.\n");	

	// If a file is playing (the user didn't call stop first) , stop it for them.
	if (pFilePlayer_ && !bIsDone_) 
	{
		//Note: StopMidiFile calls the listener.  The caller of this destructor
		//probably holds the mixer lock and/or the audio MPI lock.  If the
		//listener tries to operate on the mixer or audio mpi, we'll get a
		//deadlock!  Move all callback stuff to Mixer.cpp to avoid this.
		StopMidiFile( true );
		bIsDone_ = true;
	}

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

	// Free MPIs
	if (pKernelMPI_)
		delete pKernelMPI_;

}	// ---- end ~CMidiPlayer() ----

// ==============================================================================
// RewindFile: Seek to beginning of file
// ==============================================================================
void CMidiPlayer::RewindFile()
{
	if (pFilePlayer_)
	{
		MIDIFilePlayer_Rewind( pFilePlayer_ ); 
		bIsDone_ = false;
	}
}

// ==============================================================================
// GetAudioTime_mSec:	Return file position in time (milliSeconds)
// ==============================================================================
U32 CMidiPlayer::GetAudioTime_mSec( void )
{
	U32	time = 0;
	
	if (pFilePlayer_)
	{
		U32 frames = MIDIFilePlayer_GetFrameTime( pFilePlayer_ );
		time = 1000 * frames / kMIDI_SamplingFrequency;
	}
	return time;
}

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

	const tEventPriority	kPriorityTBD = kMIDI_EventPriority;
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

	const tEventPriority	kPriorityTBD = kMIDI_EventPriority;
	tAudioMsgLoopEnd		data;
	data.audioID = id_;			
	data.payload = loopCount_;
	data.count	 = loopCounter_;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}	// ---- end SendLoopEndMsg() ----

// ==============================================================================
// SendMetaEventMsg: Send message to Event listener on MIDI file meta events
// ==============================================================================
void CMidiPlayer::SendMetaEventMsg( int trackIndex, 
									int metaEventType, 
									const char* addr,
									int numChars,
									void* userData )
{
	if (!pListener_)
		return;

	tAudioMsgMidiEvent	data;
	data.midiPlayerID	= id_;			
	data.payload 		= payload_;
	data.trackIndex		= (U8)trackIndex;
	data.metaEventType	= (U8)metaEventType;
	data.addrFileImage	= addr;
	data.numChars		= numChars;
	data.userData		= userData;

	CEventMPI			event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kMIDI_EventPriority, pListener_);
}

// ==============================================================================
// StartMidiFile
// ==============================================================================
tErrType CMidiPlayer::StartMidiFile( tAudioStartAudioInfo *pInfo )
{
	tErrType result = 0;
	int imageSize, bytesRead;
	FILE *file = NULL;
	struct stat fileStat;

	if (pFilePlayer_) 
	{
		return kAudioMidiUnavailable;
	}

	pMidiFileImage = NULL;

	result = stat(pInfo->path->c_str(), &fileStat);
	if(result != 0) {
		result = kAudioMidiErr;
		goto error;
	}
	imageSize = fileStat.st_size;

	pMidiFileImage = (U8 *)pKernelMPI_->Malloc(fileStat.st_size);
	if(pMidiFileImage == 0) {
		result = kAudioMidiErr;
		goto error;
	}
	
	// Load image
	file = fopen(pInfo->path->c_str(), "r" );
	if(!file) {
		result = kAudioMidiErr;
		goto error;
	}

	bytesRead = fread(pMidiFileImage, sizeof(char), fileStat.st_size, file);
	if(bytesRead != fileStat.st_size) {
		result = kAudioMidiErr;
		goto error;
	}

	SetVolume(pInfo->volume);
	SetPan(0);

	pListener_ = pInfo->pListener;

	optionsFlags_ = pInfo->flags;
	shouldLoop_	  = (0 < pInfo->payload) && (0 != (pInfo->flags & kAudioOptionsLooped));
	loopCount_	  = pInfo->payload;
	loopCounter_  = 0;
	bSendDoneMessage_ = ((kNull != pListener_) &&
						 (0 != (pInfo->flags & kAudioOptionsDoneMsgAfterComplete)));
	bSendLoopEndMessage_ = ((kNull != pListener_) &&
							(0 != (pInfo->flags & kAudioOptionsLoopEndMsg)));
	bSendMetaEventMessage_ = ((kNull != pListener_) &&
							(0 != (pInfo->flags & kAudioOptionsMidiEvent)));

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
		err = MIDIFile_ScanForPrograms( programList, pMidiFileImage, imageSize );
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
									pMidiFileImage,
									imageSize );
	bIsDone_ = false;
	
	// Register callback function for MIDI file meta events, if selected
	if (bSendMetaEventMessage_) 
	{
		// Register local callback function to avoid C++ namespace warnings
		MIDIFilePlayer_SetTextCallback( pFilePlayer_, 
				&LocalMetaEventMsg, 
				this );
	}

 error:
	if(file)
		fclose(file);	
	if(result) {
		pKernelMPI_->Free(pMidiFileImage);
		pMidiFileImage = NULL;
	}
	
	return result;
}	// ---- end StartMidiFile() ----

// ==============================================================================
// Pause
// ==============================================================================
void CMidiPlayer::Pause( void )
{	
	for (U8 ch = 0; ch < 16; ch++)
	{
		U8 cmd = 0xB0 | ch;
		SPMIDI_WriteCommand( pContext_, (int)cmd,
							 (int)kMIDI_Controller_AllSoundOff, (int)0 );
		SPMIDI_WriteCommand( pContext_, (int)cmd,
							 (int)kMIDI_Controller_AllNotesOff, (int)0 );
	}
	bPaused_ = true;
}

// ==============================================================================
// Resume
// ==============================================================================
void CMidiPlayer::Resume( void ) 
{
	bPaused_ = false;
}

// ==============================================================================
// StopMidiFile
// ==============================================================================
tErrType CMidiPlayer::StopMidiFile( Boolean noDoneMsg ) 
{
	tErrType result = 0;
		
	if (pListener_	&& !noDoneMsg)
		SendDoneMsg();

	SPMUtil_Reset( pContext_ );
	if (pFilePlayer_)	
	{
		MIDIFilePlayer_Delete( pFilePlayer_ );
		pFilePlayer_ = kNull;
	}
	pListener_ = kNull;

	if(pMidiFileImage) {
		pKernelMPI_->Free(pMidiFileImage);
		pMidiFileImage = NULL;
	}
	
	bIsDone_ = true;

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
	if ( pFilePlayer_ && !bIsDone_ )
	{
		// TODO: make this bulletproof.  Right now no check for sizes.  Figure
		// out how many calls to spmidi we need to make to get a full output
		// buffer
		framesToRead = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / framesToRead;			
		long fileEndReached = false;
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
				bIsDone_ = true;
				if (shouldLoop_) 
				{
					if (loopCounter_++ < loopCount_)
					{
						MIDIFilePlayer_Rewind( pFilePlayer_ ); 
						bIsDone_ = false;
						fileEndReached = false;
						// Send loop end message
						if (bSendLoopEndMessage_)
						{
							SendLoopEndMsg();
						}
					} 
				}
				// File done, delete player and reset engine			  
				if (bIsDone_)
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
	//
	// BUGFIX for programmatic synthesis working in GM Brio 3011:
	// File-based rendering is only effective when pFilePlayer_ object present,
	// otherwise fallback to default Mobileer rendering for programmatic cases.
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