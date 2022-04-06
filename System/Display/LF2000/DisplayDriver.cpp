//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayDriver.cpp
//
// Description:
//		Display driver for hardware target.
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>
#include <BrioOpenGLConfigPrivate.h>

#include <list>
#include <algorithm>
#include <sys/stat.h>

LF_BEGIN_BRIO_NAMESPACE()

extern std::list<tDisplayContext*>	gDisplayList;
extern tMutex gListMutex;

namespace 
{
	static CDisplayDriver*		pDriver = NULL;
	
	//------------------------------------------------------------------------
	inline bool HaveFramebufferDriver(void)
	{
		struct stat st;

		return stat("/sys/class/graphics", &st) == 0;
	}
}

//============================================================================
// CDisplayDriver:
//============================================================================
//----------------------------------------------------------------------------
CDisplayDriver::CDisplayDriver(CDisplayModule* pModule) :
	pModule_(pModule),
	pdcVisible_(NULL),
	dbg_(kGroupDisplay) 
{
	dbg_.SetDebugLevel(kDisplayDebugLevel);
}

//----------------------------------------------------------------------------
CDisplayDriver::~CDisplayDriver() 
{
}

//============================================================================
// CDisplayModule: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	if (HaveFramebufferDriver())
		pDriver = new CDisplayFB(this);
#ifdef LF1000
	else
		pDriver = new CDisplayLF1000(this);
#endif
	pDriver->InitModule();
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
	pDriver->DeInitModule();
	delete pDriver;
}

//----------------------------------------------------------------------------
U32	CDisplayModule::GetScreenSize()
{
	return pDriver->GetScreenSize();
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayModule::GetPixelFormat(void)
{
	return pDriver->GetPixelFormat();
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, U8 *pBuffer)
{
	return pDriver->CreateHandle(height, width, colorDepth, pBuffer);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	return pDriver->DestroyHandle(hndl, destroyBuffer);
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayModule::GetCurrentDisplayHandle()
{
	if (pdcVisible_ && pdcVisible_->flippedContext)
		return pdcVisible_->flippedContext;
	else
		return pdcVisible_;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	pDriver->pdcVisible_ = pdcVisible_;
	return pDriver->RegisterLayer(hndl, xPos, yPos);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegisterLayer(tDisplayHandle hndl)
{
	pDriver->pdcVisible_ = pdcVisible_;
	return pDriver->UnRegisterLayer(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Update(tDisplayContext* dc, int sx, int sy, int dx, int dy, int width, int height)
{
	pDriver->pdcVisible_ = pdcVisible_;
	return pDriver->Update(dc, sx, sy, dx, dy, width, height);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	pDriver->pdcVisible_ = pdcVisible_;

	tDisplayContext* ctx = (tDisplayContext*)hndl;
	if (ctx->openGLScaler || (ctx->isAllocated && !ctx->offset))
	{
		tErrType error = kNoImplErr;
		kernel_.LockMutex(gListMutex);
		std::list<tDisplayContext*>::iterator it = find(gDisplayList.begin(), gDisplayList.end(), hndl);
		if(it != gDisplayList.end())
		{
			gDisplayList.erase(it);
			gDisplayList.push_back(ctx);
			error = kNoErr;
		}
		kernel_.UnlockMutex(gListMutex);
		if(ctx->openGLScaler)
			pDriver->Update(ctx, 0, 0, ctx->x, ctx->y, ctx->width, ctx->height);
		else
			Invalidate(0, NULL);
		return error;
	} else {
		return pDriver->SwapBuffers(hndl, waitVSync);
	}
}

//----------------------------------------------------------------------------
Boolean	CDisplayModule::IsBufferSwapped(tDisplayHandle hndl)
{
	return pDriver->IsBufferSwapped(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, Boolean visible)
{
	return pDriver->SetWindowPosition(hndl, x, y, width, height, visible);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::GetWindowPosition(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, Boolean& visible)
{
	return pDriver->GetWindowPosition(hndl, x, y, width, height, visible);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetVideoScaler(tDisplayHandle hndl, U16 width, U16 height, Boolean centered)
{
	return pDriver->SetVideoScaler(hndl, width, height, centered);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::GetVideoScaler(tDisplayHandle hndl, U16& width, U16& height, Boolean& centered)
{
	return pDriver->GetVideoScaler(hndl, width, height, centered);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetAlpha(tDisplayHandle hndl, U8 level, Boolean enable)
{
	return pDriver->SetAlpha(hndl, level, enable);
}

//----------------------------------------------------------------------------
U8 	CDisplayModule::GetAlpha(tDisplayHandle hndl) const
{
	return pDriver->GetAlpha(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetBacklight(tDisplayScreen screen, S8 backlight)
{
	return pDriver->SetBacklight(screen, backlight);
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBacklight(tDisplayScreen screen)
{
	return pDriver->GetBacklight(screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetBrightness(tDisplayScreen screen, S8 brightness)
{
	(void)screen;
	(void)brightness;
	return kNoImplErr; // not implemented
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetContrast(tDisplayScreen screen, S8 contrast)
{
	(void)screen;
	(void)contrast;
	return kNoImplErr; // not implemented
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBrightness(tDisplayScreen screen)
{
	(void)screen;
	return 0; // not implemented
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetContrast(tDisplayScreen screen)
{
	(void)screen;
	return 0; // not implemented
}

//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
	return pDriver->InitOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL(void* pCtx)
{
	return pDriver->DeinitOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::EnableOpenGL(void* pCtx)
{
	return pDriver->EnableOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::DisableOpenGL(void* pCtx)
{
	return pDriver->DisableOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::UpdateOpenGL(void* pCtx)
{
	return pDriver->UpdateOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::WaitForDisplayAddressPatched(void)
{
	return pDriver->WaitForDisplayAddressPatched();
}

//----------------------------------------------------------------------------
void CDisplayModule::SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress)
{
	return pDriver->SetOpenGLDisplayAddress(DisplayBufferPhysicalAddress);
}

//----------------------------------------------------------------------------
U32	CDisplayModule::GetDisplayMem(tDisplayMem memtype)
{
	return pDriver->GetDisplayMem(memtype);
}

//----------------------------------------------------------------------------

EGLClientBuffer CDisplayModule::CreateEglClientBuffer(tDisplayHandle hndl)
{
	return pDriver->CreateEglClientBuffer(hndl);
}
//----------------------------------------------------------------------------
void CDisplayModule::DestroyEglClientBuffer(EGLClientBuffer egl_client_buffer)
{
	return pDriver->DestroyEglClientBuffer(egl_client_buffer);
}
//----------------------------------------------------------------------------

BrioOpenGLConfigPrivate* CDisplayModule::CreateBrioOpenGLConfigPrivate(U32 size1D, U32 size2D)
{
	return new BrioOpenGLConfigPrivate(size1D, size2D);
}

BrioOpenGLConfigPrivate* CDisplayModule::CreateBrioOpenGLConfigPrivate(enum tBrioOpenGLVersion brioOpenGLVersion)
{
	return new BrioOpenGLConfigPrivate(brioOpenGLVersion);
}

void CDisplayModule::DestroyBrioOpenGLConfigPrivate(BrioOpenGLConfigPrivate* brioOpenGLPrivate)
{
	delete brioOpenGLPrivate;
}

LF_END_BRIO_NAMESPACE()
// EOF
