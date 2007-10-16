//============================================================================
//  sf2brio (sfconvert)  
//
//============================================================================
#include <math.h>
#include <stdlib.h>
#include <stdio.h>#include <string.h>

#include "sfconfig.h"
#include "sndfile.h"
#include <time.h>
#include <sys/time.h>

//#include "util.h"
#include "sndfileutil.h"

#include "main.h"
#include "sf.h"

#define kAppType_sf2brio   0
char appName[100]; // sf2brio
long appType = kAppType_sf2brio;

long verbose = False;

// Sound file variables
SNDFILE	*inSoundFile = NULL;
SF_INFO	inSoundFileInfo ;
long inSoundFileInfo_wordWidthBits = 16;
char inSoundFilePath[500];	
double inFileTime = 0.0; // = ((double) frames)/(double)vi->rate;

FILE *outH = NULL;
SNDFILE	*outSoundFile = NULL;
SF_INFO	outSoundFileInfo ;

#define kBlockLength	1024
#define kMaxBlockLength 50000
#define kBlockStorageSize kMaxBlockLength
long inBlockLength  = kBlockLength;
long outBlockLength = kBlockLength;

static short inBlock [kBlockStorageSize];
static short outBlock[kBlockStorageSize];

long writeHeaderlessRawFile = False;
long outFileFormat = kOutFileFormat_Brio;

long useLargeBlockLength = False;

long channels = 2;
long outSamplingFrequencySpecified = False;
long inSamplingFrequency  = kSamplingFrequency_Unspecified;
long outSamplingFrequency = kSamplingFrequency_Unspecified;

// **********************************************************************************
// Stricmp:	Compare two strings with case insensitivity.
//			Return 0 if strings are the same.	
// ********************************************************************************** 
	int   
Stricmp(char *inS1, char *inS2)
{
char s1[1000], s2[1000];
long i, j;
long returnValue = 1;

if ((!inS1) || (!inS2))
	return (1);

strcpy(s1, inS1);
strcpy(s2, inS2);
// Force all values to lower case
for (i = 0; s1[i] != '\0'; i++)
	{
	if (s1[i] >= 'A' && s1[i] <= 'Z')
		s1[i] = s1[i] - 'A' + 'a';
	}
for (i = 0; s2[i] != '\0'; i++)
	{
	if (s2[i] >= 'A' && s2[i] <= 'Z')
		s2[i] = s2[i] - 'A' + 'a';
	}
//printf("inS1 '%s' -> '%s'\n", inS1, s1);
//printf("inS2 '%s' -> '%s'\n", inS2, s2);

for (i = 0; s1[i] != '\0' && s2[i] != '\0'; i++)
	{
	if (s1[i] != s2[i])
		break;
	}
// Check if reached end of either string.  If both, strings are the same
if (s1[i] == '\0' && s2[i] == '\0')
	return (0);
return (1);
}	// ---- end Stricmp() ---- 

//============================================================================
// PrintUsage:		
//============================================================================
	void   
PrintUsage()
{
printf("Usage:  %s <infile> <outfile>\n", appName);
//printf("Usage:  %s [Options] <infile><outfile>\n", appName);
printf("Options:\n");
printf("    -outFormat brio, aiff, wav, raw (default: brio)\n");
printf("                       \n");
printf("    -v  verbose operation\n");
//printf("    -i      infile \n");
//printf("    -o      outfile \n");
//
printf("\n");
}	// ---- end PrintUsage() ---- 

//============================================================================
// CleanUpAndExit() : 
//============================================================================
	void 
CleanUpAndExit() 
{
CloseSoundFile(&inSoundFile);
CloseSoundFile(&outSoundFile);
if (outH)		
	fclose(outH);
exit(-1);
}	// ---- end CleanUpAndExit() ---- 

//============================================================================
// main(): 
//============================================================================
	int 
