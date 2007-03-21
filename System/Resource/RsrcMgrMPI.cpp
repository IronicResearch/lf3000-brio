//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio Kernel module.
//
//==============================================================================

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>

#include <CoreMPI.h>
#include <KernelMPI.h>
#include <RsrcMgrMPI.h>
#include <ModuleRsrc.h>
#include <EventListener.h>

#include <string.h> // for strcmp
//static const CString kernelMPIVersion = "KernelMPI";

const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMpiName = "RsrcMgrMPI";


//----------------------------------------------------------------------------
class CRsrcMgrMPIImpl {
public:
	CRsrcMgrMPIImpl() : lastSearchHndl(0)	{}
	tRsrcHndl lastSearchHndl;
};

tRsrcDescriptor** rsrcDescPtrArray = NULL;
U32 rsrcDescPtrArraySize = 0;

//----------------------------------------------------------------------------
// @FIXME/dg: temp bringup functionality to create "backstore"
void	CRsrcMgrMPI::InstallArrayOfDescriptorPtrs(tRsrcDescriptor** pPtrArray, U32 numDescriptors)
{
	rsrcDescPtrArray = pPtrArray;
	rsrcDescPtrArraySize = numDescriptors;
}

//----------------------------------------------------------------------------
void	CRsrcMgrMPI::UninstallArrayOfDescriptorPtrs()
{
	rsrcDescPtrArray = NULL;
	rsrcDescPtrArraySize = 0;
}

//----------------------------------------------------------------------------
tRsrcDescriptor* GetRsrcDescriptor(tRsrcHndl hndl)
{
	U32 rsrcNum = (U32)hndl;

	if (rsrcNum > rsrcDescPtrArraySize)
		return NULL;
	
	return rsrcDescPtrArray[rsrcNum];
}


//----------------------------------------------------------------------------
CRsrcMgrMPI::CRsrcMgrMPI() : mpImpl(new CRsrcMgrMPIImpl)
{
}

//----------------------------------------------------------------------------
CRsrcMgrMPI::~CRsrcMgrMPI()
{
	delete mpImpl;
}

//----------------------------------------------------------------------------
Boolean	CRsrcMgrMPI::IsValid() const
{
	return (mpImpl != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CRsrcMgrMPI::GetMPIVersion(tVersion &version) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CRsrcMgrMPI::GetMPIName(ConstPtrCString &pName) const
{
	
	pName = &kMpiName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CRsrcMgrMPI::GetModuleVersion(tVersion &version) const
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CRsrcMgrMPI::GetModuleName(ConstPtrCString &pName) const
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CRsrcMgrMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType 	CRsrcMgrMPI::AddRsrcRef(tRsrcHndl hndl)
{
	return kNoErr;
}

tErrType	CRsrcMgrMPI::DeleteRsrcRef(tRsrcHndl hndl)
{
	return kNoErr;
}

Boolean	CRsrcMgrMPI::RsrcIsLoaded(tRsrcHndl hndl)
{
	return (GetRsrcDescriptor(hndl) != NULL) ? true : false;
}


tErrType	CRsrcMgrMPI::GetNumRsrcs(U32 *pCount, const CURI *pURIPath)
{
	if (pCount == NULL)
		return kInvalidParamErr;

	*pCount = rsrcDescPtrArraySize; 
	return kNoErr;
}
 	
tErrType	CRsrcMgrMPI::GetNumRsrcs(tRsrcType type, U32 *pCount, 
						const CURI *pURIPath)
{
	U32 index=0, count=0;
	tRsrcDescriptor *pRsrcDesc;

	if (pCount == NULL)
		return kInvalidParamErr;

	while (index < rsrcDescPtrArraySize)
	{
		if (rsrcDescPtrArray[index]->type == type)
			count++;
		index++;
	}

	*pCount = count; 
	return kNoErr;
}
 
tErrType	CRsrcMgrMPI::FindRsrc(const CURI &rsrcURI, tRsrcHndl &hndl,
						const CURI *pURIPath)
{
	Boolean bFound = false;
	 
	for (U16 count = 0; count < rsrcDescPtrArraySize &&	!bFound; ++count)
	{
		if (rsrcDescPtrArray[count]->name == rsrcURI)
		{
			hndl = (tRsrcHndl)(count);
			bFound = true;
		}
	}
	
	if (!bFound) 	
		return kInvalidParamErr;

	return kNoErr;
}
 
tErrType	CRsrcMgrMPI::FindRsrc(tRsrcID rsrcID, tRsrcHndl &hndl,
						const CURI *pURIPath)
{
	return kInvalidParamErr;
}

tErrType	CRsrcMgrMPI::FindRsrcs(tRsrcHndl &hndl, const CURI *pURIPath)
{
	hndl = 1;
	mpImpl->lastSearchHndl = hndl;

	return kNoErr;
}
 
tErrType	CRsrcMgrMPI::FindRsrcs(tRsrcType type, tRsrcHndl &hndl, 
						const CURI *pURIPath)
{
		return kInvalidParamErr;
}
 
tErrType	CRsrcMgrMPI::FindNextRsrc(tRsrcHndl &hndl)
{
		return kInvalidParamErr;
}
 
tErrType	CRsrcMgrMPI::GetRsrcURI(tRsrcHndl hndl, ConstPtrCURI& pURI)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	// FIXME/dg: finish	
	return kNoImplErr;
}

tErrType	CRsrcMgrMPI::GetRsrcName(tRsrcHndl hndl, ConstPtrCString & pName)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	// FIXME/dg: finish	
	return kNoImplErr;
}

tErrType 	CRsrcMgrMPI::GetRsrcID(tRsrcHndl hndl, tRsrcID &id)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	id = pRsrc->id;
	return kNoErr;
}

tErrType 	CRsrcMgrMPI::GetRsrcType(tRsrcHndl hndl, tRsrcType &rsrcType)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	rsrcType = pRsrc->type;
	return kNoErr;
}

tErrType 	CRsrcMgrMPI::GetRsrcVersion(tRsrcHndl hndl, tVersion &version)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	version = pRsrc->version;
	return kNoErr;
}

tErrType 	CRsrcMgrMPI::GetRsrcVersionStr(tRsrcHndl hndl, ConstPtrCString &pVersionStr)	
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	// FIXME/dg: finish	
	return kNoImplErr;
}

tErrType 	CRsrcMgrMPI::GetRsrcPackedSize(tRsrcHndl hndl, U32 &size)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	size = pRsrc->packedSize;
	return kNoErr;
}

tErrType 	CRsrcMgrMPI::GetRsrcUnpackedSize(tRsrcHndl hndl, U32 &size)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;

	size = pRsrc->unpackedSize;
	return kNoErr;
}

tErrType  	CRsrcMgrMPI::GetRsrcPtr(tRsrcHndl hndl, tPtr &pRsrcPtr)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kUnspecifiedErr;		

	if (pRsrcPtr == NULL)
		return kInvalidParamErr;

	pRsrcPtr = pRsrc->pRsrc;
	return kNoErr;
}


// EOF
