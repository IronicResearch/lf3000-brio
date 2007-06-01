/* $Id: playmf_me2000.c,v 1.3 2006/02/22 01:05:28 philjmsl Exp $ */
/**
 *
 * Play a MIDI File using the ME1000 Synthesizer.
 * Input filename is passed on the command line.
 * demonstrates track enable and other features.
 * 
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


#include "midi.h"
#include "spmidi.h"
#include "spmidi_util.h"
#include "spmidi_load.h"
#include "spmidi_play.h"
#include "midifile_player.h"
#include "portaudio.h"

#include "midifile_names.h"

/* Set velocity based EQ which attenuates low notes if pitch < 64. */
/* #define VEQ_GAIN_AT_ZERO   (0) */
#define VEQ_GAIN_AT_ZERO   (32)

#define MAX_VOICES          (SPMIDI_MAX_VOICES)
#define NUM_CHANNELS        (2)
#define SAMPLE_RATE         (44100)
#define OPTIMIZE_VOLUME     (0)
#define COMPRESSOR_ONOFF    (1)

//#define SOLO_TRACK          (1)

/* Use these macros so that we don't block in getchar(). */
#ifdef WIN32
#define GETCH        _getch
#define KBHIT        _kbhit
#define PUTCH(c)     _putch((char)(c))
#endif

/****************************************************************/
static void usage( void )
{
	printf("playmf [-rRATE] [-wWAVFILE] [-mMAXVOICES]\n");
	printf("       [-nNUMREPS] [-cCHANNELS] [-vRHYTHMVOLUME] midiFileName\n");
	printf("Hit keys to control playback:\n");
	printf(" 0-9 = enter index\n");
	printf(" c = Show number of voices on each Channel\n");
	printf(" d = Disable indexed channel\n");
	printf(" D = Disable all channels\n");
	printf(" e = Enable indexed channel\n");
	printf(" E = Enable all channels\n");
	printf(" g = Go to indexed frame position.\n");
	printf(" m = set Max voices to index\n");
	printf(" p = Pause\n");
	printf(" r = Resume\n");
	printf(" s = print Status of instruments and notes playing\n");
	printf(" t = disable indexed Track\n");
	printf(" T = enable indexed Track\n");
	printf(" v = set rhythm volume to index\n");
	printf(" ? = print this message\n");
	printf(" q = quit\n");
	fflush(stdout);
}

/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyTextCallback( int trackIndex, int metaEventType,
                     const char *addr, int numChars, void *userData )
{
	int i;
	(void)userData;
	printf("MetaEvent type %d on track %d: ", metaEventType, trackIndex );
	for( i=0; i<numChars; i++ )
		printf("%c", addr[i] );
	printf("\n");
}

/****************************************************************/
static void ShowTrackStatus( MIDIFilePlayer *player )
{
	int i;
	int numTracks = MIDIFilePlayer_GetTrackCount( player );
	printf("\n");
	for( i=0; i<numTracks; i++ )
	{
		int c = (MIDIFilePlayer_GetTrackEnable( player, i ) > 0) ? '+' : '-';
		printf("%2d%c,", i, c );
	}
	printf("\n");
	fflush(stdout);
}

/****************************************************************/
static void ShowChannelEnableStatus( SPMIDI_Context *spmidiContext )
{
	int i;
	printf("\n");
	for( i=0; i<MIDI_NUM_CHANNELS; i++ )
	{
		int c = (SPMIDI_GetChannelEnable( spmidiContext, i ) > 0) ? '+' : '-';
		printf("%2d%c,", i, c );
	}
	printf("\n");
	fflush(stdout);
}

/****************************************************************/
static void ShowChannelNoteCounts( SPMIDI_Context *spmidiContext )
{
	int i;
	printf("\n");
	for( i=0; i<MIDI_NUM_CHANNELS; i++ )
	{
		int numNotes = SPMIDI_GetChannelActiveNoteCount( spmidiContext, i );
		if( numNotes > 0 )
			printf("%d:%d, ", i, numNotes );
	}
	printf("\n");
	fflush(stdout);
}

