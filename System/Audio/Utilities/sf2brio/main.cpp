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

//#include "spmidi.h"

// Brio headers
//#include <CoreTypes.h>
//#include "AudioTypes.h"
//#include <AudioConfig.h>
//#include <AudioOutput.h>

#include "Dsputil.h"
#include "util.h"
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
#define kBlockStorageSize (kMaxBlockLength+kSRC_Filter_MaxDelayElements)
long inBlockLength  = kBlockLength;
long outBlockLength = kBlockLength;

static short inBlock [kBlockStorageSize];
static short outBlock[kBlockStorageSize];
#define kMaxTempBufs	4
static short tmpBuffers[kMaxTempBufs][kBlockStorageSize];static short *tmpBufPtrs[kMaxTempBufs];

long writeHeaderlessRawFile = False;
long outFileFormat = kOutFileFormat_Brio;

long processOnlyOneBlock = False;
long inputIsImpulse = False;

long useFixedPoint = False;
long useTimer      = False;
long useLargeBlockLength = False;

long channels = 2;
float inGain  = 1.0f;
float outGain = 1.0f;
long outSamplingFrequencySpecified = False;
long inSamplingFrequency  = kSamplingFrequency_Unspecified;
long outSamplingFrequency = kSamplingFrequency_Unspecified;

// Timer variables
long iteration, totalIterations = 1;
time_t startTT, endTT;
double startTime, endTime;
double iterationTime, totalTime;
long iTotalTime;
time_t startTs, endTs;
struct timeval totalTv;
struct timeval taskStartTv, taskEndTv;
struct timeval iterationStartTv, iterationEndTv;
// 280 MHz = Blue board
// 320 MHz = Magic Eyes 
// 385 MHz = target Lightning clock frequency
double cpuClockMHz = 385.0;

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
//printf("    -reps   iterations \n");
//printf("    -t      timer : Use timer for rendering\n");
//printf("    -c      channels \n");
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
//SF_INFO *isfi = &inSoundFileInfo;
//SF_INFO *osfi = &outSoundFileInfo;
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
else if (!Stricmp(s, "-reps") || !Stricmp(s, "-iterations"))
	{
	totalIterations = atoi(argv[++i]);
//printf("totalIterations=%d\n", totalIterations);
	}
else if (!Stricmp(s, "-inGain"))
	{
	inGain = atof(argv[++i]);
//printf("inGain=%g\n", inGain);
	}
else if (!Stricmp(s, "-inGainDB"))
	{
	float inGainDB = atof(argv[++i]);
	inGain = DecibelToLinear(inGainDB);
//printf("inGainDB = %g -> %g\n", inGainDB, inGain);
	}
else if (!Stricmp(s, "-outGain"))
	{
	outGain = atof(argv[++i]);
//printf("outGain=%g\n", outGain);
	}
else if (!Stricmp(s, "-outGainDB"))
	{
	float outGainDB = atof(argv[++i]);
	outGain = DecibelToLinear(outGainDB);
//printf("outGainDB = %g -> %g\n", outGainDB, outGain);
	}

//
// --------- General parameters
//
else if (!strcmp(s, "-c") || !strcmp(s, "-channels"))
	{
	channels = atoi(argv[++i]);
//printf("channels=%d\n", channels);
	}
else if (!Stricmp(s, "-fs") || !Stricmp(s, "-fs_out"))
	{
	outSamplingFrequency = atoi(argv[++i]);
	outSamplingFrequencySpecified = True;
printf("outSamplingFrequency=%d Hz\n", outSamplingFrequency);
	}
