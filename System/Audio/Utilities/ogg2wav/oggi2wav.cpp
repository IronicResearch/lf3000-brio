//  oggi2wav : application to read OggVorbis audio files and convert to PCM, including benchmarking output

// Modified to take input file, decompress Ogg stream and write uncompressed, 16-bit output file

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
//#include <stropts.h>  // messes up everything
#include <pthread.h>
#include <sched.h> // NEEDED ?

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>

//#include <tremor/ivorbiscodec.h>
//#include <tremor/ivorbisfile.h>
#include <ivorbiscodec.h>
#include <ivorbisfile.h>

#include "sfconfig.h"
#include "sndfile.h"

#include "Dsputil.h"
#include <util.h>
//#include "sf.h"
#include "sndfileutil.h"

#define False 0
#define True  1

char *appName = "oggi2wav";
long verbose = False;
long highThreadPriority = True;

// Messages for output log files
FILE *_stOut = NULL;
char stOutFilePath[500];
char _msgStr[1000];

long minOutput = False;

// Input and output files
#define kMax_InFiles    5
long inFileCount = 0;
FILE *inH[kMax_InFiles];
char inFilePath[kMax_InFiles][500];
OggVorbis_File inVorbisFile[kMax_InFiles];
vorbis_info *inVorbisInfo[kMax_InFiles];

typedef struct {
    float cpuMIPS;
    long channels;
    long bitrate;
    long samplingFrequency;
} FILESUMMARY;
FILESUMMARY inFileSummary[kMax_InFiles];

//#define FILE_OUTPUT_SAMPLES_AS_BINARY
#define FILE_OUTPUT_SAMPLES_AS_WAVFILE
char outFilePath[500];
#ifdef FILE_OUTPUT_SAMPLES_AS_BINARY
FILE *outH = NULL;
#endif
#ifdef FILE_OUTPUT_SAMPLES_AS_WAVFILE
SNDFILE	*outSoundFile = NULL;
SF_INFO	outSoundFileInfo;
#endif

// High resolution timer variables
#ifdef _CPU_IS_ARM
long useHRT = True;
#ifdef EMULATION
static const char default_rtc[] = "/dev/rtc";
#else
static const char default_rtc[] = "/dev/rtc0";
#endif
#else
long useHRT = False;
static const char default_rtc[] = "/dev/rtc";
#endif
int _hrtFD = 0;
unsigned long long _hrtClockFrequency = 0;
pthread_mutex_t mutexValue_T1 = PTHREAD_MUTEX_INITIALIZER;
	

time_t startTT, endTT;
double startTime, endTime;
unsigned long long totalTimeHRT;
unsigned long long totalTimeHRT_BlockAccumulated = 0;

time_t startTs, endTs;
struct timeval totalTv, totalSumTv;
struct timeval taskStartTv, taskEndTv;

double iterationTime[kMax_InFiles], totalTime;
long iTotalTime;
long iteration, totalIterations = 1;
struct timeval iterationStartTv[kMax_InFiles], iterationEndTv[kMax_InFiles];
unsigned long long iterationTimeStartHRT[kMax_InFiles], iterationTimeEndHRT[kMax_InFiles];

long blockCount;
struct timeval blockStartTv, blockEndTv;
unsigned long long blockTimeStartHRT, blockTimeEndHRT;

long minBlockIndex = 0;
long maxBlockIndex = 0;
struct timeval firstBlockTimeTv;
struct timeval minBlockTimeTv;
struct timeval maxBlockTimeTv;
unsigned long long firstBlockTimeHRT;
unsigned long long minBlockTimeHRT;
unsigned long long maxBlockTimeHRT;

// 280 MHz = Blue board
// 320 MHz = Magic Eyes 
// 380 MHz = target Lightning clock frequency
// 385 MHz = blue Lightning development board
double cpuClockMHz = 385.0;
double execTime = 0.0;
double inFileTime_Seconds[kMax_InFiles]; // = ((double) frames)/(double)vi->rate;
double blockTimeInterval_Seconds[kMax_InFiles];

long useTotalTimer = False;
long useBlockTimer = False;

long _useLogFile = False;
char _logFilePath[500];
FILE *_hLogFile = NULL;

#define kMaxBytesToDecode 16384
long pcmOut[kMaxBytesToDecode]; 
//long pcmSum[kMaxBytesToDecode]; 

#define kDefault_BecodeBytesPerBlock 256
long decodeBytesPerBlock; //kMaxBytesToDecode;
long decodeBytesPerBlockSpecified = False;
// Default to 256, 128, or 64, which yield in decode in perfect multiples of that #
// larger block values are broken up into variable length segments.

long totalFrames[kMax_InFiles], totalSamples[kMax_InFiles];
long framesPerBlock[kMax_InFiles];
double secondsPerBlock[kMax_InFiles];

long blocksDecoded [kMax_InFiles];
long blocksToDecode[kMax_InFiles];
struct timeval *_blockTimingDataTv = NULL;

#define kCompareTimeval_A_EqualTo_B       0
#define kCompareTimeval_A_GreaterThan_B   1
#define kCompareTimeval_A_LessThan_B    (-1)

#define kStricmp_Equal    0
#define kStricmp_NotEqual 1

char s[500];

// Defines  
// FIXME/tp: "VALIDATE" rather than "ASSERT"?  We are not aborting execution here.
// FIXME/tp: Use Kernel::Printf for consistency
//#define ASSERT_POSIX_CALL(err) \
//if(err) \
//{ \
//	int errsave = errno; \
//	printf("\n***** POSIX function fails with error # (%d). File (%s), Line (%d)\n", \
//	(int)err, __FILE__, __LINE__); \
//	printf("Error string: %s\n", strerror(errsave)); \
//	fflush(stdout); \
//	return(0); \
//}
// 	return(AsBrioErr(errsave)); \


//============================================================================
// DualPrintf:		
//============================================================================
	void   
DualPrintf(char *s)
{
printf("%s", _msgStr);
if (_stOut)
    fprintf(_stOut, _msgStr);
}	// ---- end DualPrintf() ---- 

// **********************************************************************************
// ClearTimeval:	Set timeval structs to zero
// ********************************************************************************** 
	void   
