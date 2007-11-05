//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// 
// File:
//		Audio.cpp
//
// Description:
//		Underlying implementation code for the Audio Manager module.
//
//==============================================================================
#include <errno.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <EventMPI.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTask.h>
#include <AudioMsg.h>
#include <AudioTypesPriv.h>

#include <algorithm>
#include <map>
#include <set>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

const CURI	kModuleURI	= "/Somewhere/AudioModule";

// single instance of module object.
static CAudioModule*	sinst = NULL;

//============================================================================
// CAudioCmdMessages
//============================================================================
//------------------------------------------------------------------------------
CAudioMsgSetMasterVolume::CAudioMsgSetMasterVolume( const U8 x ) {
	type_ = kAudioCmdMsgTypeSetMasterVolume;
	messageSize = sizeof(CAudioMsgSetMasterVolume);
	messagePriority = kAudioMsgDefaultPriority;
	masterVolume_ = x;
}
CAudioMsgSetOutputEqualizer::CAudioMsgSetOutputEqualizer( const U8 x ) {
	type_ = kAudioCmdMsgTypeSetOutputEqualizer;
	messageSize = sizeof(CAudioMsgSetOutputEqualizer);
	messagePriority = kAudioMsgDefaultPriority;
	outputEqualizerEnabled_ = x;
}

