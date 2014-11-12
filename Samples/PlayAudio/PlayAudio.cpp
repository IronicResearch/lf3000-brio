//============================================================================
// PlayAudio : Play any audio file from the command line. (No MIDI)
//
//              Written by Darren Gibbs and Gints Klimanis, 2007
//============================================================================

#define	PROGRAM_VERSION_STRING	"2.0"
#define	PROGRAM_DATE_STRING		"10/22/08"

// Includes
#include <assert.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EmulationConfig.h>
#include <string.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

CAudioMPI*		pAudioMPI  = NULL;
CKernelMPI*		pKernelMPI = NULL;

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif


//============================================================================
// GetAppRsrcFolder
//============================================================================
inline CPath GetAppRsrcFolder( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir;
#else
	return "./";
#endif
}

// Prototypes
void		PrintUsage( void );
Boolean	ParseCommandLine ( int argc, char **argv );

// Globals
char *gProgramName = NULL;
long verbose = false;
long supressPlayDoneMessage = false;

#define kMax_AudioFiles 19
#define kMax_InputFiles (kMax_AudioFiles)
#define kMax_InputFile_NameLength 500
char	gInputFileNames[kMax_InputFiles][kMax_InputFile_NameLength];
long    inFileCount      = 0;
long    inAudioFileCount = 0;

#define kFileType_Audio   0
#define kFileType_Unknown 2
long	inFileType[kMax_InputFiles];
tAudioID playIDs[kMax_InputFiles];

U8		gVolumes[kMax_InputFiles];
S8		gPans   [kMax_InputFiles];
long    gPanIndex    = 0;
long    gVolumeIndex = 0;
long    gUnisonPlay  = false;

//	kAudioOptionsNoDoneMsg				= 0x00,	// No done message
//	kAudioOptionsDoneMsgAfterComplete	= 0x01,	// Done message after audio is complete 
//	kAudioOptionsLooped					= 0x02	// Loop audio sample playback
int		gAudioOptions = 0;		
int		gLoopCount	  = 0;		

#define kAudioPriority_Default 1
//#define kAudioPriority_Min 1
//#define kAudioPriority_Max 1
tAudioPriority	gAudioPriority = kAudioPriority_Default;  // Range [1 .. ?]

// *****************************************************************************
// Stricmp:	Compare two strings with case insensitivity.
//			Return 0 if strings are the same.	
// ***************************************************************************** 
int Stricmp(char *inS1, const char *inS2)
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
	// Check if reached end of either string.  If both, strings are the same
	if (s1[i] == '\0' && s2[i] == '\0')
		return (0);
	return (1);
}	// ---- end Stricmp() ---- 

// =============================================================================
// PrintUsage:
// =============================================================================
void PrintUsage( void )
{
	printf("%s: Plays audio files using Brio Audio.\n", gProgramName);
	printf("Usage: -i <filename>\n");
	printf("Version %s, %s\n", PROGRAM_VERSION_STRING, PROGRAM_DATE_STRING );
	printf( "    -i  up to %d file names of file TYPE {Ogg, WAV/AIFF, Brio .raw}\n", kMax_InputFiles);
	printf( "    -volume [   0 - 100]\n" );
	printf( "    -pan    [-100 - 100]\n" );
	printf( "NOTE: use additional volume and pan args for each file \n");
	printf( "    -loop       [1 ..N] iterations\n" );
	printf( "    -unison     Play all files at once\n" );
	printf( "    -sendDone   Send Audio Done message when file completed\n" );
	printf( "    -pri        Audio priority (Default=%d)\n", kAudioPriority_Default);
	printf( " \n");
	printf( "    -h  help : Print usage\n");
	printf( "    -v  verbose : show operational messages\n");

}   // ---- end PrintUsage() ----