ClearTimeval(struct timeval *x)
{
x->tv_sec  = 0;
x->tv_usec = 0;
}	// ---- end ClearTimeval() ---- 

// **********************************************************************************
// SetTimeval:	Set timeval structs to constant
// ********************************************************************************** 
	void   
SetTimeval(struct timeval *y, long seconds, long uSeconds)
{
y->tv_sec  = seconds;
y->tv_usec = uSeconds;
}	// ---- end SetTimeval() ---- 

// **********************************************************************************
// AssignTimeval:	 y = x
// ********************************************************************************** 
	void   
AssignTimeval(struct timeval *y, struct timeval *x)
{
y->tv_sec  = x->tv_sec;
y->tv_usec = x->tv_usec;
}	// ---- end AssignTimeval() ---- 

// **********************************************************************************
// SetTimevalMin:	Set timeval to minimum value
// ********************************************************************************** 
	void   
SetTimevalMin(struct timeval *y)
{
y->tv_sec  = 0;
y->tv_usec = 0;
}	// ---- end SetTimevalMin() ---- 

// **********************************************************************************
// SetTimevalMax:	Set timeval to maximum value
// ********************************************************************************** 
	void   
SetTimevalMax(struct timeval *y)
{
y->tv_sec  = 0x10000000;
y->tv_usec = 0;
}	// ---- end SetTimevalMax() ---- 

// **********************************************************************************
// PrintTimeval:	Print struct stuff to console
// ********************************************************************************** 
	void   
PrintTimeval(char *label, struct timeval *tv)
{
if (!label)
    label = "";

printf("PrintTimeval: %s sec=%d usec=%d -> %f seconds\n",  label, (int)tv->tv_sec, (int)tv->tv_usec,
	((double) tv->tv_sec) + 0.000001*(double) tv->tv_usec);
}	// ---- end PrintTimeval() ---- 

// **********************************************************************************
// SecondsFromTimeval:	Return time in seconds	
// ********************************************************************************** 
	double   
SecondsFromTimeval(struct timeval *tv)
{
double time = ((double) tv->tv_sec) + 0.000001*(double) tv->tv_usec;
return (time);
}	// ---- end SecondsFromTimeval() ---- 

// **********************************************************************************
// SecondsFromTimevalDiff:	Return time in seconds from two timeval structs	
// ********************************************************************************** 
	double   
SecondsFromTimevalDiff(struct timeval *startTv, struct timeval *endTv)
{
double endTime   = ((double)  endTv->tv_sec) + 0.000001*(double)  endTv->tv_usec;
double startTime = ((double)startTv->tv_sec) + 0.000001*(double)startTv->tv_usec;
return (endTime - startTime);
}	// ---- end SecondsFromTimevalDiff() ---- 

// **********************************************************************************
// AddTimeval:	Add timeval structs using integer-only arithmetic
//			Result is in argument a = a + b
// ********************************************************************************** 
	void   
AddTimeval(struct timeval *a, struct timeval *b)
{
a->tv_sec  += b->tv_sec;
a->tv_usec += b->tv_usec;

// Check and compensate for any overflow in microsecond addition
if (a->tv_usec >= 1000000)
	{
	a->tv_sec  += 1;
	a->tv_usec -= 1000000;
	}
}	// ---- end AddTimeval() ---- 

//**********************************************************************************
// SubTimeval:	Subtract timeval structs using integer-only arithmetic
//			Result is in argument a = b - a
//********************************************************************************** 
	void   
SubTimeval(struct timeval *a, struct timeval *b)
{
a->tv_sec  = b->tv_sec  - a->tv_sec;
a->tv_usec = b->tv_usec - a->tv_usec;

// Check and compensate for any overflow in microsecond addition
if (a->tv_usec < 0)
	{
	a->tv_sec  -= 1;
	a->tv_usec += 1000000;
	}
}	// ---- end SubTimeval() ---- 

//**********************************************************************************
// DiffTimeval:	Subtract timeval structs using integer-only arithmetic
//			Result is in argument diff = b - a
//********************************************************************************** 
	void   
DiffTimeval(struct timeval *diff,  struct timeval *a, struct timeval *b)
{
diff->tv_sec  = b->tv_sec  - a->tv_sec;
diff->tv_usec = b->tv_usec - a->tv_usec;

// Check and compensate for any overflow in microsecond addition
if (diff->tv_usec < 0)
	{
	diff->tv_sec  -= 1;
	diff->tv_usec += 1000000;
	}
}	// ---- end DiffTimeval() ---- 

// **********************************************************************************
// CompareTimeval:	Return  0 if a = b
//			Return  1 if a > b
//			Return -1 if a < b
// ********************************************************************************** 
	long  
CompareTimeval(struct timeval *a, struct timeval *b)
{
// Compare seconds, then microseconds
if (b->tv_sec == a->tv_sec) 
	{
	if 	(a->tv_usec == b->tv_usec)
		return (kCompareTimeval_A_EqualTo_B);
	else if (a->tv_usec  > b->tv_usec)
		return (kCompareTimeval_A_GreaterThan_B);
	else //if (a->tv_usec < b->tv_usec)
		return (kCompareTimeval_A_LessThan_B);
	}

if (a->tv_sec > b->tv_sec) 
	return (kCompareTimeval_A_GreaterThan_B);

//if (a->tv_sec < b->tv_sec) 
	return (kCompareTimeval_A_LessThan_B);
}	// ---- end CompareTimeval() ---- 

// **********************************************************************************
// AddTimevalDiff:	Add timeval structs using integer-only arithmetic
//			Result is in argument y += (b-a)
// ********************************************************************************** 
	void   
AddTimevalDiff(struct timeval *y, struct timeval *a, struct timeval *b)
{
struct timeval diff;

//SetTimeval(&diff, a);
//SubTimeval(&diff, b);
DiffTimeval(&diff,  a, b);

AddTimeval(y, &diff);
}	// ---- end AddTimevalDiff() ---- 

// **********************************************************************************
// Stricmp:	Compare two strings with case insensitivity.
//			Return 'kStricmp_Equal' if strings are the same, otherwise return
//	        kStricmp_NotEqual 1
//
// ********************************************************************************** 
	int   
