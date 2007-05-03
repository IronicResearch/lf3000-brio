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
Boolean CFontMPI::LoadFont(const CString* pName, tFontProp prop)
{
	if (!pModule_)
		return false;
	return pModule_->LoadFont(pName, prop);
}

//----------------------------------------------------------------------------
Boolean CFontMPI::UnloadFont()
{
	if (!pModule_)
		return false;
	return pModule_->UnloadFont();
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
Boolean	CFontMPI::DrawString(CString* pStr, int x, int y, void* pCtx)
{
	if (!pModule_)
		return false;
	return pModule_->DrawString(pStr, x, y, pCtx);
}

// EOF
