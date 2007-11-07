//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reservedspmidi
//==============================================================================
//
// File:
//		MidiPlayer.cpp
//
// Description:
//		The class to manage the playing of Midi data.
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
#include <EventMPI.h>

#include <EmulationConfig.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define kMIDI_SamplesPerFrame   2
#define kMIDI_BitsPerSample     (sizeof(S16)*8)
#define kMIDI_ChannelCount	16
#define kMIDI_MaxVoices     16

// MIDI Enginer operates at system fs/kMIDI_SamplingFrequencyDivisor
#define kMIDI_SamplingFrequencyDivisor (2)
#define kMIDI_SamplingFrequency     (kAudioSampleRate/kMIDI_SamplingFrequencyDivisor)
#define kMIDI_FramesPerIteration    256

#define kPan_Default    0
#define kPan_Min    (-100)
#define kPan_Max      100

#define kVolume_Default  100
#define kVolume_Min        0
#define kVolume_Max      100

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
//	return "/Didj/Data/rsrc/"; //MIDI";
	return "/Didj/Base/Brio/Module/";
#endif	// EMULATION
}

//==============================================================================
// CMidiPlayer implementation
//==============================================================================

CMidiPlayer::CMidiPlayer( tMidiPlayerID id )
{
//{static long c=0; printf("CMidiPlayer::CMidiPlayer: start %ld\n", c++);}

	tErrType			err;
	S16					midiErr;
	Boolean				ret;
	const tMutexAttr 	attr = {0};
	
	// Setup member vars...
	id_ = id;
	
	pMidiRenderBuffer_ = new S16[kAudioOutBufSizeInWords];

	framesPerIteration_ = kMIDI_FramesPerIteration;	// FIXXX/rdg: don't hardcode this 
	samplesPerFrame_    = kMIDI_SamplesPerFrame;
	bitsPerSample_      = kMIDI_BitsPerSample;

	pListener_ = kNull;

	SetPan(kPan_Default);
	SetVolume(kVolume_Default);
    
	bFilePaused_ = false;
	bFileActive_ = false;
	bActive_     = false;

//	shouldLoop_  = (0 < payload_) && (0 != (optionsFlags_ & kAudioOptionsLooped));
//    loopCount_   = payload_;
//    loopCounter_ = 0;

	// Get Debug MPI
	pDebugMPI_ =  new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CAudioModule::ctor: Couldn't create DebugMPI.\n");

	// Set debug level from a constant
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CAudioModule::ctor: Couldn't create KernelMPI.\n");

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::ctor -- Creating new MIDI player.\n");	
	
	// Setup Mutex object for protecting render calls
  	err = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Couldn't init mutex.\n");
	
	// Initialize SPMIDI Library
	SPMIDI_Initialize();

	// Start SP-MIDI synthesis engine using the desired sample rate.
	midiErr = SPMIDI_CreateContext( &pContext_, kMIDI_SamplingFrequency);  
	pDebugMPI_->Assert((midiErr == kNoErr), "CAudioModule::ctor: SPMIDI_CreateContext() failed.\n");

	// For now only one MIDI player for whole system!  
	pFilePlayer_ = kNull;

// Mobileer "spmidi" variables
	spmidi_orchestra_ = NULL;
// FIXXX: should set Mobileer sampling frequency here
	
	SPMIDI_SetMaxVoices( pContext_, kMIDI_MaxVoices );
}   // ---- end CMidiPlayer() ----

//==============================================================================
// ~CMidiPlayer
//==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	tErrType 				result;
	tAudioStopMidiFileInfo 	info;
	
