//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioMixer.cpp
//
// Description:
//		The class to manage the processing of audio data on an audio channel.
//
//==============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sndfile.h"

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Mixer.h>
#include <AudioOutput.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define	BRIO_MIDI_PLAYER_ID		1	// Hard code player ID for now...

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CAudioMixer implementation
//==============================================================================
CAudioMixer::CAudioMixer( U8 numChannels )
{
long i, ch;

	masterVolume_ = 100;
	numChannels_  = numChannels;

	DefaultBrioMixer(&pDSP_);
	pDSP_.channelCount = numChannels_;
	
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CAudioMixer::CAudioMixer: numChannels_=%d \n", numChannels_);

	// Allocate audio channels
printf("CAudioMixer::CAudioMixer: numChannels_=%d \n", numChannels_);
	pChannels_ = new CChannel[ numChannels_ ];

	pDebugMPI_->Assert((pChannels_ != kNull), "CAudioMixer::CAudioMixer: Mixer couldn't allocate channels!\n" );

	// Init midi player ptr
	pMidiPlayer_   = new CMidiPlayer( BRIO_MIDI_PLAYER_ID );
	
long fsRack[3];
fsRack[0] = kAudioSampleRate_Div4;
fsRack[1] = kAudioSampleRate_Div2;
fsRack[2] = kAudioSampleRate_Div1;
for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	for (ch = 0; ch < 2; ch++)
		{
		mixBinBufferPtrs_[i][ch] = new S16[2*kAudioOutBufSizeInWords * sizeof(S16)];

// Initialize sampling rate converters  
		DefaultSRC(&src_[i][ch]);
		src_[i][ch].type = kSRC_Interpolation_Type_Triangle; // Linear, Triangle, FIR
		SRC_SetInSamplingFrequency (&src_[i][ch], (float)fsRack[i]);
		SRC_SetOutSamplingFrequency(&src_[i][ch], (float)kAudioSampleRate);
		UpdateSRC(&src_[i][ch]);
		ResetSRC(&src_[i][ch]);
		}
	mixBinFilled_[i] = False;
	}

// Configure DSP engine
for (i = 0; i < pDSP_.channelCount; i++)
	{
	pChannels_[i].SetMixerChannelDataPtr(&pDSP_.channels[i]);
	}

for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
	long bufWords = 2*(kAudioOutBufSizeInWords + kSRC_Filter_MaxDelayElements);
	S16 *p = new S16[ bufWords * sizeof(S16)];
	pTmpBuffers_[i] = p;
	tmpBufOffsetPtrs_[i] = &p[2*kSRC_Filter_MaxDelayElements];
	}

readInSoundFile   = false;
writeOutSoundFile = false;
inSoundFile  = NULL;
outSoundFile = NULL;

if (writeOutSoundFile)
	{
	outSoundFileInfo.frames     = 0;		
	outSoundFileInfo.samplerate = kAudioSampleRate;
	outSoundFileInfo.channels   = 2;
	outSoundFileInfo.format     = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
	outSoundFileInfo.sections = 1;
	outSoundFileInfo.seekable = 1;

	outSoundFile = OpenSoundFile( "brioMixerOut.wav", &outSoundFileInfo, SFM_WRITE);
	}
if (readInSoundFile)
	{
// SINE08.WAV, SINE16.WAV, SINE32.WAV
// SINE08ST.WAV, SINE16ST.WAV, SINE32ST.WAV
	char *inFilePath = "/home/lfu/workspace/Brio2/Lightning/Samples/BrioAudio/Build/Lightning_emulation/SINE32.WAV";
	inSoundFile = OpenSoundFile(inFilePath, &inSoundFileInfo, SFM_READ);	if (!inSoundFile)
		{
		printf("Unable to open input file '%s'\n", inFilePath);
		readInSoundFile = False;
		}
	else
		printf("CAudioMixer::CAudioMixer: opened input file '%s'\n", inFilePath);
// inSoundFileInfo.samplerate, frames, channels
	printf("CAudioMixer::CAudioMixer: inFile: fs=%d frames=%d ch=%d \n", 
		inSoundFileInfo.samplerate, (int)inSoundFileInfo.frames, inSoundFileInfo.channels);
	}

}  // ---- end CAudioMixer::CAudioMixer() ----

