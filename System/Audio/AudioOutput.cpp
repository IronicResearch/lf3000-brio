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
#include <string.h>

#include <CoreTypes.h>

#include <AudioConfig.h>
#include <AudioOutput.h>

#include "portaudio.h"

#include <DebugMPI.h>
#include <DebugTypes.h>
#include <GroupEnumeration.h>
#include <KernelMPI.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <linux/soundcard.h>

LF_BEGIN_BRIO_NAMESPACE()

// Enables direct callback vs PortAudio (embedded only)
#ifndef EMULATION
#define USE_DIRECT_CALLBACK	1
#else
#define USE_DIRECT_CALLBACK	0
#endif

// Enables ALSA for callback vs OSS
#define USE_ALSA_CALLBACK	1

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
static bool bAlsaEnabled = false;

// Debug output
static U32 gCallbackCount = 0;			
#ifndef USE_REAL_CALLBACK
static paTestData gTestData;
#endif


#if USE_DIRECT_CALLBACK

//==============================================================================
// Callback variables	
//==============================================================================
extern CKernelMPI*	pKernelMPI_;
volatile bool 		bRunning = true;
volatile bool 		bRendering = false;
int 				fddsp = -1;
tTaskHndl 			hndlThread = 0;
S16*				pOutputBuffer = NULL;
U32					nFramesPerBuffer = kAudioFramesPerBuffer;
int					nBytesPerBuffer = kAudioOutBufSizeInBytes; // Brio (4K)
int					nBytesPerOutput = kAudioOutBufSizeInBytes; // OSS  (2K)

//==============================================================================
// Callback thread	
//==============================================================================
static void* CallbackThread(void* pCtx)
{
	int r = 0;
	struct pollfd fdlist[1];
	memset(fdlist, 0, sizeof(fdlist));
	fdlist[0].fd = fddsp;
	fdlist[0].events = POLLOUT;
	
	while (bRunning)
	{
		if (bRendering)
		{
			// Brio render callback 
			do { 
				r = gAudioRenderCallback(pOutputBuffer, nFramesPerBuffer, pCtx);
				if (r != kNoErr)
					pKernelMPI_->TaskSleep(10);
			}
			while (r != kNoErr);

			// Output Brio render buffer (4K) to OSS output buffer (2K)
			U8* pOut = (U8*)pOutputBuffer;
			for (int i = 0; i < nBytesPerBuffer; i += nBytesPerOutput, pOut += nBytesPerOutput)
			{
				r = poll(fdlist, 1, -1);
	
				if ((r > 0) && (fdlist[0].revents & POLLOUT)) 
				{
					write(fddsp, pOut, nBytesPerOutput);
				}
			}
		}
		else
		{
			pKernelMPI_->TaskSleep(10);
		}
	}
}
 
//==============================================================================
// InitAudioOutput	
//==============================================================================
int InitAudioOutput( BrioAudioRenderCallback* callback, void* pUserData )
{	
	gAudioRenderCallback = callback;
	gCallbackUserData = pUserData;

#if USE_ALSA_CALLBACK
	if (kNoErr == InitAudioOutputAlsa(callback, pUserData)) {
		bAlsaEnabled = true;
		return kNoErr;
	}
#endif
	
	// Open audio driver
	fddsp = open("/dev/dsp", O_WRONLY);
	if (fddsp < 0)
		return fddsp;
	
	int temp = AFMT_S16_LE; // 0x10;
    int r = ioctl( fddsp, SNDCTL_DSP_SETFMT, &temp );
    if (r < 0)
    	return r;
    
    temp = kAudioNumOutputChannels;
    r = ioctl( fddsp, SNDCTL_DSP_CHANNELS, &temp );
    if (r < 0)
    	return r;
    
    temp = kAudioSampleRate;
    r = ioctl( fddsp, SNDCTL_DSP_SPEED, &temp );
    if (r < 0)
    	return r;

    temp = 0;
	r = ioctl( fddsp, SNDCTL_DSP_GETBLKSIZE, &temp );
	if (r < 0)
		return r;
	
	// Allocate output buffer for render callbacks
	// NOTE: OSS buffer size (2K) does not equal Brio buffer size (4K)
	nBytesPerOutput = temp; // != kAudioOutBufSizeInBytes
	nBytesPerBuffer = kAudioOutBufSizeInBytes;
	pOutputBuffer = (S16*)pKernelMPI_->Malloc(nBytesPerBuffer);
	if (!pOutputBuffer)
		return -1;

	// Create callback thread
	tTaskProperties props;
	props.TaskMainFcn = &CallbackThread;
	props.taskMainArgCount = 1;
	props.pTaskMainArgValues = pUserData;
	r = pKernelMPI_->CreateTask(hndlThread, props, NULL);
	if (r != kNoErr)
		return r;
	
	return kNoErr; // 0
}

// ==============================================================================
// StartAudioOutput	 
// ==============================================================================
int StartAudioOutput( void )
{
#if USE_ALSA_CALLBACK
	if (bAlsaEnabled)
		return StartAudioOutputAlsa();
#endif
	// Enable rendering callbacks for output
	bRendering = true;
	return 0;
}

// ==============================================================================
// StopAudioOutput	
// ==============================================================================
int StopAudioOutput( void )
{
#if USE_ALSA_CALLBACK
	if (bAlsaEnabled)
		return StopAudioOutputAlsa();
#endif
	// Disable rendering callbacks for output
	bRendering = false;
	return 0;
}

//==============================================================================
// DeInitAudioOutput  
//==============================================================================
int DeInitAudioOutput( void )
{
#if USE_ALSA_CALLBACK
	if (bAlsaEnabled)
		return DeInitAudioOutputAlsa();
#endif
	// Kill callback thread
	void* retval;
	bRunning = false;
	pKernelMPI_->JoinTask(hndlThread, retval);

	// Release resources
	pKernelMPI_->Free(pOutputBuffer);
	pOutputBuffer = NULL;
	
	// Close audio driver
	close(fddsp);
	fddsp = -1;
	
	return 0;
}

#else // USE_PORT_AUDIO

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

#endif // USE_PORT_AUDIO

LF_END_BRIO_NAMESPACE()
// EOF
