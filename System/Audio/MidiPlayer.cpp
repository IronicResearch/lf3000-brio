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

// ==============================================================================
// CMidiPlayer implementation
// ==============================================================================
CMidiPlayer::CMidiPlayer( tMidiPlayerID id )
{
//{static long c=0; printf("CMidiPlayer::CMidiPlayer: start %ld\n", c++);}

	tErrType			err;
	S16					midiErr;
	Boolean				ret;
#ifdef USE_MIDI_PLAYER_MUTEX
	const tMutexAttr 	attr = {0};
#endif	
	// Setup member vars...
	id_ = id;
	
	pMIDIRenderBuffer_ = new S16[kAudioOutBufSizeInWords];

	framesPerIteration_ = kMIDI_FramesPerIteration;	// FIXXX/rdg: don't hardcode this 
	samplesPerFrame_    = kMIDI_SamplesPerFrame;
	bitsPerSample_      = kMIDI_BitsPerSample;

	pListener_ = kNull;

	SetPan(kPan_Default);
	SetVolume(kVolume_Default);
    
	bFilePaused_ = false;
	bFileActive_ = false;
	bActive_     = false;

	// Get Debug MPI
	pDebugMPI_ =  new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CMidiPlayer::ctor: Couldn't create DebugMPI.\n");

	// Set debug level from a constant
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

#ifdef USE_MIDI_PLAYER_MUTEX
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CMidiPlayer::ctor: Couldn't create KernelMPI.\n");
	// Setup Mutex object for protecting render calls
  	err = pKernelMPI_->InitMutex( renderMutex_, attr );
	pDebugMPI_->Assert((kNoErr == err), "CMidiPlayer::ctor: Couldn't init mutex.\n");
#endif

	// Initialize SPMIDI Library
	SPMIDI_Initialize();

	// Start SP-MIDI synthesis engine using the desired sample rate.
	midiErr = SPMIDI_CreateContext( &pContext_, kMIDI_SamplingFrequency);  
	pDebugMPI_->Assert((midiErr == kNoErr), "CMidiPlayer::ctor: SPMIDI_CreateContext() failed.\n");

	// For now only one MIDI player for whole system!  
	pFilePlayer_ = kNull;

// Mobileer "spmidi" variables
	spMIDI_orchestra_ = NULL;
// FIXXX: should set Mobileer sampling frequency here
	
	SPMIDI_SetMaxVoices( pContext_, kMIDI_MaxVoices );
}   // ---- end CMidiPlayer() ----

// ==============================================================================
// ~CMidiPlayer
// ==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	tErrType 				result;
	tAudioStopMidiFileInfo 	info;
	
//{static long c=0; printf("CMidiPlayer::~: start %ld\n", c++);}

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::~ -- Cleaning up.\n");	
	
#ifdef USE_MIDI_PLAYER_MUTEX
	result = pKernelMPI_->LockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::~ -- Couldn't lock mutex.\n");
#endif

	// If a file is playing (the user didn't call stop first) , stop it for them.
	if (bFileActive_) 
        {
		pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::~: set bFileActive_ to false\n");	
		bFileActive_ = false;
		info.suppressDoneMsg = true;
	    }
	else
 		info.suppressDoneMsg = !bDoneMessage_;
 	StopMidiFile( &info );

	if (pFilePlayer_)
        {
		MIDIFilePlayer_Delete( pFilePlayer_ );
        pFilePlayer_ = kNull;
        }

	SPMIDI_DeleteContext( pContext_ );
	SPMIDI_Terminate();
	
#ifdef USE_MIDI_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "~CMidiPlayer: Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "~CMidiPlayer: Couldn't deinit mutex.\n");
if (pKernelMPI_)
	delete pKernelMPI_;
#endif

	delete pMIDIRenderBuffer_;

// Free MPIs
if (pDebugMPI_)
	delete pDebugMPI_;
//{static long c=0; printf("CMidiPlayer::~: end %ld\n", c++);}
}   // ---- end ~CMidiPlayer() ----

// ==============================================================================
// NoteOn
// ==============================================================================
    tErrType 	
