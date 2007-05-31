#include <math.h>
#include <stdlib.h>
#include <stdio.h>#include <string.h>
#include "Dsputil.h"

#include "sfconfig.h"
#include "sndfile.h"

//#include <CoreTypes.h>
//#include "AudioTypes.h"
//#include <AudioConfig.h>
//#include <AudioOutput.h>

char *appName = "sf2brio";

#define TEST_SNDFILE_READ_MAX_SAMPLES 50000
short shortBlock[TEST_SNDFILE_READ_MAX_SAMPLES];
SNDFILE	*audioFile = NULL;
SF_INFO	audioFileInfo ;
long audioFileInfo_wordWidthBits = 16;
char audioFilePath[500];	

typedef unsigned long U32;
typedef unsigned long U16;
typedef U32 tRsrcType;

struct tAudioHeader {
	U32				offsetToData;		// Offset from the start of the header to
										// the start of the data (std is 16)
	tRsrcType		type;				// AudioRsrcType
	U16				flags;				// Bit mask of audio flags
										// (Bit0: 0=mono, 1=stereo)
	U16				sampleRateInHz;		// Sample rate in Hz			
	U32				dataSize;			// Data size in bytes
};

tAudioHeader brioFileHeader;

//==============================================================================
// OpenAudioFile  : Short term support for audio file playback
//
//					Return handle of sound file
//==============================================================================
SNDFILE	*OpenAudioFile( char *path, SF_INFO *afi)
{	     
long i = 0;
// printf("OpenAudioFile: start\n");
  
 for (i = 0; i < TEST_SNDFILE_READ_MAX_SAMPLES; i++)
	shortBlock[i] = 0;
 
// Initialize FILE in structure
	memset (afi, 0, sizeof(SF_INFO)) ;
// "sineorig.wav" "FightClub_first_time1.wav"
// "fragMono.wav" "BlueNile44m.wav"
// "SquareC2.wav" "noise44_30.wav"
// "FightClub_first_time_44100_16bit_stereo.wav"
// "FightClub_first_time_44100_16bit_mono.wav"
//strcpy(audioFilePath, ".wav");

printf("OpenAudioFile: gonna load '%s' \n", path);

//	format		= (SF_FORMAT_WAV | SF_FORMAT_PCM_16) ;
if (!(audioFile = sf_open (path, SFM_READ, afi)))
	{	
	printf ("OpenAudioFile : Not able to open input file '%s'\n", path) ;
	return (NULL);
	} 

strcpy(audioFilePath, path);
//printf("OpenAudioFile samplerate=%d frames=%d ch=%d format=%X\n",
//	   afi->samplerate, afi->frames, afi->channels, afi->format) ;

// Reject files that aren't WAV
long fileFormatMajor =  afi->format & SF_FORMAT_TYPEMASK;
long fileFormatMinor =  afi->format & SF_FORMAT_SUBMASK  ;
if (SF_FORMAT_WAV == fileFormatMajor)
{
	printf ("OpenAudioFile : This is a WAV file \n") ;
}
else if (SF_FORMAT_AIFF == fileFormatMajor)
{
	printf ("OpenAudioFile : This is a AIFF file \n") ;
}
else
{
	printf ("OpenAudioFile : Unsupported major file format type %X\n", fileFormatMajor) ;
	return (audioFile);
}

{
long audioFileInfo_wordWidthBits = 16;

if 	(SF_FORMAT_PCM_16 == fileFormatMinor)
	audioFileInfo_wordWidthBits = 16;
else if (SF_FORMAT_PCM_24 == fileFormatMinor)
	audioFileInfo_wordWidthBits = 24;
else if (SF_FORMAT_PCM_32 == fileFormatMinor)
	audioFileInfo_wordWidthBits = 32;
// FIXXXX: Don't support signed/unsigned 8-bit case separately
else if (SF_FORMAT_PCM_S8 == fileFormatMinor || SF_FORMAT_PCM_U8 == fileFormatMinor)
	audioFileInfo_wordWidthBits = 8;
else
{
	printf("OpenAudioFile: unsupported minor sound file format %X \n", fileFormatMinor);
	return (audioFile);
}

printf("OpenAudioFile samplerate=%d frames=%d ch=%d width=%d\n",
	   afi->samplerate, afi->frames, afi->channels, 
	   audioFileInfo_wordWidthBits) ;
}

	   	   	
	return (audioFile);
}

