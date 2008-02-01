// ==============================================================================
// BrioMixer.cpp
//
// Test playing multiple audio files triggered by key presses
//                      Written by Gints Klimanis, 2007
// ==============================================================================

#include <SystemErrors.h>
#include <StringTypes.h>
#include <EventMessage.h>

#include <BrioOpenGLConfig.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>

#include <AudioMPI.h>
#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <FontMPI.h>
#include <KernelMPI.h>

#include <AudioTypes.h>

#include "AudioTypesPriv.h"
#include "AudioMsg.h"

#include <math.h>
#include <sys/stat.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

LF_USING_BRIO_NAMESPACE()
#include "2d.h"
#include "3d.h"
#include "cube.h"
#include "font.h"
#include "linebrowser.h"
#include "bitstrip.h"

#define CALL_MPI_AUDIOSTATE
//#define USE_LOG_FILE

//#define TEST_VOLUME_MASTER
#define TEST_VOLUME_INDIVIDUAL
//#define FONT_PRERENDER_GLYPHS

#ifndef kAudio_Volume_Min
#define kAudio_Volume_Min 0
#endif
#ifndef kAudio_Volume_Max
#define kAudio_Volume_Max 100
#endif
#ifndef kAudio_Volume_Default
#define kAudio_Volume_Default 100
#endif
#ifndef kAudio_Pan_Min
#define kAudio_Pan_Min (-100)
#endif
#ifndef kAudio_Pan_Max
#define kAudio_Pan_Max   100
#endif
#ifndef kAudio_Pan_Default
#define kAudio_Pan_Default   0
#endif

#if NEED_TO_DEFINE_AUDIOSTATE_STRUCT
// defined in AudioTypesPriv.h
/typedef struct taudiostate {
U8     useOutDSP;
U8     useOutEQ;
U8     srcType;

U8     useOutSoftClipper;
S16    softClipperPreGainDB;
S16    softClipperPostGainDB;

S16    outLevels_ShortTime[2];
S16    outLevels_LongTime [2];
S16    outLevels_Max      [2];
U8     computeLevelMeters;

U8     readInSoundFile;
U8     writeOutSoundFile;
U8     headroomBits;
} tAudioState; 
#endif // NEED_TO_DEFINE_AUDIOSTATE_STRUCT

#define kTest_MaxFileNameLength     500

long gMIDITestEnabled        = false;
long gSoftClipperTestEnabled = false;

long gMIDITest_OrderMIDIFiles = true;

char titleString[200];
long gLineIndex_Title    = 0;
long gLineIndex_FilePath = 1;
long gLineIndex_Message  = 2;

long gLineIndex_Pan    = 0;
long gLineIndex_Volume = 0;
long gLineIndex_Tempo  = 0;

#define kTable_LinearToDecibel_Length 32768
S16 gLinearToDecibelTable[kTable_LinearToDecibel_Length];

// -----------------------------------------
// Configurable DSP
tAudioState gAudioState;

long gUseAudioLevelMeters = true;
long meterSegments = 25;
char outTopS[50], outBottomS[50];

long gUseOutputEQ         = false;

long    gUseSoftClipper        = false;
S32     gSoftClipperPreGainDB  = 0;
S32     gSoftClipperPostGainDB = 0;
Boolean gUseSoftClipper_Specified        = false;
Boolean gSoftClipperPreGainDB_Specified  = false;
Boolean gSoftClipperPostGainDB_Specified = false;

#define kAudio_SoftClipperPreGainDB_Min     -40
#define kAudio_SoftClipperPreGainDB_Max      40
#define kAudio_SoftClipperPreGainDB_Default   0
#define kSoftClipperPreGainDB_Inc             1


char *gInSoundFilePath  = "";
char *gOutSoundFilePath = "";
float gOutSoundFileTime = 10.0f;

long gSRC_Type          = 0;
long gSRC_FilterVersion = 0;
Boolean gSRC_Type_Specified          = false;
Boolean gSRC_FilterVersion_Specified = false;

long  gLineIndex_SoftClipper_PreGainDB  = 0;
long  gLineIndex_SoftClipper_PostGainDB = 0;
long  gLineIndex_SoftClipper_Enabled    = 0;

// -----------------------------------------
//char *MIDIFileDir  = "/Didj/Data/BrioMixer/rsrc/";
#define kTest_MaxMIDIFileCount     100
char MIDIFilePaths[kTest_MaxMIDIFileCount][kTest_MaxFileNameLength];
long MIDIFileIndex = 0;
long MIDIFileCount = 0;

#define kMIDIFile_TempoScale_Max    127
#define kMIDIFile_TempoScale_Min    (-kMIDIFile_TempoScale_Max)
#define kMIDIFile_TempoScale_Default 0
#define kMIDIFile_TempoScale_Inc     12
S32  midiFile_TempoScale = 0;

#ifdef EMULATION
char *midiDir = "MIDI";
#else
char *midiDir = "/Didj/Data/BrioMixer/rsrc/MIDI";
#endif

//#define DEBUG_BRIOMIXER_SOUNDFILES
#ifdef DEBUG_BRIOMIXER_SOUNDFILES
long readInSoundFile ;  // FIXXX need to connect to Brio internal
long writeOutSoundFile;
char *inSoundFileName  = "GoldenTestSet_16k/B_Quigley.wav";
char *outSoundFilePath = "brioMixerOut.wav";
#endif

long gTest_AudioMPI_DoneMessage = false;
long gTest_AudioMPI_IsPlaying   = false;

// for AudioDoneMessage tests
long gDebugAudioDoneMessage = false;   // As a sequencer
#define kSoundFileList_MaxCount     20
char *soundFileListPaths[kSoundFileList_MaxCount];
long gSoundFileList_Count = 0;
long gSoundFileList_Index = 0;

#define kLeft   0
#define kRight  1
#define LinearToDecibel(x)		(log10((x))*20.0)
#define DecibelToLinear(d)		(pow(10.0, ((double)(d))*(1.0/20.0)))

//#define LinearS16ToDecibel(x)		(log10((1.0f/32768.0f * (float)x))*20.0)

#define kS8_Min (-128)
#define kS8_Max ( 127)
#define kS8_Maxf ((float)kS8_Max)
#define kS8_Minf ((float)kS8_Min)

long use3D = false;
long use2D = true;

// *****************************************************************************
const tDebugSignature kMyApp = kFirstCartridge1DebugSig;
const char *appName = "BrioMixer";

// VH_32k_st_056kb.ogg Temptation_32k_mono_128kb.ogg
// LightningTest_SFXA_preproc_44.ogg
// GOOD : Temptation_16k_mono.ogg Temptation_16k_st.ogg
// GOOD : Temptation_32k_mono.ogg Temptation_32k_st.ogg
// GOOD : VH_32k_st_056kb.ogg

//"Saw_32k_m3dB_m.wav";
//"sine_db00_1000Hz_32k_c1.wav";
//"SFX/tennis.wav";
//"Temptation_16k_st.wav"; //"Temptation_32k_mono.ogg";
//"Temptation_32k_st.wav";

// Rossi/ deepthoughts.mid discovery.mid etiquette.mid shredfest.mid technojazz.mid 
// Rossi/ FANFARE.MID FUSION.MID MANGO.MID TBONE.MID TRIBAL.MID etiquette.mid
// MIDI/play_programs_1_8.mid
// "MIDI/TRIBAL.MID";

// NOISE
// noise05_16k.wav.raw
// noise05_08k_040kb.ogg
// noise05_16k_064kb.ogg
// noise05_32k_064kb.ogg
// noise05_32k_128kb.ogg
// SINE
// sine_db0_1000Hz_16k.ogg
// sine_db0_1000Hz_32k.ogg
// SINE08k_Q5.ogg SINE16k_Q5.ogg SINE32k_Q5.oggstr
// SINE/ : sine_db0_1000Hz_32k.wav sine_dbM3_1000Hz_32k sine_dbM6_1000Hz_32k
// at db {00, M3, M6} and fs={16k,32k} and f= { 125, 250, 500, 1000, 2000, 4000, 8000 }

#define kMax_SoundButtons  5
#define kSoundButton_A        0
#define kSoundButton_B        1
#define kSoundButton_Left     2
#define kSoundButton_Right    3
#define kSoundButton_Home     4
char *button_FilePath[kMax_SoundButtons] = {
"BrioIn_Quigley.wav",
"Temptation_32k_st.wav",
"SINE/sine_db0_1000Hz_16k.ogg",
"NOISE/noise05_32k_128kb.ogg"
};
char *button_FilePath_Respecified[kMax_SoundButtons] = {NULL, NULL, NULL, NULL, NULL};

tAudioOptionsFlags	gAudioFlags = 0; //kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped;
int verbose = false;
long gAudioLoopCount = 0;

//#define kAudio_Pan_Max    100
//#define kAudio_Pan_Min  (-100)
//#define kAudio_Pan_Default 0
#define kPan_Inc     10
//
//#define kAudio_Volume_Max    100
//#define kAudio_Volume_Min      0
//#define kAudio_Volume_Default 100
#define kVolume_Inc     10

tAudioPriority	gAudioPriority = 1;
S32 gAudioPan    = kAudio_Pan_Default;
S32 gAudioVolume = kAudio_Volume_Default;

#ifdef USE_LOG_FILE
char *logFilePath = "BrioMixer.log";
FILE *hLogFile = NULL;
#endif

// 12 Segment meters
static S16 meters12_K[12] = {-75, -60, -45, -35, -25, -17, -14, -10 ,-7, -4, -1, 0};
static S16 meters12  [12] = //-90/12
{-90, -81, -73, -65, -57, -49, -40, -32 ,-24, -16, -8, 0};  

// 18 Segment meters
static S16 meters18_K[18] = 
{-89,-81,-72,-66,-58,-51,-44,-38,-32,-27,-22,-18,-14,-10,-7,-4,-1,0};
static S16 meters18  [18] = //-90/18
{-90,-84,-79,74,-68,-63,-58,-52,-47,-42,-37,-31,-26,-21,-15,-10,-5,0};  

// 22 Segment meters
static S16 meters22_K[22] = 
{-88,-81,-74,-67,-60,-54,-48,-42,-37,-32,-27,
-23, -19,-16,-13,-10, -7, -5, -3, -2, -1, 0};
static S16 meters22 [22] = //-90/22
{-90,-85,-81,-77,-72,-68,-64,-60,-55,-51,-47,-42,
 -38,-34,-30,-25,-21,-17,-12, -8, -4, 0};

// 25 Segment meters
static S16 meters25_K[25] = 
{-89,-86,-78,-72,-66,-60,-54,-49,-45,-39,-35,-31,-27, 
 -23,-20,-17,-14,-11, -9, -7, -5, -3, -2, -1, 0};
static S16 meters25  [25] = // -90/25
{-90,-86,-82,-78,-75,-71,-67,-63,-60,-56,-52,-48,
 -45,-41,-37,-33,-30,-26,-22,-18,-15,-11, -7, -3, 0};

// **********************************************************************************
// Stricmp:	Compare two strings with case insensitivity.
//			Return 0 if strings are the same.	
// ********************************************************************************** 
	int   
Stricmp(char *inS1, char *inS2)
{
char s1[1000], s2[1000];
long i;

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
// Check if reached end of either string.  If so, strings are identical
if (s1[i] == '\0' && s2[i] == '\0')
	return (0);
return (1);
}	// ---- end Stricmp() ---- 

// ============================================================================
// UpdateSoundFilePaths: Check for new command line Sound Button file names, 
//                          as command line overrides programming
// ============================================================================
    void
UpdateSoundFilePaths()
{
for (long i = 0; i < kMax_SoundButtons; i++)
    {
    if (button_FilePath_Respecified[i])
        {
//printf("UpdateSoundFilePaths Button%ld: '%s' -> '%s'\n", i, button_FilePath[i], button_FilePath_Respecified[i]);
        button_FilePath[i] = button_FilePath_Respecified[i];
        }
    }
if (button_FilePath_Respecified[kSoundButton_Home])
    strcpy(MIDIFilePaths[0], button_FilePath_Respecified[kSoundButton_Home]);
}	// ---- end UpdateSoundFilePaths() ---- 

// ============================================================================
// PrintButtonMap
// ============================================================================
    void
