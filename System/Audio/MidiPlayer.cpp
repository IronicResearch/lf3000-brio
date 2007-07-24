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

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CMidiPlayer implementation
//==============================================================================

CMidiPlayer::CMidiPlayer()
{
	S16			midiErr;

	pMidiRenderBuffer_ = new S16[kAudioOutBufSizeInWords];
	numFrames_ = 256;	/**< @todo fixme/rdg: don't hardcode this */
	samplesPerFrame_ = 2;
	bitsPerSample_ = 16;
	pListener_ = kNull;
	volume_ = 100;
	bFilePaused_ = false;
	bFileActive_ = false;
	bActive_ = false;
	
	// Initialize SPMIDI Library
	SPMIDI_Initialize();

	// Start SP-MIDI synthesis engine using the desired sample rate.
	midiErr = SPMIDI_CreateContext( &pContext_, kAudioSampleRate );  // fixme run fixed 1/2 sample rate
	if (midiErr < 0)
	{
//		printf("SPMIDI CreateContext error!: %d\n", midiErr);
	}

	// fixme/dg: for now only one player for whole system!  maybe add support for multiple players.
	pFilePlayer_ = kNull;
	
	// Set the maximum number of voices
	SPMIDI_SetMaxVoices( pContext_, 16 );
}

//==============================================================================
//==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	bFileActive_ = false;
	
	if (pFilePlayer_ != kNull)
		MIDIFilePlayer_Delete( pFilePlayer_ );

	SPMIDI_DeleteContext( pContext_ );
	SPMIDI_Terminate();
	
	delete pMidiRenderBuffer_;
}

/*
//==============================================================================
//==============================================================================
void CMidiPlayer::Stop()
{
}

//==============================================================================
//==============================================================================
void CMidiPlayer::Pause()
{
}

//==============================================================================
//==============================================================================
void CMidiPlayer::Resume()
{
}
*/

//==============================================================================
//==============================================================================
tErrType 	CMidiPlayer::NoteOn( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags )
{
//	printf("MidiPlayer -- NoteOn: chan: %d, note: %d, vel: %d, flags: %d\n", 
//				channel, noteNum, velocity, flags );
 
	SPMUtil_NoteOn( pContext_, (int) channel, (int) noteNum, (int) velocity );

	return kNoErr;
}

tErrType 	CMidiPlayer::NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags )
{
//	printf("MidiPlayer -- NoteOn: chan: %d, note: %d, vel: %d, flags: %d\n", 
//				channel, noteNum, velocity, flags );
 
	SPMUtil_NoteOff( pContext_, (int) channel, (int) noteNum, (int) velocity );

	return kNoErr;
}

void CMidiPlayer::SendDoneMsg( void ) {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgDataCompleted	data;
	data.audioID = 99;	// dummy
	data.payload = 101;	// dummy
	data.count = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
	
	// fixme/dg: need to find a better way to do done listeners
	// support multiple midi files playing at the same time.
	//pListener_ = kNull;
}

