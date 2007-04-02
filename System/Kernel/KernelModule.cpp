
//==============================================================================
// $Source: $
//
// Copyright (c) 2002,2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelModule.cpp
//
// Description:
//		Information shared amongst all MPIs in the Kernel Module
//
//==============================================================================

#include <StringTypes.h>

#include <KernelPrivate.h>
#include <KernelModule.h>


const CString kKernelModuleName = "Kernel Module";

//============================================================================
// Informational functions
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventModule::GetModuleVersion(tVersion &version) const
{
	version = kEventMgrModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kEventMgrModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}


namespace
{
	CKernelManagerImpl* pinst = NULL;
}


//============================================================================
// Ctor & dtor
//============================================================================
//----------------------------------------------------------------------------
CEventModule::CEventModule()
{
	if (pinst == NULL)
		pinst = new CEventManagerImpl;
}

//----------------------------------------------------------------------------
CEventModule::~CEventModule()
{
	delete pinst;
}

//----------------------------------------------------------------------------
Boolean	CEventModule::IsValid() const
{
	return (pinst != NULL) ? true : false;
}


#if 0 // FIXME/BSK
tErrType GetKernelModuleVersion(tVersion *pVersion)
{
//	pVersion->major = kKernelMajorModuleVersion;
//	pVersion->minor = kKernelMinorModuleVersion;
	return kNoErr;
};


tErrType GetKernelModuleName(const CString **ppName)
{
	*ppName = &kKernelModuleName;
	return kNoErr;
};


tErrType GetKernelModuleOrigin(const CURI **ppURI)
{
	// FIXME/dg: not defined yet? commented out to get it to build
	// *ppURI = &KernelModuleURI;
	return kNoErr;
};

#endif // FIXME/BSK

// EOF