// =============================================================================
// main:
// =============================================================================
int	main( int argc, char* argv[] )
{
	long	i;
	CPath	fileExtension;
	int		strIndex;
	
	gProgramName = argv[0];

	if ( argc < 2 ) 
		{
		PrintUsage();
		exit (-1);
		}
	
	for (i = 0; i < kMax_InputFiles; i++)
		{
		gInputFileNames[i][0] = '\0';	 
		inFileType[i] = kFileType_Unknown;
		gVolumes[i] = 100;
		gPans   [i] = 0;
		}
	
	// Parse command line and check results 
	if ( !ParseCommandLine(argc, argv)) 
		{
		PrintUsage();
		exit (-1);
		}
	
	pAudioMPI  = new CAudioMPI();
	pKernelMPI = new CKernelMPI();
	
	//if (verbose)
	//    suppressPlayDoneMessage = false;
	
	if ( inFileCount < 1 ) 
		{
		PrintUsage();
		printf( "No input file(s) specified.  Exiting...\n" );
		exit (-1);
		}
	
	// Categorize input files
	// NOTE;  this just checks file extensions.  Someday, should qualify file type
	for (i = 0; i < inFileCount /*kMax_InputFiles */; i++)
		{
		// printf("InFile%ld: '%s'\n", i, &gInputFileNames[i][0]);	 
	
		CPath	fileName(gInputFileNames[i]);
	
		// Extract file extension.
		strIndex       = fileName.rfind('.', fileName.size());
		fileExtension  = fileName.substr(strIndex + 1, strIndex+3);
		char *sExt = (char *) fileExtension.c_str();
		//	printf( "PlayAudio -- Found extension: %s\n", sExt) );
	
		// Case independent file names
		if ( (!Stricmp( sExt, "ogg" )) || (!Stricmp( sExt, "aogg" )) ||
				// Misinterpretation of RAW in Brio spec :
				// In the real world, RAW is headerless audio data file.
				// In the Brio world, RAW is linear PCM audio data with a Brio Audio header
			 (!Stricmp( sExt, "raw" )) || (!Stricmp( sExt, "brio")) ||
				// WAV and AIFF not in Brio specification, but should be !!
			 (!Stricmp( sExt, "avi" )) ||
			 (!Stricmp( sExt, "aiff" )) || (!Stricmp( sExt, "aif" )) ||
			 (!Stricmp( sExt, "wav")) )
			inFileType[i] = kFileType_Audio;
		}

	//
	// ---- Launch file loop
	//
	//printf("gAudioOptions=$%X\n", gAudioOptions);
	for (i = 0; i < inFileCount /* kMax_InputFiles */; i++)
		{
		long loopAudio = (0 != (gAudioOptions & kAudioOptionsLooped));

		CPath	filename(gInputFileNames[i]);
		if (kFileType_Audio == inFileType[i])
			{
			if (verbose)
				printf( "Play%ld Audio '%s' vol=%d, pan=%d loop=%d #%d\n", 
						i, gInputFileNames[i], (int)gVolumes[i], (int)gPans[i], 
						(int) loopAudio, gLoopCount );
			playIDs[i] = pAudioMPI->StartAudio( filename, gVolumes[i], gAudioPriority, gPans[i],  kNull, gLoopCount, gAudioOptions );		
			//if (playIDs[i] == kNoAudioID)
				//printf("Unable to start%ld '%s' \n", i, gInputFileNames[i]);
			//printf("playIDs[%ld]=%ld '%s'\n", i, playIDs[i], gInputFileNames[i]);
			} 

		// If not unisonPlay, wait for file to finish before launching another
		if (!gUnisonPlay)
			{
			// Yield to other task threads, like audio
			while ( pAudioMPI->IsAudioPlaying() ) 
				pKernelMPI->TaskSleep( 100 ); 
			}
		}

	// Wait for all play tasks to complete
	if (gUnisonPlay)
		{
		while ( pAudioMPI->IsAudioPlaying() ) 
			pKernelMPI->TaskSleep( 100 ); 
		}

	//printf("before delete MPIs\n");
	delete pAudioMPI;
	delete pKernelMPI;
	
	// Successful completion 	
	return 0;
}   // ---- end main() ----

// =============================================================================
// ParseCommandLine:
// =============================================================================
Boolean	ParseCommandLine ( int argc, char **argv )
{
	long j;
	for (long i = 1; i < argc; i++)
	{
		char *s = argv[i];
		if ( !Stricmp( s, "-i" ) ) 
		{
			long noMoreInputFiles = false;

			for (j = 0; j < kMax_InputFiles && (i < argc-1) & !noMoreInputFiles; j++)
			{
				s = argv[++i];
				if (s[0] == '-')
				{
					i--;    
					noMoreInputFiles = true;
				}
				else if (inFileCount < kMax_InputFiles)
				{
					//printf("argv[%ld]='%s'\n", i, s);
					strcpy(&gInputFileNames[inFileCount][0], s);
					//printf("gInputFileNames[%ld]='%s'\n", inFileCount, gInputFileNames[inFileCount]);
					inFileCount++;
				}
			}
		}
	 	else if (!Stricmp(s, "-l") || !Stricmp(s, "-loop")) 
		{
			gAudioOptions |= kAudioOptionsLooped;
			gLoopCount = (int)atoi(argv[++i]);
			//printf("gAudioOptions=$%X gLoopCount=%d loop=%d\n", gAudioOptions, (int)gLoopCount,
			//          (0 != (gAudioOptions & kAudioOptionsLooped));
		} 
	 	else if (!Stricmp(s, "-sendDoneMsg")) 
		{
			gAudioOptions |= kAudioOptionsDoneMsgAfterComplete;
			//printf("gAudioOptions=$%X sendDone=%d\n", gAudioOptions, 
			//          (0 != (gAudioOptions & kAudioOptionsDoneMsgAfterComplete)));
		} 
		else if (!Stricmp(s, "-vol") || !Stricmp(s, "-volume") ) 
		{
			gVolumes[gVolumeIndex] = (U8)atoi(argv[++i]);
			//printf("gVolumes[%ld]=%d\n", gVolumeIndex, (int)gVolumes[gVolumeIndex]);
			gVolumeIndex++;
		} 
		else if (!Stricmp( s, "-p") || !Stricmp( s, "-pan")) 
		{
			gPans[gPanIndex] = (S8)atoi(argv[++i]);
			//printf("gPans[%ld]=%d\n", gPanIndex, (int)gPans[gPanIndex]);
			gPanIndex++;
		} 
		else if (!Stricmp( s, "-pri") || !Stricmp( s, "-priority")) 
		{
			gAudioPriority = atoi(argv[++i]);
			//printf("gAudioPriority=%d\n", gAudioPriority);
		} 
		else if (!Stricmp( s, "-u") || !Stricmp( s, "-unison") || !Stricmp( s, "-unisonplay")) 
		{
			gUnisonPlay = true;
			//printf("unisonPlay=%ld\n", unisonPlay);
		} 
		else if (!Stricmp( s, "-h") || !Stricmp( s, "-help")) 
		{
			PrintUsage();
			exit(-1);
		} 
		else if (!Stricmp( s, "-v") || !Stricmp( s, "-verbose")) 
		{	
			verbose = true;
		}
		else
		{
			// Unknown flag encountered
			printf("Unknown option '%s'\n", argv[i]);
			return (false);
		}
	}
	
	return (true);
}   // ---- end ParseCommandLine() ----