CMidiPlayer::NoteOn( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags /*flags*/ )
{
char noteS[50];
MIDINoteToNotation(noteNum, noteS, False);
//printf("CMidiPlayer::NoteOn : channel=%2d note=%3d (%3s) vel=%3d flags=$%X\n", 
//                channel, noteNum, noteS, velocity, (unsigned int) flags );
 
SPMUtil_NoteOn( pContext_, (int) channel, (int) noteNum, (int) velocity );

return (kNoErr);
}   // ---- end NoteOn() ----

// ==============================================================================
// NoteOff
// ==============================================================================
    tErrType 	
CMidiPlayer::NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags /*flags*/ )
{
char noteS[50];
MIDINoteToNotation(noteNum, noteS, False);
//printf("CMidiPlayer::NoteOff: channel=%2d note=%3d (%3s) vel=%3d flags=$%X\n", 
//                channel, noteNum, noteS, velocity, (unsigned int) flags );
 
SPMUtil_NoteOff( pContext_, (int) channel, (int) noteNum, (int) velocity );

return (kNoErr);
}   // ---- end NoteOff() ----

// ==============================================================================
// SendCommand
// ==============================================================================
    tErrType 
CMidiPlayer::SendCommand( U8 cmd, U8 data1, U8 data2 )
{
pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::SendCommand cmd=%3d data1=%3d data2=%3d\n", cmd, data1, data2);	

SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, (int)data2 );

return (kNoErr);
}   // ---- end SendCommand() ----

// ==============================================================================
// SendDoneMsg
// ==============================================================================
    void 
CMidiPlayer::SendDoneMsg( void ) 
{
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgMidiCompleted	data;
	data.midiPlayerID = id_;
	data.payload      = loopCount_;	
	data.count        = 1;

printf("CMidiPlayer::SendDoneMsg midiPlayerID=%d\n", id_);

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
	
// fixme/dg: need to find a better way to do done listeners
// support multiple midi files playing at the same time.
	//pListener_ = kNull;
//printf("CMidiPlayer::SendDoneMsg end\n");
}   // ---- end SendDoneMsg() ----

// ==============================================================================
// StartMidiFile
// ==============================================================================
    tErrType 	
