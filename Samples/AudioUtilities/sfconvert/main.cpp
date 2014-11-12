//============================================================================
//  sfconvert :     program to convert among file formats and test audio DSP routines
//              Written by Gints Klimanis, 2007
//============================================================================
#include <math.h>
#include <stdlib.h>
#include <stdio.h>#include <string.h>

#include "sfconfig.h"
#include "sndfile.h"
#include <time.h>
#include <sys/time.h>

#include "spmidi.h" // Mobileer header file

// Brio headers
//#include <CoreTypes.h>
//#include "AudioTypes.h"
//#include <AudioConfig.h>
//#include <AudioOutput.h>

// LibDsputil headers
#include "Dsputil.h"
#include "util.h"
#include "eq.h"
#include "fir.h"
#include "iir.h"
#include "mix.h"
#include "shape.h"
#include "src.h"

#include "main.h"
#include "sf.h"
#include "sndfileutil.h"

#define kAppType_sf2brio   0
#define kAppType_sfconvert 1
char appName[100]; // sf2brio, sfconvert
long appType = kAppType_sfconvert;

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

// Sampling rate conversion parameters
// AddDrop, Linear, FIR, IIR, Unfiltered
static long srcType  = kSRC_Interpolation_Type_AddDrop;
#define kSRC_Order_Unspecified (-1)
static long srcOrder = kSRC_Order_Unspecified;
static long srcFilterVersion = kSRC_FilterVersion_Default;

SRC srcData[kMaxChannels];
#define kBlockLength	1024
#define kMaxBlockLength 50000
#define kBlockStorageSize (kMaxBlockLength+kSRC_Filter_MaxDelayElements)
long inBlockLength  = kBlockLength;
long outBlockLength = kBlockLength;

static short inBlock [kBlockStorageSize];
static short outBlock[kBlockStorageSize];
#define kMaxTempBufs	4
static short tmpBuffers[kMaxTempBufs][kBlockStorageSize];static short *tmpBufPtrs[kMaxTempBufs];

long useFIR = False;
FIR firData[kMaxChannels];

long useIIR = False;
IIR iirData[kMaxChannels];

long useEQ = False;
EQ eqData[kMaxChannels];

long useSoftClipper = False;
WAVESHAPER softClipperData[kMaxChannels];

long writeHeaderlessRawFile = False;
long outFileFormat = kOutFileFormat_Unspecified;

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

// MIDI variables
long midiSamplingFrequency = 22050;
long midiVoiceCount_ForTimingCalculation = 1;
long midiVoiceLimit        = SPMIDI_MAX_VOICES;

long inputIsMIDIFile = False;

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
// 380 MHz = target Lightning clock frequency
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
printf("    -outFormat brio, aiff, wav, raw (default: same as input file)\n");
printf("                       \n");
printf("    -v  verbose operation\n");
printf("    -reps   iterations \n");
printf("    -t      timer : Use timer for MIDI rendering\n");
printf("    -1      singleblock : process only one block\n");
printf("    -c      channels \n");
printf("    -i      infile \n");
printf("    -o      outfile \n");
printf("    -largeBlocks \n");
printf("    -fs     sampling frequency (Hz)\n");
printf("    -fixedpt	Use 16-bit Fixed point arithmetic    \n");
printf("    -floatingpt	Use 32-bit floating point arithmetic (Default)    \n");
printf("    -inGain     input  gain (Linear)   \n");
printf("    -inGainDB   input  gain (Decibels) \n");
printf("    -outGain    output gain (Linear)   \n");
printf("    -outGainDB  output gain (Decibels) \n");
printf("---- MIDI Parameters \n");
printf("    -fs_midi  MIDI engine internal sampling frequency (Hz)\n");
printf("    -midi     Input file rendered with General MIDI \n");
printf("    -voices   MIDI voice count\n");
printf("---- Equalizer Parameters \n");
printf("    -eqMode  {LowPass,HighPass,BandStop,LowShelf,HighShelf} \n");
printf("    -eqFc     Center frequency (Hz) \n");
printf("    -eqQ      fc/bandwidth \n");
printf("    -eqGainDB Gain at Fc \n");
printf("---- FIR Filter Parameters \n");
printf("    -fir    \n");
printf("    -firType  {LowPass,HighPass,BandPass,BandStop} \n");
printf("---- IIR Filter Parameters \n");
printf("    -iir    \n");
printf("    -iirType   \n");
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

