//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayEmul.cpp
//
// Description:
//		Configure display manager for emulation target
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// CDisplayModule: Emulation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	// Initialize display manager for emulation target
	// TODO:
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
	// On the target, this is where file descriptors get closed, etc
	// in emulation, you probably don't need to do anything here
}

//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
}

//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL()
{
}

tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width,
                                        tPixelFormat colorDepth, U8 *pBuffer)
{
    dbg_.DebugOut(kDbgLvlCritical, "CreateHandle not implemented\n");
    return kInvalidDisplayHandle;
}

tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                            tDisplayHandle insertAfter, tDisplayScreen screen)
{
    dbg_.DebugOut(kDbgLvlCritical, "Register not implemented\n");
    return kNoImplErr;
}

tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                             tDisplayZOrder initialZOrder,
                             tDisplayScreen screen)
{
    dbg_.DebugOut(kDbgLvlCritical, "Register not implemented\n");
    return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
    dbg_.DebugOut(kDbgLvlCritical, "Invalidate not implemented\n");
    return kNoImplErr;
}

//============================================================================
//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
    dbg_.DebugOut(kDbgLvlCritical, "UnRegister not implemented\n");
    return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
    dbg_.DebugOut(kDbgLvlCritical, "DestroyHandle not implemented\n");
    return kNoImplErr;
}

//----------------------------------------------------------------------------
U8* CDisplayModule::GetBuffer(tDisplayHandle hndl) const
{
    dbg_.DebugOut(kDbgLvlCritical, "GetBuffer not implemented\n");
    return NULL;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetHeight(tDisplayHandle hndl) const
{
    dbg_.DebugOut(kDbgLvlCritical, "GetHeight not implemented\n");
    return 0;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetWidth(tDisplayHandle hndl) const
{
    dbg_.DebugOut(kDbgLvlCritical, "GetWidth not implemented\n");
    return 0;
}

//----------------------------------------------------------------------------
U32 CDisplayModule::GetScreenSize()
{
	return (U32)((320<<16)|(240));
}

enum tPixelFormat CDisplayModule::GetPixelFormat(void)
{
	return kPixelFormatARGB8888;
}

LF_END_BRIO_NAMESPACE()
// EOF