{static long c=0; printf("CMidiPlayer::~CMidiPlayer: start %ld\n", c++);}

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::dtor -- Cleaning up.\n");	
	
	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::dtor -- Couldn't lock mutex.\n");

	// If a file is playing (the user didn't call stop first) do it for them.
	if (bFileActive_) {
		pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CMidiPlayer::dtor -- setting bFileActive_ to false...\n");	
		bFileActive_ = false;
		info.suppressDoneMsg = true;
		StopMidiFile( &info );
	}
	
	if (pFilePlayer_)
        {
		MIDIFilePlayer_Delete( pFilePlayer_ );
        pFilePlayer_ = kNull;
        }

	SPMIDI_DeleteContext( pContext_ );
	SPMIDI_Terminate();
	
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::dtor --  Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::dtor --  Couldn't deinit mutex.\n");

	delete pMidiRenderBuffer_;

	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;

	// Free kernel MPI
	if (pKernelMPI_)
		delete pKernelMPI_;
{static long c=0; printf("CMidiPlayer::~CMidiPlayer: end %ld\n", c++);}
}   // ---- end ~CMidiPlayer() ----

//==============================================================================
// NoteOn
//==============================================================================
tErrType 	CMidiPlayer::NoteOn( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CMidiPlayer::NoteOn: chan=%d note=%d vel=%d, flags=$%X\n", 
                channel, noteNum, velocity, static_cast<int>(flags) );
 
	SPMUtil_NoteOn( pContext_, (int) channel, (int) noteNum, (int) velocity );

	return kNoErr;
}   // ---- end NoteOn() ----

//==============================================================================
// NoteOff
//==============================================================================
tErrType 	CMidiPlayer::NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::NoteOff: chan=%d note=%d vel=%d flags=$%X\n", 
            channel, noteNum, velocity, static_cast<int>(flags) );
 
	SPMUtil_NoteOff( pContext_, (int) channel, (int) noteNum, (int) velocity );

	return kNoErr;
}   // ---- end NoteOff() ----

//==============================================================================
// SendCommand
//==============================================================================
tErrType CMidiPlayer::SendCommand( U8 cmd, U8 data1, U8 data2 )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::SendCommand -- ...\n");	

	SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, (int)data2 );

	return kNoErr;
}   // ---- end SendCommand() ----

// ==============================================================================
// SendDoneMsg
// ==============================================================================
void CMidiPlayer::SendDoneMsg( void ) 
{
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgMidiCompleted	data;
	data.midiPlayerID = id_;
	data.payload = 0;	
	data.count = 1;

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::SendDoneMsg -- Sending done event.\n");	
printf("CMidiPlayer::SendDoneMsg \n");

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
	
	// fixme/dg: need to find a better way to do done listeners
	// support multiple midi files playing at the same time.
	//pListener_ = kNull;
}   // ---- end SendDoneMsg() ----

