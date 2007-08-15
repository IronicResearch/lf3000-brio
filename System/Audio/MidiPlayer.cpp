//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
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
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define kSamplesPerFrame   (2)
#define kBitsPerSample     (sizeof(S16)*8)
#define kNumMIDIChannels	16

#define kMIDI_SamplingFrequencyDivisor (2)
#define kMIDI_SamplingFrequency     (kAudioSampleRate/kMIDI_SamplingFrequencyDivisor)

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CMidiPlayer implementation
//==============================================================================

CMidiPlayer::CMidiPlayer( tMidiPlayerID id )
{
	tErrType			err;
	S16					midiErr;
	Boolean				ret;
	const tMutexAttr 	attr = {0};
	
	// Setup member vars...
	id_ = id;
	
	pMidiRenderBuffer_ = new S16[kAudioOutBufSizeInWords];

	numFrames_       = 256;	/**< @todo fixme/rdg: don't hardcode this */
	samplesPerFrame_ = kSamplesPerFrame;
	bitsPerSample_   = kBitsPerSample;

	pListener_ = kNull;
	volume_ = 100;
	bFilePaused_ = false;
	bFileActive_ = false;
	bActive_ = false;

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
	pDebugMPI_->Assert((kNoErr == err), "CMidiPlayer::ctor: Couldn't init mutex.\n");
	
	// Initialize SPMIDI Library
	SPMIDI_Initialize();

	// Start SP-MIDI synthesis engine using the desired sample rate.
	midiErr = SPMIDI_CreateContext( &pContext_, kMIDI_SamplingFrequency);  
	pDebugMPI_->Assert((midiErr == kNoErr), "CAudioModule::ctor: SPMIDI_CreateContext() failed.\n");

	// fixme/dg: for now only one player for whole system!  maybe add support for multiple players.
	pFilePlayer_ = kNull;
	
	// Set the maximum number of voices
	SPMIDI_SetMaxVoices( pContext_, 16 );
}

//==============================================================================
//==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	tErrType 				result;
	tAudioStopMidiFileInfo 	info;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::dtor -- Cleaning up.\n");	
	
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
	
	if (pFilePlayer_ != kNull)
		MIDIFilePlayer_Delete( pFilePlayer_ );

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
}

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::NoteOn( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CMidiPlayer::NoteOn -- chan: %d, note: %d, vel: %d, flags: %d\n", channel, noteNum, velocity, static_cast<int>(flags) );
 
	SPMUtil_NoteOn( pContext_, (int) channel, (int) noteNum, (int) velocity );

	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::NoteOff -- chan: %d, note: %d, vel: %d, flags: %d\n", channel, noteNum, velocity, static_cast<int>(flags) );
 
	SPMUtil_NoteOff( pContext_, (int) channel, (int) noteNum, (int) velocity );

	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::SendCommand( U8 cmd, U8 data1, U8 data2 )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::SendCommand -- ...\n");	

	SPMIDI_WriteCommand( pContext_, (int)cmd, (int)data1, (int)data2 );

	return kNoErr;
}

//==============================================================================
//==============================================================================