tErrType 	CMidiPlayer::StartMidiFile( tAudioStartMidiFileInfo* 	pInfo ) 
{
	tErrType result = 0;
	
	// If a pre-emption is happening, delete the previous player.
	if (pFilePlayer_ != kNull) {
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
	result = MIDIFilePlayer_Create( &pFilePlayer_, (int)kAudioSampleRate, pInfo->pMidiFileImage, pInfo->imageSize );
//		if( result < 0 )
//			printf("Couldn't create a midifileplayer!\n");
	
	bFileActive_ = true;
	
	return result;
}

tErrType 	CMidiPlayer::PauseMidiFile( void ) 
{
	tErrType result = 0;
	
	if (bFileActive_)
		bFilePaused_ = true;
	
	return result;
}

tErrType 	CMidiPlayer::ResumeMidiFile( void ) 
{
	tErrType result = 0;
	
	if (bFileActive_)
		bFilePaused_ = false; 
	
	return result;
}

tErrType 	CMidiPlayer::StopMidiFile( tAudioStopMidiFileInfo* pInfo ) 
{
	tErrType result = 0;;
	
	bFileActive_ = false;
	bFilePaused_ = false;
	if ((pListener_ != kNull) && !pInfo->suppressDoneMsg)
		SendDoneMsg();

	SPMUtil_Reset( pContext_ );
	MIDIFilePlayer_Delete( pFilePlayer_ );
	pFilePlayer_ = kNull;
	pListener_ = kNull;
	
	return result;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::EnableTracks(tMidiTrackBitMask trackBitMask)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::TransposeTracks(tMidiTrackBitMask trackBitMask, S8 transposeAmount)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::ChangeInstrument(tMidiTrackBitMask trackBitMask, tMidiInstr instr)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::ChangeTempo(S8 Tempo)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CMidiPlayer::SendCommand(U8 cmd, U8 data1, U8 data2)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
U32	CMidiPlayer::RenderBuffer( S16* pMixBuff, U32 numStereoFrames )
{
	U32 	i, midiLoopCount;
	int 	mfp_result;
	U32		spmidiFramesPerBuffer;
	S16* 	pBuffer = pMidiRenderBuffer_;  // local ptr to buffer for indexing
	S32 	outSamp;
	S32 	sum;
	U32 	framesRead = 0;
	U32 	numStereoSamples = numStereoFrames * kAudioBytesPerSample;

	// Initialize the output buffer to 0
	bzero( pMidiRenderBuffer_, kAudioOutBufSizeInBytes );
	
	// Get a local copy of buffer ptr that we can modify.
	pBuffer = pMidiRenderBuffer_;
	

//	printf("CMidiPlayer::RenderBuffer -- numFrames %d, bFileActive = %ul, bFilePaused_ = %d\n", 
//			(int)numStereoFrames, (int)bFileActive_, (int)bFilePaused_);

	// If there is a midi file player, service it
	if ( bFileActive_ && !bFilePaused_ ) {
		// fixme/rdg: make this bulletproof.  Right now no check for sizes.
		// Figure out how many calls to spmidi we need to make to get a full output buffer
		spmidiFramesPerBuffer = SPMIDI_GetFramesPerBuffer();
		midiLoopCount = numStereoFrames / spmidiFramesPerBuffer;

		for (i = 0; i < midiLoopCount; i++) {
			mfp_result = MIDIFilePlayer_PlayFrames( pFilePlayer_, pContext_, spmidiFramesPerBuffer );
			if (mfp_result < 0)
				printf("!!!!! CMidiPlayer::RenderBuffer -- MIDIFilePlayer PlayFrames failed!!!\n");
	
			// fixme/dg: rationalize numStereoFrames and numFrames_!!
			framesRead = SPMIDI_ReadFrames( pContext_, pBuffer, spmidiFramesPerBuffer,
		    		samplesPerFrame_, bitsPerSample_ );
	
			pBuffer += framesRead * 2;
// 			printf("CMidiPlayer::RenderBuffer -- loop %d, framesRead = %ul, pBuffer = %d\n", i, framesRead, pBuffer);
			
			// Returning a 1 means MIDI file done.
			if (mfp_result > 0) {
				if (loopMidiFile_) {
					MIDIFilePlayer_Rewind( pFilePlayer_ ); 
				} else {
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
		framesRead = SPMIDI_ReadFrames( pContext_, pMidiRenderBuffer_, numFrames_,
	    		samplesPerFrame_, bitsPerSample_ );
	}
	
//	printf("!!!!! CMidiPlayer::RenderBuffer -- framesRead: %u, pContext_ 0x%x, pMidiRenderBuffer_ 0x%x!!!\n\n", framesRead, pContext_, pMidiRenderBuffer_);

	// fixme/dg: mix to 32 bit buffer and clip once after
	pBuffer = pMidiRenderBuffer_;
	for ( i = 0; i < numStereoSamples; i++)
	{
		// Be sure the total sum stays within range
		outSamp = (S32)*pBuffer++;
		outSamp = (outSamp * volume_) >> 7; //  Apply gain
		sum = *pMixBuff + (S16)outSamp;			

		if (sum > kS16Max) sum = kS16Max;
		else if (sum < kS16Min) sum = kS16Min;
		
		// Convert mono midi render to stereo output
		*pMixBuff++ = sum;
//		*pMixBuff = sum;
	}

	return framesRead;
}

LF_END_BRIO_NAMESPACE()
// EOF	