//==============================================================================
// CAudioMixer::~CAudioMixer ::
//==============================================================================
CAudioMixer::~CAudioMixer()
{
long i;
printf("CAudioMixer::~CAudioMixer : HERE \n");

	// Deallocate channels
	if (pChannels_)
	{
		//fixme/dg: will this delete[] the array + the channels in the array too?
		delete [] pChannels_;
	}
	
	// Deallocate MIDI player
	if (pMidiPlayer_)
		delete pMidiPlayer_;

// Dellocate buffers 
for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
	if (pTmpBuffers_[i])
		free(pTmpBuffers_[i]);
	}

for (long i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	if (mixBinBufferPtrs_[i][0])
		free(mixBinBufferPtrs_[i][0]);
	if (mixBinBufferPtrs_[i][1])
		free(mixBinBufferPtrs_[i][1]);
	}

if (writeOutSoundFile)
	CloseSoundFile(&outSoundFile);

}  // ---- end ~CAudioMixer::CAudioMixer() ----

//==============================================================================
// CAudioMixer::FindChannelUsing
//==============================================================================
CChannel* CAudioMixer::FindChannelUsing( tAudioPriority priority )
{
	// For now, just search for a channel not in use
	for (long i = 0; i < numChannels_; i++)
	{
		if (!pChannels_[i].IsInUse()) 
			return &pChannels_[i];
	}
	
	// Reaching this point means all channels are currently in use
	return kNull;
}

//==============================================================================
// CAudioMixer::FindChannelUsing
//==============================================================================
CChannel* CAudioMixer::FindChannelUsing( tAudioID id )
{
	U8 				iChan;
	tAudioID		idFromPlayer;
	CChannel*		pChan;
	CAudioPlayer* 	pPlayer;
	
	// Loop through mixer channels, look for one that's in use and then
	// test the player's ID against the requested id.
	for (long i = 0; i < numChannels_; i++)
	{
		pChan = &pChannels_[i];
		if (pChan->IsInUse()) {
			pPlayer = pChan->GetPlayer();
			idFromPlayer = pPlayer->GetAudioID();
			if ( idFromPlayer == id )
				return &pChannels_[i];
		}
	}
	
	// Reaching this point means all no ID matched.
	return kNull;
}

//==============================================================================
// IsAnyAudioActive
//==============================================================================
Boolean CAudioMixer::IsAnyAudioActive( void )
{
	Boolean 	result = false;
	U32 iChan = 0;
	CChannel*	pChan = NULL;
	
// Loop over the number of channels
for (long i = 0; i < numChannels_; i++)
	{
	if (pChannels_[i].IsInUse())
		return (true);
	}
	
	return false;
}

//==============================================================================
// GetMixBinIndex : determine mix bin index from sampling frequency
//==============================================================================
long CAudioMixer::GetMixBinIndex( long samplingFrequency )
{
long index =  kAudioMixer_MixBin_Index_FsDiv1;

// FIXXX: currently matches numbers.  In the future, should assign mix bin
// with closest sampling frequency and do conversion
switch (samplingFrequency)
	{
	default:
	case kAudioSampleRate :
		index = kAudioMixer_MixBin_Index_FsDiv1;
	break;
	case kAudioSampleRate_Div2 :
		index = kAudioMixer_MixBin_Index_FsDiv2;
	break;
	case kAudioSampleRate_Div4 :
		index = kAudioMixer_MixBin_Index_FsDiv4;
	break;
	}

return (index);
}  // ---- end CAudioMixer::GetMixBinIndex() ----

//==============================================================================
// GetSamplingRateDivisor : Determine divisor sampling frequency, which is your operating
//				sampling frequency
//==============================================================================
long CAudioMixer::GetSamplingRateDivisor( long samplingFrequency )
{
long div =  1;

// FIXXX: currently matches numbers.  In the future, should assign mix bin
// with closest sampling frequency and do conversion
switch (samplingFrequency)
	{
	default:
	case kAudioSampleRate :
		div = 1;
	break;
	case kAudioSampleRate_Div2 :
		div = 2;
	break;
	case kAudioSampleRate_Div4 :
		div = 4;
	break;
	}

return (div);
}  // ---- end CAudioMixer::GetSamplingRateDivisor() ----

