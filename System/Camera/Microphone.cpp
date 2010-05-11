/*
 * Brio wrapper of alsa-lib for capturing audio from the camera/microphone widget.
 * The capture implementation uses ALSA's asynchronous interface and memory-mapped
 * transfers.
 *
 * Based on the example here:
 * http://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_8c-example.html
 *
 * This implementation uses a pipe (FIFO) to decouple reading from the microphone and
 * writing to a (avi/wav) file.  ALSA provides microphone data asynchronously,
 * and potentially in an interrupt context where blocking is forbidden.
 */
#include <KernelMPI.h>
#include <CameraPriv.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>


LF_BEGIN_BRIO_NAMESPACE()

static const unsigned int MIC_RATE		= 16000;	/* desired sampling rate */
static const unsigned int MIC_CHANS		= 1;		/* desired channels */
static const snd_pcm_format_t MIC_FMT	= SND_PCM_FORMAT_S16_LE;	/* desired format */

static const char *cap_name = "plughw:1,0";
/* Opening hw:1,0 would provide raw access to the microphone hardware and therefore no
 * automatic conversion.  Opening plughw:1,0 uses the alsa plug plugin to open hw:1,0
 * as a slave device, but allows rate/channel/format conversion.
 *
 * hw:* is defined in /usr/share/alsa/alsa.conf under pcm.hw, and likewise for plughw
 * under pcm.plughw.  These are standard alsa devices supplied by the default alsa.conf.
 *
 * alsa.conf could be modified to include an entry like this:
 * >pcm.ratehw {
 * >	... (args collection, same as plughw)
 * >	type rate
 * >	slave.pcm {
 * >		type hw
 * >		card $CARD
 * >		device $DEV
 * >		subdevice $SUBDEV
 * >	}
 * >}
 * which would provide only rate conversion, and raw access to the hardware's native
 * channels and format.  The camera widget supplies 16-bit, mono audio at 48KHz, and
 * the camera API supplies 16-bit, mono audio at 16KHz.  This means the rate plugin
 * would be sufficient, and the functionality provided by the plug plugin is
 * superfluous.  However, this implementation uses the plug plugin so as not to deviate
 * from the standard alsa.conf.
 *
 * See also:
 * http://www.alsa-project.org/main/index.php/ALSAresampler
 * http://article.gmane.org/gmane.linux.alsa.user/25063
 */

static const U32 DRAIN_SIZE = 32768;	/* 1 second of 16-bit, 16 KHz mono */

static void RecordCallback(snd_async_handler_t *ahandler);

static int set_hw_params(struct tMicrophoneContext *pCtx);
static int set_sw_params(struct tMicrophoneContext *pCtx);

//==============================================================================
// Defines
//==============================================================================
#define DATA_LOCK dbg_.Assert((kNoErr == kernel_.LockMutex(micCtx_.dlock)),\
									  "Couldn't lock mutex.\n")

#define DATA_UNLOCK dbg_.Assert((kNoErr == kernel_.UnlockMutex(micCtx_.dlock)),\
										"Couldn't unlock mutex.\n");

//============================================================================
// Local variables
//============================================================================
namespace
{
	CPath				apath = "";

#if USE_PROFILE
	// Profile vars
#endif
}