main(int argc, char *argv[]) 
{
long i = 0, j = 0;
double execTime = 0.0;
#define MAX_FILENAME_LENGTH 500
char inFilePath [MAX_FILENAME_LENGTH];
char outFilePath[MAX_FILENAME_LENGTH];
long result = 0;

// Determine application type by its name
appType = kAppType_sf2brio;
strcpy(appName, argv[0]);

for (i = 0; i < kMaxChannels; i++)
	{
	}

inFilePath [0] = '\0';
outFilePath[0] = '\0';

if (argc < 3)
   {
    PrintUsage();
    exit(0);
    }

// Parse file arguments
for (i = 1; i < argc; i++)
{
char *s = argv[i];

if 	(!Stricmp(s, "-v") || !Stricmp(s, "-verbose"))
	verbose = True;
else if (!Stricmp(s, "-h") || !Stricmp(s, "-help"))
	{
	PrintUsage();
	exit(0);
	}
else if (!Stricmp(s, "-i") || !Stricmp(s, "-infile"))
	{
	strcpy(inFilePath , argv[++i]);
	printf("inFilePath = '%s'\n", inFilePath);
	}
else if (!Stricmp(s, "-o") || !Stricmp(s, "-outfile"))
	{
	strcpy(outFilePath , argv[++i]);
	printf("outFilePath = '%s'\n", outFilePath);
	}
else if (!Stricmp(s, "-outFormat"))
	{
	char *s = argv[++i];
	if 	    (!Stricmp(s, "brio"))
		outFileFormat = kOutFileFormat_Brio;
	else if (!Stricmp(s, "aiff"))
		outFileFormat = kOutFileFormat_AIFF;
	else if (!Stricmp(s, "wav"))
		outFileFormat = kOutFileFormat_WAV;
	else if (!Stricmp(s, "raw"))
		outFileFormat = kOutFileFormat_Raw;
	else
		{
		printf("Invalid %s : '%s'\n", argv[i-1], s);
		exit(-1);
		}
//	printf("outputFileFormat = %d '%s'\n", outFileFormat, s);
	}
// Assign random parameters to input and output files - kinda dumb
else if (inFilePath[0] == '\0')
	strcpy(inFilePath , argv[i]);
else if (outFilePath[0] == '\0')
	strcpy(outFilePath , argv[i]);
else
	{
	printf("Invalid arguments\n");
	exit(-1);
	}
}  // ---- end argument parsing
   

if (verbose)
{
printf("inFilePath  = '%s'\n", inFilePath);
printf("outFilePath = '%s'\n", outFilePath);
}

// 
// -------------------- Convert audio file
//
{
long framesRead = 0, framesWritten = 0;
long loopCount, loopRemnants;

// Open audio file if input is not a generated impulse
	inSoundFile = OpenSoundFile(inFilePath, &inSoundFileInfo, SFM_READ);	if (!inSoundFile)
		{
		printf("Unable to open input file '%s'\n", inFilePath);
		CleanUpAndExit();
		}

// Select sampling rate, for which command-line specification overrides the file value
	inSamplingFrequency  = inSoundFileInfo.samplerate;
	if (!outSamplingFrequencySpecified)
		outSamplingFrequency = inSamplingFrequency;

// Select output format as input format, if not already specified
//  FIXXX: incomplete:  doesn't handle Brio or RAW
if (kOutFileFormat_Unspecified == outFileFormat)
	{
//#define CHOOSE_OUTFORMAT_FROM_INFORMAT
#ifdef CHOOSE_OUTFORMAT_FROM_INFORMAT
	long fileFormatType = (inSoundFileInfo.format & SF_FORMAT_TYPEMASK);
	if 	(SF_FORMAT_AIFF == fileFormatType)
		outFileFormat = kOutFileFormat_AIFF;
	else if (SF_FORMAT_WAV == fileFormatType)
		outFileFormat = kOutFileFormat_WAV;
	else
		{
		printf("%s: currently do not support this input file type: %d\n", appName, fileFormatType);
		CleanUpAndExit();
		}
#else
    outFileFormat = kOutFileFormat_BRIO;
#endif
	}

// Clear buffers
    bzero(inBlock , kBlockStorageSize*sizeof(short));
    bzero(outBlock, kBlockStorageSize*sizeof(short));

// Open output file , if specified
	if (outFilePath[0] != '\0')
		{
	// Brio or RAW: need to open a RAW binary file as LibSndFile doesn't help here.
		if (kOutFileFormat_Brio == outFileFormat || kOutFileFormat_Raw == outFileFormat)
			{
			outH = fopen(outFilePath, "wb");
			if (!outH)
				{
				printf("Unable to open output file '%s'\n", outFilePath);
				CleanUpAndExit(); 
				}
			}
	// AIFF or WAV
		else if (kOutFileFormat_AIFF == outFileFormat || kOutFileFormat_WAV == outFileFormat)
			{
			outSoundFileInfo.frames     = 0;		
			outSoundFileInfo.samplerate = outSamplingFrequency;
			outSoundFileInfo.channels   = inSoundFileInfo.channels;
			outSoundFileInfo.format     = SF_FORMAT_PCM_16;
			if (kOutFileFormat_AIFF == outFileFormat)
				outSoundFileInfo.format |= SF_FORMAT_AIFF;
			else
				outSoundFileInfo.format |= SF_FORMAT_WAV;
			outSoundFileInfo.sections = 1;
			outSoundFileInfo.seekable = 1;

			outSoundFile = OpenSoundFile( outFilePath, &outSoundFileInfo, SFM_WRITE);
			}
		}
	
	// Prepare and write Brio audio file header
	if (kOutFileFormat_Brio == outFileFormat)
		{
		tAudioHeader brioFileHeader;

		long sampleSizeInBytes = inSoundFileInfo_wordWidthBits / 8;

		brioFileHeader.sampleRate   = outSamplingFrequency;
		brioFileHeader.dataSize     = ((inSoundFileInfo.frames * inSoundFileInfo.channels) * sampleSizeInBytes); 
		brioFileHeader.offsetToData = sizeof(tAudioHeader);
		brioFileHeader.type         = 0x10001C05;	// Brio raw type
		if (1 == inSoundFileInfo.channels)
			brioFileHeader.flags = 0;  // Mono for Brio
		else
			brioFileHeader.flags = 1;  // Stereo for Brio
		fwrite(&brioFileHeader, sizeof(char), sizeof(tAudioHeader), outH);

        if (verbose)
            {
            PrintBrioAudioHeader(&brioFileHeader);
            printf("Wrote Brio header file in %d bytes\n", sizeof(tAudioHeader));
            }
        }

inFileTime = ((double) inSoundFileInfo.frames)/(double)inSoundFileInfo.samplerate;
if (verbose)
    printf("inFileTime = %g  seconds \n", inFileTime);

inBlockLength  = inSamplingFrequency;
outBlockLength = outSamplingFrequency;

// ---- Write audio file samples (input sampling rate = output sampling rate)
{
short *inBlockP  = &inBlock [0];
short *outBlockP = &outBlock[0];
long length = inBlockLength;

loopCount    = inSoundFileInfo.frames/length;
loopRemnants = inSoundFileInfo.frames%length;

	for (long i = 0; i < loopCount; i++)
		{
		framesRead = sf_readf_short(inSoundFile, inBlockP, length);
        if (verbose && framesRead != length)
		    printf("Short read of %d/%d samples\n", framesRead, length);
		
			if (outH)
				fwrite(inBlockP, sizeof(short), length*inSoundFileInfo.channels, outH);
			else if (outSoundFile)
				framesWritten = sf_writef_short(outSoundFile, inBlockP, length);
        }

// Write remainder of samples
	if (loopRemnants)
		{
	//	printf("writing remaining %d frames\n", remnants);
		framesRead = sf_readf_short(inSoundFile, inBlockP, loopRemnants);

		if (outH)
		   fwrite(inBlockP, inSoundFileInfo.channels*sizeof(short), loopRemnants, outH);
		else if (outSoundFile)
			framesWritten = sf_writef_short(outSoundFile, inBlockP, loopRemnants);
		}
}
} // end Convert Audio File

printf("\n");
CleanUpAndExit(); 
return 0;
}  // ---- end main() ----