PrintButtonMap()
{
#ifdef EMULATION
printf("\n\
QWERTY      \n\
------      \n\
Arrow Up    Volume    Increment\n\
Arrow Down  Volume    Decrement\n\
Arrow Left  Pan Left  Increment\n\
Arrow Right Pan Right Increment\n\
\n\
A Button    SOUND= %s\n\
B Button    SOUND= %s\n\
Delete      SOUND= %s\n\
Page Down   SOUND= %s\n\
\n\
Home        MIDI = %s\n\
Page Up     Quit \n",
button_FilePath[kSoundButton_A],
button_FilePath[kSoundButton_B],
MIDIFilePaths[MIDIFileIndex],
button_FilePath[kSoundButton_Left ],
button_FilePath[kSoundButton_Right]
);
// Print button map for Target platform
#else
printf("\n\
BUTTON \n\
------ \n\
Up     Volume    Increment \n\
Down   Volume    Decrement \n\
Left   Pan Left  Increment \n\
Right  Pan Right Increment \n\
\n\
A       SOUND= %s\n\
B       SOUND= %s\n\
Left    SOUND= %s\n\
Right   SOUND= %s\n\
\n\
Hint    MIDI = %s/%s\n\
Pause   Quit \n",
button_FilePath[kSoundButton_A],
button_FilePath[kSoundButton_B],
midiDir,
MIDIFilePaths[MIDIFileIndex],
button_FilePath[kSoundButton_Left ],
button_FilePath[kSoundButton_Right]
);
#endif // EMULATION
}	// ---- end PrintButtonMap() ----

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

// ============================================================================
// GetAppRsrcFolder
// ============================================================================
CPath GetAppRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir;
#else	
	return "/Didj/Data/BrioMixer/rsrc/";
#endif	// EMULATION
} // ---- end GetAppRsrcFolder() ----

// ============================================================================
// IsDirectoryValid
// ============================================================================
Boolean IsDirectoryValid( char *s )
{
struct stat filestat;
return ((0 == stat(s, &filestat)) && (filestat.st_mode & S_IFDIR));
} // ---- end IsDirectoryValid() ----

// ============================================================================
// IsFileValid
// ============================================================================
Boolean IsFileValid( char *s )
{
struct stat filestat;
Boolean valid = ((0 == stat(s, &filestat)) && (filestat.st_mode & S_IFREG));
#ifdef NEEDED
if (!valid) 
    {
printf("IsFileValid:  not valid ---------- \n");
printf("     path    ='%s'\n", s);
printf("------------------------- \n");
    }
#endif
return (valid);
} // ---- end IsFileValid() ----

// ============================================================================
// CheckFilePath
// ============================================================================
    Boolean
CheckFilePath(char *path)
{
char fullPath[1000];

CString rsrcPath = GetAppRsrcFolder();
//printf("CheckFilePath:  rsrcPath='%s'\n", rsrcPath.c_str());
strcpy(fullPath, rsrcPath.c_str());
strcat(fullPath, path);

Boolean valid = IsFileValid(fullPath);
//printf("CheckFilePath:  Valid=%d '%s'\n", valid, sPath);
#ifdef NEEDED
if (!valid) 
    {
printf("CheckFilePath: not valid ---------- \n");
printf("     rsrcPath='%s'\n", rsrcPath.c_str());
printf("     fullPath='%s'\n", fullPath);
printf("     path    ='%s'\n", path);
printf("------------------------- \n");
    }
#endif
return (valid);
}	// ---- end CheckFilePath() ---- 

// ============================================================================
// LinearS16ToDecibel  :  
// ============================================================================
    S16
LinearS16ToDecibel(S16 x)
{
// Some weird stuff to deal with abs() and log()
if (-32768 == x) 
    x = -32767;
if (0 == x)
    x = 1;
else if (x < 0)
    x = -x;

#define USE_TABLE
#ifdef USE_TABLE
return (gLinearToDecibelTable[x]);
#else
float y = (20.0*log10((1.0f/32768.0f * (float)x)));
return ((S16)y);
#endif
}	// ---- end LinearS16ToDecibel() ----

// ============================================================================
// InitLevelToDBTable:  Generate table of Decibel values indexed by unsigned 16-bit value
// ============================================================================
    void
InitLevelToDBTable(S16 *d, long length)
{
//printf("InitLevelToDBTable: Generating table size=%d \n", kTable_LinearToDecibel_Length);

float k = 1.0f/(float)kTable_LinearToDecibel_Length;
for (long i = 0; i < kTable_LinearToDecibel_Length; i++)
    {
    d[i] = (S16)(20.0*log10(k * (float)i));
//    printf("InitLevelToDBTable %6ld : %d\n", i, d[i]);
    }
}	// ---- end InitLevelToDBTable() ---- 

// ============================================================================
// LevelToStringDB  :  Convert linear level to String value
//                      Return Decibel value
// ============================================================================
    S16
LevelToStringDB(S16 x, char *outS)
{
// Short time value: convert from linear to log
S16 db = LinearS16ToDecibel(x);
if (db <= -90)
    strcpy( outS, "    ");
else
    sprintf(outS, "%4d", db);
return (db);
}	// ---- end LevelToStringDB() ---- 

// ============================================================================
// ASCIIMeterLabels:  generate a two row index of numbers for meter labels
// ============================================================================
    void 
ASCIIMeterLabels(long segments, char *outTopS, char *outBottomS, long orientation, long stride )
{
S16 *metersN;
if       (25 == segments)
    metersN = meters25_K;
else  if (22 == segments)
    metersN = meters22_K;
else  if (18 == segments)
    metersN = meters18_K;
else // if (12 == segments)
    {
    segments = 12;
    metersN = meters12_K;
    }

//stride = 1;
long i;
for (i = 0; i < segments; i++)
    {
    S16 t;
    S16 x = metersN[i];
    if (x < 0) 
        x = -x;
    t = (x/10);

// Fill for every 'stride' value, leaving others blank
// Though, ensure that first and last values are filled 
    if ((0 == i%stride) || (i == segments-1))
        {
        if (t)
            outTopS[i] = '0' + t;
        else
            outTopS[i] = ' ';
        outBottomS[i] = '0' + (x%10);
        }
    else
        {
        outTopS   [i] = ' ';
        outBottomS[i] = ' ';
        }
    }
outTopS   [i] = '\0';
outBottomS[i++] = 'd';
outBottomS[i++] = 'B';
outBottomS[i++] = '\0';
//printf("ASCIIMeterLabels: segments=%ld\n", segments);
//printf("%s\n", outTopS);
//printf("%s\n", outBottomS);
}	// ---- end ASCIIMeterLabels() ----

// ============================================================================
// DecibelToASCIIMeter
// ============================================================================
    void 
DecibelToASCIIMeter(S16 x, S16 xPeak, char *outMeterS , long segments, long orientation)
{
long i;
char onC  = '|';
char offC = '.';
char *outS = outMeterS;
S16  *metersN;
long xIndex = 0, xPeakIndex = 0;

//x     = 0;
//xPeak = -60;
//printf("------------\n");
for (i = 0; i <= segments; i++)
    *outS++ = ' ';
*outS++ = '\0';

#define kDecibelToASCIIMeter_ComputePeak
#define kDecibelToASCIIMeter_ComputeRMS

if       (25 == segments)
    metersN = meters25_K;
else  if (22 == segments)
    metersN = meters22_K;
else  if (18 == segments)
    metersN = meters18_K;
else // if (12 == segments)
    {
    segments = 12;
    metersN = meters12_K;
    }

// Compute continuous meters
#ifdef kDecibelToASCIIMeter_ComputeRMS
outS = outMeterS;
*outS = offC;
for (i = 0; i < segments; i++)
    {
    *outS = onC;
//printf("RMS : x=%3d metersN[%2ld]=%d -> '%s'\n", x, i, metersN[i], outMeterS);
    outS++;
    xIndex = i;
    if (x <= metersN[i])
        break;
    }
while (i < segments)
    {
    *outS++ = offC;
    i++;
    };
#endif // kDecibelToASCIIMeter_ComputeRMS

// Add peak meter
#ifdef kDecibelToASCIIMeter_ComputePeak
for (i = 0; i < segments; i++)
    {
//printf("Peak: x=%3d metersN[%2ld]=%d -> '%s'\n", xPeak, i, metersN[i], outMeterS);
    if (metersN[i] >= (xPeak+0))
        {
        xPeakIndex = i;
        outMeterS[i] = '+';
//printf("Peak: x=%3d metersN[%2ld]=%d -> '%s'\n", x, i, metersN[i], outMeterS);
        break;
        }
    }
#endif // kDecibelToASCIIMeter_ComputePeak

outMeterS[segments  ] = '>';
outMeterS[segments+1] = '\0';

//#define DEBUG_DecibelToASCIIMeter
#ifdef DEBUG_DecibelToASCIIMeter
//printf("DecibelToASCIIMeter: x=%3d at #%2ld xPeak=%3d at #%2ld\n", x, xIndex, xPeak, xPeakIndex);
printf("DecibelToASCIIMeter: segments%ld: (%3d,%3d) -> \n%s \n", segments, x, xPeak,outMeterS);

char outTopS[50], outBottomS[50];
ASCIIMeterLabels(segments, outTopS, outBottomS, 0, 2);
printf("%s\n", outTopS);
printf("%s\n", outBottomS);

char outNumbers[50];
for (i = 0; i < segments; i++)
    outNumbers[i] = '0' + i%10;
outNumbers[segments] = '\0';
//printf("%s\n", outNumbers);
#endif // DecibelToASCIIMeter

//exit(0);
} // ---- end DecibelToASCIIMeter() ----

// ============================================================================
// BoundS16  :   Bound to range [lower .. upper]
//
//  Warning : fails when values reach limits of Signed 16-bit
// ============================================================================
	S16
BoundS32(S16 *d, S16 lower, S16 upper)
{	     
S16 x = *d;
if      (x <= lower)
    x = lower;
else if (x >= upper)
    x = upper;
*d = x;
return (x);
}	// ---- end BoundS16() ----

// ============================================================================
// IncrementAndBoundS16  :   Add increment to value and bound to range [lower .. upper]
//
//  Warning : fails when values reach limits of Signed 32-bit, 
//            which is used for internal calculations
// ============================================================================
	S16
IncrementAndBoundS16(S16 *d, S16 inc, S16 lower, S16 upper)
{	
S32 x = *d;     
//printf("IncrementAndBoundS16: START x=%ld inc=%ld [%ld .. %ld]\n", x, inc, lower, upper);
x += inc;
if      (x <= (S32)lower)
    x = (S32)lower;
else if (x >= (S32)upper)
    x = (S32)upper;

*d = (S16)x;
return ((S16)x);
}	// ---- end IncrementAndBoundS16() ----

// ============================================================================
// BoundS32  :   bound to range [lower .. upper]
//
//  Warning : fails when values reach limits of Signed 32-bit, 
// ============================================================================
	S32
BoundS32(S32 *d, S32 lower, S32 upper)
{	     
S32 x = *d;
if      (x <= lower)
    x = lower;
else if (x >= upper)
    x = upper;
*d = x;
return (x);
}	// ---- end BoundS32() ----

// ============================================================================
// IncrementAndBoundS32  :   Add increment to value and bound to range [lower .. upper]
//
//  Warning : fails when values reach limits of Signed 32-bit, 
// ============================================================================
	S32
IncrementAndBoundS32(S32 *d, S32 inc, S32 lower, S32 upper)
{	
S32 x = *d;     
//printf("IncrementAndBoundS32: START x=%ld inc=%ld [%ld .. %ld]\n", x, inc, lower, upper);
x += inc;
if      (x <= lower)
    x = lower;
else if (x >= upper)
    x = upper;

*d = x;
return (x);
}	// ---- end IncrementAndBoundS32() ----

// ============================================================================
// TranslateSRC_ModeID:   Translate ID to English string
// ============================================================================
    char * 
TranslateSRC_ModeID(int id)
{
#define kSRC_Interpolation_Type_AddDrop	   0
#define kSRC_Interpolation_Type_Linear	   1
#define kSRC_Interpolation_Type_FIR	       2
#define kSRC_Interpolation_Type_IIR	       3
#define kSRC_Interpolation_Type_Unfiltered 4
#define kSRC_Interpolation_Type_Triangle   5
#define kSRC_Interpolation_Type_Box	       6

#define kSRC_Interpolation_Type_TestFIR	   10

switch (id)
	{
	 case kSRC_Interpolation_Type_AddDrop:
		return ("AddDrop");
	 case kSRC_Interpolation_Type_Linear:
		return ("Linear");
	 case kSRC_Interpolation_Type_FIR:
		return ("FIR");
	 case kSRC_Interpolation_Type_IIR:
		return ("IIR");
	 case kSRC_Interpolation_Type_Unfiltered:
		return ("Unfiltered");
	 case kSRC_Interpolation_Type_Triangle:
		return ("Triangle");
	 case kSRC_Interpolation_Type_Box:
		return ("Box");
	}
return ("Bogus");
}	// ---- end TranslateSRC_ModeID() ---- 

// ============================================================================
// TranslateButtonIDToName:   Translate button ID to English string
// ============================================================================
    char * 