CMidiPlayer::StartMidiFile( tAudioStartMidiFileInfo* 	pInfo ) 
{
//{static long c=0; printf("CMidiPlayer::StartMidiFile: start %ld\n", c++);}

tErrType result = 0;

//pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::StartMidiFile: Start...\n");	

// If pre-emption is happening, delete previous player.
if (pFilePlayer_) 
    {
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::StartMidiFile deleting active player\n");	
	MIDIFilePlayer_Delete( pFilePlayer_ );
	pFilePlayer_ = kNull;
    }

//	pInfo->midiID;			// fixme/dg: midiEngineContext object?

SetVolume(pInfo->volume);
SetPan(0); //pInfo->pan);

if ( pInfo->pListener )
	pListener_ = pInfo->pListener;

    optionsFlags_ = pInfo->flags;
	shouldLoop_   = (0 < pInfo->payload) && (0 != (pInfo->flags & kAudioOptionsLooped));
    loopCount_    = pInfo->payload;
    loopCounter_  = 0;
	bDoneMessage_ = ((kNull != pListener_) && (0 != (pInfo->flags & kAudioOptionsDoneMsgAfterComplete)));

// kAudioOptionsLooped
//printf("CMidiPlayer::StartMidiFile: loopFile_=%d loopCount=%ld flags=$%X kAudioOptionsLooped=$%X\n",
//        shouldLoop_, loopCount_, (unsigned int)pInfo->flags, (unsigned int)kAudioOptionsLooped);
//printf("CMidiPlayer:ctor: payload_=%d optionsFlags=$%X -> shouldLoop=%d \n", 
//        (int)payload_, (unsigned int) optionsFlags_, shouldLoop_);
//#define DEBUG_MIDIPLAYER_OPTIONS
#ifdef DEBUG_MIDIPLAYER_OPTIONS
{
char s[50];
s[0] = '\0';
if (optionsFlags_ & kAudioOptionsLooped)
    strcat(s, "Loop=On");
else
    strcat(s, "Loop=Off");
if (optionsFlags_ & kAudioOptionsDoneMsgAfterComplete)
    strcat(s, " SendDone=On");
else
    strcat(s, " SendDone=Off");

printf("CMidiPlayer::ctor: listener=%d flags=$%X '%s'\n", (kNull != pListener_), (unsigned int)optionsFlags_, s);
printf("CMidiPlayer::ctor: bDoneMessage_=%d shouldLoop_=%d loopCount=%ld\n", bDoneMessage_, shouldLoop_, loopCount_);
}
#endif // DEBUG_MIDIPLAYER_OPTIONS

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
printf("orchestraFileName = '%s'\n", orchestraFileName.c_str());

// "/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/apprsrc/orchestra_071002.mbis"; // 358,796 Bytes
// "/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/apprsrc/orch_100207.mbis";
// "D:\\mobileer_work\\A_Orchestra\\exports\\exported.mbis";

//	printf( "Num blocks allocated before SPMIDI_CreateProgramList = %d\n", SPMIDI_GetMemoryAllocationCount() );
	err = SPMIDI_CreateProgramList( &programList );
	if( err < 0 ) 
		printf("CMidiPlayer::StartMidiFile: SPMIDI_CreateProgramList failed\n");

	// Scan the MIDIFile to see what instruments we should load.
	err = MIDIFile_ScanForPrograms( programList, pInfo->pMidiFileImage, pInfo->imageSize );
	if( err < 0 ) 
		printf("CMidiPlayer::StartMidiFile: MIDIFile_ScanForPrograms failed\n");

	// Load an Orchestra file into a memory stream and parse it.
//	orchestraImage = SPMUtil_LoadFileImage( (char *)orchestraFileName.c_str(), (int *)&( orchestraFileSize ) );
//	if( orchestraImage == NULL )
//		printf("Error: can't open orchestra file '%s'\n", (char *)orchestraFileName.c_str());
//    else 
//        printf("CMidiPlayer::StartMidiFile: loaded orchestraFile='%s'\n", orchestraFileName.c_str());

	// Create a stream object for reading the orchestra.
//	sio = Stream_OpenImage( (char *)orchestraImage, orchestraFileSize );
	sio = Stream_OpenFile( (char *)orchestraFileName.c_str(), "rb");
	if ( NULL == sio )
		printf("CMidiPlayer::StartMidiFile: Stream_OpenFile failed on '%s'\n", (char *)orchestraFileName.c_str());
    else
        {
//	printf( "Num blocks allocated before SPMIDI_LoadOrchestra = %d\n", SPMIDI_GetMemoryAllocationCount() );

	preOrchestraCount = SPMIDI_GetMemoryAllocationCount();
	// Scan MIDI file and load minimal set of the instruments 
	if ( SPMIDI_LoadOrchestra( sio, programList, &spMIDI_orchestra_ ) < 0 )
		printf( "CMidiPlayer::StartMidiFile: SPMIDI_LoadOrchestra failed on '%s'\n", (char *)orchestraFileName.c_str());
	
	// Close orchestra stream
		Stream_Close( sio );
        }
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
	
//	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::StartMidiFile: setting bFileActive_ \n");	
	
	bFileActive_ = true;
	
	return result;
}   // ---- end StartMidiFile() ----

// ==============================================================================
// PauseMidiFile
// ==============================================================================
    tErrType 	
CMidiPlayer::PauseMidiFile( void ) 
{
//	tErrType result = 0;
	
//pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::PauseMidiFile: setting bFilePaused_ to true.\n");	
	
if (bFileActive_)
	bFilePaused_ = true;
	
return (0); //result;
}   // ---- end PauseMidiFile() ----

// ==============================================================================
// ResumeMidiFile
// ==============================================================================
    tErrType 	
CMidiPlayer::ResumeMidiFile( void ) 
{
//	tErrType result = 0;
	
//pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::ResumeMidiFile: Setting bFilePaused_ to false.\n");	

if (bFileActive_)
	bFilePaused_ = false; 

return (0); //result;
}   // ---- end ResumeMidiFile() ----

// ==============================================================================
// StopMidiFile
// ==============================================================================
    tErrType 	
