#ifndef LF_BRIO_MICROPHONEPRIV_H
#define LF_BRIO_MICROPHONEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		MicroPhonePriv.h
//
// Description:
//		Defines the interface for the private underlying Microphone manager module.
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <MicrophoneTypes.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <KernelTypes.h>
#include <queue>

/*
 * This is a workaround for the old (broken) headers installed on lightning-release
 * and the stale nfsroot on emplhwbuild01.leapfrog.com.
 */
#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#define FOO__STRICT_ANSI__
#endif
#include <asm/types.h>
#ifdef FOO__STRICT_ANSI__
#define __STRICT_ANSI__
#undef FOO__STRICT_ANSI__
#endif

#include <alsa/asoundlib.h>
#include <sndfile.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kMicrophoneModuleName		= "Microphone";
const tVersion			kMicrophoneModuleVersion	= 2;
const tEventPriority	kMicrophoneEventPriority	= 0;
const tDebugLevel		kMicrophoneDebugLevel		= kDbgLvlImportant;

const tVidCapHndl		kStreamingActive		= 0x80000000;
const tVidCapHndl		kStreamingThread		= 0x40000000;
const tVidCapHndl		kStreamingFrame			= 0x20000000;	// frame-by-frame, i.e., no thread

const U64	MIN_FREE	= 1*1024*1024;		/* NOTE: UBIFS is internally padded,
											 * so this isn't strictly needed */

#define IS_STREAMING_HANDLE(x) \
	(((x) & kStreamingActive) && (camCtx_.hndl == x))

#define IS_THREAD_HANDLE(x) \
	((x) & kStreamingThread)

#define IS_FRAME_HANDLE(x) \
	((x) & kStreamingFrame)

#define STREAMING_HANDLE(x) \
	((x) | kStreamingActive)

#define THREAD_HANDLE(x) \
	((x) | kStreamingThread)

#define FRAME_HANDLE(x) \
	((x) | kStreamingFrame)

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_GSM610              (0x0031)
#endif

//==============================================================================
// Typedefs
//==============================================================================
struct tMicrophoneContext {
	snd_pcm_t *				pcm_handle;
	snd_async_handler_t *	ahandler;
	snd_pcm_hw_params_t	*	hwparams;
	snd_pcm_sw_params_t *	swparams;

	int						fd[2];		/* pipe for ALSA callback->file output*/
	unsigned short *		poll_buf;	/* to transfer from pipe to file */

	unsigned int			rate;		/* sampling rate from alsa-lib, not HW */
	unsigned int			channels;	/* sampling rate from alsa-lib, not HW */
	snd_pcm_format_t		format;
	int						sbits;		/* sample width (significant bits) */

	tAudCapHndl				hndl;

	tTaskHndl				hMicThread;

	CPath					path;
	Boolean					bPaused;
	U32						reqLength;		// length in seconds requested by app
	U32						maxLength;		// length in seconds available on FS

	IEventListener			*pListener;

	tMutex					dlock;

	unsigned int			period_time;	// period time in usec for callbacks
	snd_pcm_uframes_t		period_size;

	unsigned int			bytesRead;		// bytes read per callback
	unsigned int			bytesWritten;	// bytes written total
	unsigned int			counter;		// block counter
	unsigned int			block_size;		// block size in bytes = period_size * 2
	
	snd_pcm_status_t* 		status;
	snd_timestamp_t 		tstamp;

	S32						threshold;		// threshold for trigger event
	S32						duration;		// duration for trigger event
	S32						clipCount;		// clip count for trigger event
	S32						rateAdjust;		// sample rate adjustment

	CDebugMPI				*dbg;
};

//==============================================================================
// External function prototypes
//==============================================================================

//==============================================================================
class CMicrophoneModule : public ICoreModule {
	// internal listener for hotplug events
	class MicrophoneListener : public IEventListener
	{
		CMicrophoneModule*	pMod;
		Boolean			running;
	public:
		MicrophoneListener(CMicrophoneModule* ctx);
		~MicrophoneListener();
		tEventStatus Notify(const IEventMessage& msg);
	};
public:
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT Boolean		IsMicrophonePresent();

	VTABLE_EXPORT tErrType		SetAudioPath(const CPath& path);
	VTABLE_EXPORT CPath*		GetAudioPath();

	VTABLE_EXPORT tAudCapHndl	StartAudioCapture(const CPath& path, IEventListener * pListener,\
								                    const U32 maxLength, const Boolean paused);
	VTABLE_EXPORT Boolean		PauseAudioCapture(const tAudCapHndl hndl);
	VTABLE_EXPORT Boolean		ResumeAudioCapture(const tAudCapHndl hndl);
	VTABLE_EXPORT Boolean		IsAudioCapturePaused(const tAudCapHndl hndl);
	VTABLE_EXPORT Boolean		StopAudioCapture(const tAudCapHndl hndl);

	VTABLE_EXPORT Boolean		SetMicrophoneParam(enum tMicrophoneParam param, S32 value);
	VTABLE_EXPORT S32			GetMicrophoneParam(enum tMicrophoneParam param);
	VTABLE_EXPORT unsigned int	CameraWriteAudio(void* avi);

private:
	CPath				sysfs;		// e.g., "/sys/class/usb_device/usbdev1.2/"
	MicrophoneListener		*listener_;
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
	CEventMPI			event_;
	tMicrophoneContext	micCtx_;

	Boolean				valid;
	tMutex				mutex_;

	// Limit object creation to the Module Manager interface functions
	CMicrophoneModule();
	virtual ~CMicrophoneModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));

	friend Boolean EnumMicrophoneCallback(const CPath& path, void* pctx);

	// Implementation-specific functionality
	Boolean		InitMicrophoneInt();
	Boolean		DeinitMicrophoneInt();

	tErrType	InitMicInt();
	tErrType	DeinitMicInt();
	int			XlateAudioFormatAVI(snd_pcm_format_t fmt);
	int			XlateAudioFormatSF(snd_pcm_format_t fmt);
	Boolean		StartAudio(Boolean reset = true);
	unsigned int WriteAudio(void *avi);
	Boolean		WriteAudio(SNDFILE *wav);
	Boolean		FlushAudio();
	Boolean		StopAudio();

	friend void* MicTaskMain(void* arg);
	friend tErrType InitMicTask(CMicrophoneModule* module);
	friend tErrType DeInitMicTask(CMicrophoneModule* module);

};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_MICROPHONEPRIV_H

// EOF