//==============================================================================
// StartMidiFile
//==============================================================================
tErrType 	CMidiPlayer::StartMidiFile( tAudioStartMidiFileInfo* 	pInfo ) 
{
//{static long c=0; printf("CMidiPlayer::StartMidiFile: start %ld\n", c++);}

tErrType result = 0;

//pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::StartMidiFile: Start...\n");	

// If pre-emption is happening, delete previous player.
if (pFilePlayer_) 
    {
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StartMidiFile -- deleting active player...\n");	
	MIDIFilePlayer_Delete( pFilePlayer_ );
	pFilePlayer_ = kNull;
    }

//	pInfo->midiID;			// fixme/dg: midiEngineContext object?

SetVolume(pInfo->volume);
SetPan(0); //pInfo->pan);

if ( pInfo->pListener )
	pListener_ = pInfo->pListener;

	shouldLoop_  = (0 < pInfo->payload) && (0 != (pInfo->flags & kAudioOptionsLooped));
    loopCount_   = pInfo->payload;
    loopCounter_ = 0;

// kAudioOptionsLooped
//printf("CMidiPlayer::StartMidiFile: loopFile_=%d loopCount=%ld flags=$%X kAudioOptionsLooped=$%X\n",
//        shouldLoop_, loopCount_, (unsigned int)pInfo->flags, (unsigned int)kAudioOptionsLooped);

//
// ----Selectively load instruments
//
{
	int err;
//	void *orchestraImage = NULL;
//	int  orchestraFileSize;
	int preOrchestraCount = 0;

	StreamIO *sio = NULL;
	
	SPMIDI_ProgramList *programList =  NULL;
    CPath orchestraFileName;

// orch_100207.mbis , goofy_test.mbis
//#define GK_ORCHESTRALOAD_TEST
#ifdef GK_ORCHESTRALOAD_TEST
    orchestraFileName = "/home/lfu/workspace/Brio2/ThirdParty/Mobileer/Libs/orch_100207.mbis";
//"/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/apprsrc/orch_100207.mbis"; 
#else
// orchestra.mbis, goofy_test.mbis
    orchestraFileName = GetAppRsrcFolder() + "orchestra.mbis";
#endif
//orchestraFileName = "/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/apprsrc/orch_100207.mbis"; 
//orchestraFileName = "/Didj/Base/Brio/rsrc/orch_100207.mbis"; 
//printf("orchestraFileName = '%s'\n", orchestraFileName.c_str());

// "/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/apprsrc/orchestra_071002.mbis"; // 358,796 Bytes
// "/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/apprsrc/orch_100207.mbis";
// "D:\\mobileer_work\\A_Orchestra\\exports\\exported.mbis";

//	printf( "Num blocks allocated before SPMIDI_CreateProgramList = %d\n", SPMIDI_GetMemoryAllocationCount() );
	err = SPMIDI_CreateProgramList( &programList );
	if( err < 0 ) 
		printf("SPMIDI_CreateProgramList failed\n");

	// Scan the MIDIFile to see what instruments we should load.
	err = MIDIFile_ScanForPrograms( programList, pInfo->pMidiFileImage, pInfo->imageSize );
	if( err < 0 ) 
		printf("MIDIFile_ScanForPrograms failed\n");

	// Load an Orchestra file into a memory stream and parse it.
//	orchestraImage = SPMUtil_LoadFileImage( (char *)orchestraFileName.c_str(), (int *)&( orchestraFileSize ) );
//	if( orchestraImage == NULL )
//		printf("Error: can't open orchestra file '%s'\n", (char *)orchestraFileName.c_str());
//    else 
//        printf("CMidiPlayer::StartMidiFile: loaded orchestraFile='%s'\n", orchestraFileName.c_str());

	// Create a stream object for reading the orchestra.
//	sio = Stream_OpenImage( (char *)orchestraImage, orchestraFileSize );
	sio = Stream_OpenFile( (char *)orchestraFileName.c_str(), "rb");
	if( sio == NULL )
		printf("Stream_OpenFile failed on '%s'\n", (char *)orchestraFileName.c_str());

//	printf( "Num blocks allocated before SPMIDI_LoadOrchestra = %d\n", SPMIDI_GetMemoryAllocationCount() );

	preOrchestraCount = SPMIDI_GetMemoryAllocationCount();
	// Load just the instruments we need from the orchestra t play the song.
	if( SPMIDI_LoadOrchestra( sio, programList, &spmidi_orchestra_ ) < 0 )
		printf( "SPMIDI_LoadOrchestra failed on '%s'\n", (char *)orchestraFileName.c_str());
	
	// Close the orchestra stream.
	if( sio != NULL )
		Stream_Close( sio );
//	free( orchestraImage );  // FIXXXX: Is this a leak ?

//	printf( "Num blocks allocated before SPMIDI_DeleteOrchestra = %d\n", SPMIDI_GetMemoryAllocationCount() );
//	TOO SOON SPMIDI_DeleteOrchestra( spmidi_orchestra_ );
//	printf( "Num blocks allocated after SPMIDI_DeleteOrchestra = %d\n", SPMIDI_GetMemoryAllocationCount() );

	SPMIDI_DeleteProgramList( programList );
//	printf( "Final num blocks after SPMIDI_DeleteProgramList = %d\n", SPMIDI_GetMemoryAllocationCount() );
}
// **************************************************************************************
// **************************************************************************************

	// Create a player, parse MIDIFile image and setup tracks.
	result = MIDIFilePlayer_Create( &pFilePlayer_, (int)kMIDI_SamplingFrequency, pInfo->pMidiFileImage, pInfo->imageSize );
//    if( result < 0 )
//        printf("CMidiPlayer::StartMidiFile: Couldn't create a midifileplayer!\n");
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StartMidiFile -- setting bFileActive_ to true...\n");	
	
	bFileActive_ = true;
	
	return result;
}   // ---- end StartMidiFile() ----

