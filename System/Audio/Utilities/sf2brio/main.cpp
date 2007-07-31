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

#include "spmidi.h"

// Brio headers
//#include <CoreTypes.h>
//#include "AudioTypes.h"
//#include <AudioConfig.h>
//#include <AudioOutput.h>

#include "Dsputil.h"
#include "util.h"
#include "fir.h"
#include "mix.h"
#include "src.h"

#include "main.h"

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
double cpuClockMHz = 320.0;

// Some types defined for Brio
typedef unsigned long  U32;
typedef unsigned short U16;
typedef                U32 tRsrcType;

// Brio audio file header
struct tAudioHeader {
	U32	  offsetToData;		// Offset from the start of the header to
					// the start of the data (std is 16)
	tRsrcType type;			// AudioRsrcType
	U16	  flags;		// Bit mask of audio flags
					// (Bit0: 0=mono, 1=stereo)
	U16	 sampleRate;		// Sample rate in Hz			
	U32	 dataSize;		// Data size in bytes
};

//============================================================================
// Print_SF_INFO  :   Print SF_INFO data structure
//============================================================================
	void
Print_SF_INFO(SF_INFO *d)
{	     
//printf("Print_SF_INFO: start \n");

// Reject files that aren't WAV or AIFF
long fileFormatMajor =  d->format & SF_FORMAT_TYPEMASK;
long fileFormatMinor =  d->format & SF_FORMAT_SUBMASK ;
char *sMajor = "Dunno";
char *sMinor = "Dunno";

if 	(SF_FORMAT_WAV == fileFormatMajor)
	sMajor = "WAV";
else if (SF_FORMAT_AIFF == fileFormatMajor)
	sMajor = "AIFF";

if 	(SF_FORMAT_PCM_16 == fileFormatMinor)
	sMinor = "16";
else if (SF_FORMAT_PCM_24 == fileFormatMinor)
	sMinor = "24";
else if (SF_FORMAT_PCM_32 == fileFormatMinor)
	sMinor = "32";
else if (SF_FORMAT_PCM_S8 == fileFormatMinor)
	sMinor = "S8";
else if (SF_FORMAT_PCM_U8 == fileFormatMinor)
	sMinor = "U8";

//sf_count_t	frames ;
//printf("Print_SF_INFO: %ld frames , %d Hz, %d channels \n", d->frames, d->samplerate, d->channels);
//printf("               sections=%d seekable=%d \n", d->sections, d->seekable);
//printf("               format = '%s' SF_FORMAT_PCM_%s\n", sMajor, sMinor);
}	// ---- end Print_SF_INFO() ----

//==============================================================================
// OpenSoundFile  : Short term support for audio file playback
//
//					Return handle of sound file
//==============================================================================
SNDFILE	*OpenSoundFile( char *path, SF_INFO *sfi, long rwType)
// rwType : SFM_READ, SFM_WRITE
{	     
SNDFILE	*sndFile;
// printf("OpenSoundFile: start  '%s'\n", path);
   
//
// ---------------------- Open input sound file
//
if (SFM_READ == rwType)
	{
// Initialize FILE in structure
	memset (sfi, 0, sizeof(SF_INFO)) ;

// "sineorig.wav" "FightClub_first_time1.wav"
// "fragMono.wav" "BlueNile44m.wav"
// "SquareC2.wav" "noise44_30.wav"
// "FightClub_first_time_44100_16bit_stereo.wav"
// "FightClub_first_time_44100_16bit_mono.wav"
//strcpy(audioFilePath, ".wav");

//printf("OpenSoundFile: gonna load '%s' \n", path);
	if (!(sndFile = sf_open (path, rwType, sfi)))
		{	
		printf ("OpenSoundFile : Not able to open input file '%s'\n", path) ;
		return (NULL);
		} 

	strcpy(inSoundFilePath, path);
	//printf("OpenSoundFile samplerate=%d frames=%d ch=%d format=%X\n",
	//	   sfi->samplerate, sfi->frames, sfi->channels, sfi->format) ;
	Print_SF_INFO(sfi);

	// Reject files that aren't WAV or AIFF
	long fileFormatMajor =  sfi->format & SF_FORMAT_TYPEMASK;
	long fileFormatMinor =  sfi->format & SF_FORMAT_SUBMASK  ;
	if (SF_FORMAT_WAV == fileFormatMajor)
	{
	//	printf ("OpenSoundFile : This is a WAV file \n") ;
	}
	else if (SF_FORMAT_AIFF == fileFormatMajor)
	{
	//	printf ("OpenSoundFile : This is a AIFF file \n") ;
	}
	else
	{
		printf ("OpenSoundFile : Unsupported major file format type %X\n", fileFormatMajor) ;
		return (inSoundFile);
	}

	{
	long wordWidthBits = 16;

	if 	(SF_FORMAT_PCM_16 == fileFormatMinor)
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
		printf("OpenSoundFile: unsupported minor sound file format %X \n", fileFormatMinor);
		return (sndFile);
		}

	printf("OpenSoundFile samplerate=%d frames=%d ch=%d width=%d\n",
		   sfi->samplerate, sfi->frames, sfi->channels, wordWidthBits) ;
	}
	}   	  
