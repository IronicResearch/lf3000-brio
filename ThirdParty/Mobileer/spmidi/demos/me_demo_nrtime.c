/*
 * (C) 2004 Mobileer, All Rights Reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "me_demo.h"

#define SAMPLE_RATE           (22050)
#define NUM_SECONDS            (60 * 5)
#define SAMPLES_PER_FRAME        (2)
#define FRAMES_PER_BLOCK   (7 * 256)

/* Mono buffer from video or other source. */
short inputBuffer[FRAMES_PER_BLOCK] = { 0 };
short mixedBuffer[FRAMES_PER_BLOCK * SAMPLES_PER_FRAME];

/*******************************************************************/
int main(void);
int main(void)
{
	int             i;
	int             result;

	printf("Play Audio from Mobileer Demo using PortAudio.\n");
	fflush(stdout);

	result = MobileerDemo_Init( SAMPLE_RATE );

	for( i=0; i<(NUM_SECONDS * SAMPLE_RATE); i += FRAMES_PER_BLOCK )
	{

		result = MobileerDemo_Synthesize( inputBuffer,
		                                  mixedBuffer,
		                                  FRAMES_PER_BLOCK, SAMPLES_PER_FRAME );
	}

	MobileerDemo_Term();

	printf(" ME Demo complete.\n" );
	fflush(stdout);
	return 0;

}