// ==============================================================================
// PauseMidiFile
// ==============================================================================
tErrType 	CMidiPlayer::PauseMidiFile( void ) 
{
	tErrType result = 0;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::PauseMidiFile -- setting bFilePaused_ to true...\n");	
	
	if (bFileActive_)
		bFilePaused_ = true;
	
	return result;
}   // ---- end PauseMidiFile() ----

//==============================================================================
// ResumeMidiFile
//==============================================================================
tErrType 	CMidiPlayer::ResumeMidiFile( void ) 
{
	tErrType result = 0;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::ResumeMidiFile -- setting bFilePaused_ to false...\n");	

	if (bFileActive_)
		bFilePaused_ = false; 
	
	return result;
}   // ---- end ResumeMidiFile() ----

//==============================================================================
// StopMidiFile
//==============================================================================
tErrType 	CMidiPlayer::StopMidiFile( tAudioStopMidiFileInfo* pInfo ) 
{
	tErrType result = 0;
//{static long c=0; printf("CMidiPlayer::StopMidiFile: start %ld\n", c++);}
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StopMidiFile -- Entering method, about to lock render_mutex...\n");	

	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::StopMidiFile -- Couldn't lock mutex.\n");

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StopMidiFile -- setting bFilePaused_ and bFileActive_ to false...\n");	

	bFileActive_ = false;
	bFilePaused_ = false;
	if (pListener_  && !pInfo->suppressDoneMsg)
		SendDoneMsg();

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StopMidiFile -- resetting engine and deleting player...\n");	

	SPMUtil_Reset( pContext_ );
    if (pFilePlayer_)   
        {
    	MIDIFilePlayer_Delete( pFilePlayer_ );
    	pFilePlayer_ = kNull;
        }

	pListener_ = kNull;
	
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::StopMidiFile -- Couldn't unlock mutex.\n");

// SPMIDI_DeleteOrchestra( orchestra );

	return result;
}   // ---- end StopMidiFile() ----

//==============================================================================
// GetEnableTracks
//==============================================================================
tErrType CMidiPlayer::GetEnableTracks( tMidiTrackBitMask* d ) 
{
	U32					chan;
	tMidiTrackBitMask	mask = 0;
	int					isEnabled;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::GetEnableTracks -- ...\n");	

	// Loop through the channels and set mask
	for (chan = 0; chan < kMIDI_ChannelCount; chan++) {
		isEnabled = SPMIDI_GetChannelEnable( pContext_, chan );
		if (isEnabled){
			mask |= 1 << chan;
		}
	}

*d = mask;

return kNoErr;
}   // ---- end GetEnableTracks() ----

// ==============================================================================
// SetEnableTracks: 
// ==============================================================================
#define kMIDI_TrackEnable		1
#define kMIDI_TrackDisable		0
tErrType CMidiPlayer::SetEnableTracks( tMidiTrackBitMask d )
{
pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::SetEnableTracks -- PROBABLY DOESN'T WORK ...\n");	
printf("CMidiPlayer::SetEnableTracks -- PROBABLY DOESN'T WORK ...\n");	

// Loop through channels and set mask
// FIXXXXXXXXX: rdg's below code probably doesn't work:  GK NOTE
for (U32 chan = 0; chan < kMIDI_ChannelCount; chan++) 
    {
	tMidiTrackBitMask mask = 1 << chan;		// set bit corresponding to this channel
	if (d && mask)  // WHAT ?!?
		SPMIDI_SetChannelEnable( pContext_, chan, kMIDI_TrackEnable );
	else
		SPMIDI_SetChannelEnable( pContext_, chan, kMIDI_TrackDisable );
	}

return kNoErr;
}   // ---- end SetEnableTracks() ----

