/*
 * Benchmark a single instrument with many voices.
 * Copyright 2002 Mobileer
 *
 */
#include <stdio.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_print.h"
#include "bench_tools.h"
#include "spmidi/include/song_player.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

//#define FILE_NAME  ("carumba.mxmf")
//#define FILE_NAME    ("charge_alaw.mxmf")
//#define FILE_NAME    ("charge.mxmf")
//#define FILE_NAME    ("TalkinReggae.mxmf")
#define FILE_NAME    ("wantya.mxmf")

#define FRAME_RATE              (22050)
#define SAMPLES_PER_FRAME       (1)
#define NUM_LOOPS               (1)

unsigned char  *fileStart;
int             fileSize;

/*******************************************************************/
int Benchmark_Init( void )
{

	/* Load the file into memory */
	printf("Attempt to load file %64s\n", FILE_NAME );
	fileStart = SPMUtil_LoadFileImage( FILE_NAME, &( fileSize ) );
	if( fileStart == NULL )
	{
		printf("ERROR: file %s not found.\n", FILE_NAME );
		return -1;
	}
	
	SPMIDI_Initialize();
	
	return 0;
}

/*******************************************************************/
int Benchmark_Run( long *checkSumPtr, long *numFramesPtr, long *frameRatePtr )
{

	SPMIDI_Context *spmidiContext;
	SongPlayer     *songPlayerContext;
	int  result;
	int i;
	
	for( i=0; i<NUM_LOOPS; i++ )
	{
		spmidiContext = NULL;
		songPlayerContext = NULL;
	
		result = SPMIDI_CreateContext( &spmidiContext, FRAME_RATE );
		if( result < 0 ) goto error;

		/* Create a player for the song */
		result = SongPlayer_Create( &songPlayerContext, spmidiContext,
			(unsigned char  *) fileStart, fileSize );
		if( result < 0 ) goto error;

		if( songPlayerContext != NULL )
		{
			SongPlayer_Delete( songPlayerContext );
		}
			
		SPMIDI_DeleteContext(spmidiContext);
	}
	
	*checkSumPtr = 0xCAFEBABE;
	*numFramesPtr = FRAME_RATE;
	*frameRatePtr = FRAME_RATE;

	return 0;

error:
	if( result < 0 )
	{
		PRTMSG("Error parsing file = ");
		PRTNUMD( result );
		PRTMSG( SPMUtil_GetErrorText( (SPMIDI_Error)result ) );
		PRTMSG("\n");
	}
	return result;

}

/*******************************************************************/
int Benchmark_Term( void )
{
	SPMIDI_Terminate();
	return 0;
}

#if 0
int main( void )
{
	long checkSum = 0;
	long frameRate;
	long numFrames;
	Benchmark_Init();
	Benchmark_Run( &checkSum, &numFrames, &frameRate );
	Benchmark_Term();
	printf("checkSum = 0x%08lX\n", checkSum);
	printf("numFrames = %ld\n", numFrames);
	printf("frameRate = %ld\n", frameRate);
	return 0;
}
#endif