//----------------------------------------------------------------------------
tErrType CCameraModule::SetCameraAudioPath(const CPath &path)
{
	DATA_LOCK;

	apath = path;
	if (apath.length() > 1 && apath.at(apath.length()-1) != '/')
		apath += '/';

	DATA_UNLOCK;

	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CCameraModule::GetCameraAudioPath()
{
	CPath *path;

	DATA_LOCK;

	path = &apath;

	DATA_UNLOCK;

	return path;
}

//----------------------------------------------------------------------------
tErrType CCameraModule::InitMicInt()
{
	tErrType	kErr = kUnspecifiedErr;
	int 		err;

	micCtx_.poll_buf	= NULL;
	micCtx_.pcm_handle	= NULL;
	micCtx_.fd[0]		= -1;
	micCtx_.fd[1]		= -1;

	snd_pcm_hw_params_alloca(&micCtx_.hwparams);
	snd_pcm_sw_params_alloca(&micCtx_.swparams);

	do
	{
		/*
		 * Pipe capacity is 64k.  This is a 2 second buffer @ 16-bit, mono 16 KHz.
		 */
		if(( err = pipe(micCtx_.fd)) != 0)
		{
			continue;
		}

		/*
		 * The write FD must be non-blocking to allow instant dumping of PCM data
		 * from the mic.  The read FD can use normal blocking access.  select() is
		 * used to poll for available data.
		 */
		if(( err = fcntl(micCtx_.fd[1], F_SETFL, O_NONBLOCK)) != 0)
		{
			continue;
		}

		/*
		 * If the pipe fills up, the mic audio will overrun and data will be lost.
		 * A polling loop periodically drains the pipe to prevent this from happening.
		 * The draining process requires an intermediate buffer.
		 */
		micCtx_.poll_buf = static_cast<unsigned short *>(kernel_.Malloc(DRAIN_SIZE));

		if ((err = snd_pcm_open(&micCtx_.pcm_handle, cap_name, SND_PCM_STREAM_CAPTURE, 0)) < 0)
		{
			continue;
		}

		if((err = set_hw_params(&micCtx_)) < 0)
		{
			continue;
		}

		if((err = set_sw_params(&micCtx_)) < 0)
		{
			continue;
		}

		if((err = snd_async_add_pcm_handler(&micCtx_.ahandler, micCtx_.pcm_handle, RecordCallback, &micCtx_)) < 0)
		{
			continue;
		}

		err = snd_pcm_prepare(micCtx_.pcm_handle);

	} while(0);

	if(err < 0)
	{
		if(micCtx_.poll_buf)
		{
			kernel_.Free(micCtx_.poll_buf);
			micCtx_.poll_buf = NULL;
		}


		if(micCtx_.pcm_handle)
		{
			snd_pcm_close(micCtx_.pcm_handle);
			micCtx_.pcm_handle = NULL;
		}

		if(micCtx_.fd[0] != -1)
		{
			close(micCtx_.fd[0]);
			micCtx_.fd[0] = -1;
		}

		if(micCtx_.fd[1] != -1)
		{
			close(micCtx_.fd[1]);
			micCtx_.fd[1] = -1;
		}
	}

	kErr = kNoErr;
	return kErr;
}

//----------------------------------------------------------------------------
tErrType	CCameraModule::DeinitMicInt()
{
	tErrType			kErr	= kNoErr;

	kernel_.Free(micCtx_.poll_buf);
	micCtx_.poll_buf	= NULL;

	close(micCtx_.fd[0]);
	close(micCtx_.fd[1]);
	micCtx_.fd[0]		= -1;
	micCtx_.fd[1]		= -1;

	if (micCtx_.pcm_handle)
		snd_pcm_close(micCtx_.pcm_handle);
	micCtx_.pcm_handle = NULL;

    return kErr;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::StartAudio()
{
	int err = snd_pcm_prepare(micCtx_.pcm_handle);

	if(err == 0)
	{
		err = snd_pcm_start(micCtx_.pcm_handle);
	}

	return (err == 0) ? true : false;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::StopAudio()
{
	Boolean bRet = false;;
	int err, bytes;
	ssize_t len;
	fd_set rfds;
	struct timeval tv = {0,0};	/* immediate timeout */

	err = snd_pcm_drop(micCtx_.pcm_handle);

	/* purge pipe */
	FD_ZERO(&rfds);
	FD_SET(micCtx_.fd[0], &rfds);

	err = select(micCtx_.fd[0]+1, &rfds, NULL, NULL, &tv);

	if(err == 0)	/* no data */
	{
		bRet = true;
	}
	else if(err > 0)
	{
		/* This is a blocking read and the source has stopped, so don't attempt
		 * to read more than the pipe contains.
		 */
		err = ioctl(micCtx_.fd[0], FIONREAD, &bytes);
		if(err != -1)
		{
			bRet = true;

			do
			{
				len = read(micCtx_.fd[0], micCtx_.poll_buf, MIN(DRAIN_SIZE, bytes));
				bytes -= len;
			} while(bytes);
		}
	}

	return bRet;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::WriteAudio(avi_t *avi)
{
	Boolean ret = false;
	int err;
	fd_set rfds;
	struct timeval tv = {0,0};	/* immediate timeout */

	ssize_t len;

	FD_ZERO(&rfds);
	FD_SET(micCtx_.fd[0], &rfds);

	err = select(micCtx_.fd[0]+1, &rfds, NULL, NULL, &tv);

	if(err < 0)
	{
		ret = false;
	}
	else if(err == 0)	/* no data */
	{
		ret = true;
	}
	else
	{
		len = read(micCtx_.fd[0], micCtx_.poll_buf, DRAIN_SIZE);
		if(len > 0)
		{
			AVI_write_audio(avi, reinterpret_cast<char*>(micCtx_.poll_buf), len);
//		while(len == DRAIN_SIZE)
//		{
//			len = read(micCtx_.fd[0], micCtx_.poll_buf, DRAIN_SIZE);
//			if(len > 0)
//			{
//				AVI_append_audio(avi, buf, len);
//			}
//		}
		}
		ret = true;
	}

	return ret;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::WriteAudio(SNDFILE *wav)
{
	Boolean ret = false;
	int err;
	fd_set rfds;
	struct timeval tv = {0,0};	/* immediate timeout */

	ssize_t len;
	sf_count_t wrote;

	FD_ZERO(&rfds);
	FD_SET(micCtx_.fd[0], &rfds);

	err = select(micCtx_.fd[0]+1, &rfds, NULL, NULL, &tv);

	if(err < 0)
	{
		ret = false;
	}
	else if(err == 0)	/* no data */
	{
		ret = true;
	}
	else
	{
		len = read(micCtx_.fd[0], micCtx_.poll_buf, DRAIN_SIZE);
		if(len > 0)
		{
			wrote = len;
			wrote = sf_write_raw(wav, micCtx_.poll_buf, wrote);
		}
		ret = true;
	}

	return ret;
}

//----------------------------------------------------------------------------
static void RecordCallback(snd_async_handler_t *ahandler)
{
	snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
	struct tMicrophoneContext *pCtx = static_cast<struct tMicrophoneContext *>(snd_async_handler_get_callback_private(ahandler));
	const snd_pcm_channel_area_t *my_area;
	snd_pcm_uframes_t offset, frames, size;
	snd_pcm_sframes_t avail, commitres;
	snd_pcm_state_t state;

	unsigned char *samples;
	int step, err;

	ssize_t res;

	state = snd_pcm_state(handle);
	if(state != SND_PCM_STATE_RUNNING)
	{
		switch(state) {
		case SND_PCM_STATE_SUSPENDED:
			while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			{
				sleep(1);
			}
			if(err >= 0)
				break;
		case SND_PCM_STATE_XRUN:
			err = snd_pcm_prepare(handle);
			break;
		}

		if(err < 0)
		{
			/* TODO: could not recover - abort recording?*/
		}
		else
		{
			snd_pcm_start(handle);
		}
	}

	avail = snd_pcm_avail_update(handle);

	frames = avail;//pCtx->period_size;
	err = snd_pcm_mmap_begin(handle, &my_area, &offset, &frames);

	samples	= (((unsigned char *)my_area->addr) + (my_area->first / 8));
	step	= my_area->step / 8;
	samples += offset * step;

	res = write(pCtx->fd[1], samples, frames * 2);

	commitres = snd_pcm_mmap_commit(handle, offset, res / 2);
}

//----------------------------------------------------------------------------
tAudCapHndl CCameraModule::StartAudioCapture(const CPath& path, IEventListener * pListener, const U32 maxLength, const Boolean paused)
{
	tAudCapHndl hndl = kInvalidAudCapHndl;

	if(micCtx_.hndl != kInvalidAudCapHndl)
	{
		return hndl;
	}

	micCtx_.reqLength = maxLength;
	micCtx_.pListener = pListener;
	micCtx_.bPaused   = paused;

	if(path.at(0) == '/')
	{
		micCtx_.path	= path;
	}
	else
	{
		DATA_LOCK;
		micCtx_.path	= apath + path;
		DATA_UNLOCK;
	}


	//hndl must be set before thread starts.  It is used in thread initialization.
	micCtx_.hndl = STREAMING_HANDLE(THREAD_HANDLE(1));

	if(kNoErr == InitMicTask(this))
	{
		hndl = micCtx_.hndl;
	}
	else
	{
		micCtx_.hndl = hndl;
	}

	return hndl;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::PauseAudioCapture(const tAudCapHndl hndl)
{
	if(micCtx_.hndl == kInvalidAudCapHndl || micCtx_.bPaused)
	{
		return false;
	}

	micCtx_.bPaused = StopAudio();

	return micCtx_.bPaused;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::ResumeAudioCapture(const tAudCapHndl hndl)
{
	Boolean bRet;

	if(micCtx_.hndl == kInvalidAudCapHndl || !micCtx_.bPaused)
	{
		return false;
	}

	bRet = StartAudio();

	if(bRet == true)
	{
		micCtx_.bPaused = false;
	}

	return bRet;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::IsAudioCapturePaused(const tAudCapHndl hndl)
{
	if(micCtx_.hndl == kInvalidAudCapHndl)
	{
		return false;
	}

	return micCtx_.bPaused;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::StopAudioCapture(const tAudCapHndl hndl)
{
	int err;

	if(micCtx_.hndl == kInvalidAudCapHndl)
	{
		return false;
	}

	if(IS_THREAD_HANDLE(hndl))
	{
		DeInitMicTask(this);
	}

	micCtx_.hndl = kInvalidAudCapHndl;

	return true;
}
//----------------------------------------------------------------------------
static int set_hw_params(struct tMicrophoneContext *pCtx)
{
	snd_pcm_t*				handle	= pCtx->pcm_handle;
	snd_pcm_hw_params_t*	params	= pCtx->hwparams;
	unsigned int 			val		= 0;
	int						err;

	do
	{
		if((err = snd_pcm_hw_params_any(handle, params)) < 0)
		{
			continue;
		}

		/*
		 * Even though the Mic is mono, it must be set to INTERLEAVED access
		 * NONINTERLEAVED fails
		 */
		if((err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0)
		{
			continue;
		}

		/* sample rate conversion */
		err = 0;
		pCtx->rate = MIC_RATE;
		if((err = snd_pcm_hw_params_set_rate_near(handle, params, &pCtx->rate, &err)) < 0)
		{
			pCtx->rate = 0;
			continue;
		}

		/*
		 * See the note above about plughw.  The plug plugin does not assume any of the
		 * default settings of its slave pcm (hw) device.  Attempting to get() channels
		 * or format will return -EINVAL if they are not set() beforehand.
		 */
		err = 0;
		pCtx->channels = MIC_CHANS;
		if((err = snd_pcm_hw_params_set_channels_near(handle, params, &pCtx->channels)) < 0)
		{
			pCtx->channels = 0;
			continue;
		}

		pCtx->format = MIC_FMT;
		if((err = snd_pcm_hw_params_set_format(handle, params, pCtx->format)) < 0)
		{
			pCtx->format = SND_PCM_FORMAT_UNKNOWN;
			continue;
		}

		if((err = snd_pcm_hw_params_get_sbits(params)) < 0)
		{
			pCtx->sbits = 0;
			continue;
		}
		pCtx->sbits = err;

		/* commit changes */
		if((err = snd_pcm_hw_params(handle, params)) < 0)
		{
			continue;
		}
	} while (0);

	return err;
}

//----------------------------------------------------------------------------
int CCameraModule::XlateAudioFormatAVI(snd_pcm_format_t fmt)
{
	int ret = WAVE_FORMAT_UNKNOWN;

	if(fmt == SND_PCM_FORMAT_UNKNOWN)
	{
		ret = WAVE_FORMAT_UNKNOWN;
	}
	else if(fmt <= SND_PCM_FORMAT_FLOAT64_BE)
	{
		ret = WAVE_FORMAT_PCM;
	}
	else
	{
		switch(fmt)
		{
		case (SND_PCM_FORMAT_A_LAW):
			ret = WAVE_FORMAT_ALAW;
			break;
		case (SND_PCM_FORMAT_MU_LAW):
			ret = WAVE_FORMAT_MULAW;
			break;
		case (SND_PCM_FORMAT_IMA_ADPCM):
			ret = WAVE_FORMAT_DVI_ADPCM;
			break;
		case (SND_PCM_FORMAT_GSM):
			ret = WAVE_FORMAT_GSM610;
			break;
		}
	}

	return ret;
}

//----------------------------------------------------------------------------
int CCameraModule::XlateAudioFormatSF(snd_pcm_format_t fmt)
{
	int ret = 0;

	switch(fmt)
	{
	case (SND_PCM_FORMAT_S8):
		ret = SF_FORMAT_PCM_S8;
		break;

	case (SND_PCM_FORMAT_U8):
		ret = SF_FORMAT_PCM_U8;
		break;

	case (SND_PCM_FORMAT_S16_LE):
	case (SND_PCM_FORMAT_S16_BE):
	case (SND_PCM_FORMAT_U16_LE):
	case (SND_PCM_FORMAT_U16_BE):
		ret = SF_FORMAT_PCM_16;
		break;

	case (SND_PCM_FORMAT_S24_LE):
	case (SND_PCM_FORMAT_S24_BE):
	case (SND_PCM_FORMAT_U24_LE):
	case (SND_PCM_FORMAT_U24_BE):
		ret = SF_FORMAT_PCM_24;
		break;

	case (SND_PCM_FORMAT_S32_LE):
	case (SND_PCM_FORMAT_S32_BE):
	case (SND_PCM_FORMAT_U32_LE):
	case (SND_PCM_FORMAT_U32_BE):
		ret = SF_FORMAT_PCM_32;
		break;

	case (SND_PCM_FORMAT_FLOAT_LE):
	case (SND_PCM_FORMAT_FLOAT_BE):
		ret = SF_FORMAT_FLOAT;
		break;

	case (SND_PCM_FORMAT_FLOAT64_LE):
	case (SND_PCM_FORMAT_FLOAT64_BE):
		ret = SF_FORMAT_DOUBLE;
			break;

	case (SND_PCM_FORMAT_A_LAW):
		ret = SF_FORMAT_ALAW;
		break;

	case (SND_PCM_FORMAT_MU_LAW):
		ret = SF_FORMAT_ULAW;
		break;

	case (SND_PCM_FORMAT_IMA_ADPCM):
		ret = SF_FORMAT_IMA_ADPCM;
		break;

	case (SND_PCM_FORMAT_GSM):
		ret = SF_FORMAT_GSM610;
		break;
	}

	return ret;
}

//----------------------------------------------------------------------------
static int set_sw_params(struct tMicrophoneContext *pCtx)
{
	snd_pcm_t*				handle		= pCtx->pcm_handle;
	snd_pcm_sw_params_t*	swparams	= pCtx->swparams;
	int						err;

	do
	{
		if((err = snd_pcm_sw_params_current(handle, swparams)) < 0)
		{
			continue;
		}

		/*
		 * snd_pcm_sw_params_set_start_threshold()
		 * and
		 * snd_pcm_sw_params_set_avail_min()
		 * are not needed here because capture always(?) uses an explicit snd_pcm_start().
		 */

		/*
		 * snd_pcm_sw_params_set_period_event() is not needed either
		 */

		err = snd_pcm_sw_params(handle, swparams);
	} while (0);

	return err;
}

LF_END_BRIO_NAMESPACE()
