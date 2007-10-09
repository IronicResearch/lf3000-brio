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
// Globals
//============================================================================
namespace 
{	
	tDisplayContext*	pdcListHead = NULL;
}

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
	dbg_.SetDebugLevel(kDisplayDebugLevel);
	InitModule();	// delegate to platform or emulation initializer
	pdcListHead = NULL;
	pdcPrimary_ = reinterpret_cast<tDisplayContext*>
		(CreateHandle(240, 320, kPixelFormatARGB8888, NULL));
}

//----------------------------------------------------------------------------
CDisplayModule::~CDisplayModule()
{
	tDisplayContext*	pdc = pdcListHead;
	while (pdc != NULL)
	{
		DestroyHandle(pdc, true);
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}
	if (pdcPrimary_)
		DestroyHandle(pdcPrimary_, true);
	DeInitModule(); // delegate to platform or emulation cleanup
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
const tDisplayScreenStats* CDisplayModule::GetScreenStats(tDisplayScreen /*screen*/)
{
	U32 size = GetScreenSize();
	enum tPixelFormat format = GetPixelFormat();

	if(format == kPixelFormatError) //FIXME: is this the right thing to do?
		dbg_.DebugOut(kDbgLvlCritical, "unknown PixelFormat returned\n");

	static const tDisplayScreenStats kLightningStats = {
							(size>>16),
							(size & 0xFFFF),
							format,
							(size & 0xFFFF) * 4,
							"LCD"};
	return &kLightningStats;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::LockBuffer(tDisplayHandle /*hndl*/)
{
	// Nothing to do when no 2D accelerator
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnlockBuffer(tDisplayHandle hndl, tRect* /*pDirtyRect*/)
{
	// Nothing to do when no 2D accelerator
	return kNoErr;
}

//----------------------------------------------------------------------------
// Linked list management for all display contexts
//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                            tDisplayHandle insertAfter, tDisplayScreen screen)
{
	// Register HW or emulation layer
	RegisterLayer(hndl, xPos, yPos);
	
	// Insert display context at head or tail of linked list
	tDisplayContext* 	dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext*	pdc = pdcListHead;
	tDisplayContext*	pdcAfter = reinterpret_cast<tDisplayContext*>(insertAfter);
	if (pdcListHead == NULL)
	{
		// Start from empty list
		pdcListHead = dc;
		dc->pdc = NULL;
	}
	else while (pdc != NULL)
	{
		// Walk list to insert after selected handle, or at tail
		if (pdc == pdcAfter || pdc->pdc == NULL)
		{
			dc->pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
			pdc->pdc = reinterpret_cast<tDisplayContext*>(dc);
			break;
		}
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                             tDisplayZOrder initialZOrder,
                             tDisplayScreen screen)
{
	// Register HW or emulation layer
	RegisterLayer(hndl, xPos, yPos);
	
	// Insert display context at head or tail of linked list
	tDisplayContext* 	dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext*	pdc = pdcListHead;
	if (pdcListHead == NULL)
	{
		// Start from empty list
		pdcListHead = dc;
		dc->pdc = NULL;
	}
	else if (kDisplayOnTop == initialZOrder)
	{
		// Replace previous head of list
		pdcListHead = dc;
		dc->pdc = pdc;
	}
	else while (pdc != NULL)
	{
		// Walk list to insert at tail
		if (pdc->pdc == NULL)
		{
			pdc->pdc = dc;
			dc->pdc = NULL;
			break;
		}
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	// UnRegister HW or emulation layer
	UnRegisterLayer(hndl);
	
	tDisplayContext*	dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext*	pdc = pdcListHead;
	tDisplayContext*	pdcPrev = pdcListHead;
	tDisplayContext*	pdcNext;

	// Remove display context from linked list
	while (pdc != NULL)
	{
		if (pdc == dc)
		{
			// Link next context pointer to previous context
			pdcNext = reinterpret_cast<tDisplayContext*>(pdc->pdc);
			pdcPrev->pdc = reinterpret_cast<tDisplayContext*>(pdcNext);
			// List is empty again?
			if (pdcPrev == pdcListHead)
				pdcListHead = NULL;
			break;
		}
		pdcPrev = pdc;
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	tDisplayContext*	pdc = pdcListHead;
	tErrType		 	rc = kNoErr;

	// Walk list of display contexts to update screen
	while (pdc != NULL)
	{
		rc = Update(pdc);
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}
	return rc;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CDisplayModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CDisplayModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG


// EOF
