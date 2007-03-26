
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



// EOF

