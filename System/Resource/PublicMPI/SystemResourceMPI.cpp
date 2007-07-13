//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		SystemResourceMPI.cpp
//
// Description:
//		Extends the ResourceMPI to give Brio system modules special powers!
//		(Namely, access to all opened devices, packages and resources).
//
//============================================================================

#include <SystemResourceMPI.h>
#include <ResourcePriv.h>

LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "SystemResourceMPI";

//============================================================================
//----------------------------------------------------------------------------
CSystemResourceMPI::CSystemResourceMPI(eSynchState block, 
									const IEventListener *pListener)
		 : CResourceMPI(block, pListener)
{
	if (pModule_)
		pModule_->MakeOmniscient(id_);
}

//----------------------------------------------------------------------------
const CString* CSystemResourceMPI::GetMPIName() const
{
	return &kMPIName;
}	

LF_END_BRIO_NAMESPACE()
// EOF
