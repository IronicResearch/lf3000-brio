
//==============================================================================
// $Source: $
//
// Copyright (c) 2002,2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		Kernel.cpp
//
// Description:
//		Underlying implementation code used by the KernelMPI
//
//==============================================================================

#include <StringTypes.h>

#include <KernelPrivate.h>
#include <KernelModule.h>


extern tErrType GetModuleVersion(tVersion *pVersion);
extern tErrType GetModuleName(const CString **ppName);
extern tErrType GetModuleOrigin(const CURI **ppURI);
void			FirstKernelFunction();


tKernelMPIFcnTable KernelFunctionTable =
{
	&GetModuleVersion,
	&GetModuleName,
	&GetModuleOrigin,
	&FirstKernelFunction
};


void FirstKernelFunction()
{

}

// EOF