// ==============================================================================
// TransposeTracks:    NOT IMPLEMENTED
// ==============================================================================
tErrType CMidiPlayer::TransposeTracks( tMidiTrackBitMask /* d */, S8 /* transposeAmount */)
{
pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::TransposeTracks -- NOT IMPLEMENTED...\n");	
printf("CMidiPlayer::TransposeTracks -- NOT IMPLEMENTED...\n");	

	return kNoErr;
}   // ---- end TransposeTracks() ----

// ==============================================================================
// ChangeProgram: NOT IMPLEMENTED
// ==============================================================================
tErrType CMidiPlayer::ChangeProgram( tMidiTrackBitMask /* d */, tMidiInstr /* instr */ )
{
pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::ChangeProgram -- NOT IMPLEMENTED...\n");	
printf("CMidiPlayer::ChangeProgram -- NOT IMPLEMENTED...\n");	
	return kNoErr;
}   // ---- end ChangeProgram() ----

// ==============================================================================
// ChangeTempo
// ==============================================================================
tErrType CMidiPlayer::ChangeTempo( S8 /* Tempo */)
{
pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::ChangeTempo not implemented ...\n");	
printf("CMidiPlayer::ChangeTempo -- NOT IMPLEMENTED...\n");	

	return kNoErr;
}   // ---- end ChangeTempo() ----