Stricmp(char *s1, char *s2)
{
long i;
assert(s1 != NULL);
assert(s2 != NULL);
//printf("\n");
//printf("Stricmp(): s1 = '%s'\n", s1);
//printf("Stricmp(): s2 = '%s'\n", s2);
//printf("Stricmp(): 'A'=%d .. 'Z'=%d \n", 'A', 'Z');
//printf("Stricmp(): 'a'=%d .. 'z'=%d \n", 'a', 'z');
//exit(0);

for (i = 0; s1[i] != '\0' && s2[i] != '\0'; i++)
	{
    long c1 = (long)s1[i];
    long c2 = (long)s2[i];

// Force all values to lower case for comparison
// 'A' = 65 , 'Z' = 90
// 'a' = 97 , 'z' = 122
	if (c1 >= 'A' && c1 <= 'Z')
		c1 = c1 - 'A' + 'a';
	if (c2 >= 'A' && c2 <= 'Z')
		c2 = c2 - 'A' + 'a';

	if (c1 != c2)
    {
//printf("Stricmp(): c1=%c != c2=%c \n", (char)c1, (char) c2);
        return (kStricmp_NotEqual);
    }
	}

// Check if reached end of both strings, strings are the same
if (s1[i] == '\0' && s2[i] == '\0')
	return (kStricmp_Equal);

return (kStricmp_NotEqual);
}	// ---- end Stricmp() ---- 

// ============================================================================
// GetHighResolutionTime_OPEN:		Aquire resolution timer structure.  
//                              
//                      return fd 
// ============================================================================
	int 
GetHighResolutionTime_OPEN() 
{
//		struct rtc_pll_info rtc_timer;  // struct rtc_pll_info{
								//	int pll_ctrl;     placeholder for fancier control 
								//	int pll_value;    get/set correction value 
								//	int pll_max;      max +ve (faster) adjustment value 
								//	int pll_min;      max -ve (slower) adjustment value 
								//	int pll_posmult;  factor for +ve correction 
								//	int pll_negmult;  factor for -ve correction 
								//	long pll_clock;   base PLL frequency 
								//};
struct rtc_pll_info rtc_timer;  

int err = pthread_mutex_lock( &mutexValue_T1);
// ASSERT_POSIX_CALL( err );

int fd = open(default_rtc, O_RDONLY);
// ASSERT_POSIX_CALL( errno );

// Get the timer structure to setup clock variable
ioctl(fd, RTC_PLL_GET, rtc_timer);
// err = errno;
// ASSERT_POSIX_CALL( errno );
_hrtClockFrequency = rtc_timer.pll_clock;
//printf("GetHighResolutionTime_OPEN: pll_clock = %llu \n", rtc_timer.pll_clock);

return (fd);
}	// ---- end GetHighResolutionTime_OPEN() ---- 

// ============================================================================
// GetHighResolutionTime_CLOSE:		Aquire resolution timer structure.  
//                              
//                      Elapsed time since System startup 
// ============================================================================
	void 
GetHighResolutionTime_CLOSE(int fd) 
{
close(fd);
int err = pthread_mutex_unlock( &mutexValue_T1);
// ASSERT_POSIX_CALL( err );
}	// ---- end GetHighResolutionTime_CLOSE() ---- 

// ============================================================================
// GetHighResolutionTime_CORE:		Aquire resolution timer structure.  
//                              
//                      Elapsed time since System startup 
// ============================================================================
	void 
GetHighResolutionTime_CORE(struct rtc_pll_info *rtc_timer) 
{
//_hrtFD = GetHighResolutionTime_OPEN();
ioctl(_hrtFD, RTC_PLL_GET, rtc_timer);
//GetHighResolutionTime_CLOSE(fd);
}	// ---- end GetHighResolutionTime_CORE() ---- 

// ============================================================================
// GetHighResolutionTime_Clock:		Aquire resolution clock value.  
// ============================================================================
	unsigned long long 
GetHighResolutionTime_Clock() 
{
struct rtc_pll_info rtc_timer;
ioctl(_hrtFD, RTC_PLL_GET, &rtc_timer);
return (rtc_timer.pll_clock);
}	// ---- end GetHighResolutionTime_Clock() ---- 

// ============================================================================
// HRT_ValueToMicroSeconds:		Convert PLL clock time to microseconds  
// ============================================================================
	unsigned long long 
HRT_ValueToMicroSeconds(unsigned long long value, unsigned long long clockFrequency) 
{
return ((1000000 * value) / clockFrequency);
}	// ---- end HRT_ValueToMicroSeconds() ---- 

// ============================================================================
// GetHighResolutionTime:		Aquire resolution timer structure.  
//                              
//                      Elapsed time since System startup 
// ============================================================================
	void 
GetHighResolutionTime(struct rtc_pll_info *rtc_timer) 
{
//		struct rtc_pll_info rtc_timer;  // struct rtc_pll_info{
								//	int pll_ctrl;     placeholder for fancier control 
								//	int pll_value;    get/set correction value 
								//	int pll_max;      max +ve (faster) adjustment value 
								//	int pll_min;      max -ve (slower) adjustment value 
								//	int pll_posmult;  factor for +ve correction 
								//	int pll_negmult;  factor for -ve correction 
								//	long pll_clock;   base PLL frequency 
								//};
int err = pthread_mutex_lock( &mutexValue_T1);
// ASSERT_POSIX_CALL( err );

int fd = open(default_rtc, O_RDONLY);
// ASSERT_POSIX_CALL( errno );

ioctl(fd, RTC_PLL_GET, rtc_timer);
// err = errno;
// ASSERT_POSIX_CALL( errno );

close(fd);
err = pthread_mutex_unlock( &mutexValue_T1);
// ASSERT_POSIX_CALL( err );

#if 0 // FIXME/BSK
printf("GetHighResolutionTime: rtc_timer.pll_value=%u rtc_timer.pll_clock=%d uSec=%u\n",
       (unsigned )rtc_timer.pll_value, (int )rtc_timer.pll_clock, (unsigned)uSec);
#endif        		
}	// ---- end GetHighResolutionTime() ---- 

// ============================================================================
// GetHighResolutionTime_uSeconds:		Aquire value of high resolution timer, in microseconds (uSec)
//                      Elapsed time since System startup 
// ============================================================================
	unsigned long long 
