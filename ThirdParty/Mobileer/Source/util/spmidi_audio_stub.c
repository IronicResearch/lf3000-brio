/* $Id: spmidi_audio_stub.c,v 1.1 2005/05/09 00:05:26 philjmsl Exp $ */
/**
 * Stub for audio functions used by the Mobileer examples.
 * This is often used on simulators and can be used as a template
 * for a platform specific impementation.
 *
 * Copyright 2002 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 */

#include "pablio.h"
#include "spmidi_audio.h"

/****************************************************************/
/* Just keep the linker happy. */
int SPMUtil_StartAudio( SPMIDI_AudioDevice *devicePtr, int sampleRate, int samplesPerFrame )
{
	(void) devicePtr;
	(void) sampleRate;
	(void) samplesPerFrame;
	return SPMIDI_Error_Unsupported;
}

/****************************************************************/
void SPMUtil_WriteAudioBuffer( SPMIDI_AudioDevice device, short *audioSamples, int numFrames )
{
	(void) device;
	(void) audioSamples;
	(void) numFrames;
}

/****************************************************************/
int SPMUtil_StopAudio( SPMIDI_AudioDevice device )
{
	(void) device;
	return 0;
}