///------------------------------------------------------------------------------
CAudioMsgStartAudio::CAudioMsgStartAudio( void ) {
	type_ = kAudioCmdMsgTypeStartAllAudio;
	messageSize = sizeof(CAudioMsgStartAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgStartAudio::CAudioMsgStartAudio( const tAudioStartAudioInfo& data )
	: data_(data)
{
	type_ = kAudioCmdMsgTypeStartAudio;
	messageSize = sizeof(CAudioMsgStartAudio);
	messagePriority = kAudioMsgDefaultPriority;
}
//------------------------------------------------------------------------------
CAudioMsgIsAudioPlaying::CAudioMsgIsAudioPlaying( void ) {
	type_ = kAudioCmdMsgTypeIsAnyAudioPlaying;
	messageSize = sizeof(CAudioMsgIsAudioPlaying);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgIsAudioPlaying::CAudioMsgIsAudioPlaying( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeIsAudioPlaying;
	messageSize = sizeof(CAudioMsgIsAudioPlaying);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------
CAudioMsgGetAudioTime::CAudioMsgGetAudioTime( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeGetAudioTime;
	messageSize = sizeof(CAudioMsgGetAudioTime);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------
CAudioMsgGetAudioVolume::CAudioMsgGetAudioVolume( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeGetAudioVolume;
	messageSize = sizeof(CAudioMsgGetAudioVolume);
	messagePriority = kAudioMsgDefaultPriority;
	vi_.id = id;
}

CAudioMsgSetAudioVolume::CAudioMsgSetAudioVolume( const tAudioVolumeInfo vi ) {
	type_ = kAudioCmdMsgTypeSetAudioVolume;
	messageSize = sizeof(CAudioMsgSetAudioVolume);
	messagePriority = kAudioMsgDefaultPriority;
	vi_.id = vi.id;
	vi_.volume = vi.volume;
}

//------------------------------------------------------------------------------
CAudioMsgGetAudioPriority::CAudioMsgGetAudioPriority( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeGetAudioPriority;
	messageSize = sizeof(CAudioMsgGetAudioPriority);
	messagePriority = kAudioMsgDefaultPriority;
	pi_.id = id;
}

CAudioMsgSetAudioPriority::CAudioMsgSetAudioPriority( const tAudioPriorityInfo pi ) {
	type_ = kAudioCmdMsgTypeSetAudioPriority;
	messageSize = sizeof(CAudioMsgSetAudioPriority);
	messagePriority = kAudioMsgDefaultPriority;
	pi_.id = pi.id;
	pi_.priority = pi.priority;
}

//------------------------------------------------------------------------------
CAudioMsgGetAudioPan::CAudioMsgGetAudioPan( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeGetAudioPan;
	messageSize = sizeof(CAudioMsgGetAudioPan);
	messagePriority = kAudioMsgDefaultPriority;
	pi_.id = id;
}

CAudioMsgSetAudioPan::CAudioMsgSetAudioPan( const tAudioPanInfo pi ) {
	type_ = kAudioCmdMsgTypeSetAudioPan;
	messageSize = sizeof(CAudioMsgSetAudioPan);
	messagePriority = kAudioMsgDefaultPriority;
	pi_.id = pi.id;
	pi_.pan = pi.pan;
}

//------------------------------------------------------------------------------
CAudioMsgGetAudioListener::CAudioMsgGetAudioListener( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeGetAudioListener;
	messageSize = sizeof(CAudioMsgGetAudioListener);
	messagePriority = kAudioMsgDefaultPriority;
	li_.id = id;
}

CAudioMsgSetAudioListener::CAudioMsgSetAudioListener( const tAudioListenerInfo li ) {
	type_ = kAudioCmdMsgTypeSetAudioListener;
	messageSize = sizeof(CAudioMsgSetAudioListener);
	messagePriority = kAudioMsgDefaultPriority;
	li_.id = li.id;
	li_.pListener = li.pListener;
}

//------------------------------------------------------------------------------
CAudioMsgStopAudio::CAudioMsgStopAudio( void ) {
	type_ = kAudioCmdMsgTypeStopAllAudio;
	messageSize = sizeof(CAudioMsgStopAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgStopAudio::CAudioMsgStopAudio( const tAudioStopAudioInfo& data ) {
	type_ = kAudioCmdMsgTypeStopAudio;
	messageSize = sizeof(CAudioMsgStopAudio);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgPauseAudio::CAudioMsgPauseAudio( void ) {
	type_ = kAudioCmdMsgTypePauseAllAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgPauseAudio::CAudioMsgPauseAudio(  const tAudioID id ) {
	type_ = kAudioCmdMsgTypePauseAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------
CAudioMsgResumeAudio::CAudioMsgResumeAudio( void ) {
	type_ = kAudioCmdMsgTypeResumeAllAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgResumeAudio::CAudioMsgResumeAudio( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeResumeAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------

CAudioMsgAcquireMidiPlayer::CAudioMsgAcquireMidiPlayer( void ) {
	type_ = kAudioCmdMsgTypeAcquireMidiPlayer;
	messageSize = sizeof(CAudioMsgAcquireMidiPlayer);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgReleaseMidiPlayer::CAudioMsgReleaseMidiPlayer( void ) {
	type_ = kAudioCmdMsgTypeReleaseMidiPlayer;
	messageSize = sizeof(CAudioMsgReleaseMidiPlayer);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgMidiNoteOn::CAudioMsgMidiNoteOn( const tAudioMidiNoteInfo& data ) {
	type_ = kAudioCmdMsgTypeMidiNoteOn;
	messageSize = sizeof(CAudioMsgMidiNoteOn);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgMidiNoteOff::CAudioMsgMidiNoteOff( const tAudioMidiNoteInfo& data ) {
	type_ = kAudioCmdMsgTypeMidiNoteOff;
	messageSize = sizeof(CAudioMsgMidiNoteOff);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgStartMidiFile::CAudioMsgStartMidiFile( const tAudioStartMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypeStartMidiFile;
	messageSize = sizeof(CAudioMsgStartMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgPauseMidiFile::CAudioMsgPauseMidiFile( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypePauseMidiFile;
	messageSize = sizeof(CAudioMsgPauseMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

CAudioMsgResumeMidiFile::CAudioMsgResumeMidiFile( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypeResumeMidiFile;
	messageSize = sizeof(CAudioMsgResumeMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

CAudioMsgStopMidiFile::CAudioMsgStopMidiFile( const tAudioStopMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypeStopMidiFile;
	messageSize = sizeof(CAudioMsgStopMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgIsMidiFilePlaying::CAudioMsgIsMidiFilePlaying( void ) {
	type_ = kAudioCmdMsgTypeIsAnyMidiFilePlaying;
	messageSize = sizeof(CAudioMsgIsMidiFilePlaying);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgIsMidiFilePlaying::CAudioMsgIsMidiFilePlaying( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypeIsMidiFilePlaying;
	messageSize = sizeof(CAudioMsgIsMidiFilePlaying);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------
CAudioMsgMidiFilePlaybackParams::CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypeGetEnabledMidiTracks;
	messageSize = sizeof(CAudioMsgMidiFilePlaybackParams);
	messagePriority = kAudioMsgDefaultPriority;
	data_.id = id;
}

CAudioMsgMidiFilePlaybackParams::CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const tMidiTrackBitMask trackBitMask ) {
	type_ = kAudioCmdMsgTypeSetEnableMidiTracks;
	messageSize = sizeof(CAudioMsgMidiFilePlaybackParams);
	messagePriority = kAudioMsgDefaultPriority;
	data_.id = id;
	data_.trackBitMask = trackBitMask;
}

CAudioMsgMidiFilePlaybackParams::CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const tMidiTrackBitMask trackBitMask, const S8 transposeAmount ) {
	type_ = kAudioCmdMsgTypeTransposeMidiTracks;
	messageSize = sizeof(CAudioMsgMidiFilePlaybackParams);
	messagePriority = kAudioMsgDefaultPriority;
	data_.id = id;
	data_.trackBitMask = trackBitMask;
	data_.transposeAmount = transposeAmount;
}

CAudioMsgMidiFilePlaybackParams::CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const tMidiTrackBitMask trackBitMask, const tMidiInstr instr ) {
	type_ = kAudioCmdMsgTypeChangeMidiInstrument;
	messageSize = sizeof(CAudioMsgMidiFilePlaybackParams);
	messagePriority = kAudioMsgDefaultPriority;
	data_.id = id;
	data_.trackBitMask = trackBitMask;
	data_.instrument = instr;
}

CAudioMsgMidiFilePlaybackParams::CAudioMsgMidiFilePlaybackParams( const tMidiPlayerID id, const S8 tempo ) {
	type_ = kAudioCmdMsgTypeChangeMidiTempo;
	messageSize = sizeof(CAudioMsgMidiFilePlaybackParams);
	messagePriority = kAudioMsgDefaultPriority;
	data_.id = id;
	data_.tempo = tempo;
}

//------------------------------------------------------------------------------
CAudioReturnMessage::CAudioReturnMessage( ) {
	messageSize = sizeof(CAudioReturnMessage);
	messagePriority = kAudioMsgDefaultPriority;
}

//------------------------------------------------------------------------------
CAudioCmdMsgExitThread::CAudioCmdMsgExitThread( ) {
	type_ = kAudioCmdMsgExitThread;
	messageSize = sizeof(CAudioCmdMsg);
	messagePriority = kAudioMsgDefaultPriority;
}


//============================================================================
// MPI state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	// MPIInstanceState
	//------------------------------------------------------------------------
	struct MPIInstanceState
	{
		//--------------------------------------------------------------------
		// Each MPI interface object has some state (default listener, volume 
		// pan, etc.).  The Audio module stores this state on
		// behalf of an interface object in this MPIInstanceState class.
		// The CAudioMPI ctor calls CAudioModule::Register() to create
		// an instance of this class that gets added to a global "gMPIMap".
		// Register() returns a U32 key into this map, and every call from
		// the MPI interface object to the CResourceModule passes this U32
		// key so that the operation uses that interface object's state.
		// CAudioMPI instance.
		//--------------------------------------------------------------------
		MPIInstanceState( void )
						: path(NULL),
						pListener(NULL),
						volume(100),
						pan(0),
						priority(0)
		{ }
						
		const CPath* 			path;
		const IEventListener*	pListener;
		U8 						volume;
		S8						pan;
		tAudioPriority 			priority;
	};
	typedef std::map<U32, MPIInstanceState>	MPIMap;
	
	MPIMap gMPIMap;
	
	//============================================================================
	// Utility functions
	//============================================================================
	//----------------------------------------------------------------------------
	MPIInstanceState& RetrieveMPIState(U32 id)
	{
		// Register() creates an MPIInstanceState entry in "gMPIMap" and 
		// returns a key/U32 id that the MPI instance uses to identify itself
		// in subsequent calls.
		// This function retrieves the MPIInstanceState for a given key.
		// FIXME/tp: multithreading issues.. MAYBE.  Should be safe
		// because all calling functions are already mutex protected.
		//
		MPIMap::iterator it = gMPIMap.find(id);
		if (it != gMPIMap.end())
			return it->second;
		
		CDebugMPI	dbg(kGroupAudio);
		dbg.Assert(false, "CAudioModule::RetrieveMPIState: configuration failure, unregistered MPI id!");

		return it->second;	// dummy return to avoid compiler warning
	}
} // end anon namespace

//==============================================================================
//==============================================================================
CAudioModule::CAudioModule( void )
{

	tErrType			err = kNoErr;
	Boolean				ret = false;
	const tMutexAttr 	attr = {0};
		
	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CAudioModule::ctor -- Couldn't create KernelMPI.\n");

	// Get Debug MPI
	pDebugMPI_ =  new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CAudioModule::ctor -- Couldn't create DebugMPI.\n");

	// Set debug level from a constant
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
		
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CAudioModule::ctor -- Initalizing Audio Module...");
	
	// Create the Audio Task... this wont return until the task is running.
	err = InitAudioTask();
	pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Audio task create failed.\n");

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CAudioModule::ctor: AudioModule thinks the audio task is running now...\n");	
	
	// First, Open the msg queue that allows the Audio Task to RECEIVE msgs from us.
	tMessageQueuePropertiesPosix msgQueueProperties = 
	{
	    0,							// msgProperties.blockingPolicy;  
	    "/audioTaskIncomingQ",		// msgProperties.nameQueue
	    0,							// msgProperties.mode 
	    B_O_WRONLY,					// msgProperties.oflag  
	    0,							// msgProperties.priority
	    0,							// msgProperties.mq_flags
	    kAUDIO_MAX_NUM_MSGS,		// msgProperties.mq_maxmsg
	    kAUDIO_MAX_MSG_SIZE,		// msgProperties.mq_msgsize
	    0							// msgProperties.mq_curmsgs
	};
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::ctor: Opening module outgoing Q. size = %d\n", 
				static_cast<int>(kAUDIO_MAX_MSG_SIZE) );	
	
	err = pKernelMPI_->OpenMessageQueue( hSendMsgQueue_, msgQueueProperties, NULL );

    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor:Trying to open Module outgoing msg queue. err = %d \n", 
    		static_cast<int>(err) );

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.oflag = B_O_RDONLY;

	err = pKernelMPI_->OpenMessageQueue( hRecvMsgQueue_,  msgQueueProperties, NULL );

    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Trying to open Module incoming msg queue. Err = %d \n", 
    		static_cast<int>(err) );

    // Init mutex for serialization of MPI calls
 	err = pKernelMPI_->InitMutex( mpiMutex_, attr );
	pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Couldn't init mutex.\n");

	masterVolume_           = 100;  // FIXX TODO: hack, should get this from the mixer.
	outputEqualizerEnabled_ = false;  // FIXX TODO: hack, should get this from the mixer.
}

//==============================================================================
//==============================================================================
CAudioModule::~CAudioModule( void )
{
	tErrType result = kNoErr;
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::dtor: dtor called\n" );	

	// Send message to thread to exit main while loop
	CAudioCmdMsgExitThread msg;

	SendCmdMessage( msg ); 

	// This won't return until the task is exited.
	DeInitAudioTask();
		
	result = pKernelMPI_->DeInitMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::dtor --  Couldn't deinit mutex.\n");

}

//============================================================================
// Informational functions
//============================================================================
Boolean CAudioModule::IsValid() const
{
	return (sinst != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
tVersion CAudioModule::GetModuleVersion() const
{
	return kAudioModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CAudioModule::GetModuleName() const
{
	return &kAudioModuleName;
}

//----------------------------------------------------------------------------
const CURI* CAudioModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//==============================================================================
// MPI State Functions
//==============================================================================
//----------------------------------------------------------------------------
U32 CAudioModule::Register( void )
{
	tErrType 	result = kNoErr;
	static U32	sNextIndex = 0;
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Register -- Couldn't lock mutex.\n");

	gMPIMap.insert(MPIMap::value_type(++sNextIndex, MPIInstanceState()));

	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Register --  Couldn't unlock mutex.\n");

	return sNextIndex;
}

//----------------------------------------------------------------------------
void CAudioModule::Unregister( U32 id )
{
	tErrType result = kNoErr;
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Unregister -- Couldn't lock mutex.\n");

	MPIMap::iterator it = gMPIMap.find(id);
	if (it != gMPIMap.end())
	{
		gMPIMap.erase(it);
	}

	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Unregister --  Couldn't unlock mutex.\n");
}

//==============================================================================
// 		Overall Audio System Control
//==============================================================================
tErrType CAudioModule::StartAudioSystem( void )
{
    CAudioMsgStartAudio msg;

   	SendCmdMessage( msg ); 

	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::StopAudioSystem( void )
{
    CAudioMsgStopAudio msg;

  	SendCmdMessage( msg ); 
	
	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::PauseAudioSystem( void )
{
    CAudioMsgPauseAudio msg;

	SendCmdMessage( msg ); 
	
	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::ResumeAudioSystem( void )
{
    CAudioMsgResumeAudio msg;

 	SendCmdMessage( msg ); 
	
	return WaitForStatus();
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::RegisterAudioEffectsProcessor( /* tRsrcType type, */ /*CAudioEffectsProcessor *pChain*/ )
{
	return kNoImplErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::RegisterGlobalAudioEffectsProcessor( /*CAudioEffectsProcessor *pChain*/ )
{
	return kNoImplErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeAudioEffectsProcessor( /*tAudioID id, CAudioEffectsProcessor *pChain*/ )
{
	return kNoImplErr;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetMasterVolume( U8 volume )
{
	tErrType 	result = kNoErr;

	// Keep local copy.
	masterVolume_ = volume;
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::SetMasterVolume -- Couldn't lock mutex.\n");

	// Need to inform the audio mixer that the master volume has changed
	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetMasterVolume; volume = %d\n", volume );	

	CAudioMsgSetMasterVolume	msg( volume );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 

 	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
 	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::SetMasterVolume --  Couldn't unlock mutex.\n");
}

//==============================================================================
//==============================================================================
U8 CAudioModule::GetMasterVolume( void )
{
	// TODO: This probably should call through to the mixer...
	return masterVolume_;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetOutputEqualizer(U8 x  )
{
	tErrType 	result = kNoErr;

	// Keep local copy.
	outputEqualizerEnabled_ = x;
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::SetOutputEqualizer -- Couldn't lock mutex.\n");

	// Need to inform the audio mixer that the master volume has changed
	// Generate the command message to send to the audio Mgr task
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
//		"CAudioModule::SetOutputEqualizer: outputEqualizerEnabled_ = %d\n", outputEqualizerEnabled_ );	

	CAudioMsgSetOutputEqualizer	msg( outputEqualizerEnabled_ );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 

 	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
 	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::SetOutputEqualizer --  Couldn't unlock mutex.\n");
}

//==============================================================================
//==============================================================================
U8 CAudioModule::GetOutputEqualizer( void )
{
	// TODO: This probably should call through to the mixer...
	return outputEqualizerEnabled_;
}
//==============================================================================
//==============================================================================
tErrType CAudioModule::SetAudioResourcePath( U32 mpiID, const CPath &path )
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetAudioResourcePath -- mpiID = %d; path: %s.\n", static_cast<int>(mpiID), path.c_str() );	
	
	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );

	mpiState.path = new CPath( path );
	
	return kNoErr;
}
//==============================================================================
//==============================================================================

const CPath* CAudioModule::GetAudioResourcePath( U32 mpiID )
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetAudioResourcePath -- mpiID = %d\n", static_cast<int>(mpiID) );	
	
	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	return mpiState.path;
}


//==============================================================================
//==============================================================================
tAudioID CAudioModule::StartAudio( U32 mpiID, const CPath &path, U8 volume,  
	tAudioPriority priority, S8 pan, const IEventListener *pListener, 
	tAudioPayload payload, tAudioOptionsFlags flags )
{
	tErrType 	result = kNoErr;
	tAudioID	id;

	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Register -- Couldn't lock mutex.\n");

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );

	// If the path passed to us is a full path, then use it, otherwise
	// append what was passed to the MPI's default path.
	CPath fullPath = (path[0] == '/')
			? path
			: *mpiState.path + path;

	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::StartAudio --  path = %s\n", fullPath.c_str() );	

	tAudioStartAudioInfo msgData( &fullPath, volume, priority, pan, 
								pListener, payload, flags );

	CAudioMsgStartAudio	msg( msgData );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
 	id = WaitForAudioID();

 	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
 	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Register --  Couldn't unlock mutex.\n");

 	return id;
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::StartAudio( U32 mpiID, const CPath &path, tAudioPayload payload, 
				tAudioOptionsFlags flags)
{
	tErrType 			result = kNoErr;
	tAudioID			id;
	MPIInstanceState&	mpiState = RetrieveMPIState( mpiID );
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::StartAudio -- Couldn't lock mutex.\n");

	// If the path passed to us is a full path, then use it, otherwise
	// append what was passed to the MPI's default path.
	CPath fullPath = (path[0] == '/')
			? path
			: *mpiState.path + path;

	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::StartAudio --  path = %s\n", fullPath.c_str() );	


	tAudioStartAudioInfo msgData( &fullPath, mpiState.volume, mpiState.priority, mpiState.pan, 
			mpiState.pListener, payload, flags);

	CAudioMsgStartAudio	msg( msgData );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
	id = WaitForAudioID();

	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::StartAudio --  Couldn't unlock mutex.\n");

	return id;

}

//==============================================================================
//==============================================================================
U32 CAudioModule::GetAudioTime( tAudioID id )
{
	tErrType 				result = kNoErr;
	U32						data;
	CAudioMsgGetAudioTime 	msg( id );
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Register -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	data = WaitForU32Result();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::Register --  Couldn't unlock mutex.\n");

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetAudioTime() -- Stream time = %u...\n", 
		static_cast<unsigned int>(data));	

	return data;
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsAudioPlaying( tAudioID id )
{
	tErrType 					result = kNoErr;
	Boolean						data;
	CAudioMsgIsAudioPlaying 	msg( id );
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::IsAudioPlaying -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	data = WaitForBooleanResult();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::IsAudioPlaying --  Couldn't unlock mutex.\n");

	return data;
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsAudioPlaying()
{
	tErrType 					result = kNoErr;
	CAudioMsgIsAudioPlaying 	msg;
	Boolean						data;
	
	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::IsAudioPlaying -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	data = WaitForBooleanResult();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::IsAudioPlaying --  Couldn't unlock mutex.\n");
	
	return data; 
}

//==============================================================================
//==============================================================================
void CAudioModule::StopAudio( tAudioID id, Boolean surpressDoneMessage )
{
	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::StopAudio -- ID = %d\n", static_cast<int>(id) );	

	tAudioStopAudioInfo msgData;
	
	msgData.id = id;
	msgData.suppressDoneMsg = surpressDoneMessage;

	
	CAudioMsgStopAudio	msg( msgData );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseAudio( tAudioID id )
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::PauseAudio -- ID = %d\n", static_cast<int>(id) );	

	CAudioMsgPauseAudio	msg( id );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeAudio( tAudioID id )
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::ResumeAudio -- ID = %d\n", static_cast<int>(id) );	

	CAudioMsgResumeAudio	msg( id );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
U8 CAudioModule::GetAudioVolume( tAudioID id ) 
{
	tErrType 						result = kNoErr;
	U8								volume;
	CAudioMsgGetAudioVolume			msg( id );
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetAudioVolume -- ID = %d\n", static_cast<int>(id) );	

	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioVolume -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	volume = (U8)WaitForU32Result();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioVolume --  Couldn't unlock mutex.\n");

	return volume;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetAudioVolume( tAudioID id, U8 x ) 
{
	tAudioVolumeInfo				info;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetAudioVolume -- ID = %d\n", static_cast<int>(id) );	

	info.id = id;
	info.volume = x;

	CAudioMsgSetAudioVolume		msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioModule::GetAudioPriority( tAudioID id ) 
{
	tErrType 						result = kNoErr;
	tAudioPriority					priority;
	tAudioPriorityInfo				info;
	CAudioMsgGetAudioPriority	 	msg( id );
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetAudioPriority -- ID = %d\n", static_cast<int>(id) );	

	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioPriority -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	priority = (tAudioPriority)WaitForU32Result();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioPriority --  Couldn't unlock mutex.\n");

	return priority;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetAudioPriority( tAudioID id, tAudioPriority priority ) 
{
	tAudioPriorityInfo				info;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetAudioPriority -- ID = %d\n", static_cast<int>(id) );	

	info.id = id;
	info.priority = priority;

	CAudioMsgSetAudioPriority 	msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
S8 CAudioModule::GetAudioPan( tAudioID id ) 
{
	tErrType 				result = kNoErr;
	S8						pan;
	tAudioPanInfo			info;
	CAudioMsgGetAudioPan	msg( id );
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetAudioPan -- ID = %d\n", static_cast<int>(id) );	

	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioPan -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	pan = (S8)WaitForU32Result();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioPan --  Couldn't unlock mutex.\n");

	return pan;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetAudioPan( tAudioID id, S8 pan ) 
{
	tAudioPanInfo			info;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetAudioPan -- ID = %d\n", static_cast<int>(id) );	

	info.id = id;
	info.pan = pan;

	CAudioMsgSetAudioPan 	msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
const IEventListener* CAudioModule::GetAudioEventListener( tAudioID id ) 
{
	tErrType 					result = kNoErr;
	const IEventListener*		pListener;
	tAudioPanInfo				info;
	CAudioMsgGetAudioListener	 msg( id );
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetAudioPan -- ID = %d\n", static_cast<int>(id) );	

	result = pKernelMPI_->LockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioPan -- Couldn't lock mutex.\n");

	SendCmdMessage( msg );

	pListener = (const IEventListener*)WaitForU32Result();
	
	result = pKernelMPI_->UnlockMutex( mpiMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CAudioModule::GetAudioPan --  Couldn't unlock mutex.\n");

	return pListener;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetAudioEventListener( tAudioID id, const IEventListener *pListener) 
{
	tAudioListenerInfo		info;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetAudioEventListener -- ID = %d\n", static_cast<int>(id) );	

	info.id = id;
	info.pListener = pListener;

	CAudioMsgSetAudioListener 	msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
U8 CAudioModule::GetDefaultAudioVolume( U32 mpiID ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetDefaultAudioVolume -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	return mpiState.volume;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetDefaultAudioVolume( U32 mpiID, U8 x ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetDefaultAudioVolume -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	mpiState.volume = x;
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioModule::GetDefaultAudioPriority( U32 mpiID ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetDefaultAudioPriority -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	return mpiState.priority;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetDefaultAudioPriority( U32 mpiID, tAudioPriority priority ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetDefaultAudioPriority -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	mpiState.pan = priority;
}

//==============================================================================
//==============================================================================
S8 CAudioModule::GetDefaultAudioPan( U32 mpiID ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetDefaultAudioPan -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	return mpiState.pan;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetDefaultAudioPan( U32 mpiID, S8 pan ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SetDefaultAudioPan -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	mpiState.pan = pan;
}

//==============================================================================
//==============================================================================
const IEventListener* CAudioModule::GetDefaultAudioEventListener( U32 mpiID ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetDefaultAudioEvenListener -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	return mpiState.pListener;
}

//==============================================================================
//==============================================================================
void CAudioModule::SetDefaultAudioEventListener( U32 mpiID, const IEventListener *pListener ) 
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::GetDefaultAudioEvenListener -- mpiID = %d\n", static_cast<int>(mpiID) );	

	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );
	mpiState.pListener = pListener;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::AcquireMidiPlayer( /*tAudioPriority priority, IEventListener *pHandler, */ tMidiPlayerID *id )
{
	CAudioMsgAcquireMidiPlayer msg;

	// Send the message and wait to get the id back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
 	*id = WaitForMidiID();
 	return kNoErr;
}
//==============================================================================
//==============================================================================
tErrType CAudioModule::ReleaseMidiPlayer(/* tMidiPlayerID id */)
{
	CAudioMsgReleaseMidiPlayer msg;

	// Send the message.
 	SendCmdMessage( msg );
 	
	return WaitForStatus();
}

tAudioID	CAudioModule::GetAudioIDForMidiID(/* tMidiPlayerID id*/ ) 
{
	return kNoAudioID;
}
//==============================================================================
//==============================================================================
tErrType CAudioModule::MidiNoteOn( /*tMidiPlayerID id,*/ U8 channel, U8 noteNum, U8 velocity, 
										tAudioOptionsFlags flags )
{
	tAudioMidiNoteInfo info( channel, noteNum, velocity, flags );
		
	CAudioMsgMidiNoteOn	msg( info );
	
	SendCmdMessage( msg );
	
	return kNoErr;
}
	
//==============================================================================
//==============================================================================
tErrType CAudioModule::MidiNoteOff( /*tMidiPlayerID id,*/ U8 channel, U8 noteNum, U8 velocity, 
										tAudioOptionsFlags flags )
{
	tAudioMidiNoteInfo info( channel, noteNum, velocity, flags );
	
	CAudioMsgMidiNoteOff msg( info );
	
	SendCmdMessage( msg );
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::StartMidiFile( 	U32 				mpiID, 
										tMidiPlayerID		id,
										const CPath 		&path, 
										U8					volume, 
										tAudioPriority		priority,
										IEventListener*		pListener,
										tAudioPayload		payload,
										tAudioOptionsFlags	flags )
{
	tAudioStartMidiFileInfo info;
	
	MPIInstanceState& mpiState = RetrieveMPIState( mpiID );

	// If the path passed to us is a full path, then use it, otherwise
	// append what was passed to the MPI's default path.
	CPath fullPath = (path[0] == '/')
			? path
			: *mpiState.path + path;

	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::StartMidiFile --  path = %s\n", fullPath.c_str() );	

	info.id = id;
	info.path = &fullPath;
	info.volume = volume;
	info.priority = priority;
	info.pListener = pListener;
	info.priority = priority;
	info.payload = payload;
	info.flags = flags;
	
	CAudioMsgStartMidiFile msg( info );
	
	SendCmdMessage( msg );
	
	return WaitForStatus();
}

tErrType CAudioModule::StartMidiFile( 	U32 				mpiID, 
										tMidiPlayerID		id,
										const CPath 		&path, 
										tAudioPayload		payload,
										tAudioOptionsFlags	flags )
{
	tAudioStartMidiFileInfo info;
	MPIInstanceState& 		mpiState = RetrieveMPIState( mpiID );
	
	// If the path passed to us is a full path, then use it, otherwise
	// append what was passed to the MPI's default path.
	CPath fullPath = (path[0] == '/')
			? path
			: *mpiState.path + path;

	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::StartMidiFile --  path = %s\n", fullPath.c_str() );	

	info.id = id;
	info.path = &fullPath;
	info.volume = mpiState.volume;
	info.priority = mpiState.priority;
	info.pListener = mpiState.pListener;
	info.payload = payload;
	info.flags = flags;
	
	CAudioMsgStartMidiFile msg( info );
	
	SendCmdMessage( msg );
	
	return WaitForStatus();
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsMidiFilePlaying( tMidiPlayerID id )
{
	CAudioMsgIsMidiFilePlaying 	msg( id );
	
	SendCmdMessage( msg );

	return WaitForBooleanResult();
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsMidiFilePlaying( void )
{
	CAudioMsgIsMidiFilePlaying 	msg;
	
	SendCmdMessage( msg );

	return WaitForBooleanResult();
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseMidiFile( tMidiPlayerID id )
{
	CAudioMsgPauseMidiFile msg( id );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeMidiFile( tMidiPlayerID id )
{
	CAudioMsgResumeMidiFile msg( id );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
void CAudioModule::StopMidiFile( tMidiPlayerID id, Boolean surpressDoneMessage )
{
	tAudioStopMidiFileInfo info;

	info.id = id;
	info.suppressDoneMsg = surpressDoneMessage;
	
	CAudioMsgStopMidiFile msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
tMidiTrackBitMask CAudioModule::GetEnabledMidiTracks( tMidiPlayerID id )
{
	tMidiTrackBitMask bm;
	U32 temp;
	CAudioMsgMidiFilePlaybackParams msg( id );
	
	SendCmdMessage( msg );

	temp = WaitForU32Result();
	
	bm = (tMidiTrackBitMask)temp;
	
	return bm;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::SetEnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask )
{
	CAudioMsgMidiFilePlaybackParams msg( id, trackBitMask );
	
	SendCmdMessage( msg );

	return WaitForStatus();
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, S8 transposeAmount )
{
	CAudioMsgMidiFilePlaybackParams msg( id, trackBitMask, transposeAmount );
	
	SendCmdMessage( msg );

	return WaitForStatus();
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr )
{
	CAudioMsgMidiFilePlaybackParams msg( id, trackBitMask, instr );
	
	SendCmdMessage( msg );

	return WaitForStatus();
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeMidiTempo( tMidiPlayerID id, S8 tempo ) 
{
	CAudioMsgMidiFilePlaybackParams msg( id, tempo );
	
	SendCmdMessage( msg );

	return WaitForStatus();
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::SendMidiCommand(/*tMidiPlayerID id, U8 cmd, U8 data1, U8 data2*/ )
{
	return kNoErr;
}
//==============================================================================
//==============================================================================
void CAudioModule::SendCmdMessage( CAudioCmdMsg& msg ) 
{
	tErrType err;
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SendCmdMessage -- Sending message to audio task. size = %d; type = %d\n", 
							msg.GetMessageSize(), msg.GetCmdType());	
	
    err = pKernelMPI_->SendMessage( hSendMsgQueue_, msg );
    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::SendCmdMessage -- After call SendMessage err = %d \n", 
    		static_cast<int>(err) );
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::WaitForAudioID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForAudioID -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForAudioID -- Could not get audio ID from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForAudioID -- Got ID = %d \n",
				static_cast<int>(msg->GetAudioID()));
	    	  
	return msg->GetAudioID();
}

//==============================================================================
//==============================================================================
tMidiPlayerID CAudioModule::WaitForMidiID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForMidiID -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForMidiID -- Could not get MIDI Player ID from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForMidiID -- Got ID = %d \n",
	    	  msg->GetMidiID() );  
	    	  
	return msg->GetMidiID();
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::WaitForStatus( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForStatus -- Waiting for message from audio task. MsgSize = %d.\n", sizeof(CAudioReturnMessage) );fflush(stdout);

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForStatus -- Could not get status from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	err = msg->GetAudioErr();
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForStatus -- Got status = %d \n",
	    	   static_cast<int>(err) );  
		    	  
	return err;
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::WaitForBooleanResult( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;
	Boolean					result;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForBooleanResult -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForBooleanResult -- Could not get result from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	result = msg->GetBooleanResult();
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForBooleanResult -- Boolean = %d \n",
	    	   static_cast<int>(result) );  
		    	  
	return result;
}

//==============================================================================
//==============================================================================
U32 CAudioModule::WaitForU32Result( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;
	U32						result;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForU32Result -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForU32Result -- Could not get U32 value from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	result = msg->GetU32Result();
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForU32Result -- Got U32 = %d \n",
	    	   static_cast<int>(result) );  
		    	  
	return result;
}


LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()
extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CAudioModule;
		return sinst;
	}

	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG

// EOF