// ==============================================================================
// RenderBuffer:    Render MIDI file to audio samples
//
//          FIXXXX: Return something.  Number of frames rendered from MIDI file,
//                  not total frames rendered, which can be zero-padded
// ==============================================================================
U32	CMidiPlayer::RenderBuffer( S16* pOutBuff, U32 numStereoFrames, long addToOutput )
{
	tErrType result;
	U32 	i, midiLoopCount;
	int 	mfp_result = 0;
	U32		spmidiFramesPerBuffer;
	U32 	framesRead = 0;
	U32 	numStereoSamples = numStereoFrames * kAudioBytesPerSample;

//    U32     numFrames = numSamples/2;

//{static long c=0; printf("CMidiPlayer::RenderBuffer: %ld start numFrames=%ld numSamples=%ld kAudioOutBufSizeInBytes=%d\n", c++, numFrames, numSamples, kAudioOutBufSizeInBytes);}

	result = pKernelMPI_->TryLockMutex( render_mutex_ );
	if (EBUSY == result)
		return 0;
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::RenderBuffer -- Couldn't lock mutex.\n");
	
	// Clear output buffer
// FIXXXX: move to DSP loop and clear only if needed
	bzero( pMidiRenderBuffer_, kAudioOutBufSizeInBytes );
	
//	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//		"CMidiPlayer::RenderBuffer -- numFrames=%d, bFileActive=%u, bFilePaused_=%d\n", 
//		(int)numFrames, (int)bFileActive_, (int)bFilePaused_);
//{printf("CMidiPlayer::RenderBuffer: here 111\n");}

	S16 *pBuffer = pMidiRenderBuffer_;  // local ptr to buffer for indexing

	// If there is a midi file player, service it
	if ( bFileActive_ && !bFilePaused_ ) {
		// TODO: make this bulletproof.  Right now no check for sizes.
		// Figure out how many calls to spmidi we need to make to get a full output buffer
		spmidiFramesPerBuffer = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / spmidiFramesPerBuffer;
//printf("222 spmidiFramesPerBuffer=%ld midiLoopCount=%ld spmidiFramesPerBuffer=%ld numFrames=%ld\n", 
//        spmidiFramesPerBuffer, midiLoopCount, spmidiFramesPerBuffer, numFrames);

//		pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//			"CMidiPlayer::RenderBuffer -- spmidiFramesPerBuffer = %d, midiLoopCount = %d\n", 
//			(int)spmidiFramesPerBuffer, (int)midiLoopCount );
			
		for (i = 0; i < midiLoopCount; i++) {
//			pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::RenderBuffer: BEFO MIDIFilePlayer_PlayFrames()\n");
//{printf("CMidiPlayer::RenderBuffer: 333 BEFO MIDIFilePlayer_PlayFrames\n");}
            if (pFilePlayer_)
                {
			    mfp_result = MIDIFilePlayer_PlayFrames( pFilePlayer_, pContext_, spmidiFramesPerBuffer );
//{printf("CMidiPlayer::RenderBuffer: 333 AFTA MIDIFilePlayer_PlayFrames\n");}
			    if (mfp_result < 0)
				    printf("!!!!! CMidiPlayer::RenderBuffer -- MIDIFilePlayer PlayFrames failed!!!\n");

			// fixme/dg: rationalize numStereoFrames and numFrames_!!
//			pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//				"CMidiPlayer::RenderBuffer -- About to SPMIDI_ReadFrames()\n");
			framesRead = SPMIDI_ReadFrames( pContext_, pBuffer, spmidiFramesPerBuffer,
		    		samplesPerFrame_, bitsPerSample_ );
	
			pBuffer += framesRead *kMIDI_SamplesPerFrame;
// 			printf("CMidiPlayer::RenderBuffer -- loop %d, framesRead = %ul, pBuffer = %d\n", i, framesRead, pBuffer);
	            }
//            else
//                mfp_result = 1;
//{printf("CMidiPlayer::RenderBuffer: here 444 loopMidiFile_=%d\n", loopMidiFile_);}
			
			// Returning a 1 means MIDI file done.
			if (mfp_result > 0)
             {
//{static long c=0; printf("CMidiPlayer::RenderBuffer: %ld file done, looping=%d\n",c++, loopFile_);}
				if (shouldLoop_) 
                    {
					pDebugMPI_->DebugOut(kDbgLvlVerbose," CMidiPlayer::RenderBuffer: file done, looping...\n");	

{static long c=0; printf("CMidiPlayer::RenderBuffer: Rewind %ld loop%ld/%ld\n", c++, loopCount_, loopCounter_);}
bFileActive_ = false;
            if (++loopCounter_ < loopCount_)
                {
			    MIDIFilePlayer_Rewind( pFilePlayer_ ); 
                bFileActive_ = true;
                }
				    } 
                else 
                    {
//printf("CMidiPlayer::RenderBuffer -- file done, deleting player and resetting engine\n");	                

bFileActive_ = false;
					if (pListener_ ) //&& bDoneMessage_)
						SendDoneMsg();
//{printf("CMidiPlayer::RenderBuffer: here 3 PRE pFilePlayer_=%ld\n", (U32) pFilePlayer_);}
                    if (pFilePlayer_)
                        {
					    MIDIFilePlayer_Delete( pFilePlayer_ );
    					pFilePlayer_ = kNull;
                        }
//{printf("CMidiPlayer::RenderBuffer: here 3 POST \n");}
					SPMUtil_Reset( pContext_ );
					break; // bail out of loop.
				}
			}
		}
	} else {
		// A midi file is not playing, but notes might be turned on programatically...
		// TODO: rationalize numStereoFrames and numFrames_!!
//		pDebugMPI_->DebugOut(kAudioDebugLevel, 
//			"CMidiPlayer::RenderBuffer -- About to SPMIDI_ReadFrames(), no file active\n");

		// TODO: make this bulletproof.  Right now no check for sizes.
		// Figure out how many calls to spmidi we need to make to get a full output buffer
		spmidiFramesPerBuffer = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / spmidiFramesPerBuffer;
//printf("222 spmidiFramesPerBuffer=%ld midiLoopCount=%ld\n", spmidiFramesPerBuffer, midiLoopCount);

		for (i = 0; i < midiLoopCount; i++) 
            {
//printf("BEFO%ld/%ld \n", spmidiFramesPerBuffer, midiLoopCount);
			framesRead = SPMIDI_ReadFrames( pContext_, pBuffer, spmidiFramesPerBuffer,
	    		samplesPerFrame_, bitsPerSample_ );
			pBuffer += framesRead * kMIDI_SamplesPerFrame;
		    }
//printf("333 spmidiFramesPerBuffer=%ld\n", spmidiFramesPerBuffer);
	}
	
//	printf("!!!!! CMidiPlayer::RenderBuffer -- framesRead=%ld, pContext_ 0x%x, pMidiRenderBuffer_ 0x%x!!!\n\n", (long)framesRead, (unsigned int) pContext_, (unsigned int) pMidiRenderBuffer_);

//
//  Pan and scale MIDI output and mix into output buffer
//      Mobileer engine output is stereo, but we may want mono for stereo panning
//
{
//pBuffer = pMidiRenderBuffer_;
S32 y;

//printf("AddToOutput=%ld\n", addToOutput);
//printf("levelsf L=%f R=%f\n", levelsf[0], levelsf[1]);
#define OLDE_MIDI_LOOP
#ifdef OLDE_MIDI_LOOP
if (addToOutput)
	{
//    U32 samplesToProcess = 128;
//	for ( i = 0; i < numFrames*2; i++)
	for ( i = 0; i < numStereoSamples; i++)
		{
	 //  Apply gain
//		y = pOutBuff[i] + (S16)((volume_ * (S32)pBuffer[i])>>7);        // ORIG rdg
//		y = pOutBuff[i] + (S32)(levelsf * (float)pBuffer[i]);	        // FLOAT keep			
		y = pOutBuff[i] + (S32) MultQ15(levelsi[kLeft], pMidiRenderBuffer_[i]);	// Q15  1.15 Fixed-point		

	// Saturate to 16-bit range
		if      (y > kS16Max) y = kS16Max;
		else if (y < kS16Min) y = kS16Min;
	// Convert mono midi render to stereo output  GK: ?
		pOutBuff[i] = y;
		}
	}
else
	{
	for ( i = 0; i < numStereoSamples; i++)
		{
	 // Apply gain
		y = (S32) MultQ15(levelsi[kLeft], pMidiRenderBuffer_[i]);	// Q15  1.15 Fixed-point		

	// Saturate to 16-bit range
		if      (y > kS16Max) y = kS16Max;
		else if (y < kS16Min) y = kS16Min;
	// Convert mono midi render to stereo output  GK: ?
		pOutBuff[i] = y;
		}
	}
#else
if (addToOutput)
	{
	for ( i = 0; i < numFrames; i += 2)
		{
	 //  Apply gain
//		y = pOutBuff[i] + (S16)((volume_ * (S32)pBuffer[i])>>7);        // ORIG rdg
//		y = pOutBuff[i] + (S32)(levelsf * (float)pBuffer[i]);	        // FLOAT keep			
		y = pOutBuff[i] + (S32) MultQ15(levelsi[kLeft], pMidiRenderBuffer_[i]);	// Q15  1.15 Fixed-point		
	// Saturate to 16-bit range
		if      (y > kS16Max) y = kS16Max;
		else if (y < kS16Min) y = kS16Min;
	// Convert mono midi render to stereo output  GK: ?
		pOutBuff[i] = y;

		y = pOutBuff[i+1] + (S32) MultQ15(levelsi[kRight], pMidiRenderBuffer_[i+1]); // Q15  1.15 Fixed-point		
	// Saturate to 16-bit range
		if      (y > kS16Max) y = kS16Max;
		else if (y < kS16Min) y = kS16Min;
	// Convert mono midi render to stereo output  GK: ?
		pOutBuff[i+1] = y;
		}
	}
else
	{
	for ( i = 0; i < numFrames; i += 2)
		{
	 // Apply gain and Saturate to 16-bit range
		y = (S32) MultQ15(levelsi[kLeft], pMidiRenderBuffer_[i]);	// Q15  1.15 Fixed-point		
		if      (y > kS16Max) y = kS16Max;
		else if (y < kS16Min) y = kS16Min;
	// Convert mono midi render to stereo output  GK: ?
		pOutBuff[i] = y;

	 // Apply gain and Saturate to 16-bit range
		y = (S32) MultQ15(levelsi[kRight], pMidiRenderBuffer_[i+1]);	// Q15  1.15 Fixed-point		
		if      (y > kS16Max) y = kS16Max;
		else if (y < kS16Min) y = kS16Min;
	// Convert mono midi render to stereo output  GK: ?
		pOutBuff[i+1] = y;
		}
	}
#endif // OLDE_MIDI_LOOP
}

result = pKernelMPI_->UnlockMutex( render_mutex_ );
pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::RenderBuffer -- Couldn't unlock mutex.\n");

//{static long c=0; printf("CMidiPlayer::RenderBuffer: %ld end\n", c++);}
	return framesRead;
}   // ---- end RenderBuffer() ----

