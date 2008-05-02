#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sndfileutil.h"

//============================================================================
// Print_SF_INFO  :   Print SF_INFO data structure
//============================================================================
	void
Print_SF_INFO(SF_INFO *d)
{	     

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

printf("Print_SF_INFO: %lld frames , %d Hz, %d channels \n", d->frames, d->samplerate, d->channels);
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

//
// ---------------------- Open input sound file
//
if (SFM_READ == rwType)
	{
// Initialize FILE in structure
	memset (sfi, 0, sizeof(SF_INFO)) ;

	printf("%s.%d: gonna load '%s' \n", __FUNCTION__, __LINE__, path);
	if (!(sndFile = sf_open (path, rwType, sfi)))
		{	
		printf ("%s.%d: Not able to open input file '%s'\n",
			__FUNCTION__, __LINE__, path) ;
		return (NULL);
		} 

	printf("OpenSoundFile samplerate=%d frames=%lld ch=%d format=%d\n",
		   sfi->samplerate, sfi->frames, sfi->channels, sfi->format) ;
	Print_SF_INFO(sfi);

	// Reject files that aren't WAV or AIFF
	long fileFormatMajor =  sfi->format & SF_FORMAT_TYPEMASK;
	long fileFormatMinor =  sfi->format & SF_FORMAT_SUBMASK  ;
	if (SF_FORMAT_WAV == fileFormatMajor)
	{
		printf ("OpenSoundFile : This is a WAV file \n") ;
	}
	else if (SF_FORMAT_AIFF == fileFormatMajor)
	{
		printf ("OpenSoundFile : This is a AIFF file \n") ;
	}
	else
	{
		printf ("OpenSoundFile : Unsupported major file format type %X\n", (unsigned int)fileFormatMajor) ;
		return (sndFile);
	}

	{
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
		printf("OpenSoundFile: unsupported minor sound file format %X \n", (unsigned int) fileFormatMinor);
		return (sndFile);
		}

	printf("OpenSoundFile fs=%d Hz frames=%d bits=%d ch=%d (%.3f Seconds)\n",
		sfi->samplerate, (int)sfi->frames, (int)wordWidthBits,
		(int) sfi->channels, ((float)sfi->frames)/(float) sfi->samplerate);
	}
	}
//
// ---------------------- Open output sound file
//
else if (SFM_WRITE == rwType)
	{
	if (!(sndFile = sf_open (path, rwType, sfi)))
		{	
		printf ("OpenSoundFile : Not able to open for writing '%s'\n", path) ;
		return (NULL);
		} 
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
 
 if (*soundFile)
	 {
	 sf_close(*soundFile);
	 *soundFile = NULL;
	 return (true);
	 }
 	
	   	   	
return (false);
}		// ---- end CloseSoundFile() ----

//==============================================================================
// RewindSoundFile  :   Seek to beginning of file
//
//        NOTE: Short term support for audio file playback
//
//					Return Boolean success
//==============================================================================
int RewindSoundFile( SNDFILE *soundFile )
{	     

//#define NEED_TO_USE_REWIND_CRUDE_SEEK
#ifdef NEED_TO_USE_REWIND_CRUDE_SEEK
// Quick hack for initial implementation
CloseSoundFile(soundFile);
*soundFile = OpenSoundFile(path, sfi, SFM_READ);
if (!*soundFile)
    {
 	printf("RewindSoundFile: unable to reopen '%s'\n", path);
	return (false);
    }
return (true);
#else
return (sf_seek(soundFile, 0, SEEK_SET) == 0);
#endif
}		// ---- end RewindSoundFile() ----