CMidiPlayer::StopMidiFile( tAudioStopMidiFileInfo* pInfo ) 
{
	tErrType result = 0;
{static long c=0; printf("CMidiPlayer::StopMidiFile: start %ld\n", c++);}

#ifdef USE_MIDI_PLAYER_MUTEX
	printf("CMidiPlayer::StopMidiFile: Locking renderMutex\n");	
	result = pKernelMPI_->LockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::StopMidiFile: Unable to lock mutex.\n");
#endif

	bFileActive_ = false;
	bFilePaused_ = false;
	if (pListener_  && !pInfo->suppressDoneMsg)
		SendDoneMsg();

	printf("CMidiPlayer::StopMidiFile: reset engine and delete player\n");	

	SPMUtil_Reset( pContext_ );
	printf("CMidiPlayer::StopMidiFile: BEFO delete player\n");	
    if (pFilePlayer_)   
        {
    	MIDIFilePlayer_Delete( pFilePlayer_ );
    	pFilePlayer_ = kNull;
        }
	printf("CMidiPlayer::StopMidiFile: AFTA delete player\n");	

	pListener_ = kNull;
	
#ifdef USE_MIDI_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::StopMidiFile Unable to unlock mutex.\n");
#endif

// SPMIDI_DeleteOrchestra( orchestra );

{static long c=0; printf("CMidiPlayer::StopMidiFile: end %ld\n", c++);}
	return result;
}   // ---- end StopMidiFile() ----

// ==============================================================================
// GetEnableTracks
// ==============================================================================
    tErrType 
CMidiPlayer::GetEnableTracks( tMidiTrackBitMask *d ) 
{
	tMidiTrackBitMask	mask = 0;

	// Loop through channels and set bits
	for (U32 chan = 0; chan < kMIDI_ChannelCount; chan++) 
        {
		if (SPMIDI_GetChannelEnable( pContext_, chan ))
			mask |= 1 << chan;
	    }
//printf("CMidiPlayer::GetEnableTracks $%X \n", (unsigned int) mask);	

*d = mask;

return kNoErr;
}   // ---- end GetEnableTracks() ----

// ==============================================================================
// SetEnableTracks: 
// ==============================================================================
    tErrType 
CMidiPlayer::SetEnableTracks( tMidiTrackBitMask d )
{
//printf("CMidiPlayer::SetEnableTracks: $%X \n", (unsigned int) d);	

// Loop through channels and set mask
for (U32 chan = 0; chan < kMIDI_ChannelCount; chan++) 
    SPMIDI_SetChannelEnable( pContext_, chan, 0 != (d & (1 << chan)) );

return kNoErr;
}   // ---- end SetEnableTracks() ----

// ==============================================================================
// TransposeTracks:   FIXXX: parameter shouldn't be track bit mask ???
//                  This API is non-sensical
// ==============================================================================
    tErrType 
CMidiPlayer::TransposeTracks( tMidiTrackBitMask /* d */, S8 semitones)
{
//printf("CMidiPlayer::TransposeTracks semitones=%d \n", semitones);	
SPMIDI_SetParameter( pContext_, SPMIDI_PARAM_TRANSPOSITION, semitones );

return (kNoErr);
}   // ---- end TransposeTracks() ----

// ==============================================================================
// ChangeProgram:       Change program (instrument) on specified MIDI channel
//
//              FIXXXX: this API is non-sensical.  Add another call that takes 
//              channel # as argument.
// ==============================================================================
    tErrType 
CMidiPlayer::ChangeProgram( tMidiTrackBitMask channel , tMidiPlayerInstrument number )
{
//printf("CMidiPlayer::ChangeProgram channel%2d number=%3d \n", (int)channel, (unsigned int) number);

#ifdef OLDE_INCORRECT_CHANGE_PROGRAM
unsigned int shift = 1;
for (long ch = 0; ch < 16; ch++)    
    {
    if (d & shift)
        {
    // Send two byte MIDI program change
        U8 cmd   = 0xA0 | channel;
        U8 data1 = (U8)(0x7f & number);
        SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, 0 );
        }
    shift <<= 1;
    }
#endif

// Send two byte MIDI program change
U8 cmd   = kMIDI_CHANNELMESSAGE_PROGRAMCHANGE | (0xF & channel);
U8 data1 = (U8)(0x7f & number);
//printf("CMidiPlayer::ChangeProgram: cmd=$%X data1=$%X\n", (int) cmd, (int)data1);
SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, 0 );

