//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Display Manager module.
//
//==============================================================================

#include <DisplayMPI.h>
#include <DisplayPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "DisplayMPI";


//============================================================================
// CDisplayMPI
//============================================================================
//----------------------------------------------------------------------------
CDisplayMPI::CDisplayMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kDisplayModuleName, kDisplayModuleVersion);
	pModule_ = reinterpret_cast<CDisplayModule*>(pModule);
}

//----------------------------------------------------------------------------
CDisplayMPI::~CDisplayMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CDisplayMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CDisplayMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CDisplayMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CDisplayMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CDisplayMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
U16 CDisplayMPI::GetNumberOfScreens() const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumberOfScreens();
}

//----------------------------------------------------------------------------
const tDisplayScreenStats* CDisplayMPI::GetScreenStats(tDisplayScreen screen) const
{
	if(!pModule_)
		return NULL;
	return pModule_->GetScreenStats(screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->Invalidate(screen, pDirtyRect);
}

//============================================================================
//----------------------------------------------------------------------------
tDisplayHandle CDisplayMPI::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	if(!pModule_)
		return kInvalidDisplayHandle;
	return pModule_->CreateHandle(height, width, colorDepth, pBuffer);
}

//----------------------------------------------------------------------------
U8* CDisplayMPI::GetBuffer(tDisplayHandle hndl) const
{
	if(!pModule_)
		return NULL;
	return pModule_->GetBuffer(hndl);
}

//----------------------------------------------------------------------------
U16 CDisplayMPI::GetHeight(tDisplayHandle hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetHeight(hndl);
}

//----------------------------------------------------------------------------
U16 CDisplayMPI::GetWidth(tDisplayHandle hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetWidth(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
								tDisplayHandle insertAfter, tDisplayScreen screen)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->Register(hndl, xPos, yPos, insertAfter, screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
							 tDisplayZOrder initialZOrder, 
                             tDisplayScreen screen)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->Register(hndl, xPos, yPos, initialZOrder, screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnRegister(hndl, screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->DestroyHandle(hndl, destroyBuffer);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::LockBuffer(tDisplayHandle hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->LockBuffer(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnlockBuffer(hndl, pDirtyRect);
}


U16 CDisplayMPI::GetPitch(tDisplayHandle hndl) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetPitch(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetAlpha(tDisplayHandle hndl, U8 level, Boolean enable)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetAlpha(hndl, level, enable);
}

//----------------------------------------------------------------------------
void CDisplayMPI::InitOpenGL(void* pCtx)
{
	if (!pModule_)
		return;
	pModule_->InitOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayMPI::DeinitOpenGL()
{
	if (!pModule_)
		return;
	pModule_->DeinitOpenGL();
}

// EOF