// ==============================================================================
// SetPan :     Channel stereo position   Left .. Center .. Right
// ==============================================================================
void CMidiPlayer::SetPan( S8 x )
{
//printf("CMidiPlayer::SetPan: x= %d\n", x);
pan_ = BoundS8(&x, kPan_Min, kPan_Max);

// Convert input range range to [0 .. 1] suitable
// ChangeRangef(x, L1, H1, L2, H2)
float xf = ChangeRangef((float)pan_, (float) kPan_Min, (float)kPan_Max, 0.0f, 1.0f);
//printf("pan_=%d -> xf=%g\n", pan_, xf);

//#define kPanValue_FullLeft ( 0.0)
//#define kPanValue_Center    (0.5)
//#define kPanValue_FullRight (1.0)
// PanValues(xf, panValuesf);
ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
if (panValuesf[kLeft] > -1e-06 && panValuesf[kLeft] < 1e-06)
    panValuesf[kLeft] = 0.0f;
if (panValuesf[kRight] > -1e-06 && panValuesf[kRight] < 1e-06)
    panValuesf[kRight] = 0.0f;

//printf("CMidiPlayer::SetPan : %d -> <%f , %f> \n", x, panValuesf[kLeft], panValuesf[kRight]);
RecalculateLevels();
}	// ---- end SetPan ----