GetHighResolutionTime_uSeconds() 
{
struct rtc_pll_info rtc_timer;  

// Fill timer structure with elapsed system time
//int err = pthread_mutex_lock( &mutexValue_T1);
//int fd  = open(default_rtc, O_RDONLY);
//ioctl(fd, RTC_PLL_GET, &rtc_timer); 		
//close(fd);
//err = pthread_mutex_unlock( &mutexValue_T1);
GetHighResolutionTime_CORE(&rtc_timer);
//ioctl(_hrtFD, RTC_PLL_GET, rtc_timer);

// Convert to uSeconds
unsigned long long freq  = rtc_timer.pll_clock;  // Constant
unsigned long long value = rtc_timer.pll_value;  
assert(freq != 0);

//printf("GetHighResolutionTime_uSeconds: freq=%llu value=%llu \n", freq, value);
return ( (1000000 * value) / freq);
}	// ---- end GetHighResolutionTime_uSeconds() ---- 

// ============================================================================
// GetHighResolutionTime_Value:		Aquire value of high resolution timer
//                      Elapsed time since System startup 
// ============================================================================
	unsigned long long 
GetHighResolutionTime_Value() 
{
struct rtc_pll_info rtc_timer;  
//GetHighResolutionTime_CORE(&rtc_timer);
ioctl(_hrtFD, RTC_PLL_GET, &rtc_timer);

// Convert to uSeconds
//unsigned long long freq  = rtc_timer.pll_clock;  // Constant
//unsigned long long value = rtc_timer.pll_value;  
//printf("GetHighResolutionTime_Value: freq=%llu value=%llu \n", freq, value);
// microseconds =  (1000000 * value) / freq;
return ( rtc_timer.pll_value);
}	// ---- end GetHighResolutionTime_Value() ---- 

//============================================================================
// GetHighResolutionTime_Seconds:		Aquire value of high resolution timer as Seconds 
//                      Elapsed time since System startup .
//============================================================================
	unsigned long long
GetHighResolutionTime_Seconds() 
{//const char *rtc = default_rtc;
struct rtc_pll_info rtc_timer;

// Fill timer structure with elapsed system time
//int err = pthread_mutex_lock( &mutexValue_T1);
//int fd  = open(default_rtc, O_RDONLY);
//ioctl(fd, RTC_PLL_GET, &rtc_timer); 		
//close(fd);
//err = pthread_mutex_unlock( &mutexValue_T1);
GetHighResolutionTime(&rtc_timer);

return ((unsigned long long)rtc_timer.pll_min);
}	// ---- end GetHighResolutionTime_Seconds() ---- 

// **********************************************************************************
// ReportTimingResults:	
// ********************************************************************************** 
    void