TranslateButtonIDToName(int id)
{
switch (id)
	{
	 case kSoundButton_A:
		return ("A");
	 case kSoundButton_B:
		return ("B");
	 case kSoundButton_Left:
		return ("Left");
	 case kSoundButton_Right:
		return ("Right");
	}
return ("Bogus");
}	// ---- end TranslateButtonIDToName() ---- 

// ============================================================================
// PrintAudioState  :  
// ============================================================================
    void
PrintAudioState(tAudioState *d)
{
printf("---- PrintAudioState %p ----\n", (void*) d);
printf("    use OutDSP=%d OutEQ=%d computeLevelMeters=%d\n", d->useOutDSP, d->useOutEQ, d->computeLevelMeters);
printf("    useOutSoftClipper=%d PreGainDB=%d PostGainDB=%d\n", d->useOutSoftClipper,
d->softClipperPreGainDB, d->softClipperPostGainDB);
if (kSRC_Interpolation_Type_FIR == d->srcType)
    printf("    SRC: Type=%d '%s' filterVersion=%d\n", 
            d->srcType, TranslateSRC_ModeID(d->srcType), d->srcFilterVersion);
else
    printf("    SRC: Type=%d '%s'\n", d->srcType, TranslateSRC_ModeID(d->srcType));

printf("    SoundFile: Read =%d Path='%s'\n", d->readInSoundFile  , d->inSoundFilePath);
printf("             : Write=%d Path='%s'\n", d->writeOutSoundFile, d->outSoundFilePath);

printf("    headroomBits=%d channelGainDB=%g\n", d->headroomBits, d->channelGainDB);
printf("    masterGain = %d <%g,%g>  \n", d->masterVolume, d->masterGainf[0], d->masterGainf[1]);
printf("    speakerDSPEnabled = %d \n", d->speakerDSPEnabled);
printf("    systemSamplingFrequency = %ld \n", d->systemSamplingFrequency);
printf("    outBufferLength = %ld \n", d->outBufferLength);

printf("---- end PrintAudioState() ----\n");
}	// ---- end PrintAudioState() ---- 

// ============================================================================
// Audio and Button Listener
// ============================================================================
const tEventType kMyControllerTypes[] = { kAllButtonEvents, kAllAudioEvents };
const U32		 kDelayTime = 5000;

