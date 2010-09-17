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

LF_BEGIN_BRIO_NAMESPACE()

namespace 
{
	static CDisplayLF1000*		pDriver = NULL;
}

//============================================================================
// CDisplayDriver:
//============================================================================
//----------------------------------------------------------------------------
//CDisplayDriver::CDisplayDriver() {};
//CDisplayDriver::~CDisplayDriver() {};

//============================================================================
// CDisplayModule: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	pDriver = new CDisplayLF1000(this);
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
/*void CDisplayModule::SetDirtyBit(int layer)
{
	return pDriver->SetDirtyBit(layer); // FIXME
}
*/
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
	return pDriver->SwapBuffers(hndl, waitVSync);
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
void CDisplayModule::InitOpenGL(void* pCtx)
{
	return pDriver->InitOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL()
{
	return pDriver->DeinitOpenGL();
}

//----------------------------------------------------------------------------
void CDisplayModule::EnableOpenGL(void* pCtx)
{
	return pDriver->EnableOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayModule::DisableOpenGL()
{
	return pDriver->DisableOpenGL();
}

//----------------------------------------------------------------------------
void CDisplayModule::UpdateOpenGL()
{
	return pDriver->UpdateOpenGL();
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

LF_END_BRIO_NAMESPACE()
// EOF
