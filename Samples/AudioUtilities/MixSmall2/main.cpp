// ============================================================================
//  Test for Small Mixer (Crammer) 
//
//              Written by Gints Klimanis
// ============================================================================

#include <stdlib.h>
#include <stdio.h>#include <string.h>
#include <ctype.h>

#include "mix_common.h" 
#include "mix_small.h" 
#include "mix_smallf.h" 
#include "dsputil_mix.h" 

char *appName = "TinyMixer";
long verbose = False;

long blockLength = 256;
#define kMax_MixerChannels 5
short *inBufPs[kMax_MixerChannels];
short *outBufPs[kMax_MixerChannels];
short *tmpBufPs[kMax_MixerChannels][kMixerChannel_TempBufferCount];

#define WRITE_TEXT_FILE_OUTPUT
#ifdef WRITE_TEXT_FILE_OUTPUT
FILE *hMixIns [kMax_MixerChannels];
FILE *hMixOuts[kMax_MixerChannels];
#endif

//============================================================================
// Stricmp:   Case insensitive strcmp()
//============================================================================
    int 
Stricmp(char * s1, char *s2)
{
char c1, c2;
int  v;

do {
    c1 = *s1++;
    c2 = *s2++;
// The casts are necessary when s1 is shorter and char is signed 
    v = (unsigned int) tolower(c1) - (unsigned int) tolower(c2);
} while ((v == 0) && (c1 != '\0') && (c2 != '\0') );

return v;
}	// ---- end Stricmp() ---- 

//============================================================================
// CreateTextFile:		Create text file with "w" property
//============================================================================
	FILE *   
CreateTextFile(char *path)
{
FILE *h = fopen(path, "w");
if (!h)
    printf("Unable to open '%s' \n", path);
return (h);
}	// ---- end CreateTextFile() ---- 

//============================================================================
// MixerTestInit()		
//============================================================================
	void   
MixerTestInit()
{
#ifdef WRITE_TEXT_FILE_OUTPUT
long i;
// Create input test files (only some are used right now)
{
char s[100];
float fs = 8000.0f;
float gainLinear = 30000;

for (i = 0; i < kMax_MixerChannels; i++)
    {
    hMixIns [i] = NULL;
    hMixOuts[i] = NULL;
    }
for (i = 0; i < kMax_MixerChannels; i++)
    {
    sprintf(s, "Test/mixIn%d.txt", i+1);
    hMixIns[i] = CreateTextFile(s);
    sprintf(s, "Test/mixOut%d.txt", i+1);
    hMixOuts[i] = CreateTextFile(s);
    }
// These sine waves are too fast, but they make it easy to see tests in text files
BlastSineOscillatorS16(inBufPs[0], blockLength, 1000.0*10.0/fs, gainLinear);
BlastSineOscillatorS16(inBufPs[1], blockLength, 1000.0*20.0/fs, gainLinear);
BlastSineOscillatorS16(inBufPs[2], blockLength, 4000.0*40.0/fs, gainLinear);
BlastSineOscillatorS16(inBufPs[3], blockLength, 4000.0*40.0/fs, -gainLinear);
//inBufPs[1] = inBufPs[0];
//inBufPs[3] = inBufPs[2];
for (i = 0; i < kMax_MixerChannels; i++)
    {
    for (long j = 0; j < blockLength; j++)
        fprintf(hMixIns[i], "%d, %d\n", j, inBufPs[i][j]);
     }
}
#endif // WRITE_TEXT_FILE_OUTPUT
}	// ---- end MixerTestInit() ---- 

// ============================================================================
// MixerTestRun()		
// ============================================================================
	void   
MixerTestRun()
{
#ifdef WRITE_TEXT_FILE_OUTPUT
    for (long j = 0; j < blockLength; j++)
        {
        fprintf(hMixOuts[kLeft ], "%d, %d\n", j, outBufPs[kLeft ][j]);
        fprintf(hMixOuts[kRight], "%d, %d\n", j, outBufPs[kRight][j]);
        }
#endif // WRITE_TEXT_FILE_OUTPUT
}	// ---- end MixerTestRun() ---- 

// ============================================================================
// MixerTestCleanUp()		
// ============================================================================
	void   
