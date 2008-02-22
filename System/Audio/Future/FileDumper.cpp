#include <IAudioEffect.h>

// Note.  This is pseudo code.  It does not work yet.

// ==============================================================================
// CAudioMixer::OpenOutSoundFile :
//
//								Return Boolean success
// ==============================================================================
static int OpenOutSoundFile(char *path)
{
	outSoundFileInfo_.frames	 = 0;		
	outSoundFileInfo_.samplerate = (long) samplingFrequency_;
	outSoundFileInfo_.channels	 = 2;
	outSoundFileInfo_.format	 = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
	outSoundFileInfo_.sections	 = 1;
	outSoundFileInfo_.seekable	 = 1;

	outSoundFile_ = OpenSoundFile(path , &outSoundFileInfo_, SFM_WRITE);
	if (!outSoundFile_)
	{
		printf("CAudioMixer::OpenOutSoundFile: unable to open '%s' \n", path);
		return (false);
	}
	strcpy(audioState_.outSoundFilePath, path);
	printf("CAudioMixer::OpenOutSoundFile: opened out file '%s'\n",
		   audioState_.outSoundFilePath);
	return (true);
}

FileDumper::FileDumper():IAudioEffect()
{
	d->outFileBufferCount = 1000;
	memset(d->outSoundFilePath, '\0', kAudioState_MaxFileNameLength);
	strcpy(d->outSoundFilePath, "BrioOut.wav");

	// Open WAV file to write output of mixer
	if (audioState_.writeOutSoundFile && !outSoundFile_)
		OpenOutSoundFile(audioState_.outSoundFilePath);
}

FileDumper::~FileDumper()
{
	if (outSoundFile_)
	{
		CloseSoundFile(&outSoundFile_);
		outSoundFile_ = NULL;
	}
}

FileDumper::ProcessEffect(U32 numSamples, S16 *pStereoBuffer)
{
	static long outBufferCounter = 0; 

	if (inSoundFileDone_ || outBufferCounter++ >= audioState_.outFileBufferCount)
	{
		printf("Closing outSoundFile '%s'\n", audioState_.outSoundFilePath);
		CloseSoundFile(&outSoundFile_);
		outSoundFile_	   = NULL;
		audioState_.writeOutSoundFile = false;
	}
	else if (outSoundFile_)
	{
		static long totalFramesWritten = 0;
		long framesWritten = sf_writef_short(outSoundFile_, pOut, numFrames);
		totalFramesWritten += framesWritten;
	}
}