ReportTimingResults()
{
double realTime = 0.0, cpuTime = 0.0, cpuMIPS;
double avgTime, avgBlockTime;
double firstBlockTime = SecondsFromTimeval(&firstBlockTimeTv);
double minBlockTime   = SecondsFromTimeval(&minBlockTimeTv);
double maxBlockTime   = SecondsFromTimeval(&maxBlockTimeTv);

double realTimeHRT = 0.0, cpuTimeHRT = 0.0;
double avgTimeHRT, avgBlockTimeHRT;

// Convert HRT to microseconds
if (useHRT)
    {
	_hrtClockFrequency = GetHighResolutionTime_Clock();

    totalTimeHRT      = HRT_ValueToMicroSeconds(totalTimeHRT, _hrtClockFrequency);

    firstBlockTimeHRT = HRT_ValueToMicroSeconds(firstBlockTimeHRT, _hrtClockFrequency);
    minBlockTimeHRT   = HRT_ValueToMicroSeconds(minBlockTimeHRT, _hrtClockFrequency);
    maxBlockTimeHRT   = HRT_ValueToMicroSeconds(maxBlockTimeHRT, _hrtClockFrequency);
    avgTimeHRT = ((double)totalTimeHRT)/(double)totalIterations;
    }

totalTime  = SecondsFromTimeval(&totalTv);
avgTime    = ((double)totalTime   )/(double)totalIterations;

if (!minOutput)
    printf("\n");
if (useHRT)
    {
if (!minOutput)
{
    printf("totalDecodeTimeHRT = %g Sec \n", 0.000001*(double)totalTimeHRT);
    if (useBlockTimer)
        printf("totalDecodeTimeHRT_BlockAccumulated = %g Sec \n", 0.000001*(double)totalTimeHRT_BlockAccumulated);
}
    }
else
    {
if (!minOutput)
    printf("totalDecodeTime    = %g Sec \n", totalTime);
    }

//if (1 < totalIterations)
//    printf("Avg  Time  =%g Sec\n", avgTime);

blockTimeInterval_Seconds[0] = inFileTime_Seconds[0]/(double)blocksDecoded[0];

if (useBlockTimer)
    {
//printf("\n");
//printf("firstBlockTime = %.3f mSec \n", 1000.0*firstBlockTime);
//printf("minBlockTime   = %.3f mSec \n", 1000.0*minBlockTime);
//printf("maxBlockTime   = %.3f mSec \n", 1000.0*maxBlockTime);
avgBlockTime = (avgTime-firstBlockTime)/(double)(blocksDecoded[0]-1);
//printf("AvgBlockTime   = %.3f mSec\n", 1000.0*avgBlockTime);

//printf("\n");
//printf("firstBlockTimeHRT = %.3f mSec \n", 0.001*(double)firstBlockTimeHRT);
//printf("minBlockTimeHRT   = %.3f mSec \n", 0.001*(double)minBlockTimeHRT);
//printf("maxBlockTimeHRT   = %.3f mSec \n", 0.001*(double)maxBlockTimeHRT);
avgBlockTimeHRT = (avgTimeHRT-firstBlockTimeHRT)/(double)(blocksDecoded[0]-1);
//printf("AvgBlockTimeHRT   = %.3f mSec\n", 0.001*avgBlockTimeHRT);

if (useHRT)
    printf("BlockTime HRT (mSec): avg=%.3f, 1st=%.3f, min=%.3f, max=%.3f\n", 
            0.001*avgBlockTimeHRT, 0.001*firstBlockTimeHRT, 0.001*minBlockTimeHRT, 0.001*maxBlockTimeHRT);
else
    printf("BlockTime     (mSec): avg=%.3f, 1st=%.3f, min=%.3f, max=%.3f\n", 
            1000.0*avgBlockTime, 1000.0*firstBlockTime, 1000.0*minBlockTime, 1000.0*maxBlockTime);

    double cpuClocksPerBlock = cpuClockMHz;
    if (useHRT)
        {
        double k = 0.000001*cpuClocksPerBlock;
    sprintf(_msgStr, "BlockLoad HRT (MIPS): avg=%.2f, 1st=%.2f, min=%.2f, max=%.2f\n", 
        avgBlockTimeHRT  *k, 
        firstBlockTimeHRT*k, 
        minBlockTimeHRT  *k, 
        maxBlockTimeHRT  *k);
        DualPrintf(_msgStr);
        }
    else
        {
    sprintf(_msgStr, "BlockLoad     (MIPS): avg=%.2f, 1st=%.2f, min=%.2f, max=%.2f\n", 
        avgBlockTime  *cpuClocksPerBlock, 
        firstBlockTime*cpuClocksPerBlock, 
        minBlockTime  *cpuClocksPerBlock, 
        maxBlockTime  *cpuClocksPerBlock);
    DualPrintf(_msgStr);
        }
    //printf("blockSize=%d bytes, blocksDecoded=%d\n", bytesToRead, blocksDecoded);
    }

if (useHRT)
    {
    realTimeHRT = (inFileTime_Seconds[0])/(0.000001*avgTimeHRT);
    cpuTimeHRT  = 1.0/realTimeHRT;
    cpuMIPS     = cpuTimeHRT*cpuClockMHz;
    sprintf(_msgStr, "AverageHRT (%d MHz): CPU %% = %.2f , MIPS=%.2f , %.2f X RealTime\n", 
        (long)cpuClockMHz, cpuTimeHRT*100.0, cpuMIPS, realTimeHRT);
if (!minOutput)
    DualPrintf(_msgStr);
    }
else
    {
    realTime = inFileTime_Seconds[0]/avgTime;
    cpuTime  = 1.0/realTime;
    cpuMIPS  = cpuTime*cpuClockMHz;
    sprintf(_msgStr, "Average (%d MHz): CPU %% = %.2f , MIPS=%.2f , %.2f X RealTime\n", 
        (long)cpuClockMHz, cpuTime*100.0, cpuMIPS, realTime);
if (!minOutput)
    DualPrintf(_msgStr);
    }

if (_useLogFile)
    {
    long i;
    for (i = 0; i < blocksDecoded[0]; i++)
        {
// WGnuplot format:         
fprintf(_hLogFile, "%d, %g\n", i, cpuClockMHz*SecondsFromTimeval(&_blockTimingDataTv[i])/blockTimeInterval_Seconds[0]);
// Dplot format:         
//fprintf(_hLogFile, "%d, %g, \n", blocksDecoded, SecondsFromTimeval(&blockTimeTv));
//       printf("%d, %g\n", blocksDecoded, SecondsFromTimeval(&blockTimeTv));
        }
    }
if (minOutput)
{
char s[500];
//vorbis_info    *vi =  inVorbisInfo[0];
//OggVorbis_File *vf = &inVorbisFile[0];
sprintf(s, "%3.2f MIPS %d Kbps %d ch fs=%d KHz '%s'\n",   
cpuMIPS, inFileSummary[0].bitrate/1000, inFileSummary[0].channels, inFileSummary[0].samplingFrequency/1000, inFilePath[0]);

fprintf(_stOut, "%s\n", s);
printf("%s\n", s);
}

}	// ---- end ReportTimingResults() ---- 

// **********************************************************************************
// CleanUp:	Close file handles, deallocate buffers, etc.
//               Much of this is redundant in Linux ...
// ********************************************************************************** 
	void   
CleanUp()
{
long i;

if (_hLogFile)
    {
    fclose(_hLogFile);
    _hLogFile = NULL;
    }
if (_blockTimingDataTv)
    {
    free(_blockTimingDataTv);
    _blockTimingDataTv = NULL;
    }

if (useHRT)
    {
    GetHighResolutionTime_CLOSE(_hrtFD);
    _hrtFD = 0;
    }

for (i = 0; i < kMax_InFiles; i++)
    {
        
    }

#ifdef FILE_OUTPUT_SAMPLES_AS_WAVFILE
CloseSoundFile(&outSoundFile);
#endif
#ifdef FILE_OUTPUT_SAMPLES_AS_BINARY
if (outH)		
	fclose(outH);
#endif
}	// ---- end CleanUp() ---- 

//============================================================================
// CleanUpAndExit:		Run exit routines and exit
//============================================================================
	void   
CleanUpAndExit()
{
CleanUp();
exit(0);
}	// ---- end CleanUpAndExit() ---- 

//============================================================================
// SigIntHandler:		Catch signals. For this App, run CleanUp and kill yourself.
//============================================================================
	void   
SigIntHandler(int sig)
{
//printf("SigIntHandler(): START \n");

signal(SIGINT, SIG_DFL);  // Reset SIGINT handler to default action

CleanUp();
kill(getpid(), SIGINT);   
}	// ---- end SigIntHandler() ---- 

//============================================================================
// PrintUsage:		
//============================================================================
	void   
PrintUsage()
{
printf("Usage:  %s [Options] <infile><outfile>\n", appName);
printf("Options:\n");
printf("    -v  verbose operation\n");
printf("    -h  help print usage\n");
printf("    -reps   iterations \n");
printf("    -useLogFile <logFileName> \n");
printf("    -decodeBytesPerBlock   [1 .. %d] \n", kMaxBytesToDecode);
printf("    -useTimer \n");
printf("    -cpuClockMHz (Default=%g)\n", cpuClockMHz);
}	// ---- end PrintUsage() ---- 

