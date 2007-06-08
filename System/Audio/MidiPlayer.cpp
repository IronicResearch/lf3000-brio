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
	numFrames_ = 256;
	samplesPerFrame_ = 2;
	bitsPerSample_ = 16;
	pListener_ = kNull;
	volume_ = 1.0;
	
	// Initialize SPMIDI Library
	SPMIDI_Initialize();

	// Start SP-MIDI synthesis engine using the desired sample rate.
	midiErr = SPMIDI_CreateContext( &pContext_, kAudioSampleRate );
	if (midiErr < 0)
	{
//		printf("SPMIDI CreateContext error!: %d\n", midiErr);
	}

	// fixme/dg: for now only one player for whole system!  add support for multiple players.
	pFilePlayer_ = kNull;
	
	// Set the maximum number of voices
	SPMIDI_SetMaxVoices( pContext_, 16 );
}

//==============================================================================
//==============================================================================
CMidiPlayer::~CMidiPlayer()
{
	SPMIDI_DeleteContext( pContext_ );
	delete pMidiRenderBuffer_;
	if (pFilePlayer_ != kNull)
		MIDIFilePlayer_Delete( pFilePlayer_ );

	SPMIDI_Terminate();
}


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
	pListener_ = kNull;
}

tErrType 	CMidiPlayer::PlayMidiFile( tAudioMidiFileInfo* 	pInfo ) 
{
	tErrType result;
	
//	pInfo->midiID;			// fixme/dg: midiEngineContext object?
//	pInfo->hRsrc;			// Resource Handle, provided by app, returned from FindResource()
	volume_ = ((float)pInfo->volume) / 100.0F;
//	pInfo->priority;
	if ( pInfo->pListener != kNull )
		pListener_ = pInfo->pListener;
//	pInfo->payload;
	
	// fixme/dg: Right now only allow one midi file to play at a time!
	if (pFilePlayer_ == kNull) {
			
		if (pInfo->flags & 1)
			loopMidiFile_ = true;
	
		// Create a player, parse MIDIFile image and setup tracks.
		result = MIDIFilePlayer_Create( &pFilePlayer_, (int)kAudioSampleRate, pInfo->pMidiFileImage, pInfo->imageSize );
//		if( result < 0 )
//			printf("Couldn't create a midifileplayer!\n");
	} else {
		result = -1;
//		printf("CMidiPlayer::PlayMidiFile -- Can't play a new midifile while already playing one!\n");
	}
	
	return result;
}

//==============================================================================
//==============================================================================
U32	CMidiPlayer::RenderBuffer( S16* pMixBuff, U32 numStereoFrames )
{
	U32 	i;
	int 	mfp_result;
	S16* 	pMidiRenderBuffer = pMidiRenderBuffer_;  // local ptr to buffer for indexing
	float 	outSamp;
	S32 	sum;
	U32 	framesRead = 0;
	U32 	numStereoSamples = numStereoFrames * kAudioBytesPerSample;

//	if (numStereoFrames != numFrames_) 
//		printf("!!!!! CMidiPlayer::RenderBuffer -- System frames per buffer and midi frames per buffer disagree!!!\n\n");
	
	// If there is a midi file player, service it
	if (pFilePlayer_ != kNull) {
		mfp_result = MIDIFilePlayer_PlayFrames( pFilePlayer_, pContext_, SPMIDI_GetFramesPerBuffer()  );
//		if (mfp_result < 0)
//		{
//			printf("!!!!! CMidiPlayer::RenderBuffer -- MIDIFilePlayer PlayFrames failed!!!\n\n");
//		}
		// Returning a 1 means MIDI file done.
		if (mfp_result > 0) {
			if (loopMidiFile_) {
				MIDIFilePlayer_Rewind( pFilePlayer_ ); 
			} else {
				if (pListener_ != kNull)
					SendDoneMsg();
				MIDIFilePlayer_Delete( pFilePlayer_ );
				pFilePlayer_ = kNull;
			}
		}
	}
	
	// fixme/dg: rationalize numStereoFrames and numFrames_!!
	framesRead = SPMIDI_ReadFrames( pContext_, pMidiRenderBuffer_, numFrames_,
    		samplesPerFrame_, bitsPerSample_ );

//	printf("!!!!! CMidiPlayer::RenderBuffer -- framesRead: %u, pContext_ 0x%x, pMidiRenderBuffer_ 0x%x!!!\n\n", framesRead, pContext_, pMidiRenderBuffer_);

	// fixme/dg: mix to 32 bit buffer and clip once after
	for ( i = 0; i < numStereoSamples; i++)
	{
		// Be sure the total sum stays within range
		outSamp = (float)*pMidiRenderBuffer++;
		outSamp *= volume_; //  Apply gain
		sum = *pMixBuff + (S16)outSamp;			

		if (sum > kS16Max) sum = kS16Max;
		else if (sum < kS16Min) sum = kS16Min;
		
		*pMixBuff++ = sum;
	}

	return framesRead;
}

// EOF	
