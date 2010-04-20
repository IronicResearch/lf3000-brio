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

static const unsigned int BUF_DUR_USEC = 500000;	/* Ring buffer size */
static const unsigned int PER_DUR_USEC = 100000;	/* Callback period */
	/* 100000 uS = 100 mS = 4800 samples @ 48 KHz */

static const U32 DRAIN_SIZE = 32768;	/* 1 second of 16-bit, 16 KHz mono */

static const char *cap_name = "hw:1,0";

static void RecordCallback(snd_async_handler_t *ahandler);

static int set_hw_params(struct tMicrophoneContext *pCtx);
static int set_sw_params(struct tMicrophoneContext *pCtx);


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

	snd_pcm_close(micCtx_.pcm_handle);
	micCtx_.pcm_handle = NULL;

    return kErr;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::StartAudio()
{
	int err = snd_pcm_start(micCtx_.pcm_handle);

	return (err == 0) ? true : false;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::StopAudio()
{
	int err = snd_pcm_drop(micCtx_.pcm_handle);

	return (err == 0) ? true : false;
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
	/* TODO: handle XRUN */

	avail = snd_pcm_avail_update(handle);

	frames = pCtx->period_size;
	err = snd_pcm_mmap_begin(handle, &my_area, &offset, &frames);

	samples	= (((unsigned char *)my_area->addr) + (my_area->first / 8));
	step	= my_area->step / 8;
	samples += offset * step;

	res = write(pCtx->fd[1], samples, frames * 2);

	commitres = snd_pcm_mmap_commit(handle, offset, res / 2);
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

		val = BUF_DUR_USEC;
		err = 0;
		if((err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &val, &err)) < 0)
		{
			continue;
		}

		if((err = snd_pcm_hw_params_get_buffer_size(params, &pCtx->buffer_size)) < 0)
		{
			pCtx->buffer_size = 0;
			continue;
		}

		val = PER_DUR_USEC;
		err = 0;
		if((err = snd_pcm_hw_params_set_period_time_near(handle, params, &val, &err)) < 0)
		{
			continue;
		}

		err = 0;
		if((err = snd_pcm_hw_params_get_period_size(params, &pCtx->period_size, &err)) < 0)
		{
			continue;
		}

		/* commit changes (buffer size requests) */
		if((err = snd_pcm_hw_params(handle, params)) < 0)
		{
			continue;
		}
	} while (0);

	return err;
}

//----------------------------------------------------------------------------
static int set_sw_params(struct tMicrophoneContext *pCtx)
{
	snd_pcm_t*				handle		= pCtx->pcm_handle;
	snd_pcm_sw_params_t*	swparams	= pCtx->swparams;
	snd_pcm_uframes_t 		period_size = pCtx->period_size;
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