// ==============================================================================
// TransitionController Class
// ==============================================================================
class TransitionController : public IEventListener
{
public:
// ==============================================================================
// SetAudioState
// ==============================================================================
	void 
SetAudioState(tAudioState *d)
{
#ifdef CALL_MPI_AUDIOSTATE
audioMPI_.SAS(d);
#endif
}  // ---- end SetAudioState() ----

// ==============================================================================
// GetAudioState :
// ==============================================================================
	void 
GetAudioState(tAudioState *d)
{
#ifdef CALL_MPI_AUDIOSTATE
audioMPI_.GAS(d);
#endif
}  // ---- end GetAudioState() ----

// ==============================================================================
// SetAudioState_SoftClipper :
// ==============================================================================
	void 
SetAudioState_SoftClipper(int x)
{
//printf("SetAudioState_SoftClipper: useOutSoftClipper=%d\n", d->useOutSoftClipper);

tAudioState *d = &gAudioState;
GetAudioState(d);
d->useOutSoftClipper = x;
for (long ch = kLeft; ch <= kRight; ch++)
    {
    d->outLevels_ShortTime[ch] = 0;
    d->outLevels_LongTime [ch] = 0;
    d->outLevels_Max      [ch] = 0;
    d->outLevels_MaxCount [ch] = 0;
    }
SetAudioState(d);

if (0 != gLineIndex_SoftClipper_Enabled)
    {
    char s[100];
//    sprintf(s, "Press<Hint> SoftClipper=");
    sprintf(s, "           SoftClipper=");
    if (d->useOutSoftClipper)
        strcat(s, "On ");
    else
        strcat(s, "Off");
    _pLineBrowser->SetLine(gLineIndex_SoftClipper_Enabled, s, true);
    }
}  // ---- end SetAudioState_SoftClipper() ----

// ==============================================================================
// SetAudioState_ResetLevels
// ==============================================================================
	void 
SetAudioState_ResetLevels()
{
tAudioState *d = &gAudioState;
GetAudioState(d);

for (long ch = kLeft; ch <= kRight; ch++)
    {
    d->outLevels_ShortTime[ch] = 0;
    d->outLevels_LongTime [ch] = 0;
    d->outLevels_Max      [ch] = 0;
    d->outLevels_MaxCount [ch] = 0;
    }
SetAudioState(d);
}  // ---- end SetAudioState_ResetLevels() ----

// ==============================================================================
// GetAudioState_SpeakerEnabled
// ==============================================================================
	Boolean 
GetAudioState_SpeakerEnabled()
{
GetAudioState(&gAudioState);
//printf("GetAudioState_SpeakerEnabled: speakerDSPEnabled=%d\n", gAudioState.speakerDSPEnabled);

char s[100]; 
if (gAudioState.speakerDSPEnabled)
    sprintf(s, "%s (Speaker)", titleString);
else
    sprintf(s, "%s (HPhones)", titleString);
_pLineBrowser->SetLine(gLineIndex_Title, s, true);

return (gAudioState.speakerDSPEnabled);
}  // ---- end GetAudioState_SpeakerEnabled() ----

// ==============================================================================
// SetAudioState_SpeakerEnabled
// ==============================================================================
	void 
SetAudioState_SpeakerEnabled(Boolean x)
{
GetAudioState(&gAudioState);
gAudioState.speakerDSPEnabled = x;
SetAudioState(&gAudioState);
//printf("SetAudioState_SpeakerEnabled: speakerDSPEnabled=%d\n", x);

char s[100]; 
if (gAudioState.speakerDSPEnabled)
    sprintf(s, "%s (Speaker)", titleString);
else
    sprintf(s, "%s (HPhones)", titleString);
_pLineBrowser->SetLine(gLineIndex_Title, s, true);
}  // ---- end SetAudioState_SpeakerEnabled() ----

// ==============================================================================
// SetAllVolumes
// ==============================================================================
	void 
SetAllVolumes(long x)
{
gAudioVolume = x;

#ifdef TEST_VOLUME_MASTER
//printf("SetAllVolumes Master gAudioVolume=%ld\n", x);
audioMPI_.SetMasterVolume(x);
#endif
#ifdef TEST_VOLUME_INDIVIDUAL
//printf("SetAllVolumes Individual gAudioVolume=%ld\n", x);
for (long i = 0; i < kMax_SoundButtons; i++)
    {
    if (isPlaying_Sound_[i]) //kNoAudioID != id_Sound[i])
        audioMPI_.SetAudioVolume(id_Sound_[i], x);
    }
if (isPlaying_MIDIFile_) //kNoAudioID != id_Sound[i])
    audioMPI_.SetAudioVolume(id_MIDIPlayer_, x);
#endif

if (0 != gLineIndex_Volume)
    {
    char s[100];
#ifdef TEST_VOLUME_MASTER
    sprintf(s, "        <Up  ><Down > Master Volume=%4ld ", x);
#endif
#ifdef TEST_VOLUME_INDIVIDUAL
    sprintf(s, "        <Up  ><Down > Volume=%4ld ", x);
#endif
    _pLineBrowser->SetLine(gLineIndex_Volume, s, true);
    }
}  // ---- end SetAllVolumes() ----

// ==============================================================================
// SetAllPans
// ==============================================================================
	void 
SetAllPans(int x)
{
//printf("---- SetAllPans: gAudioPan=%d\n", x);
gAudioPan = x;

for (long i = 0; i < kMax_SoundButtons; i++)
    {
//    if (isPlaying_Sound_[i])
    if (kNoAudioID != id_Sound_[i])
        {
        audioMPI_.SetAudioPan(id_Sound_[i], x);
//printf("SetAllPans: id=%3ld : isPlaying=%d Pan=%d\n", id_Sound_[i], isPlaying_Sound_[i], x);
        }
    }
if (isPlaying_MIDIFile_) //kNoAudioID != id_Sound[i])
    audioMPI_.SetAudioPan(id_MIDIPlayer_, x);

if (0 != gLineIndex_Pan)
    {
    char s[100];
    sprintf(s, "        <Left><Right> Pan   =%4ld ", gAudioPan);
    _pLineBrowser->SetLine(gLineIndex_Pan, s, true);
    }
}  // ---- end SetAllPans() ----

// ==============================================================================
// SetSoftClipperPreGainDB
// ==============================================================================
	void 
SetSoftClipperPreGainDB(int x)
{
//printf("---- SetSoftClipperPreGainDB: =%d\n", x);
gSoftClipperPreGainDB = x;

tAudioState *d = &gAudioState;
GetAudioState(d);
d->softClipperPreGainDB = x;
// Reset levels as well
d->outLevels_Max[0] = 0;
d->outLevels_Max[1] = 0;
SetAudioState(d);

if (0 != gLineIndex_SoftClipper_PreGainDB)
    {
    char s[100];
    sprintf(s, "        <Left><Right> PreGain(dB)=%ld ", gSoftClipperPreGainDB);
    _pLineBrowser->SetLine(gLineIndex_SoftClipper_PreGainDB, s, true);
    }
}  // ---- end SetSoftClipperPreGainDB() ----

// ==============================================================================
// PlaySound:    Play sound from specified button and update Browser info
// ==============================================================================
	void 
PlaySound(int index, char *filePath)
{
long fileValid = CheckFilePath(filePath);
//{static long c=0; printf("PlaySound%ld: kButton%s '%s' valid=%ld\n", c++, TranslateButtonIDToName(index), filePath, fileValid);}

//{static long c=0; printf("PlaySound%ld: gAudioLoopCount=%ld gAudioFlags=%ld\n", c++, gAudioLoopCount, gAudioFlags);}

if (!fileValid)
    _pLineBrowser->SetLine(gLineIndex_Message, "File not found", false);
else
    _pLineBrowser->SetLine(gLineIndex_Message, " ", false);

if (!isPlaying_Sound_[index] && fileValid) 
    {
    SetAudioState_ResetLevels();
    tAudioID id = audioMPI_.StartAudio( filePath, gAudioVolume, gAudioPriority, 
                                        gAudioPan, this, gAudioLoopCount, gAudioFlags );
//printf("PlaySound:  StartAudio returned id=%ld\n", id);
    if (kNoAudioID != id)   
        {
        id_Sound_       [index] = id;
	    isPlaying_Sound_[index] = true;
//printf("PlaySound: Turn On  Button%d id=%ld \n", index, id_Sound_[index]);
        }
    else
        printf("PlaySound: unable to play sound '%s'\n", filePath);
    } 
else if (isPlaying_Sound_[index])
    {
//printf("PlaySound: Turn Off Button%d id=%ld \n", index, id_Sound_[index]);
	isPlaying_Sound_[index] = false;
	audioMPI_.StopAudio( id_Sound_[index], false );
    }
_pLineBrowser->SetLine(gLineIndex_FilePath, filePath, true);
}  // ---- end PlaySound() ----

// ==============================================================================
// SetupMeters:    
// ==============================================================================
	void 
SetupMeters()
{
long i;
// Configure 24-bit bitmaps for meters
    int meterLeft   = 130;
    int meterTop    = 200;
    int meterWidth  = 174; //(320 - meterLeft );
    int meterHeight =  42;

    _bitmapWidth  = 174;
    _bitmapHeight = 42;
    _bitmapPixels = _bitmapWidth * _bitmapHeight;
    U32 setColor[2];
    setColor[0] = LF_COLOR_GREEN;
    setColor[1] = LF_COLOR_WHITE;
    U32 meterColor32[2];
    meterColor32[0] = LF_COLOR_RED;
    meterColor32[1] = LF_COLOR_BLUE;
    for (i = 0; i < 1; i++)
        {
        long bitmapBytes = _bitmapPixels*sizeof(U32);
        _pBitmapBuffer32[i] = (U32 *) malloc(bitmapBytes);
        if (!_pBitmapBuffer32[i])
            printf("TransitionController:: Unable to allocate pBitmapBuffer=%ld bytes\n", bitmapBytes);
        else
            {
            for (long j = 0; j < _bitmapPixels; j++)
                 _pBitmapBuffer32[i][j] = meterColor32[i];
            }

        bitmapBytes = _bitmapPixels*sizeof(char)*3;
        _pBitmapBuffer24[i] = (U8 *) malloc(bitmapBytes);
        if (!_pBitmapBuffer24[i])
            printf("TransitionController:: Unable to allocate pBitmapBuffer=%ld bytes\n", bitmapBytes);
        else
            {
//#define BITMAP_FILL_WITH_COLOR
#define BITMAP_LOAD_WITH_FILE
#ifdef BITMAP_FILL_WITH_COLOR
            U8 *pBuf24 = _pBitmapBuffer24[i];
            for (long j = 0; j < _bitmapPixels; j++)
                {
                *pBuf24++ = 0x00;
                *pBuf24++ = 0x00;
                *pBuf24++ = 0xFF;
                }
#endif // BITMAP_FILL_WITH_COLOR
#ifdef BITMAP_LOAD_WITH_FILE
{
char *bitmapFileName = "LevelMetersH_175.rgb"; // 174 x 42
printf("Loading bitmap from '%s'\n", bitmapFileName);
FILE *h = fopen(bitmapFileName, "r+b");
if (!h)
    printf("Unable to open file '%s'\n", bitmapFileName);
else
    {
    long bytesRead   = 0;
    long bytesToRead = 300;
    long bytesInFile = 0;
    U8 *pRead = _pBitmapBuffer24[i];
    do {
    bytesRead = fread(pRead, sizeof(char), bytesToRead, h);
    pRead += bytesRead;
    bytesInFile += bytesRead;
    } while (bytesRead != 0);
    fclose(h);
printf("Loaded %ld pixels . Should have been %d\n", bytesInFile/3, _bitmapWidth*_bitmapHeight);
    }
}
#endif // BITMAP_LOAD_WITH_FILE
            }
        }

    for (long ch = kLeft; ch <= kLeft /*kRight*/ ; ch++)
        {
        _pMeters[ch] = new CBitStrip(&_displayMPI, _displayHandle, 
                                      meterLeft, meterTop + ch*meterHeight, meterWidth, meterHeight);
//       _pMeters[ch]->SetColor(setColor[ch]);
        _pMeters[ch]->SetBitmap(_pBitmapBuffer24[ch], 
                        _bitmapWidth, _bitmapHeight, kBitStrip_BitmapFormat_RGB_888, false);
//        _pMeters[ch]->SetBitmap(_pBitmapBuffer32[ch], 
//                        _bitmapWidth, _bitmapHeight, kBitStrip_BitmapFormat_ARGB_8888, false);
        _pMeters[ch]->Update();
        }

}  // ---- end SetupMeters() ----

// ==============================================================================
// TransitionController :    GUI handler
// ==============================================================================
	TransitionController()
		: IEventListener(kMyControllerTypes, ArrayCount(kMyControllerTypes)),
		dbgMPI_(kMyApp)
	{
    long i;
//printf("TransitionController(): START\n");

	isDone_          = false;
//	isAudioRunning_  = true;
	isAudioPaused_   = false;

    _displayHandle = SetUp2D(&_displayMPI, 0, 0, 320, 240);
    _pLineBrowser = new CLineBrowser(&_displayMPI, _displayHandle, 0, 0, 320, 240);
//SetupMeters();

    for (long i = 0; i < kMax_SoundButtons; i++)
        {
        isPlaying_Sound_[i] = false;
        id_Sound_       [i] = kNoAudioID;
        }

	isPlaying_MIDIFile_ = false;
	id_MIDIPlayer_      = kNoMidiID;

	startTime_  = kernelMPI_.GetElapsedTimeAsMSecs();
	debugLevel_ = dbgMPI_.GetDebugLevel();
	dbgMPI_.SetDebugLevel( kDbgLvlVerbose );

		    audioMPI_.SetAudioResourcePath(GetAppRsrcFolder());
//const CPath		*myAudioPath = audioMPI_.GetAudioResourcePath();
//BUSTED printf("GetAudioResourcePath()='%s'\n", (char *) myAudioPath); //->c_str());
		}
	
	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		if ( kAudioCompletedEvent == msgIn.GetEventType() )
		    {	
			const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
			const tAudioMsgAudioCompleted& data = msg.audioMsgData.audioCompleted;
            if (gTest_AudioMPI_DoneMessage)
                printf(">>>>>Audio DONE: id=%d\n", (int)data.audioID);
	
			// Change flag for completing audio resources.
			 if      (data.audioID == id_Sound_[kSoundButton_A]) 
                {
                if (gDebugAudioDoneMessage)
                   {
                    char *filePath = soundFileListPaths[gSoundFileList_Index];
//printf("--------- AudioDone: BEFO %ld: '%s'\n", gSoundFileList_Index, filePath);
                    PlaySound(kSoundButton_A, filePath);
                    if (gSoundFileList_Index >= gSoundFileList_Count)
                        gSoundFileList_Index = 0;
                    gSoundFileList_Index++;
                    }
				isPlaying_Sound_[kSoundButton_A] = false;
                }
			 else if (data.audioID == id_Sound_[kSoundButton_B]) 
				isPlaying_Sound_[kSoundButton_B] = false;
			 else if (data.audioID == id_Sound_[kSoundButton_Left]) 
				isPlaying_Sound_[kSoundButton_Left] = false;
			 else if (data.audioID == id_Sound_[kSoundButton_Right]) 
				isPlaying_Sound_[kSoundButton_Right] = false;
			 else 
				printf("Mystery audioID=%ld completed.\n", data.audioID);
		} 
		else if ( kAudioLoopEndEvent == msgIn.GetEventType() )
		{	
			const CAudioEventMessage&     msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
			const tAudioMsgLoopEnd& data = msg.audioMsgData.loopEnd;
//            if (gTest_AudioMPI_DoneMessage)
printf(">>>>>Audio Loop END: id=%d\n", (int)data.audioID);
        }
		else if ( kMidiCompletedEvent == msgIn.GetEventType() )
		{	
			const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
			const tAudioMsgMidiCompleted &data = msg.audioMsgData.midiCompleted;
printf("BrioMixer: MIDI done: id=%d, payload=%d\n", (int)data.midiPlayerID, (int)data.payload);
	
			// Change flag for completing audio resources
			 if (data.midiPlayerID == id_MIDIPlayer_) 
                {
				audioMPI_.ReleaseMidiPlayer( id_MIDIPlayer_ );		
				isPlaying_MIDIFile_ = false;
                }
			 else 
				printf("Mystery MIDI midiPlayerID=%ud completed.\n", data.midiPlayerID);
			
		} 
		else if( kButtonStateChanged == msgIn.GetEventType() )
		{	
//{static long c=0; printf("kButtonStateChanged %d\n", (int)c++);}

			const CButtonMessage& msg = dynamic_cast<const CButtonMessage&>(msgIn);
			tButtonData data = msg.GetButtonState();
			char buttonChanged = (data.buttonState & data.buttonTransition) ? '+' : '-';

			switch( data.buttonTransition )
			{ 
			// QWERTY "j"/ LF Headphone Jack Detect event
				case kHeadphoneJackDetect:
                    {
//				printf("---- %c HeadphoneJackDetect\n", buttonChanged);
                    SetAudioState_SpeakerEnabled(buttonChanged == '-');
                    }
                break;

			// QWERTY "m"/ LF Menu Button
				case kButtonMenu:
                    {
				printf("---- %c kButtonMenu\n", buttonChanged);
					if (buttonChanged == '+')
                        {
                        }
                    else
                        {
                        }
                    }
                break;

			// QWERTY "Page Up"/ LF Pause button
				case kButtonPause:
                    {
//				printf("---- %c Button Pause\n", buttonChanged);
//#define PAUSE_BUTTON_TO_PAUSE_AUDIO_SYSTEM        // OK
//#define PAUSE_BUTTON_TO_PAUSE_ALL_AUDIO_TRACKS    // OK
//#define PAUSE_BUTTON_TO_PAUSE_AUDIO_MIDI          // OK
#define PAUSE_BUTTON_TO_PAUSE_MIDI          // YES
//#define PAUSE_BUTTON_TO_QUIT

#ifdef PAUSE_BUTTON_TO_QUIT
					if (buttonChanged == '+')
                        {
                        _pLineBrowser->SetLine(gLineIndex_Message, "Quitting ...", false);
						isDone_ = true;
                    printf("Quitting ... \n");
                        }
#endif
#ifdef PAUSE_BUTTON_TO_PAUSE_MIDI
					if (buttonChanged == '+')
                        {
                        if (isAudioPaused_)
                            {
printf("isAudioPaused_=%d -> MIDI: Resuming ... \n", isAudioPaused_);
_pLineBrowser->SetLine(gLineIndex_Message, "MIDI: Resuming ...", false);
						    isAudioPaused_ = false;
                            audioMPI_.ResumeMidiFile(id_MIDIPlayer_);
                            }
                        else
                            {
printf("isAudioPaused_=%d -> MIDI: Pausing  ... \n", isAudioPaused_);
_pLineBrowser->SetLine(gLineIndex_Message, "MIDI: Pausing ...", false);
						    isAudioPaused_ = true;
                            audioMPI_.PauseMidiFile(id_MIDIPlayer_);
                            }
                        }
#endif
#ifdef PAUSE_BUTTON_TO_PAUSE_AUDIO_SYSTEM
					if (buttonChanged == '+')
                        {
                        if (isAudioPaused_)
                            {
printf("isAudioPaused_=%d -> Audio System: Resuming ... \n", isAudioPaused_);
_pLineBrowser->SetLine(gLineIndex_Message, "Audio System: Resuming ...", false);
						    isAudioPaused_ = false;
                            audioMPI_.ResumeAudioSystem();
                            }
                        else
                            {
printf("isAudioPaused_=%d -> Audio System: Pausing  ... \n", isAudioPaused_);
_pLineBrowser->SetLine(gLineIndex_Message, "Audio System: Pausing ...", false);
						    isAudioPaused_ = true;
                            audioMPI_.PauseAudioSystem();
                            }
                        }
#endif
#ifdef PAUSE_BUTTON_TO_PAUSE_ALL_AUDIO_TRACKS
					if (buttonChanged == '+')
                        {
                        if (isAudioPaused_)
                            {
                printf("isAudioPaused_=%d -> Audio Tracks: Resuming ... \n", isAudioPaused_);
						    isAudioPaused_ = false;
                            audioMPI_.ResumeAudioSystem();
                            }
                        else
                            {
                printf("isAudioPaused_=%d -> Audio Tracks: Pausing  ... \n", isAudioPaused_);
						    isAudioPaused_ = true;
                            audioMPI_.PauseAudioSystem();
                            }
                        }
#endif
                    }
					break;

			// QWERTY "Up Arrow" / LF Up Button
				case kButtonUp:
				{
//					printf("---- %c Button Up\n", change);
					if( buttonChanged == '+' ) 
                        {
                    if (gMIDITestEnabled)
                        {
                    IncrementAndBoundS32(&MIDIFileIndex, -1, 0, 127);
                    IncrementAndBoundS32(&MIDIFileIndex,  0, 0, MIDIFileCount-1);
//printf("Button Up index=%ld '%s'\n", MIDIFileIndex, MIDIFilePaths[MIDIFileIndex]);

                        char *filePath = &MIDIFilePaths[MIDIFileIndex][0];
                        _pLineBrowser->SetLine(gLineIndex_FilePath, filePath, true);
                        }

                    IncrementAndBoundS32(&gAudioVolume, kVolume_Inc, 
                                            kAudio_Volume_Min, kAudio_Volume_Max);
                    SetAllVolumes(gAudioVolume);
						}
                }
					break;

			// QWERTY "Down Arrow" / LF Down button
				case kButtonDown:
				{
//					printf("---- %c Button Down\n", ch);
					if ( buttonChanged == '+' ) 
                        {
                        if (gMIDITestEnabled)
                            {
                            IncrementAndBoundS32(&MIDIFileIndex, 1, 0, 127);
                            IncrementAndBoundS32(&MIDIFileIndex, 0, 0, MIDIFileCount-1);
    //printf("Button Down index=%ld '%s'\n", MIDIFileIndex, MIDIFilePaths[MIDIFileIndex]);
                            char *filePath = &MIDIFilePaths[MIDIFileIndex][0];
                            _pLineBrowser->SetLine(gLineIndex_FilePath, filePath, true);
                            }
                   IncrementAndBoundS32(&gAudioVolume, -kVolume_Inc, 
                                        kAudio_Volume_Min, kAudio_Volume_Max);
                   SetAllVolumes(gAudioVolume);
						}
                }
					break;

			// QWERTY "Left Arrow"/ LF Left button
				case kButtonLeft:
                {
//					printf("---- %c Button Left\n", buttonChanged);
					if ( buttonChanged == '+' ) 
                        {
                // Change MIDI File Tempo Scaling (Slow down)
                    if (gMIDITestEnabled)
                        {
                        IncrementAndBoundS32(&midiFile_TempoScale, -kMIDIFile_TempoScale_Inc, 
                                                                    kMIDIFile_TempoScale_Min, 
                                                                    kMIDIFile_TempoScale_Max);
                    // Convert to range [-N .. N]
//                        S32 tempoScale = (S32) pow(2.0, midiFile_TempoScale);
                        S32 tempoScale = midiFile_TempoScale;
//                printf(" temposcale %ld -> %ld\n", midiFile_TempoScale, tempoScale);
//					    audioMPI_.AcquireMidiPlayer( gAudioPriority, NULL, &id_MIDIPlayer_ );	
                        BoundS32(&tempoScale, kS8_Min, kS8Max);
                        midiFile_TempoScale = tempoScale;
                        audioMPI_.ChangeMidiTempo( id_MIDIPlayer_, tempoScale ); 
                        }   
                    else if (gSoftClipperTestEnabled)
                            {
                        IncrementAndBoundS32(&gSoftClipperPreGainDB, -                                               kSoftClipperPreGainDB_Inc, 
                                                kAudio_SoftClipperPreGainDB_Min,
                                                kAudio_SoftClipperPreGainDB_Max);
                           SetSoftClipperPreGainDB(gSoftClipperPreGainDB);
                            }
                        else
                            {
                        // Pan all active sounds to the left
                            IncrementAndBoundS32(&gAudioPan, -kPan_Inc, 
                                                    kAudio_Pan_Min, kAudio_Pan_Max);
                            SetAllPans(gAudioPan);
                            }
						}
                        }
					break;

			// QWERTY "Right Arrow" / LF Right button
				case kButtonRight:
                {
//					printf("---- %c Button Right\n", buttonChanged);
					if( buttonChanged == '+' ) 
                        {
               // Change MIDI File Tempo Scaling (Slow down)
                        if (gMIDITestEnabled)
                            {
                            IncrementAndBoundS32(&midiFile_TempoScale, kMIDIFile_TempoScale_Inc, 
                                                                       kMIDIFile_TempoScale_Min, 
                                                                       kMIDIFile_TempoScale_Max);
                        // Convert to range [-N .. N]
    //                        S32 tempoScale = (S32) pow(2.0, midiFile_TempoScale);
                            S32 tempoScale = midiFile_TempoScale;
    //					    audioMPI_.AcquireMidiPlayer( gAudioPriority, NULL, &idMidiPlayer_ );	
                            BoundS32(&tempoScale, kS8_Min, kS8Max);
                            midiFile_TempoScale = tempoScale;
                            audioMPI_.ChangeMidiTempo( id_MIDIPlayer_, tempoScale ); 
                            }   
                        else if (gSoftClipperTestEnabled)
                            {
                        IncrementAndBoundS32(&gSoftClipperPreGainDB, -                                               -kSoftClipperPreGainDB_Inc, 
                                                kAudio_SoftClipperPreGainDB_Min,
                                                kAudio_SoftClipperPreGainDB_Max);
                           SetSoftClipperPreGainDB(gSoftClipperPreGainDB);
                            }
                        else
                            {
               // Pan all active sounds to the right
                            IncrementAndBoundS32(&gAudioPan, kPan_Inc, 
                                                    kAudio_Pan_Min, kAudio_Pan_Max);
                            SetAllPans(gAudioPan);
                            }
 						}	
                }
					break;

			// 'a' or 'A' button
				case kButtonA:
//					printf("---- %c Button A\n", change);
					if ( buttonChanged == '+' ) 
                        {
                        char *filePath = button_FilePath[kSoundButton_A];
if (gDebugAudioDoneMessage)
    {
    if (gSoundFileList_Index >= kSoundFileList_MaxCount)
        gSoundFileList_Index = 0;
        filePath = soundFileListPaths[gSoundFileList_Index++];
    }
                        PlaySound(kSoundButton_A, button_FilePath[kSoundButton_A]);
 					    }	
					break;

			// 'b' or 'B' button
				case kButtonB:
//					printf("---- %c Button B\n", change);
					if ( buttonChanged == '+' ) 
                        {
                        PlaySound(kSoundButton_B, button_FilePath[kSoundButton_B]);
					    }
					break;

			// QWERTY "Home" / LF Hint button
				case kButtonHint:
                    {
					if ( buttonChanged == '+' ) 
                        {
                    char fullFilePath[kTest_MaxFileNameLength];
                    char *filePath = &MIDIFilePaths[MIDIFileIndex][0];
//printf("kButtonHint: filePath=%p\n", (void *) filePath);

//                    strcpy(fullFilePath, filePath);     
//printf("kButtonHint: fullFilePath='%s'\n", fullFilePath);
#ifdef EMULATION
                    strcpy(fullFilePath, midiDir);
                    strcat(fullFilePath, "/");     
#else
                    strcpy(fullFilePath, "MIDI/");
#endif
                    strcat(fullFilePath, filePath);     
//                strcpy(MIDIFilePaths[MIDIFileIndex], midiDir);
//printf("kButtonHint: fullFilePath='%s'\n", fullFilePath);
//					printf("---- %c Button Hint\n", change);

                        //  ---- Toggle SoftClipper
                            if (gSoftClipperTestEnabled)
                                {
#ifdef HINT_BUTTON_TOGGLES_SOFT_CLIPPER
                                char s[100];
                            tAudioState *d = &gAudioState;
                            GetAudioState(d);
                                d->useOutSoftClipper = !d->useOutSoftClipper;
//                                printf("useOutSoftClipper=%d\n", d->useOutSoftClipper);
                                SetAudioState_SoftClipper(d->useOutSoftClipper);
#endif // HINT_BUTTON_TOGGLES_SOFT_CLIPPER
                                }
                        // ---- Play MIDI file
                            else
                                {
                            long fileValid = CheckFilePath(fullFilePath);
{static long c=0; printf("kButtonHint %ld '%s' valid=%ld \n", c++, fullFilePath, fileValid);}
                            if (!fileValid)
                                _pLineBrowser->SetLine(gLineIndex_Message, "File not here", false);
                           else
                                _pLineBrowser->SetLine(gLineIndex_Message, " ", false);

							if (!isPlaying_MIDIFile_ && fileValid) 
                                {
                                SetAudioState_ResetLevels();
								audioMPI_.AcquireMidiPlayer( gAudioPriority, NULL, &id_MIDIPlayer_ );	
                                midiFile_TempoScale = 0;
//                                audioMPI_.ChangeMidiTempo( id_MIDIPlayer_, midiFile_TempoScale ); 

                                for (long midiCh = 0; midiCh < 16; midiCh++)
                                    {
                                    U8 d1 = kMIDI_ChannelMessage_ControlChange | midiCh;
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_AllSoundOff, 0);
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_AllNotesOff, 0);
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_ResetAllControllers, 0);
                                    }
								audioMPI_.StartMidiFile( id_MIDIPlayer_, fullFilePath, gAudioVolume,  gAudioPriority, this, gAudioLoopCount, gAudioFlags );
                            // GetAudioIDForMidiID
                                for (long midiCh = 0; midiCh < 16; midiCh++)
                                    {
                                    U8 d1 = kMIDI_ChannelMessage_ControlChange | midiCh;
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1,                          
 kMIDI_Controller_AllSoundOff, 0);
	                   audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_AllNotesOff, 0);
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_ResetAllControllers, 0);
                                    }
								isPlaying_MIDIFile_ = true;
							    } 
                            else if (isPlaying_MIDIFile_)
                                {
								isPlaying_MIDIFile_ = false;
                                for (long midiCh = 0; midiCh < 16; midiCh++)
                                    {
                                    U8 d1 = kMIDI_ChannelMessage_ControlChange | midiCh;
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_AllSoundOff, 0);
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_AllNotesOff, 0);
	                         audioMPI_.SendMidiCommand( id_MIDIPlayer_, d1, kMIDI_Controller_ResetAllControllers, 0);
                                    }
