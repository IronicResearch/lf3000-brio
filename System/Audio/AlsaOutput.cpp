//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AlsaOutput.cpp
//
// Description:
//		Implements audio output via ALSA.
//		Based on ALSA PCM output example:
//		http://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_8c-example.html
//
//==============================================================================

#include <CoreTypes.h>
#include <AudioConfig.h>
#include <AudioOutput.h>
#include <KernelMPI.h>
#include <DebugMPI.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Global variables
//==============================================================================

static const char 			*device = "plughw:0,0";		    // playback device
static const char 			*plugin = "plugdmix";		    // playback dmix plugin
static snd_pcm_t 			*handle = NULL;					// playback handle
static snd_pcm_access_t 	access = SND_PCM_ACCESS_MMAP_INTERLEAVED;	
static snd_pcm_format_t 	format = SND_PCM_FORMAT_S16;    // sample format 
static unsigned int 		rate = 32000;                   // stream rate 
static unsigned int 		channels = 2;                   // count of channels 
static unsigned int 		buffer_time = 128000;           // ring buffer length in us 
static unsigned int 		period_time = 32000;            // period time in us 
static int 					resample = 1;                   // enable alsa-lib resampling 
static int 					period_event = 0;               // produce poll event after each period 
static snd_pcm_sframes_t 	buffer_size;
static snd_pcm_sframes_t 	period_size;

static BrioAudioRenderCallback* 	gAudioRenderCallback = NULL;	// Brio callback function
static void* 						gCallbackUserData = NULL;		// Brio callback data
extern CKernelMPI*					pKernelMPI_;
extern CDebugMPI*					pDebugMPI_;

//==============================================================================
// Local functions
//==============================================================================

//----------------------------------------------------------------------------
static int set_hwparams(snd_pcm_t *handle,
					 snd_pcm_hw_params_t *params,
					 snd_pcm_access_t access)
{
	unsigned int rrate;
	snd_pcm_uframes_t size;
	int err, dir;
	
	// choose all parameters 
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
		return err;
	}
	// set hardware resampling 
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Resampling setup failed for playback: %s\n", snd_strerror(err));
		return err;
	}
	// set the interleaved read/write format 
	err = snd_pcm_hw_params_set_access(handle, params, access);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Access type not available for playback: %s\n", snd_strerror(err));
		return err;
	}
	// set the sample format 
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}
	// set the count of channels 
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
		return err;
	}
	// set the stream rate 
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
		return err;
	}
	if (rrate != rate) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
		return -EINVAL;
	}
	// set the buffer time 
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_buffer_size(params, &size);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to get buffer size for playback: %s\n", snd_strerror(err));
		return err;
	}
	buffer_size = size;
	pDebugMPI_->DebugOut(kDbgLvlImportant, "%s: buffer time=%d, size=%d\n", __FUNCTION__, buffer_time, (int)buffer_size);
	// set the period time 
	err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to get period size for playback: %s\n", snd_strerror(err));
		return err;
	}
	period_size = size;
	pDebugMPI_->DebugOut(kDbgLvlImportant, "%s: period time=%d, size=%d\n", __FUNCTION__, period_time, (int)period_size);
	// write the parameters to device 
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set hw params for playback: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

//----------------------------------------------------------------------------
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
	int err;
	
	// get the current swparams 
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to determine current swparams for playback: %s\n", snd_strerror(err));
		return err;
	}
	// start the transfer when the buffer is almost full: 
	// (buffer_size / avail_min) * avail_min 
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
		return err;
	}
	// allow the transfer when at least period_size samples can be processed 
	// or disable this mechanism when period event is enabled (aka interrupt like style processing) 
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set avail min for playback: %s\n", snd_strerror(err));
		return err;
	}
#if SND_LIB_MAJOR == 1 && SND_LIB_MINOR == 0 && SND_LIB_SUBMINOR >= 20
	// enable period events when requested 
	if (period_event) {
		err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
		if (err < 0) {
			pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set period event: %s\n", snd_strerror(err));
			return err;
		}
	}
#endif
	// write the parameters to the playback device 
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Unable to set sw params for playback: %s\n", snd_strerror(err));
		return err;
	}
	return 0;
}