// Need extra code as minimal instrument set is currently loaded, so you have to
// load those not in memory

return (kNoErr);
}   // ---- end ChangeProgram() ----

// ==============================================================================
// ChangeTempo:    Scale rate of MIDI file play with S8 number
//                  Current mapping is [-128 .. 127] to [1/16x .. 16x]
//      NOTE:  yes, this is not a great mapping of 256 values, but it's easy to
//      hit the key values of 1/16, 1/8, 1/4, 1/2, 2, 4, 8, 16 x
// ==============================================================================
    tErrType 
CMidiPlayer::ChangeTempo( S8 tempoScale )
{
//printf("CMidiPlayer::ChangeTempo tempoScale=%d  \n", tempoScale);	
if (tempoScale < -127)
    tempoScale = -127;

// Map Brio tempo range to 16.16 tempo scaler
// Change Range of [-128 .. 127] to [1/4 .. 4]
// ChangeRangef(x, L1, H1, L2, H2)
float xf = ChangeRangef((float)tempoScale, -127.0f, 127.0f, -4.0f, 4.0f);
float yf = powf(2.0f, xf);
// Convert to 16.16 range
U32 scaler_16d16 = (U32)(65536.0f*yf);

//printf("CMidiPlayer::ChangeTempo: tempo=%d : %g -> %g (%X)\n", tempoScale, xf, yf, (unsigned int) scaler_16d16);

if (pFilePlayer_)
    MIDIFilePlayer_SetTempoScaler( pFilePlayer_, scaler_16d16);

return (kNoErr);
}   // ---- end ChangeTempo() ----

