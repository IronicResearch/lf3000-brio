/*
 * (C) 2004 Mobileer, All Rights Reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pablio.h"
#include <string.h>

#include "me_demo.h"

#define SAMPLE_RATE           (22050)
#define NUM_SECONDS            (9999)
#define SAMPLES_PER_FRAME         (2)
#define FRAMES_PER_BLOCK    (8 * 256)

/* Mono buffer from video or other source. */
short inputBuffer[FRAMES_PER_BLOCK] = { 0 };
short mixedBuffer[FRAMES_PER_BLOCK * SAMPLES_PER_FRAME];

/*******************************************************************/
int main(void);
int main(void)
{
	int             i;
	int             result;
	PaError         err;
	PABLIO_Stream  *aOutStream;

	printf("Play Audio from Mobileer Demo using PortAudio.\n");
	fflush(stdout);

	/* Open simplified blocking I/O layer on top of PortAudio. */
	err = OpenAudioStream( &aOutStream, SAMPLE_RATE, paInt16,
	                       (PABLIO_WRITE | PABLIO_STEREO) );
	if( err != paNoError )
		goto error;

	result = MobileerDemo_Init( SAMPLE_RATE );

	for( i=0; i<(NUM_SECONDS * SAMPLE_RATE); i += FRAMES_PER_BLOCK )
	{

		result = MobileerDemo_Synthesize( inputBuffer,
		                                  mixedBuffer,
		                                  FRAMES_PER_BLOCK, SAMPLES_PER_FRAME );

		/* Write samples to output. */
		WriteAudioStream( aOutStream, mixedBuffer, FRAMES_PER_BLOCK );
	}

	CloseAudioStream( aOutStream );

	MobileerDemo_Term();

	printf(" ME Demo complete.\n" );
	fflush(stdout);
	return 0;

error:
	fprintf( stderr, "An error occured while using PABLIO\n" );
	fprintf( stderr, "Error number: %d\n", err );
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	return -1;
}