/****************************************************************/
static void EnableOrDisableAllChannels( SPMIDI_Context *spmidiContext, int onOrOff )
{
	int i;
	for( i=0; i<MIDI_NUM_CHANNELS; i++ )
	{
		SPMIDI_SetChannelEnable( spmidiContext, i, onOrOff );
	}
}


/****************************************************************/
int MIDIFile_Play( unsigned char *image, int numBytes, const char *fileName,
                   int sampleRate, int maxVoices, int numLoops, int numChannels,
                   int rhythmVolume )
{
	int result;
	int go = 1;
	int quit = 0;
	int paused = 0;
	int timeout;
	int msec;
	int seconds;
	int rem_msec;
	SPMIDI_Context *spmidiContext = NULL;
	MIDIFilePlayer *player = NULL;

#if OPTIMIZE_VOLUME

	int maxAmplitude;
	int testVolume = 32;
	int masterVolume;
#endif

	int value = 0;
	int oldValue, newValue;

	SPMIDI_Initialize();

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) sampleRate, image, numBytes );
	if( result < 0 )
		goto error1;

	msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
	seconds = msec / 1000;
	rem_msec = msec - (seconds * 1000);
	printf("Duration = %d.%03d seconds\n", seconds, rem_msec );

#if OPTIMIZE_VOLUME
	/* Get maximum amplitude so that we can calculate optimal volume.
	 */
	printf("Please wait while we pre-render MIDI file to measure amplitude.\n" );
	printf("This pre-rendering step is optional.\n" );
	fflush(stdout);

	result = SPMIDI_CreateContext( &spmidiContext, sampleRate );
	if( result < 0 )
		goto error1;
	SPMIDI_SetMaxVoices( spmidiContext, maxVoices );
	SPMIDI_SetMasterVolume( spmidiContext, testVolume );
	
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, COMPRESSOR_ONOFF );
	if( result < 0 )
		goto error2;

	maxAmplitude = SPMUtil_MeasureMaxAmplitude( player, spmidiContext, numChannels );
	SPMIDI_DeleteContext( spmidiContext );

	if( maxAmplitude <= 0 )
		goto error1;
	printf("At masterVolume %d, maxAmplitude is %d\n", testVolume, maxAmplitude );
	MIDIFilePlayer_Rewind( player );
#endif

	/*
	 * Set function to be called when a text MetaEvent is
	 * encountered while playing. Includes Lyric events.
	 */
	MIDIFilePlayer_SetTextCallback( player, MyTextCallback, NULL );

	/*
	 * Initialize SPMIDI Synthesizer.
	 * Output to audio or a named file.
	 */
	result = SPMUtil_Start( &spmidiContext, sampleRate, fileName, numChannels );
	if( result < 0 )
		goto error1;

#ifdef SOLO_TRACK

	EnableOrDisableAllTracks( player, 0 ); // disable all
	result = MIDIFilePlayer_SetTrackEnable( player, SOLO_TRACK, 1 );
	if( result < 0 )
		goto error2;
#endif

	result = SPMIDI_SetMaxVoices( spmidiContext, maxVoices );
	if( result < 0 )
		goto error2;

#if OPTIMIZE_VOLUME
	/* Calculate masterVolume that will maximize the signal amplitude without clipping. */
	masterVolume = (testVolume * 32767) / maxAmplitude;
	printf("New masterVolume = %d\n", masterVolume );
	SPMIDI_SetMasterVolume( spmidiContext, masterVolume );
