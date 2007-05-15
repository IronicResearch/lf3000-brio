//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Display.cpp
//
// Description:
//		Configure emulation for the environment
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Display";


//============================================================================
// CDisplayModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CDisplayModule::GetModuleVersion() const
{
	return kDisplayModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CDisplayModule::GetModuleName() const
{
	return &kDisplayModuleName;
}

//----------------------------------------------------------------------------
const CURI* CDisplayModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
// Ctor & dtor
//============================================================================
CDisplayModule::CDisplayModule() : dbg_(kGroupDisplay)
{
	InitModule();	// delegate to platform or emulation initializer
}

//----------------------------------------------------------------------------
CDisplayModule::~CDisplayModule()
{
	CleanupModule(); // delegate to platform or emulation cleanup
}

//----------------------------------------------------------------------------
Boolean	CDisplayModule::IsValid() const
{
	return true;
}


//============================================================================
//----------------------------------------------------------------------------
U16 CDisplayModule::GetNumberOfScreens() const
{
	const U16 kLightningScreenCount = 1;
	return kLightningScreenCount;
}

//----------------------------------------------------------------------------
const tDisplayScreenStats* CDisplayModule::GetScreenStats(tDisplayScreen screen) const
{
	static const tDisplayScreenStats kLightningStats = {
							240,
							320,
							kPixelFormatARGB8888,
							0,
							"LCD"};
	return &kLightningStats;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	dbg_.DebugOut(kDbgLvlCritical, "Invalidate not implemented\n");
	return kNoImplErr;
}

//============================================================================
//----------------------------------------------------------------------------
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	dbg_.DebugOut(kDbgLvlCritical, "Invalidate not implemented\n");
	return kInvalidDisplayHandle;
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
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
								tDisplayHandle insertAfter, tDisplayScreen screen)
{
	dbg_.DebugOut(kDbgLvlCritical, "Register not implemented\n");
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
							 tDisplayZOrder initialZOrder, 
                             tDisplayScreen screen)
{
	dbg_.DebugOut(kDbgLvlCritical, "Register not implemented\n");
	return kNoImplErr;
}

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
tErrType CDisplayModule::LockBuffer(tDisplayHandle hndl)
{
	dbg_.DebugOut(kDbgLvlCritical, "LockBuffer not implemented\n");
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect)
{
	dbg_.DebugOut(kDbgLvlCritical, "UnlockBuffer not implemented\n");
	return kNoImplErr;
}



LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================

LF_USING_BRIO_NAMESPACE()

static CDisplayModule*	sinst = NULL;
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion version)
{
	if( sinst == NULL )
		sinst = new CDisplayModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance(ICoreModule* ptr)
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}


// EOF