// **********************************************************************************
// main:		
// ********************************************************************************** 
int main(int argc, char **argv)
{
long i;

int current_section;
long iteration, totalIterations = 1;

// Set up signal handler
if (signal(SIGINT, SigIntHandler) == SIG_ERR) 
    {
    perror("Error in setting up SigIntHandler. Exiting ...");
    exit(1);
    }

if (argc < 2)
    {
    PrintUsage();
    exit(0);
    }

// Initialize file name structures
for (i = 0; i < kMax_InFiles; i++)
    {
    inH[i] = NULL;
    inFilePath[i][0] = '\0';

//    inVorbisFile[i] = ;
    inVorbisInfo[i] = NULL;
    }
inFileCount = 0;
outFilePath[0] = '\0';
sprintf(_logFilePath, "%s_Log.txt", appName);

// Parse file arguments
for (i = 1; i < argc; i++)
{
char *s = argv[i];
//printf("argv[%d]='%s'\n", i, argv[i]);

if 	(!Stricmp(s, "-v") || !Stricmp(s, "-verbose"))
	verbose = True;
else if 	(!Stricmp(s, "-minout"))
	minOutput = True;
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
else if (!Stricmp(s, "-cpuClockMHz") || !Stricmp(s, "-cpuMhz") || !Stricmp(s, "-mhz"))
	{
	cpuClockMHz = (float)atoi(argv[++i]);
//printf("cpuClockMHz=%g\n", cpuClockMHz);
//    cpuClockMHz *= 1000000.0f;
	}
else if (!Stricmp(s, "-useLogFile"))
	{
	strcpy(_logFilePath, argv[++i]);
    _useLogFile = True;
//printf("useLogFile=%d logFilePath='%s'\n", _useLogFile, _logFilePath);
	}
else if (!Stricmp(s, "-addSummaryTo"))
	{
    FILE *h;
	strcpy(stOutFilePath, argv[++i]);
//printf("stOutFilePath= '%s'\n", stOutFilePath);
    if (NULL == (h = fopen(stOutFilePath, "a+")))   // Create + write
    	{
    	printf(" unable to open stout file '%s'\n", stOutFilePath);
    	exit (-1);
    	}
    _stOut= h;
	}
else if (!Stricmp(s, "-decodeBytesPerBlock") || !Stricmp(s, "-bytesToRead"))
	{
	long t = atoi(argv[++i]);
    if (t < 1 || t > kMaxBytesToDecode)
        {
    	printf("Invalid decodeBytesPerBlock=%d\n", t);
    	printf("Use value in range [1 .. %d}\n", kMaxBytesToDecode);
    	exit(-1);
    	}
//    for (long j = 0; j < kMax_InFiles; j++)
//        decodeBytesPerBlock[i] = t;
        decodeBytesPerBlock = t;

    decodeBytesPerBlockSpecified = True;
//printf("decodeBytesPerBlock=%d\n", decodeBytesPerBlock);
	}

//
// --------- General parameters
//
else if (!Stricmp(s, "-t") || !Stricmp(s, "-timer"))
	{
	useTotalTimer = True;
//printf("useTotalTimer=%d\n", (int) useTotalTimer);
	}
else if (!Stricmp(s, "-nohp") || !Stricmp(s, "-nohtp") || !Stricmp(s, "-nohighthreadpriority"))
	{
	highThreadPriority = False;
//printf("highThreadPriority=%d\n", (int) highThreadPriority);
	}
else if (!Stricmp(s, "-blockTimer"))
	{
	useBlockTimer = True;
	useTotalTimer = True;
//printf("useBlockTimer=%d\n", (int) useBlockTimer);
	}
//else if (!Stricmp(s, "-fixedpt") || !Stricmp(s, "-fixedpoint"))
//	{
//	useFixedPoint = True;
// printf("useFixedPoint=%d\n", useFixedPoint);
//	}
//else if (!Stricmp(s, "-floatingpoint") || !Stricmp(s, "-floatpt"))
//	{
//	useFixedPoint = False;
// printf("useFixedPoint=%d\n", useFixedPoint);
//	}
else if (!Stricmp(s, "-o") || !Stricmp(s, "-outfile"))
	{
	strcpy(outFilePath , argv[++i]);
	printf("outFilePath = '%s'\n", outFilePath);
	}
else if (!Stricmp(s, "-i") || !Stricmp(s, "-infile"))
	{
	strcpy(inFilePath[inFileCount] , argv[++i]);
    if (!minOutput)
    	printf("inFilePath[%d] = '%s'\n", inFileCount, inFilePath[inFileCount]);
    inFileCount++;
	}

// Assign random parameters to input and output files - kinda dumb
else if ('\0' == inFilePath[0][0])
    {
	strcpy(inFilePath[0] , argv[i]);
    inFileCount = 1;
//printf("HERE inFilePath0\n");
    }
else if ('\0' == outFilePath[0])
	strcpy(outFilePath , argv[i]);
else
	{
	printf("Invalid arguments '%s'\n", argv[i]);
	exit(-1);
	}
}  // ---- end argument parsing

// Attempt to run thread at higher priority
if (highThreadPriority)
    {
    struct sched_param schedParam;
    int err;
    pid_t pid = getpid();

    schedParam.sched_priority = 99;  // 99 = RT = Real Time
    if (err = sched_setscheduler(pid, SCHED_FIFO, &schedParam))
        {
    //    if (EPERM == err)
#ifdef _CPU_IS_ARM
            printf("Unable to change thread priority to SCHED_FIFO.  Run as root\n");
#endif
        };
    //err = sched_getscheduler(pid);
    }

// Open input files
sprintf(_msgStr, "---------------- \n");
if (!minOutput)
DualPrintf(_msgStr);
//printf("inFileCount=%d\n", inFileCount);
for (i = 0; i < inFileCount; i++)
    {
    sprintf(_msgStr, "inFilePath%d ='%s'\n", i, inFilePath[i]);
if (!minOutput)
    DualPrintf(_msgStr);

    inH[i] = fopen(inFilePath[i], "rb");
    if (!inH[i])
    	{
    	sprintf(_msgStr, "Unable to open infile '%s' \n", inFilePath[i]);
        DualPrintf(_msgStr);

    	exit(0);
    	}
    if (ov_open(inH[i], &inVorbisFile[i], NULL, 0) < 0) 
    	{
          fprintf(stderr,"Input '%s' does not appear to be an Ogg bitstream.\n", inFilePath[i]);
          exit(1);
      	}
        inVorbisInfo[i] = ov_info(&inVorbisFile[i],-1);

// Print info section, which are comments already in the file
    char **ptr = ov_comment(&inVorbisFile[i],-1)->user_comments;
        while(*ptr)
        {
          fprintf(stderr,"%s\n",*ptr);
          ++ptr;
        }
    }

if ('\0' != outFilePath[0])
    {
    sprintf(_msgStr, "outFilePath='%s'\n", outFilePath);
    DualPrintf(_msgStr);
    }

// Write errors and values to log file.  Right now, this is used to block timing data
if (_useLogFile)
    {
//    if (NULL == (_hLogFile = fopen(_logFilePath, "a+t")))   // Append to file
    if (NULL == (_hLogFile = fopen(_logFilePath, "w+")))   // Create + write
    	{
    	printf("WriteToLogFile: unable to open log file '%s'\n", _logFilePath);
    	return (FALSE);
    	}
    }

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Gather information about input files
//
for (i = 0; i < inFileCount; i++)
{
char sLo[64], sHi[64];
vorbis_info    *vi =  inVorbisInfo[i];
OggVorbis_File *vf = &inVorbisFile[i];

totalSamples[i] = ov_pcm_total(&inVorbisFile[i],-1);
// Quite silly.  Apparently, samples = frames
totalFrames[i]  = totalSamples[i]; // vi->channels;

inFileTime_Seconds[i] = ((double) totalFrames[i])/(double)vi->rate;
sprintf(_msgStr, "%d channels, %d frames at %ld Hz (%g Seconds)\n", 
        vi->channels, totalFrames[i], vi->rate, inFileTime_Seconds[i]);
if (!minOutput)
    DualPrintf(_msgStr);

framesPerBlock [i] = decodeBytesPerBlock/(vi->channels*sizeof(short));
secondsPerBlock[i] = ((float)framesPerBlock[i])/(float)vi->rate;

blocksToDecode[i] = (ov_pcm_total(&inVorbisFile[i], -1)*sizeof(short))/decodeBytesPerBlock;
//if (_useLogFile)
//    _blockTimingDataTv = (struct  timeval *) malloc(blocksToDecode[0]*4*sizeof(timeval));

// Check for case of undefined lower and upper bitrates
//printf(" inVorbisInfo[0]->bitrate_lower = %d inVorbisInfo[0]->bitrate_upper = %d\n", inVorbisInfo[0]->bitrate_lower, inVorbisInfo[0]->bitrate_upper);

 /* The below bitrate declarations are *hints*.
     Combinations of the three values carry the following implications:
     
     all three set to the same value: 
       implies a fixed rate bitstream
     only nominal set: 
       implies a VBR stream that averages the nominal bitrate.  No hard 
       upper/lower limit
     upper and or lower set: 
       implies a VBR bitstream that obeys the bitrate limits. nominal 
       may also be set to give a nominal rate.
     none set:
       the coder does not care to speculate.
  */

// Constant bit rate (CBR) marked with -1's
if (vi->bitrate_lower == vi->bitrate_upper && vi->bitrate_lower == vi->bitrate_nominal)
    {
    sprintf(_msgStr, "Data rate = %d Kbps CBR \n",  vi->bitrate_nominal/1000);
if (!minOutput)
    DualPrintf(_msgStr);
    }
// Variable bit rate (VBR) 
else
    {
    // 0=unset bitrate
    if (vi->bitrate_lower && -1 != vi->bitrate_lower)
        sprintf(sLo, "%d", vi->bitrate_lower/1000);
    else
        strcpy(sLo, "Unset");
     if (vi->bitrate_upper && -1 != vi->bitrate_upper)
        sprintf(sHi, "%d", vi->bitrate_upper/1000);
    else
        strcpy(sHi, "Unset");
    sprintf(_msgStr, "Data rate = %d Kbps VBR <lo=%s .. hi=%s>\n",  vi->bitrate_nominal/1000, sLo, sHi);
if (!minOutput)
    DualPrintf(_msgStr);
    }

if (verbose) // || decodeBytesPerBlockSpecified)
    printf("DecodeBytesPerBlock=%d Bytes   (%d frames)\n", decodeBytesPerBlock, framesPerBlock);

//    printf("Decoded length: %ld samples\n", (long)ov_pcm_total(vf,-1));
//    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(vf-1)->vendor);
}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Initialize timer variables
if (useTotalTimer || useBlockTimer)
	{
	ClearTimeval(&totalTv);
	ClearTimeval(&totalSumTv);

	minBlockIndex = 0;
	SetTimevalMax(&minBlockTimeTv);	// Yes, set to Max
    minBlockTimeHRT = 0x3fffffffffffffll;

	maxBlockIndex = 0;
	SetTimevalMin(&maxBlockTimeTv);  // Yes, set to Min
    maxBlockTimeHRT = 0;
	}

//
// -------- Run the entire test a specified # of iterations
//
for (iteration = 0; iteration < totalIterations; iteration++)
{
if (1 < totalIterations)
    printf("iteration=%ld/%ld\n", iteration+1, totalIterations);

// Open output data file but only if specified
if ('\0' != outFilePath[0])
	{
#ifdef FILE_OUTPUT_SAMPLES_AS_BINARY
	outH = fopen(outFilePath, "wb");
	if (!outH)
		{
		printf("unable to open outfile '%s' \n", outFilePath);
		exit(0);
		}
#endif
#ifdef FILE_OUTPUT_SAMPLES_AS_WAVFILE
		outSoundFileInfo.frames     = 0;		
		outSoundFileInfo.samplerate = inVorbisInfo[0]->rate;
		outSoundFileInfo.channels   = inVorbisInfo[0]->channels;
		outSoundFileInfo.format     = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
		outSoundFileInfo.sections   = 1;
		outSoundFileInfo.seekable   = 1;

		outSoundFile = OpenSoundFile( outFilePath, &outSoundFileInfo, SFM_WRITE);
#endif
	}

if (useHRT)
    _hrtFD = GetHighResolutionTime_OPEN();

if (useTotalTimer)
    {
    if (useHRT)
        iterationTimeStartHRT[iteration] = GetHighResolutionTime_Value();
	gettimeofday(&iterationStartTv[iteration], NULL);
    }
totalTimeHRT_BlockAccumulated = 0;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Decode blocks in a loop
for (long inFileIndex = 0; inFileIndex < inFileCount; inFileIndex++)
{
long eof = 0;
ov_raw_seek(&inVorbisFile[inFileIndex], 0);
while (!eof)
	{
    	long bytesDecoded;

// Clock start time for block decode
	if (useBlockTimer)
        {
        if (useHRT)
            blockTimeStartHRT = GetHighResolutionTime_Value(); //uSeconds();
        else
		    gettimeofday(&blockStartTv, NULL);
        }
// Decode a block of Ogg-encoded data
	bytesDecoded = ov_read(&inVorbisFile[inFileIndex], (char *)pcmOut, decodeBytesPerBlock, &current_section);

// Clock end time for block decode
	if (useBlockTimer)
        {
        if (useHRT)
            blockTimeEndHRT = GetHighResolutionTime_Value(); //uSeconds();
        else
		    gettimeofday(&blockEndTv, NULL);
        }
//    if (decodeBytesPerBlock != bytesDecoded && bytesDecoded != 0)
//        printf("Block%d : decoded only %d / %d Bytes\n", blocksDecoded, bytesDecoded, decodeBytesPerBlock);

	blocksDecoded[inFileIndex] += (0 != bytesDecoded);
    eof = (0 == bytesDecoded);

// Track minimum, maximum blocks
	if (useBlockTimer)
		{
		long returnValue;
		struct timeval blockTimeTv;
        unsigned long long blockTimeHRT;

// Calculate block time from start and end times
        if (useHRT)
            {
            blockTimeHRT = blockTimeEndHRT - blockTimeStartHRT;
            totalTimeHRT_BlockAccumulated += blockTimeHRT;
            }
        else
            {
		    DiffTimeval(&blockTimeTv, &blockStartTv, &blockEndTv);

        // Add block time to summation
            if (_blockTimingDataTv)
                _blockTimingDataTv[blocksDecoded[inFileIndex]-1] = blockTimeTv;
            }

// Store first block time, but don't include this value with maxBlockTime
        if (1 == blocksDecoded[inFileIndex])
            {
            if (useHRT)
                firstBlockTimeHRT = blockTimeHRT;
            else
    	        AssignTimeval(&firstBlockTimeTv, &blockTimeTv);
            }
        else
            {
        // Update minimum and maximum block times
            if (useHRT)
                {
                if (blockTimeHRT < minBlockTimeHRT)
                    minBlockTimeHRT = blockTimeHRT;
                if (blockTimeHRT > maxBlockTimeHRT)
                    maxBlockTimeHRT = blockTimeHRT;
                }
            else
                {
        		returnValue = CompareTimeval(&blockTimeTv, &minBlockTimeTv);
        		if 	(kCompareTimeval_A_LessThan_B == returnValue)
        			{
        			minBlockIndex = blocksDecoded[inFileIndex];
        			AssignTimeval(&minBlockTimeTv, &blockTimeTv);
        //sprintf(s, "--- Block %d", blocksDecoded);
        //PrintTimeval(s, &blockTimeTv);
        			}
        		returnValue = CompareTimeval(&blockTimeTv, &maxBlockTimeTv);
        		if (kCompareTimeval_A_GreaterThan_B == returnValue)
        			{
        			maxBlockIndex = blocksDecoded[inFileIndex];
        			AssignTimeval(&maxBlockTimeTv, &blockTimeTv);
        			}
                }
    		}

//    if (_useLogFile)
//        {
// WGnuplot format:         
//fprintf(_hLogFile, "%d, %g\n", blocksDecoded, SecondsFromTimeval(&blockTimeTv));
// Dplot format:         
//fprintf(_hLogFile, "%d, %g, \n", blocksDecoded, SecondsFromTimeval(&blockTimeTv));
//       printf("%d, %g\n", blocksDecoded, SecondsFromTimeval(&blockTimeTv));
//        }

}  // end useBlockTimer

// Write blocks to file, if specified
if (bytesDecoded)
    {
#ifdef FILE_OUTPUT_SAMPLES_AS_WAVFILE
    if (outSoundFile)
        {
    long framesToWrite = bytesDecoded/(sizeof(short) * outSoundFileInfo.channels);
    long framesWritten = sf_writef_short(outSoundFile, (short *)pcmOut, framesToWrite);
    //printf("bytesDecoded=%d framesToWrite %d framesWritten=%d \n", bytesDecoded, framesToWrite, framesWritten);
        }
#endif
#ifdef FILE_OUTPUT_SAMPLES_AS_BINARY
    	if (outH)
          		fwrite(pcmOut, sizeof(char), bytesDecoded, outH);
#endif
    }
}  // end while(!eof)


//printf("blocksToDecode=%d blocksDecoded=%d\n", blocksToDecode, blocksDecoded);
//printf("blocksDecoded / blocksToDecode=%g\n", ((float)blocksDecoded)/(float)blocksToDecode);

// Collect end time information
if (useTotalTimer)
	{
    if (useHRT)
        {
        iterationTimeEndHRT[iteration] = GetHighResolutionTime_Value(); //uSeconds();
        totalTimeHRT += (iterationTimeEndHRT[iteration] - iterationTimeStartHRT[iteration]);
        }
//    else
        {
	    gettimeofday(&iterationEndTv[iteration], NULL);
//		totalTime += SecondsFromTimevalDiff(&iterationStartTv, &iterationEndTv);
	    AddTimevalDiff(&totalTv, &iterationStartTv[iteration], &iterationEndTv[iteration]);
        }
	}

#ifdef FILE_OUTPUT_SAMPLES_AS_WAVFILE
CloseSoundFile(&outSoundFile);
#endif
#ifdef FILE_OUTPUT_SAMPLES_AS_BINARY
if (outH)
    {
	fclose(outH);
    outH = NULL;
    }
#endif

// Fill structure with file info for later use
{
vorbis_info    *vi =  inVorbisInfo[i];
OggVorbis_File *vf = &inVorbisFile[i];

inFileSummary[i].channels          = vi->channels;
inFileSummary[i].bitrate           = vi->bitrate_nominal;
inFileSummary[i].samplingFrequency = vi->rate;
}
ov_clear(&inVorbisFile[i]);
} // End inFileCount
}  // END for (iterations loop)

if (useTotalTimer || useBlockTimer)
    ReportTimingResults();

CleanUp();

return(0);
}