else if (!Stricmp(s, "-t") || !Stricmp(s, "-timer"))
	{
	useTimer = True;
printf("useTimer=%d\n", useTimer);
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
   
if (inputIsImpulse)
	strcpy(inFilePath, "impulse");

if (verbose)
{
printf("inFilePath  = '%s'\n", inFilePath);
printf("outFilePath = '%s'\n", outFilePath);
}
ClearTimeval(&totalTv);

// 
// -------------------- Convert audio file
//
{
long framesRead = 0, framesWritten = 0;
long loopCount, loopRemnants;

// Open audio file if input is not a generated impulse
if (!inputIsImpulse)
	{	
	inSoundFile = OpenSoundFile(inFilePath, &inSoundFileInfo, SFM_READ);	if (!inSoundFile)
		{
		printf("Unable to open input file '%s'\n", inFilePath);
		CleanUpAndExit();
		}
	} // if (!inputIsImpulse)

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
	ClearShorts(inBlock , kBlockStorageSize);
	ClearShorts(outBlock, kBlockStorageSize);

// Set up input for generated (no input file opened) impulse
	if (inputIsImpulse)
		{
		inSoundFileInfo.samplerate = 44100;
		inSoundFileInfo.frames = inSoundFileInfo.samplerate*10;
		inSoundFileInfo.channels = 1;
		}

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

// Shorten to some block size.  Try to get the value of a few hundred .
 if ( !useLargeBlockLength && (0 == inSamplingFrequency%100 ) && (0 == outSamplingFrequency%100 ) )
	{
	inBlockLength  = inSamplingFrequency /100;
	outBlockLength = outSamplingFrequency/100;
	}
else if ( (0 == inSamplingFrequency%10  ) && (0 == outSamplingFrequency%10  ) )
	{
	inBlockLength  = inSamplingFrequency /10;
	outBlockLength = outSamplingFrequency/10;
	}
else
	{
	inBlockLength  = inSamplingFrequency;
	outBlockLength = outSamplingFrequency;
	}
// ---- Write audio file samples (input sampling rate = output sampling rate)
if (inBlockLength == outBlockLength)
{
short *inBlockP  = &inBlock [0];
short *outBlockP = &outBlock[0];
long length = inBlockLength;

	if (processOnlyOneBlock)
		{
		loopCount    = 1;
		loopRemnants = 0;
		}
	else
		{
		loopCount    = inSoundFileInfo.frames/length;
		loopRemnants = inSoundFileInfo.frames%length;
		}
	if (useTimer)
		gettimeofday(&taskStartTv, NULL);
	for (long i = 0; i < loopCount; i++)
		{
		if (inputIsImpulse)
			{
			static long impulseGenerated = False;
			ClearShorts(inBlockP, length);
			if (!impulseGenerated)
				{
				impulseGenerated = True;
				inBlockP[0] = 10000;
				}
			}
		else
			{
			framesRead = sf_readf_short(inSoundFile, inBlockP, length);
	        if (verbose && framesRead != length)
			    printf("Short read of %d/%d samples\n", framesRead, length);
			}
	if (useTimer)
		gettimeofday(&iterationStartTv, NULL);
		
			if (outH)
				fwrite(inBlockP, sizeof(short), length*inSoundFileInfo.channels, outH);
			else if (outSoundFile)
				framesWritten = sf_writef_short(outSoundFile, inBlockP, length);

	if (useTimer)
		{
		gettimeofday(&iterationEndTv, NULL);
//		totalTime += SecondsFromTimevalDiff(&iterationStartTv, &iterationEndTv);
		AddTimevalDiff(&totalTv, &iterationStartTv, &iterationEndTv);
		}
		}
// Write remainder of samples
	if (loopRemnants)
		{
	//	printf("writing remaining %d frames\n", remnants);
		framesRead = sf_readf_short(inSoundFile, inBlockP, loopRemnants);
//        if (verbose && framesRead != length)
//		    printf("Remnants Short read of %d/%d samples\n", framesRead, length);
		if (outH)
		   fwrite(inBlockP, inSoundFileInfo.channels*sizeof(short), loopRemnants, outH);
		else if (outSoundFile)
			framesWritten = sf_writef_short(outSoundFile, inBlockP, loopRemnants);
		}
	if (useTimer)
		gettimeofday(&taskEndTv, NULL);
if (processOnlyOneBlock)
	{
	for (j = 0; j < 20; j++)
		printf("inBlockP %2d : %d\n", j, inBlockP[j]);
	for (j = 0; j < 20; j++)
		printf("outBlockP %2d : %d\n", j, outBlockP[j]);
	printf("NOTE: single block test \n");
	}
	}

// 
// Report final results averaged by # of iterations
//
if (useTimer)
	{
	printf("\n");
	totalTime = SecondsFromTimeval(&totalTv);
	printf("totalTime  = %g Sec \n", totalTime);

	double realTime = 0.0, cpuTime = 0.0;
	double avgTime, startTime, endTime;
	avgTime = ((double)totalTime)/(double)totalIterations;
	if (totalIterations > 1)
		printf("Avg  Time/file  =%g Sec\n", avgTime);

	//realTime = inFileTime/avgTime;
	cpuTime  = avgTime/inFileTime;
	//printf("Real Time = %d X\n", (int)realTime);
	printf("Real Time CPU Load = %g %% (%d MIPS)\n", 100.0*cpuTime, (int) (cpuTime*cpuClockMHz + 0.5));
	printf("CPU Frequency=%d MHz:  MIPS= %g\n", (int)cpuClockMHz, cpuTime*cpuClockMHz);
	printf("\n-----------------------------------------------\n");
	}

} // end Convert Audio File

printf("\n");
CleanUpAndExit(); 
return 0;
}  // ---- end main() ----