//==============================================================================
// CloseAudioFile  :   Close audio file structure
//
//        NOTE: Short term support for audio file playback
//
//					Return Boolean success
//==============================================================================
int CloseAudioFile( SNDFILE **audioFile )
{	     
//printf("CloseAudioFile: start\n");
 
 if (*audioFile)
	 {
	 sf_close(*audioFile) ;
	 *audioFile = NULL;
	return (True);
	 }
 	
//strcpy(audioFilePath, "");
	   	   	
return (False);
}

//==============================================================================
// RewindAudioFile  :   Seek to beginning of file
//
//        NOTE: Short term support for audio file playback
//
//					Return Boolean success
//==============================================================================
int RewindAudioFile( SNDFILE **audioFile, SF_INFO *afi, char *path )
{	     
//printf("RewindAudioFile: start 222\n");

// Quick hack for initial implementation
// FIXXXX: use seek()
CloseAudioFile(audioFile);
  	 
*audioFile = OpenAudioFile(path, afi);
 if (!*audioFile)
 {
 	printf("RewindAudioFile: unable to reopen '%s'\n", path);
	return (False);
 }
 
//printf("RewindAudioFile: end\n");

	return (True);
}

// **********************************************************************************
// PrintUsage:		
// ********************************************************************************** 
	void   
PrintUsage()
{
printf("Usage:  %s <infile> <outfile>\n", appName);
//printf("Usage:  %s [Options] <infile><outfile>\n", appName);
//printf("Options:\n");
//printf("    -v  verbose operation\n");
printf("\n");
}	// ---- end PrintUsage() ---- 

//============================================================================
// main()
//============================================================================
	int 
main(int argc, char *argv[]) 
{
SF_INFO *afi = &audioFileInfo;

#define MAX_FILENAME_LENGTH 500
char inFilePath[MAX_FILENAME_LENGTH];
char outFilePath[MAX_FILENAME_LENGTH];
	
inFilePath [0] = '\0';
outFilePath[0] = '\0';
	
if (argc < 3)
   {
    PrintUsage();
    exit(0);
    }
strcpy(inFilePath , argv[1]);
strcpy(outFilePath, argv[2]);
    
//printf("inFilePath  = '%s'\n", inFilePath);
//printf("outFilePath = '%s'\n", outFilePath);

audioFile = OpenAudioFile(inFilePath, &audioFileInfo);if (!audioFile)
	{
	printf("Unable to open file '%s'\n", inFilePath);
	exit(0);
	}

#define BLOCK_SIZE	1024
	long blockSize = BLOCK_SIZE;
	long framesRead = 0;
	short shortBlock[BLOCK_SIZE*2];
	long loopCount = afi->frames/blockSize;
	long remnants = afi->frames/blockSize;

	FILE *h = fopen(outFilePath, "wb");
	if (!h)
		{
		printf("Unable to open output file '%s'\n", outFilePath);
		CloseAudioFile(&audioFile);
		exit(-1);
		}

	// Write Brio audio file header
	fwrite(afi, sizeof(SF_INFO), 1, h);

	// Write audio file samples
	for (long i = 0; i < loopCount; i++)
		{
		framesRead = sf_readf_short(audioFile, shortBlock, blockSize);
//		printf("shortBlock[0]=%d\n", shortBlock[0]);
		fwrite(shortBlock, afi->channels*sizeof(short), blockSize, h);
		}
// Write remainder of samples
if (remnants)
	{
//	printf("writing remaining %d frames\n", remnants);
			framesRead = sf_readf_short(audioFile, shortBlock, remnants);
	fwrite(shortBlock, afi->channels*sizeof(short), remnants, h);
	}
			
fclose(h);
	CloseAudioFile(&audioFile);

return 0;
}
