//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		MicrophoneMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio
//		Microphone Manager module.
//
//==============================================================================

#include <MicrophoneTypes.h>
#include <MicrophoneMPI.h>
#include <MicrophonePriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "MicrophoneMPI";

//============================================================================
// CMicrophoneEventMessage
//============================================================================
//------------------------------------------------------------------------------
CMicrophoneEventMessage::CMicrophoneEventMessage( const tMicrophoneCaptureTimeoutMsg& data )
	: IEventMessage(kMicrophoneCaptureTimeOutEvent)
{
	this->data.timeOut = data;
}

CMicrophoneEventMessage::CMicrophoneEventMessage( const tMicrophoneCaptureQuotaHitMsg& data )
	: IEventMessage(kMicrophoneCaptureQuotaHitEvent)
{
	this->data.quotaHit = data;
}

CMicrophoneEventMessage::CMicrophoneEventMessage( const tMicrophoneCaptureStoppedMsg& data )
	: IEventMessage(kMicrophoneCaptureStoppedEvent)
{
	this->data.stopped = data;
}

CMicrophoneEventMessage::CMicrophoneEventMessage( const tMicrophoneRemovedMsg& data )
	: IEventMessage(kMicrophoneRemovedEvent)
{
	this->data.removed = data;
}

CMicrophoneEventMessage::CMicrophoneEventMessage( const tMicrophoneAudioTriggeredMsg& data )
	: IEventMessage(kMicrophoneAudioTriggeredEvent)
{
	this->data.triggered = data;
}

//------------------------------------------------------------------------------
U16	CMicrophoneEventMessage::GetSizeInBytes() const
{
	return sizeof(CMicrophoneEventMessage);
}   // ---- end GetSizeInBytes() ----

//============================================================================
// CMicrophoneMPI
//============================================================================
//----------------------------------------------------------------------------
CMicrophoneMPI::CMicrophoneMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kMicrophoneModuleName, kMicrophoneModuleVersion);
	pModule_ = reinterpret_cast<CMicrophoneModule*>(pModule);
}

//----------------------------------------------------------------------------
CMicrophoneMPI::~CMicrophoneMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CMicrophoneMPI::IsValid() const
{
	if(!pModule_)
		return false;
	return pModule_->IsValid();
}

//----------------------------------------------------------------------------
const CString* CMicrophoneMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CMicrophoneMPI::GetModuleVersion() const
{
	if (!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CMicrophoneMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CMicrophoneMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

//============================================================================

Boolean CMicrophoneMPI::IsMicrophonePresent()
{
	if(!pModule_)
		return false;
	return pModule_->IsMicrophonePresent();
}

//----------------------------------------------------------------------------
tErrType CMicrophoneMPI::SetAudioPath(const CPath& path)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetAudioPath(path);
}

//----------------------------------------------------------------------------
CPath* CMicrophoneMPI::GetAudioPath()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetAudioPath();
}

//----------------------------------------------------------------------------
tAudCapHndl	CMicrophoneMPI::StartAudioCapture(const CPath& path, IEventListener * pListener, const U32 maxLength, const Boolean paused)
{
	if (!pModule_)
		return kInvalidAudCapHndl;
	return pModule_->StartAudioCapture(path, pListener, maxLength, paused);
}

//----------------------------------------------------------------------------
Boolean CMicrophoneMPI::PauseAudioCapture(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->PauseAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CMicrophoneMPI::ResumeAudioCapture(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->ResumeAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CMicrophoneMPI::IsAudioCapturePaused(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->IsAudioCapturePaused(hndl);
}

//----------------------------------------------------------------------------
Boolean CMicrophoneMPI::StopAudioCapture(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->StopAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CMicrophoneMPI::SetMicrophoneParam(enum tMicrophoneParam param, S32 value)
{
	if (!pModule_)
		return false;
	return pModule_->SetMicrophoneParam(param, value);
}

//----------------------------------------------------------------------------
S32	CMicrophoneMPI::GetMicrophoneParam(enum tMicrophoneParam param)
{
	if (!pModule_)
		return 0;
	return pModule_->GetMicrophoneParam(param);
}

LF_END_BRIO_NAMESPACE()
// EOF
