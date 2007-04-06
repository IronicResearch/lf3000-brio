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
//    03/06/07  rdg  PortAudio version for Lightning "Emulation"
//    06/26/06  ytu  Initial version
//==============================================================================
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <CoreTypes.h>

#include <AudioPriv.h>
#include <AudioOutput.h>


#include "portaudio.h"
#include "rt_trace.h"

// For debug out test tone
#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)
typedef struct
{
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
}
paTestData;

//==============================================================================
// Global variables
//==============================================================================
static PaStream *gPaStream;			// PortAudio stream context

// Debug output
static U32 gCallbackCount;			
static paTestData gTestData;
static S16 gpAudioOutBuffer[kAudioOutBufSizeInWords];

//==============================================================================
// PortAudio callback (which is faking a DMA-triggered ISR)
//==============================================================================
static int paCallback(const void*						inputBuffer,
						void*                           outputBuffer,
						unsigned long                   framesPerBuffer,
						const PaStreamCallbackTimeInfo* timeInfo,
						PaStreamCallbackFlags           statusFlags,
						void*                           userData)

{
    float *out = (float*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = gTestData.sine[gTestData.left_phase];  // left
        *out++ = gTestData.sine[gTestData.right_phase];  // right
        gTestData.left_phase += 1;
        if( gTestData.left_phase >= TABLE_SIZE ) gTestData.left_phase -= TABLE_SIZE;
        gTestData.right_phase += 3; /* higher pitch so we can distinguish left and right. */
        if( gTestData.right_phase >= TABLE_SIZE ) gTestData.right_phase -= TABLE_SIZE;
    }
    
    return paContinue;
}
	
/*	U32 i;
    (void) inputBuffer; // Prevent unused argument warning.
	S16* outBuffPtr = (S16*)outputBuffer;
	S16* pMixBuf;
	
	if (audioOutIndex == 0)
	{
		RT_AddTraceMessage( "paCallback -- Switching to outputBuffer1", gCallbackCount );
		audioOutIndex = 1;
		pMixBuf = gpAudioOutBuffer + outSizeInBytes;
	}
	else if (audioOutIndex == 1)
	{
		RT_AddTraceMessage( "paCallback -- Switching to outputBuffer0", gCallbackCount );
		audioOutIndex = 0;
		pMixBuf = gpAudioOutBuffer;
	}
	
	// Copy mix buffer to driver's output buffer.
	if (kAudioNumOutputChannels == 2) 
	{
		for( i=0; i<framesPerBuffer; i++ ) {
	        *outBuffPtr++ = *pMixBuf++;  // left 
	        *outBuffPtr++ = *pMixBuf++;  // right
	     }
	} else if (kAudioNumOutputChannels == 1) 
	{
		for( i=0; i<framesPerBuffer; i++ ) {
	        *outBuffPtr++ = *pMixBuf++;  // mono
	     }
	}
	
 	 gCallbackCount++;
 	 
     return 0;
*/


//==============================================================================
// InitAudioOutput  
//==============================================================================
int InitAudioOutput( void )
{	
	int i;
	PaError err;
    PaStreamParameters outputParameters;
    
	// Allocate a double buffer for the audio out buffer (each has size 32 * 4 * 4)
//	gpAudioOutBuffer = 
//		(S16 *)malloc( kNumAudioOutBuffer * kAudioOutBufSizeInBytes );

	// Init PortAudio layer
    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device < 0) {
		err = outputParameters.device;
		goto error;
	}
	
    outputParameters.channelCount = 2;                     /* stereo output */
    outputParameters.sampleFormat = paFloat32;		// fixme RDG = paInt16;             
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream( &gPaStream,
                         NULL,              /* No input. */
                         &outputParameters, /* As above. */
                         kAudioSampleRate,
                         kAudioFramesPerBuffer,
                         paClipOff,         /* No out of range samples expected. */
                         paCallback,
                         NULL );
 
   if( err != paNoError ) goto error;
	
    // initialise sinusoidal wavetable for debug output
    for( i=0; i<TABLE_SIZE; i++ )
    {
        gTestData.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. );
    }
    gTestData.left_phase = gTestData.right_phase = 0;

	return err;

error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while attempting to initalize the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );

	return err;
}

//==============================================================================
// StartAudioOut  
//==============================================================================
int StartAudioOutput( void )
{
	PaError err;
	err = Pa_StartStream( gPaStream );
	if( err != paNoError ) {
	    fprintf( stderr, "An error occured while attempting to start the portaudio stream\n" );
	    fprintf( stderr, "Error number: %d\n", err );
	    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );		
	}
 
 	return err;
 }

//==============================================================================
// StopAudioOut  
//==============================================================================
int StopAudioOutput( void )
{
	PaError err;
	err = Pa_StopStream( gPaStream );
	if( err != paNoError ) {
	    fprintf( stderr, "An error occured while attempting to stop the portaudio stream\n" );
	    fprintf( stderr, "Error number: %d\n", err );
	    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );		
	}

	if (gpAudioOutBuffer)
		memset(gpAudioOutBuffer, 0, kAudioOutBufSizeInBytes);
		
	return err;
}

int DeInitAudioOutput( void )
{
	Pa_CloseStream( gPaStream );
	Pa_Terminate();
	
	return 0;
}
// EOF