// ==============================================================================
// SetVolume : Convert Brio volume range to DSP values
// ==============================================================================
void CMidiPlayer::SetVolume( U8 x )
{
//printf("CMidiPlayer::SetVolume :x= %d\n", x);
volume_ = x; //BoundU8(&x, kVolume_Min, kVolume_Max);

// ChangeRangef(x, L1, H1, L2, H2)
// FIXX: move to decibels, but for now, linear volume
gainf = ChangeRangef((float)x, (float) kVolume_Min, (float)kVolume_Max, 0.0f, 1.0f);
gainf *= kDecibelToLinearf_m3dBf; // DecibelToLinearf(-3.0);
//gainf = ChangeRangef((float)x, 0.0f, 100.0f, -100.0f, 0.0f);
//gainf =  DecibelToLinearf(gainf);

//printf( "SetVolume::SetVolume - %d -> %g\n", volume_, gainf );	

//printf("%f dB -> %f \n", -3.01, DecibelToLinearf(-3.01));
//printf("%f dB -> %f \n", -6.02, DecibelToLinearf(-6.02));
//printf("sqrt(2)/2 = %g\n", 0.5f*sqrt(2.0));

RecalculateLevels();
}	// ---- end SetVolume ----

// ==============================================================================
// RecalculateLevels : Recalculate levels when either pan or volume changes
// ==============================================================================
void CMidiPlayer::RecalculateLevels()
{
//printf("CMidiPlayer::RecalculateLevels : gainf=%g panValuesf <%g,%g>\n", gainf, panValuesf[kLeft], panValuesf[kRight]);

levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
levelsf[kRight] =  panValuesf[kRight]*gainf;

// Convert 32-bit floating-point to Q15 fractional integer format 
levelsi[kLeft ] = FloatToQ15(levelsf[kLeft ]);
levelsi[kRight] = FloatToQ15(levelsf[kRight]);

//printf("CMidiPlayer::RecalculateLevels : levelsf L=%g R=%g\n", levelsf[kLeft], levelsf[kRight]);
}	// ---- end RecalculateLevels ----

LF_END_BRIO_NAMESPACE()