printf("BEFO StopMidiFile\n");
								audioMPI_.StopMidiFile( id_MIDIPlayer_, false );
printf("BEFO ReleaseMidiPlayer\n");
								audioMPI_.ReleaseMidiPlayer( id_MIDIPlayer_ );		
printf("AFTA ReleaseMidiPlayer\n");
							    }
                            _pLineBrowser->SetLine( gLineIndex_FilePath, filePath, true);
} // end gSoftClipperTestEnabled
					        } // if ( change == '+' )
                    } // end case kButtonHint
					break;

			// QWERTY "Delete" / LF Left  button
				case kButtonLeftShoulder:
//					printf("---- %c Button Left \n", change);
					if ( '+' ==  buttonChanged ) 
                        {
                        PlaySound(kSoundButton_Left, button_FilePath[kSoundButton_Left]);
					    }	
					break;

			// QWERTY "Page Down" / LF Right  button
				case kButtonRightShoulder:
                    {
//					printf("---- %c Button Right \n", ch);
					if ( '+' ==  buttonChanged ) 
                        {
                        PlaySound(kSoundButton_Right, button_FilePath[kSoundButton_Right]);
 					    }	
                    }
					break;
			}
		}

		return kEventStatusOKConsumed;
	}
	
	bool IsDone()
	{
		return isDone_;
	}

	bool ShouldTransition()
	{
		U32 now = kernelMPI_.GetElapsedTimeAsMSecs();
		if( (now - startTime_) > kDelayTime )
		{
			startTime_ = now;
			return true;
		}
		return false;
	}

	void ForceTransition()
	{
		startTime_ = 0;		// reset to force transition
	}

	CAudioMPI		audioMPI_;
	CDisplayMPI     _displayMPI;
    CLineBrowser    *_pLineBrowser;
    CBitStrip       *_pMeters[2];

    U8  *_pBitmapBuffer24[2];
    U32 *_pBitmapBuffer32[2];
    int _bitmapWidth;
    int _bitmapHeight;
    int _bitmapPixels;

private:
	Boolean		isDone_;
    Boolean     isAudioPaused_;
//	Boolean		isAudioRunning_;
	
	CDebugMPI		dbgMPI_;
    tDisplayHandle _displayHandle;

	tAudioID		id_Sound_[kMax_SoundButtons];
	tMidiPlayerID	id_MIDIPlayer_;

	Boolean		isPlaying_MIDIFile_;
	Boolean		isPlaying_Sound_[kMax_SoundButtons];

	mutable CKernelMPI	kernelMPI_;

	U32			startTime_;
	tDebugLevel	debugLevel_;
};

// ============================================================================
// PrintUsage:		
// ============================================================================
	void   
PrintUsage()
{
}	// ---- end PrintUsage() ---- 

// ============================================================================
// main()
// ============================================================================
    int 
