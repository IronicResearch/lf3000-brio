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
U32 CDisplayModule::GetScreenSize()
{
	return (U32)((320<<16)|(240));
}

LF_END_BRIO_NAMESPACE()
// EOF
