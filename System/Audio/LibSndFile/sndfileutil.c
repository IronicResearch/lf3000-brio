#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sndfileutil.h"

#ifndef False
#define False 0 
#endif

#ifndef True
#define True 1
#endif

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
printf("Print_SF_INFO: %ld frames , %d Hz, %d channels \n", d->frames, d->samplerate, d->channels);
//printf("               sections=%d seekable=%d \n", d->sections, d->seekable);
printf("               format = '%s' SF_FORMAT_PCM_%s\n", sMajor, sMinor);
}	// ---- end Print_SF_INFO() ----

//==============================================================================
// OpenSoundFile  : Open Sound file using libSndFile calls
//
//					Return handle of sound file
//==============================================================================
SNDFILE	*OpenSoundFile( char *path, SF_INFO *sfi, long rwType)
// rwType : SFM_READ, SFM_WRITE
{	     
SNDFILE	*sndFile = NULL;
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

//	strcpy(inSoundFilePath, path);
	//printf("OpenSoundFile samplerate=%d frames=%d ch=%d format=%X\n",
	//	   sfi->samplerate, sfi->frames, sfi->channels, sfi->format) ;
//	Print_SF_INFO(sfi);

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
		printf ("OpenSoundFile : Unsupported major file format type %X\n", (unsigned int)fileFormatMajor) ;
		return (sndFile);
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
		printf("OpenSoundFile: unsupported minor sound file format %X \n", (unsigned int) fileFormatMinor);
		return (sndFile);
		}

	printf("OpenSoundFile samplerate=%d frames=%d ch=%d width=%d\n",
		   sfi->samplerate, (int)sfi->frames, (int) sfi->channels, (int)wordWidthBits) ;
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
//	Print_SF_INFO(sfi);
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
	 sf_close(*soundFile);
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