//----------------------------------------------------------------------------
static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) {    // under-run 
		err = snd_pcm_prepare(handle);
		if (err < 0)
			 pDebugMPI_->DebugOut(kDbgLvlImportant, "Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			pKernelMPI_->TaskSleep(10);       // wait until the suspend flag is released 
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				pDebugMPI_->DebugOut(kDbgLvlImportant, "Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

//----------------------------------------------------------------------------
static void async_direct_callback(snd_async_handler_t *ahandler)
{
	snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
	// struct async_private_data *data = (async_private_data*)snd_async_handler_get_callback_private(ahandler);
	const snd_pcm_channel_area_t *my_areas;
	snd_pcm_uframes_t offset, frames, size;
	snd_pcm_sframes_t avail, commitres;
	snd_pcm_state_t state;
	int first = 0, err;
	
	while (1) {
		state = snd_pcm_state(handle);
		if (state == SND_PCM_STATE_XRUN) {
			err = xrun_recovery(handle, -EPIPE);
			if (err < 0) {
				pDebugMPI_->DebugOut(kDbgLvlImportant, "XRUN recovery failed: %s\n", snd_strerror(err));
				break; //exit(EXIT_FAILURE);
			}
			first = 1;
		} else if (state == SND_PCM_STATE_SUSPENDED) {
			err = xrun_recovery(handle, -ESTRPIPE);
			if (err < 0) {
				pDebugMPI_->DebugOut(kDbgLvlImportant, "SUSPEND recovery failed: %s\n", snd_strerror(err));
				break; //exit(EXIT_FAILURE);
			}
		}
		avail = snd_pcm_avail_update(handle);
		if (avail < 0) {
			err = xrun_recovery(handle, avail);
			if (err < 0) {
				pDebugMPI_->DebugOut(kDbgLvlImportant, "avail update failed: %s\n", snd_strerror(err));
				break; //exit(EXIT_FAILURE);
			}
			first = 1;
			continue;
		}
		if (avail < period_size) {
			if (first) {
				first = 0;
				err = snd_pcm_start(handle);
				if (err < 0) {
					pDebugMPI_->DebugOut(kDbgLvlImportant, "Start error: %s\n", snd_strerror(err));
					break; //exit(EXIT_FAILURE);
				}
			} else {
				break;
			}
			continue;
		}
		size = period_size;
		while (size > 0) {
			frames = size;
			err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
			if (err < 0) {
				if ((err = xrun_recovery(handle, err)) < 0) {
					pDebugMPI_->DebugOut(kDbgLvlImportant, "MMAP begin avail error: %s\n", snd_strerror(err));
					break; //exit(EXIT_FAILURE);
				}
				first = 1;
			}
			// Callback to Brio mixer
			short* pmem = (short*)my_areas->addr + (my_areas->first + offset) * 2; // stereo frames
			gAudioRenderCallback(pmem, frames, gCallbackUserData);
			
			commitres = snd_pcm_mmap_commit(handle, offset, frames);
			if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) {
				if ((err = xrun_recovery(handle, commitres >= 0 ? -EPIPE : commitres)) < 0) {
					pDebugMPI_->DebugOut(kDbgLvlImportant, "MMAP commit error: %s\n", snd_strerror(err));
					break; //exit(EXIT_FAILURE);
				}
				first = 1;
			}
			size -= frames;
		}
	}
}

//----------------------------------------------------------------------------
static void async_direct_start(snd_pcm_t *handle)
{
	int err;
	const snd_pcm_channel_area_t *my_areas;
	snd_pcm_uframes_t offset, frames, size;
	snd_pcm_sframes_t avail, commitres;
	
	// Need to submit output samples to PCM driver before calling snd_pcm_start().
	// Note similarity to code used above in async_direct_callback.
	for (int count = 0; count < 2; count++) {
		size = period_size;
		while (size > 0) {
			frames = size;
			err = snd_pcm_mmap_begin(handle, &my_areas, &offset, &frames);
			if (err < 0) {
				if ((err = xrun_recovery(handle, err)) < 0) {
					pDebugMPI_->DebugOut(kDbgLvlImportant, "MMAP begin avail error: %s\n", snd_strerror(err));
					break; //exit(EXIT_FAILURE);
				}
			}
			// Safe to Callback to Brio mixer?
			char* pmem = (char*)my_areas->addr + (my_areas->first + offset) * 4;
			memset(pmem, 0, frames * 4);
			
			commitres = snd_pcm_mmap_commit(handle, offset, frames);
			if (commitres < 0 || (snd_pcm_uframes_t)commitres != frames) {
				if ((err = xrun_recovery(handle, commitres >= 0 ? -EPIPE : commitres)) < 0) {
					pDebugMPI_->DebugOut(kDbgLvlImportant, "MMAP commit error: %s\n", snd_strerror(err));
					break; //exit(EXIT_FAILURE);
				}
			}
			size -= frames;
		}
	}
}

//----------------------------------------------------------------------------
static int direct_write_loop(snd_pcm_t *handle, signed short* samples)
{
	signed short *ptr;
	int err, cptr;
	
	{
		ptr = samples;
		cptr = period_size;
		while (cptr > 0) {
			err = snd_pcm_mmap_writei(handle, ptr, cptr);
			if (err == -EAGAIN)
				continue;
			if (err < 0) {
				if (xrun_recovery(handle, err) < 0) {
					pDebugMPI_->DebugOut(kDbgLvlImportant, "Write error: %s\n", snd_strerror(err));
					return err;
				}
				break;
			}
			ptr += err * channels;
			cptr -= err;
		}
	}
}

//==============================================================================
// Callback thread	
//==============================================================================
static volatile bool 		bRunning = true;
static volatile bool 		bRendering = false;
static tTaskHndl 			hndlThread = 0;
static S16*					pOutputBuffer = NULL;
//----------------------------------------------------------------------------
static void* CallbackThread(void* pCtx)
{
	int r = 0;
	
	while (bRunning)
	{
		if (bRendering)
		{
			// Brio render callback 
			do { 
				r = gAudioRenderCallback(pOutputBuffer, kAudioFramesPerBuffer, pCtx);
				if (r != kNoErr)
					pKernelMPI_->TaskSleep(10);
			}
			while (r != kNoErr);
	
			// Output Brio render buffer to ALSA
			direct_write_loop(handle, pOutputBuffer);
		}
		pKernelMPI_->TaskSleep(10);
	}
}
 
//==============================================================================
// InitAudioOutput	
//==============================================================================
int InitAudioOutputAlsa( BrioAudioRenderCallback* callback, void* pUserData )
{
	int err;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
    snd_async_handler_t *ahandler;
	
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);
	
	gAudioRenderCallback = callback;
	gCallbackUserData = pUserData;
	
	// Try opening dmix plugin first
	if ((err = snd_pcm_open(&handle, plugin, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Playback open error: (%s) %s\n", plugin, snd_strerror(err));
		// Else fallback to opening audio device directly
		if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			pDebugMPI_->DebugOut(kDbgLvlImportant, "Playback open error: (%s) %s\n", device, snd_strerror(err));
			return err;
		}
	}
	if ((err = set_hwparams(handle, hwparams, access)) < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Setting of hwparams failed: %s\n", snd_strerror(err));
		return err;
	}
	if ((err = set_swparams(handle, swparams)) < 0) {
		pDebugMPI_->DebugOut(kDbgLvlImportant, "Setting of swparams failed: %s\n", snd_strerror(err));
		return err;
	}

	// Create output buffer for Brio mixer 
	pOutputBuffer = (S16*)pKernelMPI_->Malloc(kAudioOutBufSizeInBytes);
	if (!pOutputBuffer)
		return -1;

	// Create callback thread for Brio mixer
	tTaskProperties props;
	props.TaskMainFcn = &CallbackThread;
	props.taskMainArgCount = 1;
	props.pTaskMainArgValues = pUserData;
	bRunning = true;
	err = pKernelMPI_->CreateTask(hndlThread, props, NULL);
	if (err != kNoErr)
		return err;

	return kNoErr;
}

// ==============================================================================
// StartAudioOutput	 
// ==============================================================================
int StartAudioOutputAlsa( void )
{
	bRendering = true;
	return snd_pcm_start(handle);
}

// ==============================================================================
// StopAudioOutput	
// ==============================================================================
int StopAudioOutputAlsa( void )
{
	bRendering = false;
	pKernelMPI_->TaskSleep(10);
	return snd_pcm_drop(handle);
}

//==============================================================================
// DeInitAudioOutput  
//==============================================================================
int DeInitAudioOutputAlsa( void )
{
	// Kill callback thread
	void* retval;
	bRunning = false;
	pKernelMPI_->JoinTask(hndlThread, retval);

	// Release resources
	pKernelMPI_->Free(pOutputBuffer);
	pOutputBuffer = NULL;
	
	return snd_pcm_close(handle);
}

LF_END_BRIO_NAMESPACE()
// EOF