#endif
		

	/* Turn compressor on or off and report final setting. */
	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, COMPRESSOR_ONOFF );
	if( result < 0 )
		goto error2;

	result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_COMPRESSOR_ON, &newValue );
	if( result < 0 )
		goto error2;
	printf("SPMIDI_PARAM_COMPRESSOR_ON: %d\n", newValue );


	/* Set velocity equalization parameters to reduce contribution from low notes.
	 * On a mobile phone you can't hear very low notes but they
	 * are in the mix and compete with higher notes.
	 */
	result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_BASS_CUTOFF, &oldValue );
	if( result < 0 )
		goto error2;

	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_VEQ_BASS_CUTOFF, 60 );
	if( result < 0 )
		goto error2;

	result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_BASS_CUTOFF, &newValue );
	if( result < 0 )
		goto error2;
	printf("VEQ_BASS_CUTOFF: %d => %d\n", oldValue, newValue );

	result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_GAIN_AT_ZERO, &oldValue );
	if( result < 0 )
		goto error2;

	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_VEQ_GAIN_AT_ZERO, VEQ_GAIN_AT_ZERO );
	if( result < 0 )
		goto error2;

	result = SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_VEQ_GAIN_AT_ZERO, &newValue );
	if( result < 0 )
		goto error2;
	printf("VEQ_GAIN_AT_ZERO: %d => %d\n", oldValue, newValue );

	result = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, rhythmVolume );
	if( result < 0 )
	{
		printf("\nERROR: Illegal value for Rhythm Volume = %d\n", rhythmVolume );
	}
	else
	{
		printf("SPMIDI_PARAM_RHYTHM_VOLUME set to %d\n", rhythmVolume );
	}

	usage();

	/* Accumulate numberic digit as track or frame number in value. */
	value = 0;
	while( go )
	{
		if( paused )
		{
			Pa_Sleep( 100 ); /* So we don't hog the CPU completely. */
		}
		else
		{
			/*
			 * Play until we hit the end of all tracks.
			 * Tell the MIDI player to move forward on all tracks
			 * by a time equivalent to a buffers worth of frames.
			 */
			if( SPMUtil_PlayFileBuffer( player, spmidiContext ) != 0 )
			{
				if( --numLoops <= 0 )
				{
					go = 0;
				}
				else
				{
					/* Rewind song. */
					SPMUtil_Reset( spmidiContext );
					MIDIFilePlayer_Rewind( player );
				}
			}
		}

		if( KBHIT() ) /* Has a key been hit? */
		{
			int c = GETCH();
			PUTCH(c);
			/* Parse key input. */
			if( (c >= '0') && (c <= '9') )
			{
				value = (value * 10) + (c - '0');
			}
			else if( c == 't' )
			{
				/* Disable track. */
				MIDIFilePlayer_SetTrackEnable( player, value, 0 );
				ShowTrackStatus( player );
				value = 0;
			}
			else if( c == 'T' )
			{
				/* Enable track. */
				MIDIFilePlayer_SetTrackEnable( player, value, 1 );
				ShowTrackStatus( player );
				value = 0;
			}
			else if( c == 'd' )
			{
				/* Disable channel. */
				SPMIDI_SetChannelEnable( spmidiContext, value, 0 );
				ShowChannelEnableStatus( spmidiContext );
				value = 0;
			}
			else if( (c == 'D') || (c == 'a') )
			{
				EnableOrDisableAllChannels( spmidiContext, 0 ); // disable all
				ShowChannelEnableStatus( spmidiContext );
			}
			else if( c == 'e' )
			{
				/* Enable channel. */
				SPMIDI_SetChannelEnable( spmidiContext, value, 1 );
				ShowChannelEnableStatus( spmidiContext );
				value = 0;
			}
			else if( (c == 'E') || (c == 'A') )
			{
				EnableOrDisableAllChannels( spmidiContext, 1 ); // enable all
				ShowChannelEnableStatus( spmidiContext );
			}
			else if( c == 'g' )
			{
				/* Seek to specified frame value. */
				MIDIFilePlayer_GoToFrame( player, spmidiContext, value );
				value = 0;
			}
			else if( c == 'c' )
			{
				ShowChannelNoteCounts(spmidiContext);
				value = 0;
			}
			else if( c == 's' )
			{
				SPMIDI_PrintStatus(spmidiContext);
				value = 0;
			}
			else if( c == 'm' )
			{
				SPMIDI_SetMaxVoices( spmidiContext, value );
				value = 0;
				printf("\nMax voices = %d\n", SPMIDI_GetMaxVoices(spmidiContext) );
			}
			else if( c == 'v' )
			{
				int err = SPMIDI_SetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, value );
				if( err < 0 )
				{
					printf("\nERROR: Illegal value for Rhythm Volume = %d\n", value );
				}
				SPMIDI_GetParameter( spmidiContext, SPMIDI_PARAM_RHYTHM_VOLUME, &value);
				printf("\nRhythm Volume = %d\n", value );
				value = 0;
			}
			else if( c == 'p' )
			{
				paused = 1;
			}
			else if( c == 'r' )
			{
				paused = 0;
			}
			else if( c == 'q' )
			{
				quit = 1;
				break;
			}
			else
			{
				usage();
			}

			if( value == 0 )
			{
				int centiseconds = MIDIFilePlayer_GetFrameTime( player ) * 100 / sampleRate;
				int fraction = centiseconds % 100;
				int seconds = centiseconds / 100;
				int minutes = seconds / 60;
				seconds -= minutes * 60;

				printf("\nt=%d:%d.%d, fr=%d, mtks=%d > ",
				       minutes, seconds, fraction,
				       MIDIFilePlayer_GetFrameTime( player ),
				       MIDIFilePlayer_GetTickTime( player )
				      );
				fflush(stdout);
			}
		}
	}

	printf("\nPlayback complete.\n");
	fflush(stdout);

	if( quit == 0 )
	{
		timeout = (sampleRate * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
		while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
		{
			SPMUtil_PlayBuffers( spmidiContext, 1 );
		}
	}
	/* Play a few extra buffers to make sure we get last tail of sound. */
	SPMUtil_PlayBuffers( spmidiContext, 4 );

	SPMIDI_PrintStatus(spmidiContext);
error2:
	SPMUtil_Stop(spmidiContext);

	MIDIFilePlayer_Delete( player );

	SPMIDI_Terminate();
error1:
	return result;
}

