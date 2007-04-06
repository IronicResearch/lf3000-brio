//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ResourceMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Event Manager module.
//
//============================================================================

#include <StringTypes.h>
#include <SystemErrors.h>

#include <ResourceMPI.h>
#include <ResourcePriv.h>
#include <Module.h>


const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMPIName = "ResourceMPI";


//============================================================================
//----------------------------------------------------------------------------
CResourceMPI::CResourceMPI(const IEventListener *pEventHandler) : mpModule(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kResourceModuleName, kResourceModuleVersion);
	mpModule = reinterpret_cast<CResourceModule*>(pModule);
	mId = mpModule->Register();
}

//----------------------------------------------------------------------------
CResourceMPI::~CResourceMPI()
{
	Module::Disconnect(mpModule);
}

//----------------------------------------------------------------------------
Boolean	CResourceMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetMPIVersion(tVersion &version) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetMPIName(ConstPtrCString &pName) const
{
	pName = &kMPIName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetModuleVersion(tVersion &version) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CResourceMPI::SetDefaultURIPath(const CURI &pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->SetDefaultURIPath(mId, pURIPath);
}

	// Searching for devices
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumDevices(U16 *pCount)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetNumDevices(pCount);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumDevices(tDeviceType type, U16 *pCount)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetNumDevices(type, pCount);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::FindDevice(tDeviceHndl *pHndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindDevice(pHndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindDevice(tDeviceType type, tDeviceHndl *pHndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindDevice(type, pHndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindNextDevice(tDeviceHndl *pHndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindNextDevice(pHndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetDeviceName(tDeviceHndl hndl, const CString **ppName)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetDeviceName(hndl, ppName);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetDeviceType(tDeviceHndl hndl, tDeviceType *pType)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetDeviceType(hndl, pType);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->OpenDevice(hndl, openOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseDevice(tDeviceHndl hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->CloseDevice(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenAllDevices(tOptionFlags openOptions,
										const IEventListener *pEventHandler,
										tEventContext eventContext)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->OpenAllDevices(openOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseAllDevices()
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->CloseAllDevices();
}

	// Searching for packages
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumRsrcPackages(U32 *pCount, 
									tRsrcPackageType type, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetNumRsrcPackages(pCount, type, pURIPath);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcPackage(const CURI *pPackageURI,
									tRsrcPackageHndl *pHndl,
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindRsrcPackage(pPackageURI, pHndl, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcPackages(tRsrcPackageType type,
									tRsrcPackageHndl *pHndl, 
									const CURI *pURIPath)	
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindRsrcPackages(type, pHndl, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindNextRsrcPackage(tRsrcPackageHndl *pHndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindNextRsrcPackage(pHndl);
}

	// Getting package info
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageURI(tRsrcPackageHndl hndl, const CURI **ppURI)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageURI(hndl, ppURI);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageName(tRsrcPackageHndl hndl, const CString **ppName)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageName(hndl, ppName);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageType(tRsrcPackageHndl hndl, tRsrcPackageType *pType)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageType(hndl, pType);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageVersion(tRsrcPackageHndl hndl, tVersion *pVersion)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageVersion(hndl, pVersion);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageVersionStr(tRsrcPackageHndl hndl, 
									const CString **ppVersionStr)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageVersionStr(hndl, ppVersionStr);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl, U32 *pSize)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageSizeUnpacked(hndl, pSize);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageSizePacked(tRsrcPackageHndl hndl, U32 *pSize)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackageSizePacked(hndl, pSize);
}

	// Opening & closing packages to find resources within them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->OpenRsrcPackage(hndl, openOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseRsrcPackage(tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->CloseRsrcPackage(hndl);
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->LoadRsrcPackage(hndl, loadOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->UnloadRsrcPackage(hndl,unloadOptions, pEventHandler, eventContext);
}

	// Searching for resources among opened & loaded packages & devices
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumRsrcs(U32 *pCount, 
									const CURI *pURIPath) 	
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetNumRsrcs(mId, pCount, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumRsrcs(tRsrcType type, U32 *pCount, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetNumRsrcs(mId, type, pCount, pURIPath);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrc(const CURI &pRsrcURI, tRsrcHndl &hndl, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindRsrc(mId, pRsrcURI, hndl,pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrc(tRsrcID rsrcID, tRsrcHndl &hndl,
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindRsrc(mId, rsrcID, hndl, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcs(tRsrcHndl &hndl, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindRsrcs(mId, hndl, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcs(tRsrcType type, tRsrcHndl &hndl, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindRsrcs(mId, type, hndl, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindNextRsrc(tRsrcHndl &hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->FindNextRsrc(mId, hndl);
}

	// Getting rsrc info
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcURI(tRsrcHndl hndl, ConstPtrCURI &pURI)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcURI(mId, hndl, pURI);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcName(tRsrcHndl hndl, ConstPtrCString &pName)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcName(mId, hndl, pName);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcID(tRsrcHndl hndl, tRsrcID &id)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcID(hndl, id);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcType(tRsrcHndl hndl, tRsrcType &rsrcType)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcType(hndl, rsrcType);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcVersion(tRsrcHndl hndl, tVersion &version)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcVersion(hndl, version);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcVersionStr(tRsrcHndl hndl, ConstPtrCString &pVersionStr)	
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcVersionStr(hndl, pVersionStr);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackedSize(tRsrcHndl hndl, U32& pSize)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPackedSize(hndl, pSize);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcUnpackedSize(tRsrcHndl hndl, U32& pSize)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcUnpackedSize(hndl, pSize);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPtr(tRsrcHndl hndl, tPtr &pRsrc)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcPtr(hndl, pRsrc);
}

	// Opening & closing resources without loading them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenRsrc(tRsrcHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->OpenRsrc(hndl, openOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseRsrc(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->CloseRsrc(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
									U32 *pNumBytesActual,
									tOptionFlags readOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->ReadRsrc(hndl, pBuffer, numBytesRequested, pNumBytesActual,
									readOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->SeekRsrc(hndl, numSeekBytes, seekOptions);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->WriteRsrc(hndl, pBuffer, numBytesRequested,pNumBytesActual,
									writeOptions, pEventHandler, eventContext);
}

	// Loading & unloading resources
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->LoadRsrc(hndl, loadOptions, pEventHandler, eventContext);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadRsrc(tRsrcHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->UnloadRsrc(hndl, unloadOptions, pEventHandler, eventContext);
}

//----------------------------------------------------------------------------
Boolean CResourceMPI::RsrcIsLoaded(tRsrcHndl hndl)
{
	if(!mpModule)
		return false;
	return mpModule->RsrcIsLoaded(hndl);
}

	// Rsrc referencing FIXME: move to smartptr hndl class
//----------------------------------------------------------------------------
tErrType CResourceMPI::AddRsrcRef(tRsrcHndl hndl)	
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->AddRsrcRef(hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrcRef(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->DeleteRsrcRef(hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcRefCount(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetRsrcRefCount(hndl);
}

	// New rsrc creation/deletion
//----------------------------------------------------------------------------
tErrType CResourceMPI::NewRsrc(tRsrcType rsrcType, void* pRsrc, tRsrcHndl *pHndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->NewRsrc(rsrcType, pRsrc, pHndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrc(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->DeleteRsrc(hndl);
}
	

// EOF