//==============================================================================
// CAudioMixer::RenderBuffer
//==============================================================================
int CAudioMixer::RenderBuffer( S16 *pOutBuff, U32 numFrames )
{
U32	i, ch;
U32 	framesRendered;
long mixBinIndex;
long channelsPerFrame = 2;

//	printf("AudioMixer::RenderBuffer -- bufPtr: 0x%x, frameCount: %u \n", (unsigned int)pOutBuff, (int)numStereoFrames );
	pDebugMPI_->Assert( ((numFrames * kAudioBytesPerStereoFrame) == kAudioOutBufSizeInBytes ),
		"AudioMixer::RenderBuffer -- frameCount doesn't match buffer size!!!\n");
	
// Clear output and temporary buffers
//ClearShorts( pOutBuff, numFrames * channelsPerFrame );
//for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
//	ClearShorts( pTmpBuffers_[i], numFrames * channelsPerFrame );

// Clear stereo mix buffers
for (i = 0; i < kAudioMixer_MixBinCount; i++)
	{
	ClearShorts( mixBinBufferPtrs_[i][0], numFrames*channelsPerFrame);
	ClearShorts( mixBinBufferPtrs_[i][1], numFrames*channelsPerFrame);
	mixBinFilled_[i] = False;
	}

//
// ---- Render each channel to mix buffer with corresponding sampling frequency
//
if (readInSoundFile)
	{
	long mixBinIndex  = GetMixBinIndex(inSoundFileInfo.samplerate);
	long framesToRead = numFrames/GetSamplingRateDivisor(inSoundFileInfo.samplerate);
	long framesRead   = 0;

// Copy stereo input to stereo mix buffer
	if (2 == inSoundFileInfo.channels)
 		framesRead = sf_readf_short(inSoundFile, mixBinBufferPtrs_[mixBinIndex][0], framesToRead);
// Replicate mono input file to both channels of stereo mix buffer
	else
		{
 		framesRead = sf_readf_short(inSoundFile, pTmpBuffers_[0], framesToRead);
		InterleaveShorts(pTmpBuffers_[0], pTmpBuffers_[0], mixBinBufferPtrs_[mixBinIndex][0], framesRead);
		}
	
	mixBinFilled_[mixBinIndex] = True;
//printf("AudioMixer::RenderBuffer: read %ld samples from WAV file \n", framesRead);
//  inSoundFileInfo.samplerate, frames, channels
	}
else
{
for (ch = 0; ch < numChannels_; ch++)
	{
	CChannel *pCh = &pChannels_[ch];

	// Render if channel is in use and not paused
	if (pCh->IsInUse() && !pCh->IsPaused())
		{
		long channelSamplingFrequency = pCh->GetSamplingFrequency();
		U32 framesToRender = (numFrames*channelSamplingFrequency)/kAudioSampleRate;
//printf("ch%ld: framesToRender=%ld for %ld Hz\n", ch, framesToRender, channelSamplingFrequency);

		framesRendered = pCh->RenderBuffer( pTmpBuffers_[0], framesToRender, False ); 

	// If player has finished, release channel.
		if ( framesRendered < framesToRender ) 
			pCh->Release( false );	// Don't suppress done msg if requested
			
	// Add output to appropriate Mix "Bin" 
		long mixBinIndex = GetMixBinIndex(channelSamplingFrequency);
// FUTURE:  convert fs/4->fs/2 with less stringent anti-aliasing filter and let fs/2 mix bin's conversion do fs/2->fs
		S16* mixBinP = mixBinBufferPtrs_[mixBinIndex][0];
		if (mixBinFilled_[mixBinIndex])
			Add2_Shortsi(pTmpBuffers_[0], mixBinP, mixBinP, framesToRender*channelsPerFrame); 
		else
			CopyShorts(pTmpBuffers_[0], mixBinP, framesToRender*channelsPerFrame);
		mixBinFilled_[mixBinIndex] = True;
		}
	}

// MIDI player renders to fs/2 output buffer if it has been activated by client.
if ( pMidiPlayer_->IsActive() )
	{
//		long rateDivisor = 2;
	long midiFrames = (numFrames/2);
	long mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
	short *mixBuf = mixBinBufferPtrs_[mixBinIndex ][0];

	framesRendered = pMidiPlayer_->RenderBuffer( pTmpBuffers_[0], midiFrames, False ); 
	Add2_Shortsi(pTmpBuffers_[0], mixBuf, mixBuf, midiFrames*channelsPerFrame);
	mixBinFilled_[mixBinIndex] = True;
	}
}

// 
// Combine Mix buffers to output, converting sampling rate if necessary
//
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv1;
if (mixBinFilled_[mixBinIndex])
	CopyShorts(mixBinBufferPtrs_[mixBinIndex][0], pOutBuff, numFrames*channelsPerFrame);
else
	ClearShorts( pOutBuff, numFrames * channelsPerFrame );

// Deinterleave and process each MixBuffer separately
mixBinIndex = kAudioMixer_MixBin_Index_FsDiv4;
// FIXXXX: mystery.  Offset Ptrs don't work !!!
short **tPs = pTmpBuffers_; // pTmpBuffers_, tmpBufOffsetPtrs_
if (mixBinFilled_[mixBinIndex])
	{
	DeinterleaveShorts(mixBinBufferPtrs_[mixBinIndex][0], tPs[2], tPs[3], numFrames/4);
	RunSRC(tPs[2], tPs[0], numFrames/4, numFrames, &src_[mixBinIndex][0]);
	RunSRC(tPs[3], tPs[1], numFrames/4, numFrames, &src_[mixBinIndex][1]);
	InterleaveShorts(tPs[0], tPs[1], tPs[6], numFrames);
	Add2_Shortsi(tPs[6], pOutBuff, pOutBuff, numFrames*channelsPerFrame);
//CopyShorts(tPs[6], pOutBuff, numFrames*channelsPerFrame);
	}

mixBinIndex = kAudioMixer_MixBin_Index_FsDiv2;
if (mixBinFilled_[mixBinIndex])
	{
	DeinterleaveShorts(mixBinBufferPtrs_[mixBinIndex][0], tPs[4], tPs[5], numFrames/2);
	RunSRC(tPs[4], tPs[0], numFrames/2, numFrames, &src_[mixBinIndex][0]);
	RunSRC(tPs[5], tPs[1], numFrames/2, numFrames, &src_[mixBinIndex][1]);
	InterleaveShorts(tPs[0], tPs[1], tPs[6], numFrames);
	Add2_Shortsi(tPs[6], pOutBuff, pOutBuff, numFrames*channelsPerFrame);
//CopyShorts(tPs[6], pOutBuff, numFrames*channelsPerFrame);
	}
		
#if 0
// fixme/rdg:  needs CAudioEffectsProcessor
// If at least one audio channel is playing, Process global audio effects
if (numPlaying && (gAudioContext->pAudioEffects != kNull))
	gAudioContext->pAudioEffects->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuff );
