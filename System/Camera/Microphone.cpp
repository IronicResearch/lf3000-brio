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

LF_BEGIN_BRIO_NAMESPACE()

#undef USE_ASYNC_PIPE								// for asynchronous callback pipe (obsolete)

//============================================================================
// Local variables
//============================================================================
namespace
{
	const tEventType LocalMicrophoneEvents[] = {kAllMicrophoneEvents};

#if USE_PROFILE
	// Profile vars
#endif
}

//============================================================================
// MicrophoneListener
//============================================================================
CCameraModule::MicrophoneListener::MicrophoneListener(IEventListener *dest, CEventMPI *event) : IEventListener(LocalMicrophoneEvents, ArrayCount(LocalMicrophoneEvents)) {
	reason = kUndefinedEventType;
	destListener = dest;
	pEvent = event;
}

//----------------------------------------------------------------------------
tEventStatus CCameraModule::MicrophoneListener::Notify(const IEventMessage& Imsg) {
	const CMicrophoneEventMessage &msg = dynamic_cast<const CMicrophoneEventMessage&>(Imsg);

	CCameraEventMessage *msgCam = NULL;

	reason = Imsg.GetEventType();
	if(reason == kMicrophoneCaptureTimeOutEvent)
	{
		tCaptureTimeoutMsg data;
		data.ahndl = msg.data.timeOut.ahndl;
		data.saved = msg.data.timeOut.saved;
		data.length = msg.data.timeOut.length;
		msgCam = new CCameraEventMessage(data);
	}
	else if(reason == kMicrophoneCaptureQuotaHitEvent)
	{
		tCaptureQuotaHitMsg data;
		data.ahndl = msg.data.quotaHit.ahndl;
		data.saved = msg.data.quotaHit.saved;
		data.length = msg.data.quotaHit.length;
		msgCam = new CCameraEventMessage(data);
	}
	else if(reason == kMicrophoneCaptureStoppedEvent)
	{
		tCaptureStoppedMsg data;
		data.ahndl = msg.data.stopped.ahndl;
		data.saved = msg.data.stopped.saved;
		data.length = msg.data.stopped.length;
		msgCam = new CCameraEventMessage(data);
	}
	else if(reason == kMicrophoneRemovedEvent)
	{
		tCameraRemovedMsg data;
		data.ahndl = msg.data.removed.ahndl;
		data.saved = msg.data.removed.saved;
		data.length = msg.data.removed.length;
		msgCam = new CCameraEventMessage(data);
	}
	else if(reason == kMicrophoneAudioTriggeredEvent)
	{
		tAudioTriggeredMsg data;
		data.ahndl = msg.data.triggered.ahndl;
		data.threshold = msg.data.triggered.threshold;
		data.timestamp = msg.data.triggered.timestamp;
		msgCam = new CCameraEventMessage(data);
	}

	if (destListener && msgCam)
		pEvent->PostEvent(*msgCam, 0, destListener);
	delete msgCam;

	return kEventStatusOK;
}

//----------------------------------------------------------------------------
tEventStatus CCameraModule::MicrophoneListener::Reset() {
	reason = kUndefinedEventType;
}

#define VALID_MICMPI(x)		if (!x) x = new CMicrophoneMPI();
//============================================================================
// Microphone Specific Functions:
//============================================================================
tErrType CCameraModule::SetCameraAudioPath(const CPath &path)
{
	VALID_MICMPI(microphone_);
	microphone_->SetAudioPath(path);
	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CCameraModule::GetCameraAudioPath()
{
	VALID_MICMPI(microphone_);
	return microphone_->GetAudioPath();
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::SetMicrophoneParam(enum tMicrophoneParam param, S32 value)
{
	VALID_MICMPI(microphone_);
	return microphone_->SetMicrophoneParam(param,value);
}

//----------------------------------------------------------------------------
S32	CCameraModule::GetMicrophoneParam(enum tMicrophoneParam param)
{
	VALID_MICMPI(microphone_);
	return microphone_->GetMicrophoneParam(param);
}

//----------------------------------------------------------------------------
tAudCapHndl CCameraModule::StartAudioCapture(const CPath& path, IEventListener * pListener, const U32 maxLength, const Boolean paused)
{
	tAudCapHndl hndl = kInvalidAudCapHndl;

	if (!microphone_)
		microphone_ = new CMicrophoneMPI();

	if(pListener == NULL) {
		hndl = microphone_->StartAudioCapture(path, NULL, maxLength, paused);
	}
	else {
		if(micListener_ != NULL)
			delete micListener_;
		micListener_ = new MicrophoneListener(pListener,&event_);
		hndl = microphone_->StartAudioCapture(path, micListener_, maxLength, paused);
	}
	return hndl;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::PauseAudioCapture(const tAudCapHndl hndl)
{
	VALID_MICMPI(microphone_);
	return microphone_->PauseAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CCameraModule::ResumeAudioCapture(const tAudCapHndl hndl)
{
	VALID_MICMPI(microphone_);
	return microphone_->ResumeAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CCameraModule::IsAudioCapturePaused(const tAudCapHndl hndl)
{
	VALID_MICMPI(microphone_);
	return microphone_->IsAudioCapturePaused(hndl);
}

//----------------------------------------------------------------------------
Boolean CCameraModule::StopAudioCapture(const tAudCapHndl hndl)
{
	VALID_MICMPI(microphone_);
	return microphone_->StopAudioCapture(hndl);
}

LF_END_BRIO_NAMESPACE()
