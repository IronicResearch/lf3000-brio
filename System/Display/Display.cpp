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
#include <list>

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
	std::list<tDisplayContext*>	gDisplayList;	// list of display contexts
	tMutex gListMutex = PTHREAD_MUTEX_INITIALIZER; // list mutex
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
	gDisplayList.clear();
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
#ifdef EMULATION	// Skip removing display contexts for cleaner appearance on exits (TTP #2010)
	tDisplayContext* pdc = NULL;
	std::list<tDisplayContext*>::iterator it;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		DestroyHandle(pdc, true);
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
	tDisplayContext* pdc = NULL;
	tDisplayContext* pdcAfter = reinterpret_cast<tDisplayContext*>(insertAfter);

	// Check for duplicate display handle and remove it first
	kernel_.LockMutex(gListMutex);
	std::list<tDisplayContext*>::iterator it;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (pdc == dc)
		{
			gDisplayList.erase(it);
			break;
		}
	}
	
	// Walk list to insert after selected handle, or at tail
	pdc = NULL;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (pdc == pdcAfter)
		{
			gDisplayList.insert(++it, dc);
			break;
		}
		pdc = NULL;
	}
	if (pdc == NULL)
		gDisplayList.push_back(dc);
	kernel_.UnlockMutex(gListMutex);

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

	// Track current onscreen display context
	kernel_.LockMutex(gListMutex);
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (!pdc->isAllocated)
			pdcVisible_ = pdc;
		else if (pdcPrimary_ != NULL)
			pdcVisible_ = pdcPrimary_;
	}
	kernel_.UnlockMutex(gListMutex);

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
	tDisplayContext* pdc = NULL;
	
	// Check for duplicate display handle and remove it first
	kernel_.LockMutex(gListMutex);
	std::list<tDisplayContext*>::iterator it;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (pdc == dc)
		{
			gDisplayList.erase(it);
			break;
		}
	}
	
	if (kDisplayOnBottom == initialZOrder)
		gDisplayList.push_front(dc);
	else
		gDisplayList.push_back(dc);
	kernel_.UnlockMutex(gListMutex);

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

	// Track current onscreen display context
	kernel_.LockMutex(gListMutex);
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (!pdc->isAllocated)
			pdcVisible_ = pdc;
		else if (pdcPrimary_ != NULL)
			pdcVisible_ = pdcPrimary_;
	}
	kernel_.UnlockMutex(gListMutex);

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	(void )screen; /* Prevent unused variable warnings. */

	// UnRegister HW or emulation layer
	UnRegisterLayer(hndl);

	tDisplayContext* dc = reinterpret_cast<tDisplayContext*>(hndl);
	tDisplayContext* pdc = NULL;

	// Remove display context from linked list
	kernel_.LockMutex(gListMutex);
	std::list<tDisplayContext*>::iterator it;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (pdc == dc)
		{
			gDisplayList.erase(it);
			break;
		}
	}
	kernel_.UnlockMutex(gListMutex);
	// display handle was never registered in list
	if (pdc == NULL)
		return kDisplayDisplayNotInListErr;

	// If list is empty of offscreen contexts, we don't need our own 
	// primary display context anymore
	if (gDisplayList.empty() && pdcPrimary_ != NULL)
	{
		// Note this calls Unregister() internally which won't be in list
		DestroyHandle(pdcPrimary_, true);
		pdcPrimary_ = NULL;
	}

	// pdcVisible_ must always refer to top layer in list
	kernel_.LockMutex(gListMutex);
	pdcVisible_ = NULL;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
		if (!pdc->isAllocated)
			pdcVisible_ = pdc;
		else if (pdcPrimary_ != NULL)
			pdcVisible_ = pdcPrimary_;
	}
	kernel_.UnlockMutex(gListMutex);

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	(void )screen; /* Prevent unused variable warnings. */
	tDisplayContext* pdc = NULL;
	tErrType rc = kNoErr;

	// Walk list of display contexts to update screen
	kernel_.LockMutex(gListMutex);
	std::list<tDisplayContext*>::iterator it;
	for (it = gDisplayList.begin(); it != gDisplayList.end(); it++)
	{
		pdc = *it;
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
	}
	kernel_.UnlockMutex(gListMutex);
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
