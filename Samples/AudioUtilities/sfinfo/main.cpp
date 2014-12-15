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

char *appName = "sfinfo";

//#define TEST_SNDFILE_READ_MAX_SAMPLES 50000
//short shortBlock[TEST_SNDFILE_READ_MAX_SAMPLES];

// Libsndfile definitions
//SNDFILE	*audioFile = NULL;
//SF_INFO	audioFileInfo ;
//long audioFileInfo_wordWidthBits = 16;

// Brio audio file definitions
typedef unsigned long U32;
typedef unsigned short U16;
typedef U32 tRsrcType;

struct tAudioHeader {
	U32	  offsetToData;	 // Offset from the start of the header to
				 // the start of the data (std is 16)
	tRsrcType type;		 // AudioRsrcType
	U16	  flags;	 // Bit mask of audio flags
				 // (Bit0: 0=mono, 1=stereo)
	U16	  sampleRate;	 // Sample rate in Hz			
	U32	  dataSize;      // Data size in bytes
};

long verbose = True;

// ************************************************************************
// PrintBrioAudioFileHeader
// ************************************************************************
	void
PrintBrioAudioFileHeader(tAudioHeader *d)
{
long i = 0;
char *channels = "mono";

printf("PrintBrioAudioFileHeader: $%X \n", d);
printf("    offsetToData = %d\n", d->offsetToData);
printf("    type         = $%X\n", d->type);

if (d->flags & 1)
	channels = "stereo";
else 
	channels = "mono";

printf("    flags        = $%X '%s'\n", d->flags, channels);
printf("    sampleRate   = %d Hz\n", d->sampleRate);

long frames = d->dataSize/(16/8); // Assumes 16-bit word width, 2 bytes / word
if (d->flags & 1)
	frames /= 2;
float seconds = ((float) frames)/(float)d->sampleRate;
printf("    dataSize     = %d (%d frames, %g seconds)\n", d->dataSize, frames, seconds);

//har *bd = (char *) d;
//for (i = 0; i < sizeof(tAudioHeader); i++)
//	printf("%02x \n", bd[i]);
} // ---- end PrintBrioAudioFileHeader() ----

// ************************************************************************
// PrintSndFileInfo
// ************************************************************************
	void
PrintSndFileInfo(SF_INFO *d)
{
long fileFormatMinor =  d->format & SF_FORMAT_SUBMASK;
long wordWidthBits = 16;

if 	    (SF_FORMAT_PCM_16 == fileFormatMinor)
	wordWidthBits = 16;
else if (SF_FORMAT_PCM_24 == fileFormatMinor)
	wordWidthBits = 24;
else if (SF_FORMAT_PCM_32 == fileFormatMinor)
	wordWidthBits = 32;
// FIXXXX: Don't support signed/unsigned 8-bit case separately
else if (SF_FORMAT_PCM_S8 == fileFormatMinor || SF_FORMAT_PCM_U8 == fileFormatMinor)
	wordWidthBits = 8;
else
{
	printf("PrintSndFileInfo: unsupported minor sound file format %X \n", fileFormatMinor);
}

printf("PrintSndFileInfo: $%X \n", d);
printf("    wordWidth = %d bits\n", wordWidthBits);
printf("    channels  = %d\n", d->channels);
printf("    fs        = %d Hz\n", d->samplerate);
float seconds = ((float) d->frames)/(float)d->samplerate;
printf("    frames    = %d (%g seconds)\n", d->frames, seconds);

} // ---- end PrintSndFileInfo() ----

// ************************************************************************
// IsBrioAudioFile:	Open file and determine if it's a Brio raw type
//				return Boolean success
// ************************************************************************
	long
IsBrioAudioFile(char *path)
{
FILE *inH = NULL;
tAudioHeader bfh;

// Open file
inH = fopen(path, "rb");
if (!inH)
	{
	printf ("IsBrioAudioFile : Not able to open input file '%s'\n", path) ;
	return (False);
	} 

// Attempt to read Brio file header
long bytesToRead = sizeof(tAudioHeader);
long bytesRead = fread(&bfh, sizeof(char), bytesToRead, inH);
if (bytesRead != sizeof(tAudioHeader))
	{
	printf("IsBrioAudioFile: read only %/% bytes \n", bytesRead, bytesToRead);
	fclose(inH);
	return (False);
	}

// Check Brio file header
if (0x10001C05 == bfh.type) // Brio raw type 
	{
	if (verbose)
		PrintBrioAudioFileHeader(&bfh);
	fclose(inH);
	return (True);
	}

fclose(inH);
return (False);
} // ---- end IsBrioAudioFile() ----

// ************************************************************************
// IsWAV:	Open file and determine if it's a Microsoft WAV file
//				Return Boolean success
// ************************************************************************
	long
IsWAV(char *path)
{
SF_INFO sfi;
SNDFILE	*sndFile = NULL;

// Open file
if (!(sndFile = sf_open (path, SFM_READ, &sfi)))
	{	
	printf ("IsWAV : Not able to open input file '%s'\n", path);
	return (False);
	} 

// Reject files that aren't WAV
long fileFormatMajor =  sfi.format & SF_FORMAT_TYPEMASK;
if (SF_FORMAT_WAV != fileFormatMajor)
{
//	printf ("IsWAV : This is NOT a WAV file \n") ;
sf_close(sndFile);
return (False);
}

printf ("IsWAV : This is a WAV file \n") ;
if (verbose)
	PrintSndFileInfo(&sfi);

sf_close(sndFile);
return (True);
} // ---- end IsWAV() ----

// ************************************************************************
// IsAIFF:	Open file and determine if it's an AIFF file
//				Return Boolean success
// ************************************************************************
	long
IsAIFF(char *path)
{
SF_INFO sfi;
SNDFILE	*sndFile = NULL;

// Open file
if (!(sndFile = sf_open(path, SFM_READ, &sfi)))
	{	
	printf ("IsAIFF : Not able to open input file '%s'\n", path) ;
	return (False);
	} 

// Reject files that aren't WAV
long fileFormatMajor =  sfi.format & SF_FORMAT_TYPEMASK;
if (SF_FORMAT_AIFF != fileFormatMajor)
{
//	printf ("IsAIFF : This is NOT an AIFF file \n") ;
sf_close(sndFile);
return (False);
}

//printf ("IsAIFF : This is an AIFF file \n") ;
if (verbose)
	PrintSndFileInfo(&sfi);

sf_close(sndFile);
return (True);
} // ---- end IsAIFF() ----

// **********************************************************************************
// PrintUsage:		
// ********************************************************************************** 
	void   
PrintUsage()
{
printf("Usage:  %s <infile> \n", appName);
//printf("Usage:  %s [Options] <infile>\n", appName);
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
//#define MAX_FILENAME_LENGTH 500
//char inFilePath[MAX_FILENAME_LENGTH];
//inFilePath [0] = '\0';
char *inFilePath;	
if (argc < 2)
   {
    PrintUsage();
    exit(0);
    }
inFilePath = argv[1];
//printf("inFilePath  = '%s'\n", inFilePath);

// Check for a few audio file typesif 	(IsBrioAudioFile(inFilePath))
	return (1);
else if (IsWAV(inFilePath))	
	return (1);
else if (IsAIFF(inFilePath))	
	return (1);
else
	printf("This is not a Brio Raw, WAV or AIFF \n");

return 0;
}
