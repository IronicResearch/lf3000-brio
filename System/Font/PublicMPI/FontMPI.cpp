//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		FontMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Font Manager module.
//
//==============================================================================

#include <FontTypes.h>
#include <FontMPI.h>
#include <FontPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "FontMPI";


//============================================================================
// CFontMPI
//============================================================================
//----------------------------------------------------------------------------
CFontMPI::CFontMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kFontModuleName, kFontModuleVersion);
	pModule_ = reinterpret_cast<CFontModule*>(pModule);
}

//----------------------------------------------------------------------------
CFontMPI::~CFontMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CFontMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CFontMPI::GetModuleVersion() const
{
	if (!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CFontMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CFontMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================

//----------------------------------------------------------------------------
tErrType CFontMPI::SetFontResourcePath(const CPath &path)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetFontResourcePath(path);
}

//----------------------------------------------------------------------------
CPath* CFontMPI::GetFontResourcePath() const
{
	if (!pModule_)
		return kNull;
	return pModule_->GetFontResourcePath();
}

#if 0	// deprecated method
//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(const CString* pName, tFontProp prop)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(pName, prop);
}
#endif

//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(const CPath& name, tFontProp prop)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(name, prop);
}

//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(const CPath& name, U8 size)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(name, size);
}

//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(const CPath& name, U8 size, U32 encoding)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(name, size, encoding);
}

#if 0	// deprecated
//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(tRsrcHndl hRsrc, tFontProp prop)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(hRsrc, prop);
}

//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(tRsrcHndl hRsrc, U8 size)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(hRsrc, size);
}

//----------------------------------------------------------------------------
tFontHndl CFontMPI::LoadFont(tRsrcHndl hRsrc, U8 size, U32 encoding)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(hRsrc, size, encoding);
}
#endif

//----------------------------------------------------------------------------
Boolean CFontMPI::UnloadFont(tFontHndl hFont)
{
	if (!pModule_)
		return false;
	return pModule_->UnloadFont(hFont);
}

//----------------------------------------------------------------------------
Boolean CFontMPI::SelectFont(tFontHndl hFont)
{
	if (!pModule_)
		return false;
	return pModule_->SelectFont(hFont);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::SetFontAttr(tFontAttr attr)
{
	if (!pModule_)
		return false;
	return pModule_->SetFontAttr(attr);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::GetFontAttr(tFontAttr* pAttr)
{
	if (!pModule_)
		return false;
	return pModule_->GetFontAttr(pAttr);
}

//----------------------------------------------------------------------------
tFontAttr* CFontMPI::GetFontAttr()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetFontAttr();
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::SetFontColor(U32 color)
{
	if (!pModule_)
		return false;
	return pModule_->SetFontColor(color);
}

//----------------------------------------------------------------------------
U32 CFontMPI::GetFontColor()
{
	if (!pModule_)
		return 0;
	return pModule_->GetFontColor();
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::SetFontAntiAliasing(Boolean antialias)
{
	if (!pModule_)
		return false;
	return pModule_->SetFontAntiAliasing(antialias);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::GetFontAntiAliasing()
{
	if (!pModule_)
		return false;
	return pModule_->GetFontAntiAliasing();
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::SetFontKerning(Boolean kern)
{
	if (!pModule_)
		return false;
	return pModule_->SetFontKerning(kern);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::GetFontKerning()
{
	if (!pModule_)
		return false;
	return pModule_->GetFontKerning();
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::SetFontUnderlining(Boolean underline)
{
	if (!pModule_)
		return false;
	return pModule_->SetFontUnderlining(underline);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::GetFontUnderlining()
{
	if (!pModule_)
		return false;
	return pModule_->GetFontUnderlining();
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::DrawString(CString* pStr, S32 x, S32 y, tFontSurf* pCtx)
{
	if (!pModule_)
		return false;
	return pModule_->DrawString(pStr, x, y, pCtx);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::DrawString(CString& str, S32& x, S32& y, tFontSurf& surf)
{
	if (!pModule_)
		return false;
	return pModule_->DrawString(str, x, y, surf);
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::DrawString(CString& str, S32& x, S32& y, tFontSurf& surf, Boolean bWrap)
{
	if (!pModule_)
		return false;
	return pModule_->DrawString(str, x, y, surf, bWrap);
}

//----------------------------------------------------------------------------
S32 CFontMPI::GetX()
{
	if (!pModule_)
		return 0;
	return pModule_->GetX();
}

//----------------------------------------------------------------------------
S32 CFontMPI::GetY()
{
	if (!pModule_)
		return 0;
	return pModule_->GetY();
}

//----------------------------------------------------------------------------
Boolean CFontMPI::GetFontMetrics(tFontMetrics* pMtx)
{
	if (!pModule_)
		return 0;
	return pModule_->GetFontMetrics(pMtx);
}

//----------------------------------------------------------------------------
tFontMetrics* CFontMPI::GetFontMetrics()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetFontMetrics();
}

//----------------------------------------------------------------------------
Boolean	CFontMPI::GetStringRect(CString* pStr, tRect* pRect)
{
	if (!pModule_)
		return false;
	return pModule_->GetStringRect(pStr, pRect);
}

//----------------------------------------------------------------------------
tRect* CFontMPI::GetStringRect(CString& str)
{
	if (!pModule_)
		return kNull;
	return pModule_->GetStringRect(str);
}

LF_END_BRIO_NAMESPACE()

// EOF
