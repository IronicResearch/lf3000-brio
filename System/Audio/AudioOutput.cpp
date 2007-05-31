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

#include <AudioConfig.h>
#include <AudioOutput.h>

#include "portaudio.h"
#include "rt_trace.h"

// this turns on callback to Brio, v.s. sine test output
#define USE_REAL_CALLBACK	1

#if USE_REAL_CALLBACK
#define kSampleFormat	paInt16
#else
#define kSampleFormat	paFloat32
#endif

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
static PaStream* gPaStream;							// PortAudio stream context
static BrioAudioRenderCallback* gAudioRenderCallback;
static void* gCallbackUserData;

// Debug output
static U32 gCallbackCount;			
static paTestData gTestData;
static unsigned long t1, t2, t3;	// debugging

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

	// Keep count for debugging purposes...
    gCallbackCount++;

    // figure time since last callback
//     t3 = gettime_usecs() - t2;
 //    t3 /= 1000; // convert to ms
// 	if (t3 > 200)
//		printf("\n\nAudioOutput:: pa_callback -- not called in time!!!! %u ms.\n\n\n", t3);

 	//	RT_AddTraceMessage( "Calling Mixer Render", gCallbackCount );
 //    t1 = gettime_usecs();

#if USE_REAL_CALLBACK
	// fixme/dg: do proper DebugMPI-based output.
//	printf("OutputDriver PortAudioCallback bufPtr: %x, frameCount: %d \n", outputBuffer, framesPerBuffer );
 
    // Call audio system callback to fill buffer
	gAudioRenderCallback( (S16*)outputBuffer, framesPerBuffer, gCallbackUserData );

//	RT_AddTraceMessage( "Back from Mixer Render", gCallbackCount );
     
/*
	for( i = 0; i < framesPerBuffer; i++ ) {
		if (out[i] != 0.0F)
       	 printf(" i = %d; samp = %f\n", i, out[i]);
	}
*/	
//	if (gCallbackCount == 50)
//		RT_DumpTraceMessages();
		
#else
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = gTestData.sine[gTestData.left_phase];  // left
        *out++ = gTestData.sine[gTestData.right_phase];  // right
        gTestData.left_phase += 1;
        if( gTestData.left_phase >= TABLE_SIZE ) gTestData.left_phase -= TABLE_SIZE;
        gTestData.right_phase += 3; /* higher pitch so we can distinguish left and right. */
        if( gTestData.right_phase >= TABLE_SIZE ) gTestData.right_phase -= TABLE_SIZE;
    }
#endif
  
//    t2 = gettime_usecs();
// 	if ((t2 - t1) > 10000)
 //		printf("\n\nAudioOutput:: pa_callback -- body of callback took longer than 1ms!!\n\n\n");

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
	
    outputParameters.channelCount = kAudioNumOutputChannels;                     /* stereo output */
    outputParameters.sampleFormat = kSampleFormat;             
    // outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    //printf("PaDefaultOutputLatency = %f\n", outputParameters.suggestedLatency);
 	//fixme/dg: figure out how to handle latency properly on various platforms.
    outputParameters.suggestedLatency = .05;
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

//	if (gpAudioOutBuffer)
//		memset(gpAudioOutBuffer, 0, kAudioOutBufSizeInBytes);
	
	return err;
}

int DeInitAudioOutput( void )
{
	Pa_CloseStream( gPaStream );
	Pa_Terminate();
	
	return 0;
}
// EOF
