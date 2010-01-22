//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioOutput.c
//
// Description:
//		Implements audio output on top of PortAudio.
//
//	  03/06/07	rdg	 PortAudio version for Lightning "Emulation"
//	  06/26/06	ytu	 Initial version
//==============================================================================
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <CoreTypes.h>

#include <AudioConfig.h>
#include <AudioOutput.h>

#include "portaudio.h"

#include <DebugMPI.h>
#include <DebugTypes.h>
#include <GroupEnumeration.h>
#include <KernelMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

// Enables callback to Brio, vs. sine output
#define USE_REAL_CALLBACK	1

#if USE_REAL_CALLBACK
#define kAudioWordType	paInt16
#else
#define kAudioWordType	paFloat32

// For debug out test tone
#ifndef M_PI
#define M_PI  (3.14159265)
#endif
#define TABLE_SIZE	 (200)
typedef struct
{
	float sine[TABLE_SIZE];
	int left_phase;
	int right_phase;
}
paTestData;
#endif


//==============================================================================
// Global variables
//==============================================================================
static PaStream* gPaStream = NULL;
static BrioAudioRenderCallback* gAudioRenderCallback = NULL;
static void* gCallbackUserData = NULL;

// Debug output
static U32 gCallbackCount = 0;			
#ifndef USE_REAL_CALLBACK
static paTestData gTestData;
#endif

//==============================================================================
// PortAudio callback (which is faking a DMA-triggered ISR)
//==============================================================================

static int paCallback(const void* inputBuffer,
					  void* outputBuffer,
					  unsigned long framesPerBuffer,
					  const PaStreamCallbackTimeInfo* timeInfo,
					  PaStreamCallbackFlags statusFlags,
					  void* userData)

{
	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags;
	(void) inputBuffer;
	(void ) userData;  /* Prevent unused variable warnings. */ 
	// Keep count for debugging purposes...
	gCallbackCount++;

#if USE_REAL_CALLBACK
	int ret = kNoErr;
	extern CKernelMPI *pKernelMPI_;

	// Call audio system callback to fill buffer
	do {
		ret = gAudioRenderCallback( (S16*)outputBuffer, framesPerBuffer, gCallbackUserData );
		if (ret != kNoErr)
			pKernelMPI_->TaskSleep(10);
	} while (ret != kNoErr);
#else
	for( i=0; i<framesPerBuffer; i++ )
	{
		*out++ = gTestData.sine[gTestData.left_phase ];	 
		*out++ = gTestData.sine[gTestData.right_phase];
		gTestData.left_phase += 1;
		if( gTestData.left_phase >= TABLE_SIZE ) 
			gTestData.left_phase -= TABLE_SIZE;
		gTestData.right_phase += 3; /* higher pitch so we can distinguish left and right. */
		if( gTestData.right_phase >= TABLE_SIZE ) 
			gTestData.right_phase -= TABLE_SIZE;
	}
#endif
  
	return paContinue;
}



//==============================================================================
// InitAudioOutput	
//==============================================================================
int InitAudioOutput( BrioAudioRenderCallback* callback, void* pUserData )
{	
	int i;
	PaError err;
	PaStreamParameters outputParameters;
	CDebugMPI          dbg(kGroupAudio);
	
	gCallbackCount = 0;
	gAudioRenderCallback = callback;
	gCallbackUserData = pUserData;
	
	// Init PortAudio layer
	err = Pa_Initialize();
	if( err != paNoError ) goto error;

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device < 0) {
		err = outputParameters.device;
		goto error;
	}
	
	outputParameters.channelCount = kAudioNumOutputChannels; // stereo output
	outputParameters.sampleFormat = kAudioWordType;
	outputParameters.hostApiSpecificStreamInfo = NULL;
  
	//printf("PaDefaultOutputLatency = %f\n", outputParameters.suggestedLatency);
	//TODO/dg: figure out how to handle latency properly on various platforms.
#if 0 //def EMULATION
	outputParameters.suggestedLatency = .05;
#else
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
#endif	  

	err = Pa_OpenStream( &gPaStream,
						 NULL,				/* No input. */
						 &outputParameters, 
						 kAudioSampleRate,
						 kAudioFramesPerBuffer,
						 paClipOff,
						 paCallback,
						 NULL );
 
	if( err != paNoError ) goto error;
	
#ifndef USE_REAL_CALLBACK
	// initialise sinusoidal wavetable for debug output
	for( i=0; i<TABLE_SIZE; i++ )
		gTestData.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. );
	gTestData.left_phase = gTestData.right_phase = 0;
#endif
	return err;

 error:
	Pa_Terminate();
	dbg.DebugOut( kDbgLvlCritical, "An error occured while attempting to initalize the portaudio stream\n" );
	dbg.DebugOut( kDbgLvlCritical, "Error number: %d\n", err );
	dbg.DebugOut( kDbgLvlCritical, "Error message: %s\n", Pa_GetErrorText( err ) );

	return err;
}

// ==============================================================================
// StartAudioOutput	 
// ==============================================================================
int StartAudioOutput( void )
{
	CDebugMPI dbg(kGroupAudio);
	PaError err = Pa_StartStream( gPaStream );
	if ( err != paNoError ) 
		dbg.DebugOut( kDbgLvlCritical, "StartAudioOutput: Error%d '%s' occured during Pa_StartStream\n",
				err , Pa_GetErrorText(err));
	return err;
}

// ==============================================================================
// StopAudioOutput	
// ==============================================================================
int StopAudioOutput( void )
{
	CDebugMPI dbg(kGroupAudio);
	PaError err = Pa_StopStream( gPaStream );
	if ( err != paNoError ) 
		dbg.DebugOut( kDbgLvlCritical, "StopAudioOutput: Error%d '%s' occured during Pa_StopStream\n",
				err , Pa_GetErrorText(err));	
	return err;
}

//==============================================================================
// DeInitAudioOutput  
//==============================================================================
int DeInitAudioOutput( void )
{
	Pa_CloseStream( gPaStream );
	Pa_Terminate();
	
	return 0;
}
LF_END_BRIO_NAMESPACE()
// EOF