#endif

// Scale output by master volume
#define ORIG_MIXER_MASTER_VOLUME
#ifdef ORIG_MIXER_MASTER_VOLUME
	for (i = 0; i < numFrames*channelsPerFrame; i++)
	{
// NOTE: volume here is interpreted as a linear value
		pOutBuff[i] = (S16)((pOutBuff[i] * (int)masterVolume_) >> 7); // fixme; 
	} 
#elif defined (
// Initial integration code:  scale stereo/interleaved buffer 
ScaleShortsf(pOutBuff, pOutBuff, length, d->outGainf[0]);
#endif

// Debug:  write output of mixer to sound file
if (writeOutSoundFile)
{
   long framesWritten = sf_writef_short(outSoundFile, pOutBuff, numFrames);
framesWritten = 0;
}

//printf("AudioMixer::RenderBuffer -- end 222 \n");
	return kNoErr;
} // ---- end AudioMixer::RenderBuffer() ----

//==============================================================================
//==============================================================================
int CAudioMixer::WrapperToCallRenderBuffer( S16 *pOut,  unsigned long numStereoFrames, void* pToObject  )
{
	// Cast void ptr to a this ptr:
	CAudioMixer* mySelf = (CAudioMixer*)pToObject;
	
	// Call member function to get a buffer full of stereo data
	return mySelf->RenderBuffer( pOut, numStereoFrames );
}

//==============================================================================
// OpenSoundFile  : Open Sound file using libSndFile calls
//
//					Return handle of sound file
//==============================================================================
SNDFILE	*CAudioMixer::OpenSoundFile( char *path, SF_INFO *sfi, long rwType)
// rwType : SFM_READ, SFM_WRITE
{	     
SNDFILE	*sndFile;
// printf("OpenSoundFile: start  '%s'\n", path);
   sndFile = NULL;

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
int CAudioMixer::CloseSoundFile( SNDFILE **soundFile )
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

LF_END_BRIO_NAMESPACE()
// EOF	