// ==============================================================================
// RenderBuffer:    Render MIDI file to audio samples
//
//          FIXXXX: Return something.  Number of frames rendered from MIDI file,
//                  not total frames rendered, which can be zero-padded
// ==============================================================================
U32	CMidiPlayer::RenderBuffer( S16* pOut, U32 numStereoFrames )
{
tErrType result;
U32 	i, midiLoopCount;
int 	mfp_result = 0;
U32		spMIDIFramesPerBuffer;
U32 	framesRead = 0;
U32 	numStereoSamples = numStereoFrames * 2;

//    U32     numFrames = numStereoFrames/2;

//{static long c=0; printf("CMidiPlayer::RenderBuffer: %ld start numFrames=%ld numSamples=%ld\n", c++, numStereoFrames, numStereoSamples);}

#ifdef USE_MIDI_PLAYER_MUTEX
	result = pKernelMPI_->TryLockMutex( render_mutex_ );
	if (EBUSY == result)
		return 0;
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::RenderBuffer -- Couldn't lock mutex.\n");
#endif
	
	// Clear output buffer
// FIXXXX: move to DSP loop and clear only if needed
	bzero( pMIDIRenderBuffer_, kAudioOutBufSizeInBytes );
	
//	printf("CMidiPlayer::RenderBuffer -- numFrames=%ld, bFileActive=%u, bFilePaused_=%d\n", 
//		numFrames, (int)bFileActive_, (int)bFilePaused_);

	S16 *pBuf = pMIDIRenderBuffer_;  // local ptr to buffer for indexing

	// If there is a midi file player, service it
	if ( bFileActive_ && !bFilePaused_ ) {
// TODO: make this bulletproof.  Right now no check for sizes.
// Figure out how many calls to spmidi we need to make to get a full output buffer
		spMIDIFramesPerBuffer = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / spMIDIFramesPerBuffer;
//printf("222 spmidiFramesPerBuffer=%ld midiLoopCount=%ld spMIDIFramesPerBuffer=%ld numFrames=%ld\n", 
//        spMIDIFramesPerBuffer, midiLoopCount, spMIDIFramesPerBuffer, numFrames);

//		printf("CMidiPlayer::RenderBuffer -- spMIDIFramesPerBuffer = %d, midiLoopCount = %d\n", 
//			(int)spMIDIFramesPerBuffer, (int)midiLoopCount );
			
        long fileEndReached = False;
		for (i = 0; i < midiLoopCount && !fileEndReached; i++) {
//	printf("CMidiPlayer::RenderBuffer: BEFO MIDIFilePlayer_PlayFrames()\n");
            if (pFilePlayer_)
                {
			    mfp_result = MIDIFilePlayer_PlayFrames( pFilePlayer_, pContext_, spMIDIFramesPerBuffer );
			    if (mfp_result < 0)
				    printf("!!!!! CMidiPlayer::RenderBuffer -- MIDIFilePlayer PlayFrames failed!!!\n");

			// fixme/dg: rationalize numStereoFrames and numFrames_!!
//			printf("CMidiPlayer::RenderBuffer -- About to SPMIDI_ReadFrames()\n");
			framesRead = SPMIDI_ReadFrames( pContext_, pBuf, spMIDIFramesPerBuffer,
		    		samplesPerFrame_, bitsPerSample_ );
	
			pBuf += framesRead *kMIDI_SamplesPerFrame;
// 	printf("CMidiPlayer::RenderBuffer loop %d, framesRead=%ul pBuf=%p\n", i, framesRead, (void*)pBuf);
	            }
			
			// Returning 1 means MIDI file done.
            fileEndReached = (mfp_result > 0);
			if (fileEndReached)
             {
//{static long c=0; printf("CMidiPlayer::RenderBuffer: %ld file done, looping=%d\n", c++, loopFile_);}
                bFileActive_ = false;
				if (shouldLoop_) 
                    {
//{static long c=0; printf("CMidiPlayer::RenderBuffer: Rewind %ld loop%ld/%ld\n", c++, loopCounter_+1, loopCount_);}
                    if (++loopCounter_ < loopCount_)
                        {
        			    MIDIFilePlayer_Rewind( pFilePlayer_ ); 
                        bFileActive_   = true;
                        fileEndReached = false;
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
    				if (pListener_ && bDoneMessage_)
    					SendDoneMsg();
    				SPMUtil_Reset( pContext_ );
                    }
			}
		}
	} else {
		// A MIDI file is not playing, but notes might be turned on programatically...
		// TODO: rationalize numStereoFrames and numFrames_!!
//	printf("CMidiPlayer::RenderBuffer -- About to SPMIDI_ReadFrames(), no file active\n");

		// TODO: make this bulletproof.  Right now no check for sizes.
		// Figure out how many calls to spmidi we need to make to get a full output buffer
		spMIDIFramesPerBuffer = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / spMIDIFramesPerBuffer;

		for (i = 0; i < midiLoopCount; i++) 
            {
			framesRead = SPMIDI_ReadFrames( pContext_, pBuf, spMIDIFramesPerBuffer,
	    		samplesPerFrame_, bitsPerSample_ );
			pBuf += framesRead * kMIDI_SamplesPerFrame;
		    }
	}
	
//	printf("!!!!! CMidiPlayer::RenderBuffer -- framesRead=%ld, pContext_ 0x%x, pMidiRenderBuffer_ 0x%x!!!\n\n", (long)framesRead, (unsigned int) pContext_, (unsigned int) pMidiRenderBuffer_);

//  Pan and scale MIDI output and mix into output buffer
//      Mobileer engine output is stereo, but we may want mono for stereo panning
//printf("levelsf L=%f R=%f\n", levelsf[0], levelsf[1]);

U32 numFrames = numStereoSamples;
for ( i = 0; i < numFrames; i += 2)
	{
 // Apply gain and Saturate to 16-bit range
	S32 y = (S32) MultQ15(levelsi[kLeft], pMIDIRenderBuffer_[i]);	// Q15  1.15 Fixed-point		
	if      (y > kS16Max) y = kS16Max;
	else if (y < kS16Min) y = kS16Min;
// Convert mono midi render to stereo output  GK: ?
	pOut[i] = (S16) y;

 // Apply gain and Saturate to 16-bit range
	y = (S32) MultQ15(levelsi[kRight], pMIDIRenderBuffer_[i+1]);	// Q15  1.15 Fixed-point		
	if      (y > kS16Max) y = kS16Max;
	else if (y < kS16Min) y = kS16Min;
// Convert mono midi render to stereo output  GK: ?
	pOut[i+1] = (S16) y;
	}

#ifdef USE_MIDI_PLAYER_MUTEX
result = pKernelMPI_->UnlockMutex( render_mutex_ );
pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::RenderBuffer -- Couldn't unlock mutex.\n");
#endif

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