/****************************************************************/
int main( int argc, char ** argv )
{
	int   i;
	void *data = NULL;
	int   fileSize;
	int   result = -1;
	int   maxVoices = MAX_VOICES;
	int   numLoops = 1;
	int   numChannels = NUM_CHANNELS;
	int   rhythmVolume = SPMIDI_DEFAULT_MASTER_VOLUME;
	int   sampleRate = SAMPLE_RATE;
	char *inputFileName = DEFAULT_FILENAME;

char *outputFileName = NULL;
//char *outputFileName = "playmf_output.wav";

	printf("(C) 2004 Mobileer, Inc. All Rights Reserved\n");

	/* Parse command line. */
	for( i=1; i<argc; i++ )
	{
		char *s = argv[i];
		if( s[0] == '-' )
		{
			switch( s[1] )
			{
			case 'w':
				outputFileName = &s[2];
				break;
			case 'r':
				sampleRate = atoi( &s[2] );
				break;
			case 'c':
				numChannels = atoi( &s[2] );
				break;
			case 'm':
				maxVoices = atoi( &s[2] );
				break;
			case 'n':
				numLoops = atoi( &s[2] );
				break;
			case 'v':
				rhythmVolume = atoi( &s[2] );
				break;
			case 'h':
			case '?':
				usage();
				break;
			}
		}
		else
		{
			inputFileName = argv[i];
		}
	}

	data = SPMUtil_LoadFileImage( inputFileName, &fileSize );
	if( data == NULL )
	{
		printf("ERROR could not find MIDI File %s\n", inputFileName );
		goto error;
	}

	if( inputFileName != NULL )
		printf("Input MIDI File: %s\n", inputFileName );
	if( outputFileName != NULL )
		printf("Output WAV File: %s\n", outputFileName );

	printf("Sample Rate: %d\n", sampleRate );

	result = MIDIFile_Play( data, fileSize, outputFileName,
	                        sampleRate, maxVoices, numLoops, numChannels, rhythmVolume );
	if( result < 0 )
	{
		printf("Error playing MIDI File = %d = %s\n", result,
		       SPMUtil_GetErrorText( result ) );
		goto error;
	}

	printf("\n");
	fflush(stdout);

error:
	if( data != NULL )
		free( data );
	return result;
}