main(int argc, char *argv[]) 
{
long i;
//printf("BrioMixer main: START\n");
//printf("sizeof(tAudioState)=%d kAUDIO_MAX_MSG_SIZE=%ld\n", sizeof(tAudioState), kAUDIO_MAX_MSG_SIZE);

//#define TEST_METER_DISPLAY
#ifdef TEST_METER_DISPLAY
{
long i;
char meterOutSL[50];
S16 x     = -13;
S16 xPeak = -13;
//DecibelToASCIIMeter(-13, -13, meterOutSL, meterSegments, 0);
//DecibelToASCIIMeter(-90,   0, meterOutSL, meterSegments, 0);

//#define TEST_RMS
#define TEST_PEAK
//#define TEST_RMS_PEAK

#ifdef TEST_RMS
xPeak = -90;
for (i = 0; i < 16; i += 2)
    DecibelToASCIIMeter(  -i, xPeak, meterOutSL, meterSegments, 0);
#endif
#ifdef TEST_PEAK
x = -90;
for (i = 0; i < 16; i += 2)
    DecibelToASCIIMeter(  x, -i, meterOutSL, meterSegments, 0);
#endif
#ifdef TEST_RMS_PEAK
DecibelToASCIIMeter(-10,  -2, meterOutSL, meterSegments, 0);
DecibelToASCIIMeter( -2, -10, meterOutSL, meterSegments, 0);
#endif

//printf("x=%3d xPeak=%3d-> '%s'\n", x, meterOutSL);
exit(0);
}
#endif

//#define GENERATE_METER_DISPLAY
#ifdef GENERATE_METER_DISPLAY
{
long  segments  = 25;
float segmentsF = (float) segments;
float x         = -90.0f;
float deltaF    = -x/(segmentsF-1);

for (long i = 0; i < segments; i++, x += deltaF)
    printf("%2ld : %d\n", i, (int)x);

exit(0);
}
#endif // GENERATE_METER_DISPLAY

//#define GENERATE_SQUARE_CURVE
#ifdef GENERATE_SQUARE_CURVE
{
long  segments  = 10;
float deltaF    = 1.0f/(float)(segments);
float x         = deltaF;

for (long i = 1; i < segments; i++, x += deltaF)
    printf("%2ld : %f %f\n", i, x, x*x);
printf("------\n");
x = deltaF;
for (long i = 1; i < segments; i++, x += deltaF)
    {
    float y  = (20.0*log10(x  ));
    float y2 = (20.0*log10(x*x));

    printf("%2ld : %2.2f %2.2f dB\n", i, y, y2);
    }

exit(0);
}
#endif // GENERATE_SQUARE_CURVE

//#define GENERATE_DB_VALUES
#ifdef GENERATE_DB_VALUES
{
printf("GENERATE_DB_VALUES\n");
float x;

x = 1.0f/3.0f;
printf("%g -> %g dB\n", x, 20.0*log10(x));
x = 2.0f/3.0f;
printf("%g -> %g dB\n", x, 20.0*log10(x));
x = 3.0f/2.0f;
printf("%g -> %g dB\n", x, 20.0*log10(x));
x = 3.0f;
printf("%g -> %g dB\n", x, 20.0*log10(x));

exit(0);
}
#endif // GENERATE_DB_VALUES

//#define TEST_DSPUTIL
#ifdef TEST_DSPUTIL
{
float k = 2.0f;
U32 *p = (U32 *)&k;

k = 0.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 0.5f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 1.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 1.5f;
//printf("%g = %08X\n", k, (unsigned int)*p);
k = 2.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 3.0f;
//printf("%g = %08X\n", k, (unsigned int)*p);
k = 4.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 5.0f;
//printf("%g = %08X\n", k, (unsigned int)*p);
k = 8.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 9.0f;
//printf("%g = %08X\n", k, (unsigned int)*p);
k = 16.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 32.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 64.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 128.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 256.0f;
printf("%g = %08X\n", k, (unsigned int)*p);
k = 512.0f;
printf("%g = %08X\n", k, (unsigned int)*p);

//long  wholeI = (long)k;
//float fracF  = k - (float)wholeI;
//ScaleShortsi(Q15 *in, Q15 *out, long length, U16 shift, Q15 frac)
exit(0);
}
#endif // TEST_DSPUTIL

button_FilePath[kSoundButton_A    ] = "BrioIn_Quigley.wav";
button_FilePath[kSoundButton_B    ] = "Temptation_32k_st.wav";
button_FilePath[kSoundButton_Left ] = "SINE/sine_db0_1000Hz_16k.ogg";
button_FilePath[kSoundButton_Right] = "NOISE/noise05_32k_128kb.ogg";

// Parse file arguments
for (i = 1; i < argc; i++)
    {
    char *s = argv[i];
//printf("arg[%ld]='%s'\n", i, s);
    if 	    (!Stricmp(s, "-v") || !Stricmp(s, "-verbose"))
    	verbose = true;

#ifdef DEBUG_BRIOMIXER_SOUNDFILES
    else if (!Stricmp(s, "-useInSF"))
    	readInSoundFile = true;
    else if (!Stricmp(s, "-useOutSF"))
    	writeOutSoundFile = true;

    else if (!Stricmp(s, "-isf") || !Stricmp(s, "-insf"))
        {
        strcpy(inSoundFileName , argv[++i]);
printf("inSoundFileName = '%s'\n", inSoundFileName);
    	readInSoundFile = true;
// inSoundFileName = "GoldenTestSet_16k/B_Quigley.wav;
        }
    else if (!Stricmp(s, "-osf") || !Stricmp(s, "-outsf"))
        {
        strcpy(outSoundFilePath , argv[++i]);
printf("outSoundFilePath = '%s'\n", outSoundFilePath);
    	writeOutSoundFile = true;
        }
#endif // end DEBUG_BRIOMIXER_SOUNDFILES
    else if (!Stricmp(s, "-loopcount"))
        {
        gAudioLoopCount =  atoi(argv[++i]);
//printf("gAudioLoopCount = %ld\n", gAudioLoopCount);
        }
    else if (!Stricmp(s, "-loopOn") || !Stricmp(s, "-loop"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags |= kAudioOptionsLooped;
//printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }
    else if (!Stricmp(s, "-loopOff") || !Stricmp(s, "-noloop"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags ^= !kAudioOptionsLooped;
//printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }
// ------------------ TESTS -------------------
    else if (!Stricmp(s, "-test_donemsg"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags |= kAudioOptionsDoneMsgAfterComplete;
        gTest_AudioMPI_DoneMessage = true;
printf("gTest_AudioMPI_DoneMessage=%ld\n", gTest_AudioMPI_DoneMessage);
printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }
    else if (!Stricmp(s, "-test_AudioMPI_IsPlaying") || !Stricmp(s, "-test_IsPlaying"))
        {
        gTest_AudioMPI_IsPlaying = true;
printf("gTest_AudioMPI_IsPlaying=%ld\n", gTest_AudioMPI_IsPlaying);
        }
// -------------------------
    else if (!Stricmp(s, "-doneMsgOn") || !Stricmp(s, "-doneMsg") || !Stricmp(s, "-senddoneMsg"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags |= kAudioOptionsDoneMsgAfterComplete;
printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }
    else if (!Stricmp(s, "-doneMsgOff") ||! Stricmp(s, "-nodoneMsg"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags ^= !kAudioOptionsDoneMsgAfterComplete;
//printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }
    else if (!Stricmp(s, "-loopEndMsgOn") || !Stricmp(s, "-loopEndMsg") || !Stricmp(s, "-sendLoopEndMsg"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags |= (kAudioOptionsLoopEndMsg | kAudioOptionsLooped);
//printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }
    else if (!Stricmp(s, "-loopEndMsgOff") ||! Stricmp(s, "-noLoopEndMsg"))
        {
        unsigned int oldFlags = gAudioFlags;
    	gAudioFlags ^= !kAudioOptionsLoopEndMsg;
//printf("%s: $%X -> $%X \n", s, oldFlags, (unsigned int) gAudioFlags);
        }

     else if (!Stricmp(s, "-run_sudhumidi") || !Stricmp(s, "-rsm") )
    	{
        gMIDITestEnabled = true;
printf("gMIDITestEnabled = %ld\n", gMIDITestEnabled);
      	}
     else if (!Stricmp(s, "-debug_audiodonemsg") || !Stricmp(s, "-dad") )
    	{
        gDebugAudioDoneMessage = true;
printf("gDebugAudioDoneMessage = %ld\n", gDebugAudioDoneMessage);
      	}

     else if (!Stricmp(s, "-use_levelmeters") || !Stricmp(s, "-lm") )
    	{
        gUseAudioLevelMeters = true;
printf("gUseAudioLevelMeters = %ld\n", gUseAudioLevelMeters);
      	}
     else if (!Stricmp(s, "-dontuse_levelmeters") || !Stricmp(s, "-nolm") )
    	{
        gUseAudioLevelMeters = false;
printf("gUseAudioLevelMeters = %ld\n", gUseAudioLevelMeters);
      	}
     else if (!Stricmp(s, "-test_softclipper") || !Stricmp(s, "-rsc") ||  !Stricmp(s, "-sc") || !Stricmp(s, "-sct"))
    	{
        gUseSoftClipper = true;
        gSoftClipperTestEnabled = true;
printf("gUseSoftClipper = %ld\n", gUseSoftClipper);
        gUseSoftClipper_Specified = true;
      	}
     else if (!Stricmp(s, "-sc_PreGainDB"))
    	{
        gSoftClipperPreGainDB = (S32) atof(argv[++i]);
printf("gSoftClipperPreGainDB = %ld\n", gSoftClipperPreGainDB);
        gSoftClipperPreGainDB_Specified  = true;
      	}
     else if (!Stricmp(s, "-sc_PostGainDB"))
    	{
        gSoftClipperPostGainDB = (S32) atof(argv[++i]);
printf("gSoftClipperPostGainDB = %ld\n", gSoftClipperPostGainDB);
        gSoftClipperPostGainDB_Specified = true;
      	}
     else if (!Stricmp(s, "-src_Type"))
    	{
        gSRC_Type = atoi(argv[++i]);
printf("gSRC_Type = %ld\n", gSRC_Type);
        gSRC_Type_Specified = true;
      	}
     else if (!Stricmp(s, "-src_FilterVersion"))
    	{
        gSRC_FilterVersion = atoi(argv[++i]);
printf("gSRC_FilterVersion = %ld\n", gSRC_FilterVersion);
        gSRC_FilterVersion_Specified = true;
      	}

    else if (!Stricmp(s, "-infile") || !Stricmp(s, "-i"))
        {
        gInSoundFilePath = argv[++i];
printf("gInSoundFilePath = '%s'\n", gInSoundFilePath);
        }
    else if (!Stricmp(s, "-outfile") || !Stricmp(s, "-o"))
        {
        gOutSoundFilePath = argv[++i];
printf("gOutSoundFilePath = '%s'\n", gOutSoundFilePath);
        }
    else if (!Stricmp(s, "-outfiletime") || !Stricmp(s, "-oft"))
        {
        gOutSoundFileTime = atof(argv[++i]);
printf("gOutSoundFileTime = %g Seconds\n", gOutSoundFileTime);
        }

// New sound file paths
    else if (!Stricmp(s, "-buttonFile_A") || !Stricmp(s, "-inA"))
        {
        if ((i+1) < argc && '-' != argv[i+1][0])
            {
            char *sArg = argv[++i];
            button_FilePath_Respecified[kSoundButton_A] = sArg;
    printf("%s = '%s'\n", s, sArg);
            }
        else
            printf("Well, provide file for '%s'\n", s);
        }
    else if (!Stricmp(s, "-buttonFile_B") || !Stricmp(s, "-inB"))
        {
        if ((i+1) < argc && '-' != argv[i+1][0])
            {
            char *sArg = argv[++i];
            button_FilePath_Respecified[kSoundButton_B] = sArg;
    printf("%s = '%s'\n", s, sArg);
            }
        else
            printf("Well, provide file for '%s'\n", s);
        }
    else if (!Stricmp(s, "-buttonFile_Left") || !Stricmp(s, "-inL"))
        {
        if ((i+1) < argc && '-' != argv[i+1][0])
            {
            char *sArg = argv[++i];
            button_FilePath_Respecified[kSoundButton_Left] = sArg;
    printf("%s = '%s'\n", s, sArg);
            }
        else
            printf("Well, provide file for '%s'\n", s);
        }
    else if (!Stricmp(s, "-buttonFile_Right") || !Stricmp(s, "-inR"))
        {
        if ((i+1) < argc && '-' != argv[i+1][0])
            {
            char *sArg = argv[++i];
            button_FilePath_Respecified[kSoundButton_Right] = sArg;
    printf("%s = '%s'\n", s, sArg);
            }
        else
            printf("Well, provide file for '%s'\n", s);
        }
    else if (!Stricmp(s, "-buttonFile_MIDI") || !Stricmp(s, "-inMIDI") ||
             !Stricmp(s, "-buttonFile_Home") || !Stricmp(s, "-inHome"))
        {
        if ((i+1) < argc && '-' != argv[i+1][0])
            {
            char *sArg = argv[++i];
            button_FilePath_Respecified[kSoundButton_Home] = sArg;
    printf("%s = '%s'\n", s, sArg);
            }
        else
            printf("Well, provide file for '%s'\n", s);
        }
     else if (!strcmp(s, "-h") || !strcmp(s, "-help"))
    	{
    	PrintButtonMap();
    	exit(0);
    	}
    }

#ifdef USE_LOG_FILE
hLogFile = fopen(logFilePath, "a");
if (!hLogFile)  
    printf("unable to open log file '%s'\n", logFilePath);
else
    fprintf(hLogFile, "maya is here\n");
#endif // USE_LOG_FILE

if (verbose)
{
char s[80];
s[0] = '\0';
if (gAudioFlags & kAudioOptionsLoopEndMsg)
    strcat(s, "LoopEnd=On ");
else
    strcat(s, "LoopEnd=Off ");
if (gAudioFlags & kAudioOptionsLooped)
    strcat(s, "Loop=On ");
else
    strcat(s, "Loop=Off ");
if (gAudioFlags & kAudioOptionsDoneMsgAfterComplete)
    strcat(s, "SendDone=On ");
else
    strcat(s, "SendDone=Off ");

printf("main: listener gAudioFlags=$%X '%s'\n", (unsigned int)gAudioFlags, s);
}

if (gDebugAudioDoneMessage)
    {
    for (i = 0; i < kSoundFileList_MaxCount; i++)
        soundFileListPaths[i] = NULL;
    gSoundFileList_Count = 0;
    gSoundFileList_Index = 0;

//    soundFileListPaths[gSoundFileList_Count++] = "Ogg/BreakItDown_32k_Q00_st.ogg";

    //soundFileListPaths[gSoundFileList_Count++] = "SFX/cannon.wav";
    //soundFileListPaths[gSoundFileList_Count++] = "SFX/alarm_clock.wav";
    //soundFileListPaths[gSoundFileList_Count++] = "SFX/bell.wav";
    soundFileListPaths[gSoundFileList_Count++] = "SFX/alarm_clock.wav";

    soundFileListPaths[gSoundFileList_Count++] = "Ogg/BreakItDown_32k_Q00_mono.ogg";
    //soundFileListPaths[gSoundFileList_Count++] = "Gold/C_FSP_Spell.ogg";
    //soundFileListPaths[gSoundFileList_Count++] = "Gold/C_FSP_whichmeans.ogg";
    //soundFileListPaths[gSoundFileList_Count++] = "Ogg/BreakItDown_32k_Q00_mono.ogg";
    //soundFileListPaths[gSoundFileList_Count++] = "Ogg/BreakItDown_32k_Q00_mono.ogg";

    soundFileListPaths[gSoundFileList_Count++] = "Music/Temptation_32k_mono.brio";
    soundFileListPaths[gSoundFileList_Count++] = "Music/Temptation_32k_st.brio";
    //soundFileListPaths[gSoundFileList_Count++] = "Music/Temptation_32k_mono.wav";
    //soundFileListPaths[gSoundFileList_Count++] = "Music/Temptation_32k_st.wav";

    soundFileListPaths[gSoundFileList_Count++] = "SFX/cannon.wav";
    //soundFileListPaths[gSoundFileList_Count++] = "SFX/glass_break.wav";
    //soundFileListPaths[gSoundFileList_Count++] = "SFX/tennis.wav";
    soundFileListPaths[gSoundFileList_Count++] = "SFX/frog.wav";
    for (i = 0; i < gSoundFileList_Count; i++)
        printf("soundFileListPaths[%ld] = '%s'\n", i, soundFileListPaths[i]);
    printf("gSoundFileList_Count=%ld\n", gSoundFileList_Count);
    }

// Initialize OpenGL state (lets us get button msgs in Emulation)
// (FIXME: get rid of glesXXX globals)
	BrioOpenGLConfig	config;
	glesDisplay = config.eglDisplay;
	glesSurface = config.eglSurface;
	glesContext = config.eglContext;

	// Setup button handler
	// FIXME: Order of events here is important on embedded target
	CDebugMPI				dbg(kMyApp);
//    CDisplayMPI             displayMPI;  // KEEP THIS IN THIS POSITION
	CButtonMPI				buttonMPI;
	CKernelMPI				kernelMPI;
	TransitionController	controller;

	if (kNoErr != buttonMPI.RegisterEventListener(&controller)) 
        {
		dbg.DebugOut(kDbgLvlCritical, "Failed to load button manager!\n");
		return -1;
	    }
	
dbg.SetDebugLevel(kDbgLvlCritical); // kDbgLvlCritical, kDbgLvlVerbose

//SetAppRsrcFolder(".");
CPath appRsrcFolder = GetAppRsrcFolder();
if (verbose)
    printf("main(): GetAppRsrcFolder()='%s'\n", appRsrcFolder.c_str());

MIDIFileCount = 0;
MIDIFileIndex = 0;
for (long i = 0; i < kTest_MaxMIDIFileCount; i++)
    MIDIFilePaths[i][0] = '\0';
//strcpy(&MIDIFilePaths[0][0], "play60.mid");
strcpy(&MIDIFilePaths[0][0], "Neutr_3_noDrums.mid");
//strcpy(&MIDIFilePaths[0][0], "CheeseLoop02.mid");

if (gMIDITestEnabled)
    {
    static char s[200];

// -------------------
    struct dirent *dirEnt;
//    printf("midiDir='%s'\n", midiDir);
    DIR *dirHandle = opendir(midiDir);

    if (dirHandle)
        {
        int filesFound = 0;
        while (false != (dirEnt = readdir(dirHandle)))
            {
            DIR *isADirectory = opendir(dirEnt->d_name);

         // Load up if it's not a directory
            if (dirEnt->d_name[0] != '.' && !isADirectory)// && MIDIFileCount < kTest_MaxMIDIFileCount)
                {
            // Order MIDI Files
                if (gMIDITest_OrderMIDIFiles)  
                    {
                 // Check for file number "000_" in range [0..127] and order accordingly
                    char *s = dirEnt->d_name;
                    int prefix = -1;
                    if ((s[0] >= '0' && s[0] <= '1') &&
                        (s[1] >= '0' && s[1] <= '9') &&
                        (s[2] >= '0' && s[2] <= '9') &&
                         s[3] == '_')
                        {
                        char prefixS[4];
                        prefixS[0] = s[0];
                        prefixS[1] = s[1];
                        prefixS[2] = s[2];
                        prefixS[3] = '\0';
                        prefix = atoi(prefixS);
//printf("prefix '%s' -> %d\n", prefixS, prefix);
                        if (prefix >= 0 && prefix <= 127)
                            strcpy(MIDIFilePaths[prefix], s);
                        else
                            {
printf("FAILED prefix '%s' -> %d\n", prefixS, prefix);
                            strcpy(MIDIFilePaths[MIDIFileIndex], s);    
                            }             
                        }
                    }
                else
                    {
                    strcpy(MIDIFilePaths[MIDIFileIndex], dirEnt->d_name);
                    MIDIFileIndex++;
                    if (MIDIFileIndex >= kTest_MaxMIDIFileCount)
                        MIDIFileIndex = kTest_MaxMIDIFileCount-1;
                    }
//                printf("%ld : '%s'\n", MIDIFileIndex, MIDIFilePaths[MIDIFileIndex]);
                MIDIFileCount++;
                }
            };
        closedir(dirHandle);
        MIDIFileIndex = 0;
//        for (i = 0; i < kTest_MaxMIDIFileCount; i++)
//            printf("%3ld : '%s'\n", i, MIDIFilePaths[i]);
//    printf("midiDir='%s'\n", midiDir);
//        getcwd(s, 200);
//    printf("getcwd='%s'\n", s);
        }
    else
        printf("Unable to open dirHandle midiDir='%s'\n", midiDir);
// ------------------

    strcpy(s, MIDIFilePaths[0]);
//    strcat(s, "---> Press Hint button");
    }

//    draw3dText = true;
    use2D      = true;
    use3D      = false;
    CDisplayMPI  *pDisplayMPI = &controller._displayMPI;
    CAudioMPI    *pAudioMPI   = &controller.audioMPI_;
    CLineBrowser *lineBrowser = controller._pLineBrowser;
    CBitStrip *meters[2];
    meters[kLeft ] = controller._pMeters[kLeft ];
    meters[kRight] = controller._pMeters[kRight];

// 
// ---------- Configure Mixer parameters
//
//printf("BEFORE Configure Mixer parameters\n");
{
tAudioState *d = &gAudioState;
controller.GetAudioState(d);
d->useOutEQ  = false;

// Level Meters Zero out max 
d->computeLevelMeters = gUseAudioLevelMeters;
d->outLevels_Max[kLeft ] = 0;
d->outLevels_Max[kRight] = 0;

// Set up SoftClipper
if (gUseSoftClipper_Specified)
    d->useOutSoftClipper = gUseSoftClipper;
if (gSoftClipperPreGainDB_Specified)
    d->softClipperPreGainDB  = (S16) gSoftClipperPreGainDB;
if (gSoftClipperPostGainDB_Specified)
    d->softClipperPostGainDB = (S16) gSoftClipperPostGainDB;

// Configure Sampling Rate Conversion (SRC)
if (gSRC_Type_Specified)
    d->srcType = gSRC_Type;
if (gSRC_FilterVersion_Specified)
    d->srcFilterVersion = gSRC_FilterVersion;

// Depends on activation of EQ, Soft Clipper and Level Meter
d->useOutDSP = (d->useOutEQ || d->useOutSoftClipper || d->computeLevelMeters);

if ('\0' != gInSoundFilePath[0])
    {
    d->readInSoundFile = true;
    strcpy(d->inSoundFilePath, gInSoundFilePath);
    }
if ('\0' != gOutSoundFilePath[0])
    {
    d->writeOutSoundFile = true;
    strcpy(d->outSoundFilePath, gOutSoundFilePath);
    }
#ifdef EMULATION
d->outFileBufferCount = (long)(gOutSoundFileTime*32000.0f/256.0f);
#else
d->outFileBufferCount = (long)(gOutSoundFileTime*32000.0f/512.0f);
#endif

controller.SetAudioState(d);
controller.GetAudioState(d);
if (verbose)
    PrintAudioState(d);
}

//    if (use2D)
        {
//        _lineBrowser->SetGradientDirection(k2D_GradientDirection_BottomToTop);
        lineBrowser->SetEnableGradient   (true);

// Verdana.ttf impact.ttf cour.ttf courbd.ttf
// COPRGTB.TTF COPRGTL.TTF WEDGIE__.TTF 
//        lineBrowser->SetTitleFont      ("courbd.ttf", 18, LF_COLOR_WHITE);
//        lineBrowser->SetTextFont       ("courbd.ttf", 12, LF_COLOR_WHITE);
//        lineBrowser->SetBackgroundColor(LF_COLOR_GREEN );
#ifdef EMULATION
            lineBrowser->SetBackgroundColor_Intensity(0xC0);
#else
            lineBrowser->SetBackgroundColor_Intensity(0xFF);
#endif

// 
// ---- Configure GUI according to test type
//
//printf("BEFORE Configure GUI\n");
        if      (gMIDITestEnabled)
            {
printf("Configuring MIDI Test GUI ... \n");
            gAudioFlags = kAudioOptionsDoneMsgAfterComplete;
           
        // Configure GUI
            int line = 0;
            char s[200];
            lineBrowser->SetTitleFont      ("courbd.ttf", 18, LF_COLOR_RED);
            lineBrowser->SetTextFont       ("courbd.ttf", 14, LF_COLOR_WHITE);
            lineBrowser->SetBackgroundColor(LF_COLOR_WHITE );

            gLineIndex_Title = line;
            strcpy(titleString, "MIDI File Chromatic Test");
            lineBrowser->SetLine(line++, titleString, false);

            lineBrowser->SetLine(line++, MIDIFilePaths[MIDIFileIndex], false);
            lineBrowser->SetLine(line++, " ", false);
            lineBrowser->SetLine(line++, " ", false);
            lineBrowser->SetLine(line++, "Press <Hint>        = Play File", false);
            lineBrowser->SetLine(line++, "      <Up  ><Down>  = Change File", false);
            lineBrowser->SetLine(line++, "      <Left><Right> = Change Tempo", false);
            }
        else if (gSoftClipperTestEnabled)
            {
printf("Configuring SoftClipperTest GUI ... \n");
//            gAudioFlags | = kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped;
            gAudioFlags |= kAudioOptionsLooped;
            gAudioLoopCount = kAudioRepeat_Infinite;
            button_FilePath[kSoundButton_A    ] = "BrioIn_Quigley.wav";
            button_FilePath[kSoundButton_B    ] = "Temptation_32k_st.wav";
            button_FilePath[kSoundButton_Left ] = "SINE/sine_db0_1000Hz_16k.ogg";
            button_FilePath[kSoundButton_Right] = "NOISE/noise05_32k_128kb.ogg";
//#define WAVE_ASSIGNMENT_MARATHON_TEST
//#define WAVE_ASSIGNMENT_DUAL_SINE_MUSIC_VOICES
//#define WAVE_ASSIGNMENT_ALL_SINE_M3
//#define WAVE_ASSIGNMENT_ALL_SINE_M3_OGG
//#define WAVE_ASSIGNMENT_MEDLEY
#define WAVE_ASSIGNMENT_OGG_BUTTONS

#ifdef WAVE_ASSIGNMENT_OGG_BUTTONS
            button_FilePath[kSoundButton_A    ] = "OggButtons/L_BROM_BaseUiBgm_6.ogg";
            button_FilePath[kSoundButton_B    ] = "OggButtons/L_BROM_BaseUiBgm_6.wav";
            button_FilePath[kSoundButton_Left ] = "OggButtons/L_BROM_PauseSfx_6.ogg";
            button_FilePath[kSoundButton_Right] = "OggButtons/L_BROM_PauseSfx_6.wav";
    gAudioVolume = kAudio_Volume_Default;
    gAudioPan    = kAudio_Pan_Default;
    gAudioFlags       = kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped;
    gAudioLoopCount   = kAudioRepeat_Infinite;
#endif

#ifdef WAVE_ASSIGNMENT_MARATHON_TEST
            button_FilePath[kSoundButton_A    ] = "UncompressedSample_16k.wav";
            button_FilePath[kSoundButton_B    ] = "sine_db00_1000Hz_32k_c1_zc.wav";
            button_FilePath[kSoundButton_Left ] = "WhiteNoise_15s_0dB_mono_32kHz.wav";
            button_FilePath[kSoundButton_Right] = "LeftRightCenter_32k_c2.wav";
    gAudioVolume = kAudio_Volume_Default;
//    gAudioPan    = kAudio_Pan_Default;
    gAudioFlags       = kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped;
    gAudioLoopCount   = kAudioRepeat_Infinite;
#endif

#ifdef WAVE_ASSIGNMENT_MEDLEY
button_FilePath[kSoundButton_A    ] = "sine_db00_1000Hz_32k_c1_zc.wav";
button_FilePath[kSoundButton_B    ] = "Temptation_32k_st.wav";
//button_FilePath[kSoundButton_Left ] = "SFX/bell_32k_c1.wav";
button_FilePath[kSoundButton_Left ] = "LeftRightCenter_32k_c2.wav";
//button_FilePath[kSoundButton_Right] = "UncompressedSample_16k.wav";
button_FilePath[kSoundButton_Right] = "UncompressedCut_16k_c1.wav";
#endif

#ifdef WAVE_ASSIGNMENT_DUAL_SINE_MUSIC_VOICES
// sine_db00_1000Hz_32k_c1.wav    dbM3  dbM6
button_FilePath[kSoundButton_A] = "sine_db00_1000Hz_32k_c1_zc.wav";
//button_FilePath[kSoundButton_B] = "sine_dbM3_1000Hz_32k_c2_zc.wav";
//button_FilePath[kSoundButton_B] = "Temptation_32k_st.wav";
//button_FilePath[]  = "Temptation_32k_st.wav";
button_FilePath[kSoundButton_Left ] = "UncompressedSample_16k.wav";
button_FilePath[kSoundButton_Right] = "BrioIn_Quigley.wav";
#endif
#ifdef WAVE_ASSIGNMENT_ALL_SINE_M3
button_FilePath[kSoundButton_A    ] = "sine_dbM3_1000Hz_16k_c1_zc.wav";
button_FilePath[kSoundButton_B    ] = "sine_dbM6_1000Hz_32k_c1_zc.wav";
button_FilePath[kSoundButton_Left ] = "sine_dbM3_1000Hz_32k_c1_zc.wav";
button_FilePath[kSoundButton_Right] = "sine_db00_1000Hz_32k_c1_zc.wav";
#endif
#ifdef WAVE_ASSIGNMENT_ALL_SINE_M3_OGG
button_FilePath[kSoundButton_A    ] = "sine_dbM3_1000Hz_16k_c1_zc.ogg";
button_FilePath[kSoundButton_B    ] = "sine_dbM3_1000Hz_16k_c2_zc.ogg";
button_FilePath[kSoundButton_Left ] = "sine_dbM3_1000Hz_32k_c1_zc.ogg";
button_FilePath[kSoundButton_Right] = "sine_dbM3_1000Hz_32k_c2_zc.ogg";
#endif
            UpdateSoundFilePaths();

        // Configure GUI 
            char s[200];
            int line = 0;
            lineBrowser->SetTextFont       ("courbd.ttf", 12, LF_COLOR_WHITE);
            lineBrowser->SetTitleFont      ("courbd.ttf", 18, LF_COLOR_WHITE);
            lineBrowser->SetBackgroundColor( LF_COLOR_BLUE );

            gLineIndex_Title = line;
            strcpy(titleString, "_SOFT CLIPPER TEST_");
            lineBrowser->SetLine(line++, titleString, false);

            lineBrowser->SetLine(line++, " ", false);
            lineBrowser->SetLine(line++, " ", false);
            lineBrowser->SetLine(line++, " ", false);

            sprintf(s, "Press<A>: %s", button_FilePath[kSoundButton_A]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "Press<B>: %s", button_FilePath[kSoundButton_B]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "Press<L>: %s", button_FilePath[kSoundButton_Left]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "Press<R>: %s", button_FilePath[kSoundButton_Right]);
            lineBrowser->SetLine(line++, s, false);

            gLineIndex_SoftClipper_Enabled = line++;
            controller.SetAudioState_SoftClipper(gAudioState.useOutSoftClipper);

            gLineIndex_Volume = line++;
#ifdef TEST_VOLUME_MASTER
            controller.SetAllVolumes(gAudioState.masterVolume);
#endif
#ifdef TEST_VOLUME_INDIVIDUAL
            controller.SetAllVolumes(gAudioVolume);
#endif
            gLineIndex_SoftClipper_PreGainDB = line++;
            controller.SetSoftClipperPreGainDB(gAudioState.softClipperPreGainDB);
            }
        else
            {
printf("Configuring Regular Brio Mixer GUI ... \n");
//            gAudioFlags = kAudioOptionsDoneMsgAfterComplete | kAudioOptionsLooped;
 //           gAudioFlags |= kAudioOptionsLooped;
//            gAudioLoopCount = kAudioRepeat_Infinite;

//            button_FilePath[kSoundButton_A    ] = "BrioIn_Quigley.wav";
//            button_FilePath[kSoundButton_B    ] = "Temptation_32k_st.wav";
            button_FilePath[kSoundButton_Left ] = "SFX/bell_32k_c1.wav";
            button_FilePath[kSoundButton_Right] = "SFX/frog_32k_c1.wav";
// sine_dbM3_1000Hz_08k_c1_zc sine_db00_1000Hz_08k_c1_zc.
// sine_dbM3_1000Hz_16k_c1_zc sine_db00_1000Hz_16k_c1_zc.
// sine_dbM3_1000Hz_32k_c1_zc sine_db00_1000Hz_32k_c1_zc.
// UncompressedSample_08k.wav, UncompressedSample_16k.wav, UncompressedSample_32k.wav, 
            button_FilePath[kSoundButton_A    ] = "sine_dbM3_1000Hz_08k_c1_zc.wav";
//           button_FilePath[kSoundButton_B    ] = "sine_dbM3_1000Hz_16k_c1_zc.wav";
//             button_FilePath[kSoundButton_B    ] = "sine_db00_1000Hz_32k_c1_zc.wav";
            button_FilePath[kSoundButton_B ] = "Ogg/BreakItDown_32k_Q04_st.ogg";

            UpdateSoundFilePaths();

    // Configure GUI
            int line = 0;
            char s[200];
            lineBrowser->SetTitleFont("courbd.ttf", 18, LF_COLOR_WHITE);
            lineBrowser->SetTextFont ("courbd.ttf", 12, LF_COLOR_WHITE );
            lineBrowser->SetBackgroundColor(LF_COLOR_WHITEBLUE);

            strcpy(titleString, "__ BrioMixer __ ");
            gLineIndex_Title = line;
            lineBrowser->SetLine(line++, titleString, false);

            lineBrowser->SetLine(line++, "  ", false);
            lineBrowser->SetLine(line++, "  ", false);
            lineBrowser->SetLine(line++, "  ", false);

            sprintf(s, "<A>: %s", button_FilePath[kSoundButton_A]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "<B>: %s", button_FilePath[kSoundButton_B]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "<L>: %s", button_FilePath[kSoundButton_Left]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "<R>: %s", button_FilePath[kSoundButton_Right]);
            lineBrowser->SetLine(line++, s, false);
            sprintf(s, "<Hint>: %s", MIDIFilePaths[0]);
            lineBrowser->SetLine(line++, s, false);

            gLineIndex_Volume = line++;
#ifdef TEST_VOLUME_MASTER
            controller.SetAllVolumes(gAudioState.masterVolume);
#endif
#ifdef TEST_VOLUME_INDIVIDUAL
            controller.SetAllVolumes(gAudioVolume);
#endif
            gLineIndex_Pan = line++;
            sprintf(s, "        <Left><Right> Pan   =%4ld ", gAudioPan);
            lineBrowser->SetLine(gLineIndex_Pan, s, false);
            }
        }
//  Update GUI from command line parameters *after* constructing GUI
//NOTHING HERE YET

// Keep here as sound file paths may have been programmed or respecified on command line
//PrintButtonMap();

long gAudioLevelMeterBaseLineIndex = lineBrowser->GetLineCount();
if (gUseAudioLevelMeters)
    {   
    char s2[100];
    long i = gAudioLevelMeterBaseLineIndex;
    lineBrowser->SetLine(i  , "RMS (dB)            ", false);
    lineBrowser->SetLine(i+1, "Peak(dB)            ", false);
    lineBrowser->SetLine(i+2, "Max (dB)____ ____  <%4d,%4d>", false);
#ifdef FONT_PRERENDER_GLYPHS        
    lineBrowser->SetGlyphLine(i  , false);
    lineBrowser->SetGlyphLine(i+1, false);
    lineBrowser->SetGlyphLine(i+2, false);
#endif

// Add meter dB labels
    ASCIIMeterLabels(meterSegments, outTopS, outBottomS, 0, 2);
    sprintf(s2, "                  %s", outTopS);
    lineBrowser->SetLine(gAudioLevelMeterBaseLineIndex+3, s2, false);
    sprintf(s2, "                  %s", outBottomS);
    lineBrowser->SetLine(gAudioLevelMeterBaseLineIndex+4, s2, false);

    InitLevelToDBTable(gLinearToDecibelTable, kTable_LinearToDecibel_Length);
    }
#ifdef FONT_PRERENDER_GLYPHS        
lineBrowser->GenerateGlyphLibrary();
#endif
lineBrowser->Update();
if (gLineIndex_SoftClipper_Enabled) 
    controller.SetAudioState_SoftClipper(gAudioState.useOutSoftClipper);
controller.GetAudioState_SpeakerEnabled();

//printf("Updated rate from driver=%d\n", gAudioState.); 
// 
// Main Event Loop
//
while (!controller.IsDone())
	{
// Yield to other task threads, like audio 
	kernelMPI.TaskSleep( 1000/12 ); 
	
// Test is audioPlaying
if (gTest_AudioMPI_IsPlaying)
    {
    Boolean isAny[6];
{static long c=0;    printf("---- %ld IsAnyAudioPlaying=%d \n", c++, pAudioMPI->IsAudioPlaying());}
    printf("     IsMidiFilePlaying=%d IsMidiFilePlaying(%d)=%d \n", 
    pAudioMPI->IsMidiFilePlaying(), MIDI_PLAYER_ID, pAudioMPI->IsMidiFilePlaying(MIDI_PLAYER_ID));

    for (long i = 0; i < 6; i++)
        {
    	isAny[i] = pAudioMPI->IsAudioPlaying( (tAudioID) i );  // Is specific ID playing
        printf("    ID=%ld Playing=%d \n", i, isAny[i] );
        }
    }

// Audio output level meters
if (gUseAudioLevelMeters)
    {
    tAudioState d;
    controller.GetAudioState(&d);

    S16 shortTimeDB[2], longTimeDB[2], maxDB[2];
    char shortTimeS[2][10], longTimeS[2][10], maxS[2][10];
    char meterOutS[2][50];
    char s[100];
    for (long ch = kLeft; ch <= kRight; ch++)
        {   
    // Short/Long time value: convert from linear to log
        shortTimeDB[ch] = LevelToStringDB(d.outLevels_ShortTime[ch], shortTimeS[ch]);
        longTimeDB [ch] = LevelToStringDB(d.outLevels_LongTime [ch], longTimeS [ch]);
        maxDB      [ch] = LevelToStringDB(d.outLevels_Max      [ch], maxS      [ch]);

// Place this after computing both short and long time values
        DecibelToASCIIMeter(shortTimeDB[ch], longTimeDB[ch], meterOutS[ch], meterSegments, 0);   
        }

//printf("audioState: levels <%5d, %5d> <%5d, %5d> \n", outLevels_ShortTime[kLeft ], outLevels_ShortTime[kRight]);
//printf("audioState: levels <%s, %s> <%s, %s> \n", 
//          shortTimeS[kLeft], shortTimeS[kRight], longTimeS[kLeft], longTimeS[kRight]);
// Add short time, long time and max levels (dB)
    sprintf(s , "RMS (dB)%s %s %s", shortTimeS[kLeft], shortTimeS[kRight], meterOutS[kLeft]);
    lineBrowser->SetLine(gAudioLevelMeterBaseLineIndex, s, false);
    sprintf(s, "Peak(dB)%s %s %s", longTimeS[kLeft], longTimeS[kRight], meterOutS[kRight]);
    lineBrowser->SetLine(gAudioLevelMeterBaseLineIndex+1, s, false);
    sprintf(s, "Max (dB)%s %s <%4d,%4d>", maxS[kLeft], maxS[kRight], d.outLevels_MaxCount[kLeft], d.outLevels_MaxCount[kRight]);
    lineBrowser->SetLine(gAudioLevelMeterBaseLineIndex+2, s, false);

    lineBrowser->Update();
    if (d.speakerDSPEnabled != gAudioState.speakerDSPEnabled)
        controller.SetAudioState_SpeakerEnabled(d.speakerDSPEnabled);
    if (d.useOutSoftClipper != gAudioState.useOutSoftClipper)
        controller.SetAudioState_SoftClipper(d.useOutSoftClipper);
    } 
}  // end message loop
	
buttonMPI.UnregisterEventListener(&controller);

#ifdef USE_LOG_FILE
if (hLogFile)
    fclose(hLogFile);
#endif

return 0;
}