MixerTestCleanUp()
{
#ifdef WRITE_TEXT_FILE_OUTPUT
// Close analysis I/O text files
for (long i = 0; i < kMax_MixerChannels; i++)
    {
    if (hMixIns[i])
        fclose(hMixIns[i]);
    if (hMixOuts[i])
        fclose(hMixOuts[i]);
    }
#endif // WRITE_TEXT_FILE_OUTPUT
}	// ---- end MixerTestCleanUp() ---- 

//============================================================================
// Malloc8:		Return buffer aligned on 8 byte boundary
//============================================================================
	void *   
Malloc8(long bytes)
{
unsigned long p = 0xfffffff8 & (unsigned long)malloc(bytes + 8);
return ((void *) p);
}	// ---- end Malloc8() ---- 

//============================================================================
// MallocA:		Return buffer aligned to "power of 2" bytes boundary
//============================================================================
void * MallocA(long bytes, int align)
{
    if( !(align & (align-1)))
	return (void*)((int)malloc(bytes + align) & ~(align-1));
    return 0;
}

//============================================================================
// PrintUsage:		
//============================================================================
	void   
PrintUsage()
{
printf("Usage:  %s <Options> \n", appName);
//printf("Usage:  %s [Options] <infile><outfile>\n", appName);
printf("Options:\n");
//printf("    -v  verbose operation\n");
printf("    -h  help\n");
}	// ---- end PrintUsage() ---- 

//============================================================================
// main(): 
//============================================================================
	int 
main(int argc, char *argv[]) 
{
long i = 0, j = 0;
MIXER mixer;

// Parse file arguments
for (i = 1; i < argc; i++)
    {
    char *s = argv[i];

    if 	    (!Stricmp(s, "-v") || !Stricmp(s, "-verbose"))
    	verbose = True;
    else if (!Stricmp(s, "-h") || !Stricmp(s, "-help"))
    	{
    	PrintUsage();
    	exit(0);
    	}
    else if (!Stricmp(s, "-inGain"))
    	{
    //	inGain = atof(argv[++i]);
    	}
    else
    	{
    	printf("Invalid arguments\n");
    	exit(-1);
    	}
    }  // ---- end argument parsing

// Allocate memory buffers (aligned on 8 byte boundaries)
for (i = 0; i < kMax_MixerChannels; i++)
    {
    inBufPs [i] = (short *) Malloc8(blockLength*sizeof(short));
    ClearShorts(inBufPs[i], blockLength);

    outBufPs[i] = (short *) Malloc8(blockLength*sizeof(short));
    ClearShorts(inBufPs[i], blockLength);

    for (j = 0; j < kMixerChannel_TempBufferCount; j++)
        {
        tmpBufPs[i][j] = (short *) Malloc8(blockLength*sizeof(short));
        ClearShorts(tmpBufPs[i][j], blockLength);
        }
    }

MixerTestInit();

// Configure mixer
DefaultMixer(&mixer);
for (i = 0; i < kMax_MixerChannels; i++)
    MixerChannel_SetTempBuffers(&mixer.channels[i], &tmpBufPs[i][0], kMixerChannel_TempBufferCount);

float pan    = 0.0f;    // Range [-1 .. 1], where zero is center
float gainDB = 0.0f;    // Range [-96 .. 0] dB (Decibels) -> may want linear gain

mixer.channelCount = 3;
SetMixer_HeadroomDB(&mixer, -3.0f);
SetMixer_OutGainDB(&mixer, 0.0f);
SetMixer_ChannelParameters(&mixer, 0, kMixerChannel_Type_In1_Out2, 0.0f, 0.0f);
SetMixer_ChannelParameters(&mixer, 1, kMixerChannel_Type_In1_Out2, 0.0f, 0.0f);
SetMixer_ChannelParameters(&mixer, 2, kMixerChannel_Type_In2_Out2, 0.0f, 0.0f);
UpdateMixer(&mixer);

// Run mixer (although, this test code doesn't have real signal input)
for (i = 0; i < 1; i++)
    {
    RunMixeri(inBufPs, outBufPs, blockLength, &mixer);
    MixerTestRun();
    }

MixerTestCleanUp();

return 0;
}  // ---- end main() ----