//
// ---------------------- Open output sound file
//
else if (SFM_WRITE == rwType)
	{
//printf("OpenSoundFile: gonna open file for writing '%s' \n", path);
	if (!(sndFile = sf_open (path, rwType, sfi)))
		{	
		printf ("OpenSoundFile : Not able to open output file '%s'\n", path) ;
		return (NULL);
		} 
	Print_SF_INFO(sfi);
	}

return (sndFile);
}	// ---- end OpenSoundFile() ----

//==============================================================================
// CloseSoundFile  :   Close audio file structure
//
//        NOTE: Short term support for audio file playback
//
//					Return Boolean success
//==============================================================================
int CloseSoundFile( SNDFILE **soundFile )
{	     
//printf("CloseSoundFile: start\n");
 
 if (*soundFile)
	 {
	 sf_close(*soundFile) ;
	 *soundFile = NULL;
	return (True);
	 }
 	
//strcpy(soundFilePath, "");
	   	   	
return (False);
}		// ---- end CloseSoundFile() ----

#ifdef NEEDED
//==============================================================================
// RewindSoundFile  :   Seek to beginning of file
//
//        NOTE: Short term support for audio file playback
//
//					Return Boolean success
//==============================================================================
int RewindSoundFile( SNDFILE **soundFile, SF_INFO *afi, char *path )
{	     
//printf("RewindSoundFile: start 222\n");

// Quick hack for initial implementation
// FIXXXX: use seek()
CloseSoundFile(soundFile);
  	 
*soundFile = OpenSoundFile(path, afi);
 if (!*soundFile)
 {
 	printf("RewindSoundFile: unable to reopen '%s'\n", path);
	return (False);
 }
 
//printf("RewindSoundFile: end\n");

	return (True);
}		// ---- end RewindSoundFile() ----
#endif // end NEEDED

//============================================================================
// PrintUsage:		
//============================================================================
	void   
PrintUsage()
{
printf("Usage:  %s <infile> <outfile>\n", appName);
//printf("Usage:  %s [Options] <infile><outfile>\n", appName);
printf("Options:\n");
printf("    -v  verbose operation\n");
printf("    -reps   iterations \n");
printf("    -t      timer : Use timer for MIDI rendering\n");
printf("    -1      singleblock : process only one block\n");
printf("    -c      channels \n");
printf("    -i      infile \n");
printf("    -o      outfile \n");
printf("    -outFormat brio, aiff, wav, raw (default is in format)\n");
printf("    -largeBlocks \n");
printf("    -fs     sampling frequency (Hz)\n");
printf("    -fixedpt	Use Fixed point arithmetic (Integer)    \n");
printf("    -floatingpt	Use 32-bit floating point arithmetic (Default)    \n");
printf("    -inGain     input  gain (Linear)   \n");
printf("    -inGainDB   input  gain (Decibels) \n");
printf("    -outGain    output gain (Linear)   \n");
printf("    -outGainDB  output gain (Decibels) \n");
printf("---- MIDI Parameters \n");
printf("    -fs_midi  MIDI engine internal sampling frequency (Hz)\n");
printf("    -midi     Input file rendered with General MIDI \n");
printf("    -voices   MIDI voice count\n");
printf("---- FIR Filter Parameters \n");
printf("    -fir    \n");
printf("    -firType  {LowPass,HighPass,BandPass,BandStop} \n");
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
else if (!Stricmp(s, "-fixedpt") || !Stricmp(s, "-fixedpoint"))
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
	if 	(!Stricmp(s, "brio"))
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
//printf("Wrote Brio header file in %d bytes\n", sizeof(tAudioHeader));
		}
	inFileTime = ((double) inSoundFileInfo.frames)/(double)inSoundFileInfo.samplerate;
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
	for (i = 0; i < kMaxChannels; i++)
		PrepareFIR(&firData[i]);
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
		if (useFIR)
			{
			if (useFixedPoint)
				RunFIR_Shortsi(inBlockP, outBlockP, length, &firData[0]);
			else
				RunFIR_Shortsf(inBlockP, outBlockP, length, &firData[0]);
			if (outH)
				fwrite(outBlockP, sizeof(short), length*inSoundFileInfo.channels, outH);
			else if (outSoundFile)
				framesWritten = sf_writef_short(outSoundFile, outBlockP, length);
			}
		else {
			
			if (outH)
				fwrite(inBlockP, sizeof(short), length*inSoundFileInfo.channels, outH);
			else if (outSoundFile)
				framesWritten = sf_writef_short(outSoundFile, inBlockP, length);
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


printf("inBlockLength = %d outBlockLength=%d\n", inBlockLength, outBlockLength);

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