//#define TEST_DSPUTIL
#ifdef TEST_DSPUTIL
printf("Tests !!! Will exit \n");
Test_Dsputil();  
Test_Mixer();
exit(0);
#endif 

// Determine application type by its name
if (!Stricmp(argv[0], "-sfconvert"))
	appType = kAppType_sfconvert;
else // if (!Stricmp(s, "-sf2brio"))
	appType = kAppType_sf2brio;
strcpy(appName, argv[0]);

for (i = 0; i < kMaxChannels; i++)
	{
	DefaultSRC(&srcData[i]);
	DefaultFIR(&firData[i]);
	DefaultEQ( &eqData[i]);
	DefaultWaveShaper( &softClipperData[i]);
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
// ----------- Sampling rate conversion parameters
//
else if (!Stricmp(s, "-srcType"))
	{
	long type = -1;
	char *s = argv[++i];
	if 	(!Stricmp(s, "AddDrop"))
		type = kSRC_Interpolation_Type_AddDrop;
	else if (!Stricmp(s, "Linear"))
		type = kSRC_Interpolation_Type_Linear;
	else if (!Stricmp(s, "FIR") || !Stricmp(s, "FIR1"))
		type = kSRC_Interpolation_Type_FIR;
	else if (!Stricmp(s, "IIR"))
		type = kSRC_Interpolation_Type_IIR;
	else if (!Stricmp(s, "Triangle"))
		type = kSRC_Interpolation_Type_Triangle;
	else if (!Stricmp(s, "testfir"))
		type = kSRC_Interpolation_Type_TestFIR;
	else if (!Stricmp(s, "unfiltered"))
		type = kSRC_Interpolation_Type_Unfiltered;
	else
		{
		printf("Invalid interpolation type '%s'\n", s);
		exit(-1);
		}
	printf("srcType =%d '%s'\n", type, s);
	srcType = type;
	}
else if (!Stricmp(s, "-srcOrder"))
	{
	srcOrder = atoi(argv[++i]);
printf("srcOrder=%d\n", srcOrder);
	}
else if (!Stricmp(s, "-srcV"))
	{
	srcFilterVersion = atoi(argv[++i]);
printf("srcFilterVersion=%d\n", srcFilterVersion);
	}

// 
// ------  MIDI File playback parameters
//
else if (!Stricmp(s, "-midi"))
	{
	inputIsMIDIFile = True;
//printf("inputIsMIDIFile=%d\n", inputIsMIDIFile);
	}
else if (!Stricmp(s, "-voices"))
	{
	midiVoiceCount_ForTimingCalculation = atoi(argv[++i]);
//printf("midiVoiceCount_ForTimingCalculation=%d\n", midiVoiceCount_ForTimingCalculation);
	}
else if (!Stricmp(s, "-voicelimit"))
	{
	midiVoiceLimit = atoi(argv[++i]);
	if (midiVoiceLimit < 1 || midiVoiceLimit > SPMIDI_MAX_VOICES)
		{
		printf("Invalid MIDI VoiceLimit %d . Must be in range [1..%d]\n", midiVoiceLimit, SPMIDI_MAX_VOICES);
		}
//printf("midiVoiceLimit=%d\n", midiVoiceLimit);
	}
else if (!Stricmp(s, "-fs_midi"))
	{
	midiSamplingFrequency = atoi(argv[++i]);
printf("midiSamplingFrequency=%d Hz\n", midiSamplingFrequency);
	}

//
// ------------ Equalizer (EQ) filter parameters
//
else if (!Stricmp(s, "-useEQ"))
	{
	useEQ = True;
//printf("useEQ=%d\n", useEQ);
	}
else if (!Stricmp(s, "-eqMode"))
	{
	long mode = -1;
	char *s = argv[++i];
	if 	    (!Stricmp(s, "LowPass"))
		mode = kEQ_Mode_LowPass;
	else if (!Stricmp(s, "HighPass"))
		mode = kEQ_Mode_HighPass;
	else if (!Stricmp(s, "LowShelf") || !Stricmp(s, "LoShelf"))
		mode = kEQ_Mode_LowShelf;
	else if (!Stricmp(s, "HighShelf") || !Stricmp(s, "HiShelf"))
		mode = kEQ_Mode_HighShelf;
	else if (!Stricmp(s, "Parametric"))
		mode = kEQ_Mode_Parametric;
	else if (!Stricmp(s, "ByPass"))
		mode = kEQ_Mode_Bypass;
	else
		{
		printf("Invalid : '%s'\n", argv[i-1], s);
		exit(-1);
		}
//	printf("EQ mode =%d '%s'\n", mode, s);
	eqData[0].mode = mode;
	}
else if (!Stricmp(s, "-eqFc") || !Stricmp(s, "-eqFrequency"))
	{
	eqData[0].frequency = atof(argv[++i]);
//printf("Equalizer Fc=%g Hz\n", eqData[0].frequency);
	}
else if (!Stricmp(s, "-eqQ"))
	{
	eqData[0].q = atof(argv[++i]);
//printf("Equalizer Q=%g \n", eqData[0].q);
	}
else if (!Stricmp(s, "-eqGainDB"))
	{
	eqData[0].gainDB = atof(argv[++i]);
//printf("Equalizer GainDB=%g \n", eqData[0].gainDB);
	}

//
// ------------ SoftClipper filter parameters
//
else if (!Stricmp(s, "-useSoftClipper") || !Stricmp(s, "-useSClip"))
	{
	useSoftClipper = True;
//printf("useSoftClipper=%d\n", useSoftClipper);
	}
else if (!Stricmp(s, "-sclip_inGainDB"))
	{
	softClipperData[0].inGainDB = atof(argv[++i]);
//printf("SoftClipper inGainDB=%g \n", softClipperData[0].inGainDB);
	}
else if (!Stricmp(s, "-sclip_outGainDB"))
	{
	softClipperData[0].outGainDB = atof(argv[++i]);
//printf("SoftClipper outGainDB=%g \n", softClipperData[0].outGainDB);
	}
else if (!Stricmp(s, "-sclip_type"))
	{
	long type = -1;
	char *s = argv[++i];
	if 	    (!Stricmp(s, "V1"))
		type = kWaveShaper_Type_V1;
	else if (!Stricmp(s, "V2"))
		type = kWaveShaper_Type_V2;
	else if (!Stricmp(s, "V3"))
		type = kWaveShaper_Type_V3;
	else if (!Stricmp(s, "V4"))
		type = kWaveShaper_Type_V4;
	else if (!Stricmp(s, "Bypass"))
		type = kWaveShaper_Type_ByPass;
	else
		{
		printf("Invalid : '%s'\n", argv[i-1], s);
		exit(-1);
		}
//	printf("SoftClipper type =%d '%s'\n", type, s);
	softClipperData[0].type = type;
	}

//
// ------------ FIR filter parameters
//
else if (!Stricmp(s, "-useFIR"))
	{
	useFIR = True;
//printf("useFIR=%d\n", useFIR);
	}
else if (!Stricmp(s, "-firType"))
	{
	long firType = -1;
	char *s = argv[++i];
	if 	(!Stricmp(s, "LowPass"))
		firType = kFIR_Type_LowPass;
	else if (!Stricmp(s, "HighPass"))
		firType = kFIR_Type_HighPass;
	else if (!Stricmp(s, "BandPass"))
		firType = kFIR_Type_BandPass;
	else if (!Stricmp(s, "BandStop"))
		firType = kFIR_Type_BandStop;
// 1/2 Band filters
	else if (!Stricmp(s, "HalfBand") || !Stricmp(s, "HalfBand_15"))
		firType = kFIR_Type_HalfBand_15;
	else if (!Stricmp(s, "HalfBand_31"))
		firType = kFIR_Type_HalfBand_31;
	else if (!Stricmp(s, "HalfBand_32dB"))
		firType = kFIR_Type_HalfBand_32dB;
	else if (!Stricmp(s, "HalfBand_32dB"))
		firType = kFIR_Type_HalfBand_32dB;
	else if (!Stricmp(s, "HalfBand_58dB"))
		firType = kFIR_Type_HalfBand_58dB;
	else if (!Stricmp(s, "HalfBand_30dB_15"))
		firType = kFIR_Type_HalfBand_30dB_15;
	else if (!Stricmp(s, "HalfBand_50dB_31"))
		firType = kFIR_Type_HalfBand_50dB_31;
	else if (!Stricmp(s, "HalfBand_50dB_41"))
		firType = kFIR_Type_HalfBand_50dB_41;

// 1/3 band filters
	else if (!Stricmp(s, "ThirdBand") || !Stricmp(s, "ThirdBand_15"))
		firType = kFIR_Type_ThirdBand_15;
	else if (!Stricmp(s, "ThirdBand_31"))
		firType = kFIR_Type_ThirdBand_31;

// Triangle filters
	else if (!Stricmp(s, "Triangle_3"))
		firType = kFIR_Type_Triangle_3;
	else if (!Stricmp(s, "Triangle_9"))
		firType = kFIR_Type_Triangle_9;
	else
		{
		printf("Invalid : '%s'\n", argv[i-1], s);
		exit(-1);
		}
	printf("FIR type =%d '%s'\n", firType, s);
	firData[0].type = firType;
	}

//
// ------------ IIR filter parameters
//
else if (!Stricmp(s, "-useIIR"))
	{
	useIIR = True;
//printf("useIIR=%d\n", useIIR);
	}
else if (!Stricmp(s, "-iir_Type"))
	{
	long x = -1;
	char *s = argv[++i];
	if 	    (!Stricmp(s, "LowPass")  || !Stricmp(s, "lpf"))
		x = kIIR_Mode_LowPass;
	else if (!Stricmp(s, "HighPass") || !Stricmp(s, "hpf"))
		x = kIIR_Mode_HighPass;
	else if (!Stricmp(s, "BandPass") || !Stricmp(s, "bpf"))
		x = kIIR_Mode_BandPass;
	else if (!Stricmp(s, "BandStop") || !Stricmp(s, "bsf"))
		x = kIIR_Mode_BandStop;
	else if (!Stricmp(s, "Parametric"))
		x = kIIR_Mode_Parametric;
	else
		{
		printf("Invalid : '%s'\n", argv[i-1], s);
		exit(-1);
		}
printf("IIR type =%d '%s'\n", x, s);
	iirData[0].type = x;
	}
else if (!Stricmp(s, "-iir_Order"))
	{
	long x = atoi(argv[++i]);
printf("IIR order =%d\n", x);
	iirData[0].order = x;
	}
else if (!Stricmp(s, "-iir_Fc"))
	{
	float x = atof(argv[++i]);
printf("IIR order =%g\n", x);
	iirData[0].frequency = x;
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
else if (!Stricmp(s, "-largeBlocks"))
	{
	useLargeBlockLength = True;
printf("useLargeBlockLength=%d\n", useLargeBlockLength);
	}
else if (!Stricmp(s, "-Q15") || !Stricmp(s, "-fixedpt") || !Stricmp(s, "-fixedpoint") || !Stricmp(s, "-usefixedpt"))
	{
	useFixedPoint = True;
printf("useFixedPoint=%d\n", useFixedPoint);
	}
else if (!Stricmp(s, "-floatingpoint") || !Stricmp(s, "-floatpt"))
	{
	useFixedPoint = False;
printf("useFixedPoint=%d\n", useFixedPoint);
	}
else if (!Stricmp(s, "-1") || !Stricmp(s, "-singleblock"))
	{
	processOnlyOneBlock = True;
printf("processOnlyOneBlock = %d\n", processOnlyOneBlock);
	}
else if (!Stricmp(s, "-impulse"))
	{
	inputIsImpulse = True;
printf("inputIsImpulse = %d\n", inputIsImpulse);
	}
else if (!Stricmp(s, "-i") || !Stricmp(s, "-infile"))
	{
	strcpy(inFilePath , argv[++i]);
//	printf("inFilePath = '%s'\n", inFilePath);
	}
else if (!Stricmp(s, "-o") || !Stricmp(s, "-outfile"))
	{
	strcpy(outFilePath , argv[++i]);
//	printf("outFilePath = '%s'\n", outFilePath);
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

printf("inFilePath  = '%s'\n", inFilePath);
printf("outFilePath = '%s'\n", outFilePath);

ClearTimeval(&totalTv);


// Initialize structures for sampling rate conversion
for (i = 0; i < kMaxChannels; i++)
	{
	srcData[i].useFixedPoint        = useFixedPoint;
	srcData[i].type                 = srcType;
	srcData[i].samplingFrequency    = outSamplingFrequency;
	}
// 
// ---- Convert MIDI file using Mobileer MIDI engine
//
if (inputIsMIDIFile)
	{
// Update sampling rate conversion structures
for (i = 0; i < channels; i++)
	{
	srcData[i].samplingFrequency    = outSamplingFrequency;
	srcData[i].inSamplingFrequency  = midiSamplingFrequency;
	srcData[i].outSamplingFrequency = outSamplingFrequency;

	PrepareSRC(&srcData[i]);
	}

for (iteration = 0; iteration < totalIterations; iteration++)
	{
	if (useTimer)
		gettimeofday(&iterationStartTv, NULL);

printf("\nDecoding MIDI: iteration %d/%d \n", iteration+1, totalIterations);
	PlayMIDIFile(inFilePath, outFilePath, &inFileTime, &execTime);

	// Report time results
	if (useTimer)
		{
		gettimeofday(&iterationEndTv, NULL);
//	iterationTime = SecondsFromTimeval(&startTv, &endTv);
	totalTime += execTime; //iterationTime;
	printf("iterationTime  =%g Sec\n", execTime);
		}
	}
// Report final results averaged by # of iterations
	if (useTimer)
		{
		printf("\n-----------------------------------------------\n");
		printf("totalTime  = %g Sec for %d voices\n", totalTime, midiVoiceCount_ForTimingCalculation);

		double realTime = 0.0, cpuTime = 0.0;
		double avgTime, startTime, endTime;
		avgTime = ((double)totalTime)/(double)totalIterations;
		printf("Avg  Time/file  =%g Sec\n", avgTime);

		//realTime = inFileTime/avgTime;
		cpuTime  = avgTime/inFileTime;
		//printf("Real Time = %d X\n", (int)realTime);
		printf("Real Time CPU Load = %g %%  (%d MIPS)\n", 100.0*cpuTime, (int) (cpuTime*cpuClockMHz + 0.5));
		printf("CPU Frequency=%d MHz:  MIPS= %g/voice\n", (int)cpuClockMHz, (cpuTime*cpuClockMHz)/(double)midiVoiceCount_ForTimingCalculation);
		}
	}
// 
// -------------------- Convert audio file
//
else
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

// Update sampling rate conversion structures
for (i = 0; i < kMaxChannels; i++)
	{
	srcData[i].samplingFrequency    = inSamplingFrequency;
	srcData[i].inSamplingFrequency  = inSamplingFrequency;
	srcData[i].outSamplingFrequency = outSamplingFrequency;

	PrepareSRC(&srcData[i]);
	}

// Select output format as input format, if not already specified
//  FIXXX: incomplete:  doesn't handle Brio or RAW
if (kOutFileFormat_Unspecified == outFileFormat)
	{
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
	}

// Clear buffers
	ClearShorts(inBlock , kBlockStorageSize);
	ClearShorts(outBlock, kBlockStorageSize);

// Set up input for generated (no input file opened) impulse
	if (inputIsImpulse)
		{
		inSoundFileInfo.samplerate = 32000;
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
//printf("Wrote Brio header file in %d bytes\n", sizeof(tAudioHeader));
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
short *inBlockP  = &inBlock [kFIR_MaxDelayElements];
short *outBlockP = &outBlock[kFIR_MaxDelayElements];
long length = inBlockLength;

if (useFIR)
	{
    bcopy(&firData[0], &firData[1], sizeof(FIR));
	for (i = 0; i < kMaxChannels; i++)
		PrepareFIR(&firData[i]);
	}

if (useIIR)
	{
    iirData[0].samplingFrequency = inSamplingFrequency;
    bcopy(&iirData[0], &iirData[1], sizeof(IIR));
	for (i = 0; i < kMaxChannels; i++)
		PrepareIIR(&iirData[i]);
	}

if (useEQ)
	{
    bcopy(&eqData[0], &eqData[1], sizeof(EQ));
//    eqData[1].frequency = eqData[0].frequency;
//    eqData[1].q         = eqData[0].q;
//    eqData[1].gainDB    = eqData[0].gainDB;
//    eqData[1].mode      = eqData[0].mode;

	for (i = 0; i < kMaxChannels; i++)
        {
        eqData[i].samplingFrequency  = inSamplingFrequency;
		PrepareEQ(&eqData[i]);
        }
    }
if (useSoftClipper)
	{
    bcopy(&softClipperData[0], &softClipperData[1], sizeof(WAVESHAPER));

	for (i = 0; i < kMaxChannels; i++)
        {
        softClipperData[i].samplingFrequency  = inSamplingFrequency;
		PrepareWaveShaper(&softClipperData[i]);
        }
    }

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
	                if (framesRead != length)
			    printf("Short read of %d/%d samples\n", framesRead, length);
			}
	if (useTimer)
		gettimeofday(&iterationStartTv, NULL);
//
// ----- Processing section
//
{
short *inP  = inBlockP;
short *outP = outBlockP;

			ScaleShortsf(inP, inP, length, inGain);
// FIXXX: this assignment only works for in-place safe operations
		if (useFIR)
			{
			firData[0].useFixedPoint = useFixedPoint;
			RunFIR_Shorts(inP, outP, length, &firData[0]);
            inP = outP;
			}
		if (useIIR)
			{
			iirData[0].useFixedPoint = useFixedPoint;
//			RunIIR1_Shorts(inP, outP, length, &iirData[0]);
			RunIIR2_Shorts(inP, outP, length, &iirData[0]);

            inP = outP;
			}
		if (useEQ)
			{
            eqData[0].useFixedPoint = useFixedPoint;
            ComputeEQ(inP, outP, length, &eqData[0]);

//          ComputeEQi(inP, outP, length, &eqData[0]);
//          ComputeEQf(inP, outP, length, &eqData[0]);
            inP = outP;
			}
		if (useSoftClipper)
			{
            softClipperData[0].useFixedPoint = useFixedPoint;
            ComputeWaveShaper(inP, outP, length, &softClipperData[0]);
            inP = outP;
			}
		
		if (outH)
			fwrite(outP, sizeof(short), length*inSoundFileInfo.channels, outH);
		else if (outSoundFile)
			framesWritten = sf_writef_short(outSoundFile, outP, length);
}

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
                if (framesRead != length)
        if (verbose)
		    printf("Remnants Short read of %d/%d samples\n", framesRead, length);
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
// ---- Write audio file samples with sampling rate conversion
//
else
{
short *inBlockP  = &inBlock [kSRC_Filter_MaxDelayElements];
short *outBlockP = &outBlock[kSRC_Filter_MaxDelayElements];

//printf("inBlockLength=%d outBlockLength=%d \n", inBlockLength, outBlockLength);
for (i = 0; i < inSoundFileInfo.channels; i++)
	{
	srcData[i].useFixedPoint        = useFixedPoint;
	srcData[i].type                 = srcType;
	srcData[i].samplingFrequency    = inSamplingFrequency;
	srcData[i].inSamplingFrequency  = inSamplingFrequency;
	srcData[i].outSamplingFrequency = outSamplingFrequency;
	// To test FIR in SRC module, sampling rate conversion is bypassed
	if (kSRC_Interpolation_Type_TestFIR == srcData[i].type)
		{
	printf("TestSRC FIR ...\n");
		srcData[i].inSamplingFrequency  = inSamplingFrequency;
		srcData[i].outSamplingFrequency = outSamplingFrequency;
		inBlockLength = outBlockLength;
		}

	PrepareSRC(&srcData[i]);
	}

//printf("inBlockLength = %d outBlockLength=%d\n", inBlockLength, outBlockLength);

printf("Audio SRC : %d -> %d Hz\n", inSamplingFrequency, outSamplingFrequency);
	if (processOnlyOneBlock)
		{
		loopCount    = 1;
		loopRemnants = 0;
		}
	else
		{
		loopCount    = inSoundFileInfo.frames/inBlockLength;
		loopRemnants = inSoundFileInfo.frames%inBlockLength;
		}
	if (useTimer)
		gettimeofday(&taskStartTv, NULL);
	totalTime = 0.0;
	for (long loop = 0; loop < loopCount; loop++)
		{
		long ch;
		//  CAREFUL with the reuse of the Temp buffers.  The SRC routines write some history
		//   samples for successive iterations.
		for (ch = 0; ch < 4; ch++)
			tmpBufPtrs[ch] = &tmpBuffers[ch][kSRC_Filter_MaxDelayElements];

		framesRead = sf_readf_short(inSoundFile, inBlockP, inBlockLength);
                if (framesRead != inBlockLength)
		    printf("Short read of %d/%d samples\n", framesRead, inBlockLength);

//printf("HERE channels=%d\n", inSoundFileInfo.channels);

		if (useTimer)
			gettimeofday(&iterationStartTv, NULL);
		if (1 == inSoundFileInfo.channels)
			{
			RunSRC(inBlockP, outBlockP, inBlockLength, outBlockLength, &srcData[0]);
			}
		else
			{
		// Deinterleave and convert
			DeinterleaveShorts(inBlockP, tmpBufPtrs[0], tmpBufPtrs[1], inBlockLength);
		// Convert sampling rate (unsuitable for in-place operation)
			for (ch = 0; ch < channels; ch++)
				RunSRC(tmpBufPtrs[ch], tmpBufPtrs[2+ch], inBlockLength, outBlockLength, &srcData[ch]);
		// Interleave and write to file
			InterleaveShorts(tmpBufPtrs[2], tmpBufPtrs[3], outBlockP, outBlockLength);
		// NOTE:  will need to save last few samples of state
			}

		if (useTimer)
			{
			gettimeofday(&iterationEndTv, NULL);
//	totalTime += SecondsFromTimevalDiff(&iterationStartTv, &iterationEndTv);
			AddTimevalDiff(&totalTv, &iterationStartTv, &iterationEndTv);
			}

	// Write results to file
//		CopyShorts(inBlockP, outBlockP, inBlockLength);
		if 	(outH)
			fwrite(outBlockP, inSoundFileInfo.channels*sizeof(short), outBlockLength, outH);
		else if (outSoundFile)
			framesWritten = sf_writef_short(outSoundFile, outBlockP, outBlockLength);

		if (processOnlyOneBlock)
			{
			for (j = 0; j < 30; j++)
				printf("inBlockP %2d : %d\n", j, inBlockP[j]);
			for (j = 0; j < 30; j++)
				printf("outBlockP %2d : %d\n", j, outBlockP[j]);
			printf("NOTE: single block test \n");
			}
		}
	if (useTimer)
		gettimeofday(&taskEndTv, NULL);
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

