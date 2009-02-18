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
const CURI kModuleURI = "/LF/System/Display";

//============================================================================
// Globals
//============================================================================
namespace
{
	tDisplayContext* pdcListHead = NULL;
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

	isOpenGLEnabled_ = false;
	isLayerSwapped_ = false;

	InitModule(); // delegate to platform or emulation initializer
	pdcListHead = NULL;
#ifdef EMULATION
	// We usually need an X pixmap primary surface for any context
	pdcPrimary_ = reinterpret_cast<tDisplayContext*>
	(CreateHandle(240, 320, kPixelFormatARGB8888, NULL));
#else
	// We only need a primary surface context for offscreen contexts
	// so we defer creating it on demand to conserve framebuffer mappings 
	pdcPrimary_ = NULL;
#endif
	pdcVisible_ = NULL;
}

//----------------------------------------------------------------------------
CDisplayModule::~CDisplayModule()
{
#if 0	// Skip removing display contexts for cleaner appearance on exits (TTP #2010)
	tDisplayContext* pdc = pdcListHead;
	while (pdc != NULL)
	{
		DestroyHandle(pdc, true);
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}
	if (pdcPrimary_)
	DestroyHandle(pdcPrimary_, true);
#endif
	DeInitModule(); // delegate to platform or emulation cleanup
}

//----------------------------------------------------------------------------
Boolean CDisplayModule::IsValid() const
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

	if(format == kPixelFormatError)
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
	(void )hndl; /* Prevent unused variable warnings. */
	// Nothing to do when no 2D accelerator
	return kNoErr;
}

//----------------------------------------------------------------------------
// Linked list management for all display contexts
//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
		tDisplayHandle insertAfter, tDisplayScreen screen)
{
	(void )screen; /* Prevent unused variable warnings. */

	// Insert display context at head or tail of linked list
	tDisplayContext* dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext* pdc = pdcListHead;
	tDisplayContext* pdcAfter = reinterpret_cast<tDisplayContext*>(insertAfter);
	if (pdcListHead == NULL)
	{
		// Start from empty list
		pdcListHead = dc;
		dc->pdc = NULL;
	}
	else while (pdc != NULL)
	{
		// Walk list to insert after selected handle, or at tail
		if ((pdc == pdcAfter || pdc->pdc == NULL) && pdc != dc) // no dupe ptrs
		{
			dc->pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
			pdc->pdc = reinterpret_cast<tDisplayContext*>(dc);
			break;
		}
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}

	// Default Z order is on top
	dc->isUnderlay = false;

	// Register HW or emulation layer
	RegisterLayer(hndl, xPos, yPos);

	// Need primary context to support offscreen contexts
	if (dc->isAllocated && pdcPrimary_ == NULL && pdcVisible_ == NULL)
	{
		pdcPrimary_ = reinterpret_cast<tDisplayContext*>
			(CreateHandle(240, 320, kPixelFormatARGB8888, NULL));
	}

	// FIXME: enable visibility for top layer in list

	// Track current onscreen display context
	//	pdcVisible_ = (!dc->isAllocated) ? dc : pdcPrimary_; 
	pdc = pdcListHead;
	while (pdc != NULL)
	{
		pdcVisible_ = (!pdc->isAllocated) ? pdc : pdcPrimary_;
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
		tDisplayZOrder initialZOrder,
		tDisplayScreen screen)
{
	(void )screen; /* Prevent unused variable warnings. */

	// Insert display context at head or tail of linked list
	tDisplayContext* dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext* pdc = pdcListHead;
	if (pdcListHead == NULL)
	{
		// Start from empty list
		pdcListHead = dc;
		dc->pdc = NULL;
	}
	else if (kDisplayOnBottom == initialZOrder)
	{
		// Replace previous head of list
		if (pdc != dc) // no dupe ptrs
		{
			pdcListHead = dc;
			dc->pdc = pdc;
		}
	}
	else while (pdc != NULL)
	{
		// Walk list to insert at tail
		if (pdc->pdc == NULL && pdc != dc) // no dupe ptrs
		{
			pdc->pdc = dc;
			dc->pdc = NULL;
			break;
		}
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}

	// Default Z order is on top
	dc->isUnderlay = (kDisplayOnBottom == initialZOrder);

	// Register HW or emulation layer
	RegisterLayer(hndl, xPos, yPos);

	// Need primary context to support offscreen contexts
	if (dc->isAllocated && pdcPrimary_ == NULL && pdcVisible_ == NULL)
	{
		pdcPrimary_ = reinterpret_cast<tDisplayContext*>
			(CreateHandle(240, 320, kPixelFormatARGB8888, NULL));
	}

	// FIXME: enable visibility for top layer in list

	// Track current onscreen display context
	// 	pdcVisible_ = (!dc->isAllocated) ? dc : pdcPrimary_;
	pdc = pdcListHead;
	while (pdc != NULL)
	{
		pdcVisible_ = (!pdc->isAllocated) ? pdc : pdcPrimary_;
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	(void )screen; /* Prevent unused variable warnings. */

	// UnRegister HW or emulation layer
	UnRegisterLayer(hndl);

	tDisplayContext* dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext* pdc = pdcListHead;
	tDisplayContext* pdcPrev = pdcListHead;
	tDisplayContext* pdcNext;

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

	// If list is empty of offscreen contexts, we don't need our own 
	// primary display context anymore
	if (pdcListHead == NULL && dc->isAllocated && pdcPrimary_ != NULL)
	{
		DestroyHandle(pdcPrimary_, true);
		pdcPrimary_ = NULL;
	}

	// FIXME: pdcVisible_ must always refer to top layer in list
	
	// Track current onscreen display context
	//	if (dc == pdcVisible_)
	//		pdcVisible_ = NULL;
	pdcVisible_ = NULL;
	pdc = pdcListHead;
	while (pdc != NULL)
	{
		pdcVisible_ = (!pdc->isAllocated) ? pdc : pdcPrimary_;
		pdc = reinterpret_cast<tDisplayContext*>(pdc->pdc);
	}

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	(void )screen; /* Prevent unused variable warnings. */
	tDisplayContext* pdc = pdcListHead;
	tErrType rc = kNoErr;

	// Walk list of display contexts to update screen
	while (pdc != NULL)
	{
		// Calculate active update region parameters based on dirty rect
		if (pDirtyRect != NULL) {
			// Destination rect = intersection of context rect with dirty rect
			int dx = std::max(pdc->rect.left, pDirtyRect->left);
			int dy = std::max(pdc->rect.top, pDirtyRect->top);
			int dx2 = std::min(pdc->rect.right, pDirtyRect->right);
			int dy2 = std::min(pdc->rect.bottom, pDirtyRect->bottom);
			int dw = dx2 - dx;
			int dh = dy2 - dy;
			// Effective source offset x,y for adjusted destination rect
			int sx = (dx> pdc->rect.left) ? dx - pdc->rect.left : 0;
			int sy = (dy> pdc->rect.top) ? dy - pdc->rect.top : 0;
			// Clip against adjusted destination and source coords
			if (dw> 0 && dh> 0 && sx < pdc->width && sy < pdc->height)
			rc = Update(pdc, sx, sy, dx, dy, dw, dh);
		}
		else
			rc = Update(pdc, 0, 0, pdc->x, pdc->y, pdc->width, pdc->height);
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

static CDisplayModule* sinst= NULL;

extern "C"
{
//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
	if (sinst == NULL)
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