void CMidiPlayer::SendDoneMsg( void ) {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgMidiCompleted	data;
	data.midiPlayerID = id_;
	data.payload = 0;	
	data.count = 1;

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::SendDoneMsg -- Sending done event.\n");	
	
	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
	
	// fixme/dg: need to find a better way to do done listeners
	// support multiple midi files playing at the same time.
	//pListener_ = kNull;
}

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::StartMidiFile( tAudioStartMidiFileInfo* 	pInfo ) 
{
	tErrType result = 0;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StartMidiFile -- Starting...\n");	

	// If a pre-emption is happening, delete the previous player.
	if (pFilePlayer_ != kNull) {
		pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CMidiPlayer::StartMidiFile -- deleting active player...\n");	
		MIDIFilePlayer_Delete( pFilePlayer_ );
		pFilePlayer_ = kNull;
	}

//	pInfo->midiID;			// fixme/dg: midiEngineContext object?
//	pInfo->hRsrc;			// Resource Handle, provided by app, returned from FindResource()
	volume_ = pInfo->volume;
//	pInfo->priority;
	if ( pInfo->pListener != kNull )
		pListener_ = pInfo->pListener;
//	pInfo->payload;
	
	if (pInfo->flags & 1)
		loopMidiFile_ = true;

	// Create a player, parse MIDIFile image and setup tracks.
	result = MIDIFilePlayer_Create( &pFilePlayer_, (int)kMIDI_SamplingFrequency, pInfo->pMidiFileImage, pInfo->imageSize );
//		if( result < 0 )
//			printf("Couldn't create a midifileplayer!\n");
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StartMidiFile -- setting bFileActive_ to true...\n");	
	
	bFileActive_ = true;
	
	return result;
}

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::PauseMidiFile( void ) 
{
	tErrType result = 0;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::PauseMidiFile -- setting bFilePaused_ to true...\n");	
	
	if (bFileActive_)
		bFilePaused_ = true;
	
	return result;
}

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::ResumeMidiFile( void ) 
{
	tErrType result = 0;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::ResumeMidiFile -- setting bFilePaused_ to false...\n");	

	if (bFileActive_)
		bFilePaused_ = false; 
	
	return result;
}

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::StopMidiFile( tAudioStopMidiFileInfo* pInfo ) 
{
	tErrType result = 0;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StopMidiFile -- Entering method, about to lock render_mutex...\n");	

	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::StopMidiFile -- Couldn't lock mutex.\n");

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StopMidiFile -- setting bFilePaused_ and bFileActive_ to false...\n");	

	bFileActive_ = false;
	bFilePaused_ = false;
	if ((pListener_ != kNull) && !pInfo->suppressDoneMsg)
		SendDoneMsg();

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::StopMidiFile -- resetting engine and deleting player...\n");	

	SPMUtil_Reset( pContext_ );
	MIDIFilePlayer_Delete( pFilePlayer_ );
	pFilePlayer_ = kNull;
	pListener_ = kNull;
	
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::StopMidiFile -- Couldn't unlock mutex.\n");

	return result;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::GetEnableTracks( tMidiTrackBitMask* trackBitMask ) 
{
	U32					chan;
	tMidiTrackBitMask	mask = 0;
	int					isEnabled;
	
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::GetEnableTracks -- ...\n");	

	// Loop through the channels and set mask
	for (chan = 0; chan < kNumMIDIChannels; chan++) {
		isEnabled = SPMIDI_GetChannelEnable( pContext_, chan );
		if (isEnabled){
			mask |= 1 << chan;
		}
	}

	*trackBitMask = mask;
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
#define kMIDITrackEnable		1
#define kMIDITrackDisable		0

tErrType CMidiPlayer::SetEnableTracks( tMidiTrackBitMask trackBitMask )
{
	U32					chan;
	tMidiTrackBitMask	mask;

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CMidiPlayer::SetEnableTracks -- ...\n");	

	// Loop through the channels and set mask
	for (chan = 0; chan < kNumMIDIChannels; chan++) {
		mask = 0;					// clear it out
		mask = 1 << chan;			// set the bit corresponding to this chan
		if (trackBitMask && mask)	// if it's set, enable the channel
			SPMIDI_SetChannelEnable( pContext_, chan, kMIDITrackEnable );
		else
			SPMIDI_SetChannelEnable( pContext_, chan, kMIDITrackDisable );
	}

	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::TransposeTracks( tMidiTrackBitMask trackBitMask, S8 transposeAmount )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::TransposeTracks -- ...\n");	

	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::ChangeProgram( tMidiTrackBitMask trackBitMask, tMidiInstr instr )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::ChangeProgram -- ...\n");	

	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::ChangeTempo( S8 Tempo )
{
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CMidiPlayer::ChangeTempo -- ...\n");	

	return kNoErr;
}

//==============================================================================
//==============================================================================
U32	CMidiPlayer::RenderBuffer( S16* pOutBuff, U32 numStereoFrames, long addToOutput )
{
	tErrType result;
	U32 	i, midiLoopCount;
	int 	mfp_result;
	U32		spmidiFramesPerBuffer;
	S16* 	pBuffer = pMidiRenderBuffer_;  // local ptr to buffer for indexing
	S32 	sum;
	U32 	framesRead = 0;
	U32 	numStereoSamples = numStereoFrames * kAudioBytesPerSample;
	
	result = pKernelMPI_->TryLockMutex( render_mutex_ );
	if (result == EBUSY)
		return numStereoFrames;
	else
		pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::RenderBuffer -- Couldn't lock mutex.\n");
	
	// Initialize the output buffer to 0
	bzero( pMidiRenderBuffer_, kAudioOutBufSizeInBytes );
	
	// Get a local copy of buffer ptr that we can modify.
	pBuffer = pMidiRenderBuffer_;
	
//	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//		"CMidiPlayer::RenderBuffer -- numStereoFramesRequested = %d, bFileActive = %u, bFilePaused_ = %d\n", 
//		(int)numStereoFrames, (int)bFileActive_, (int)bFilePaused_);

	// If there is a midi file player, service it
	if ( bFileActive_ && !bFilePaused_ ) {
		// fixme/rdg: make this bulletproof.  Right now no check for sizes.
		// Figure out how many calls to spmidi we need to make to get a full output buffer
		spmidiFramesPerBuffer = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / spmidiFramesPerBuffer;

//		pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//			"CMidiPlayer::RenderBuffer -- spmidiFramesPerBuffer = %d, midiLoopCount = %d\n", 
//			(int)spmidiFramesPerBuffer, (int)midiLoopCount );
			
		for (i = 0; i < midiLoopCount; i++) {
//			pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//				"CMidiPlayer::RenderBuffer -- About to MIDIFilePlayer_PlayFrames()\n");
			mfp_result = MIDIFilePlayer_PlayFrames( pFilePlayer_, pContext_, spmidiFramesPerBuffer );
			if (mfp_result < 0)
				printf("!!!!! CMidiPlayer::RenderBuffer -- MIDIFilePlayer PlayFrames failed!!!\n");
	
			// fixme/dg: rationalize numStereoFrames and numFrames_!!
//			pDebugMPI_->DebugOut(kDbgLvlVerbose, 
//				"CMidiPlayer::RenderBuffer -- About to SPMIDI_ReadFrames()\n");
			framesRead = SPMIDI_ReadFrames( pContext_, pBuffer, spmidiFramesPerBuffer,
		    		samplesPerFrame_, bitsPerSample_ );
	
			pBuffer += framesRead * kSamplesPerFrame;
// 			printf("CMidiPlayer::RenderBuffer -- loop %d, framesRead = %ul, pBuffer = %d\n", i, framesRead, pBuffer);
			
			// Returning a 1 means MIDI file done.
			if (mfp_result > 0) {
				if (loopMidiFile_) {
					pDebugMPI_->DebugOut(kDbgLvlVerbose, 
						"CMidiPlayer::RenderBuffer -- file done, looping...\n");	
					MIDIFilePlayer_Rewind( pFilePlayer_ ); 
				} else {
					pDebugMPI_->DebugOut(kDbgLvlVerbose, 
						"CMidiPlayer::RenderBuffer -- file done, deleting player and resetting engine...\n");	
					if (pListener_ != kNull)
						SendDoneMsg();
					MIDIFilePlayer_Delete( pFilePlayer_ );
					SPMUtil_Reset( pContext_ );
					pFilePlayer_ = kNull;
					break; // bail out of loop.
				}
			}
		}
	} else {
		// A midi file is not playing, but notes might be turned on programatically...
		// fixme/dg: rationalize numStereoFrames and numFrames_!!
		pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CMidiPlayer::RenderBuffer -- About to SPMIDI_ReadFrames(), no file active\n");
		framesRead = SPMIDI_ReadFrames( pContext_, pMidiRenderBuffer_, numFrames_,
	    		samplesPerFrame_, bitsPerSample_ );
	}
	
//	printf("!!!!! CMidiPlayer::RenderBuffer -- framesRead: %u, pContext_ 0x%x, pMidiRenderBuffer_ 0x%x!!!\n\n", framesRead, pContext_, pMidiRenderBuffer_);

	// fixme/dg: mix to 32 bit buffer and clip once after
	pBuffer = pMidiRenderBuffer_;
	if (addToOutput)
		{
		for ( i = 0; i < numStereoSamples; i++)
			{
		 //  Apply gain
			sum = pOutBuff[i] + (S16)((volume_ * (S32)pMidiRenderBuffer_[i])>>7);			
		// Saturate to 16-bit range
			if      (sum > kS16Max) sum = kS16Max;
			else if (sum < kS16Min) sum = kS16Min;
		// Convert mono midi render to stereo output  GK: ?
			pOutBuff[i] = sum;
			}
		}
	else
		{
		for ( i = 0; i < numStereoSamples; i++)
			{
		 //  Apply gain
			sum = (S16)((volume_ * (S32)pMidiRenderBuffer_[i])>>7);			
		// Saturate to 16-bit range
			if      (sum > kS16Max) sum = kS16Max;
			else if (sum < kS16Min) sum = kS16Min;
		// Convert mono midi render to stereo output  GK: ?
			pOutBuff[i] = sum;
			}
		}
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CMidiPlayer::RenderBuffer -- Couldn't unlock mutex.\n");

	return framesRead;
}

LF_END_BRIO_NAMESPACE()
// EOF	
