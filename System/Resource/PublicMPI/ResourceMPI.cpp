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
// CResourceEventMessage
//============================================================================
//------------------------------------------------------------------------------
CResourceEventMessage::CResourceEventMessage( tEventType type, const tResourceMsgDat& data ) 
	: IEventMessage(type, 0)
{
	resourceMsgData = data;
}

//------------------------------------------------------------------------------
U16	CResourceEventMessage::GetSizeInBytes() const
{
	return sizeof(CResourceEventMessage);
}



//============================================================================
//----------------------------------------------------------------------------
CResourceMPI::CResourceMPI(const IEventListener *pListener) : mpModule(NULL)
{
	tErrType		err;
	
	ICoreModule*	pModule;
	err = Module::Connect(pModule, kResourceModuleName, kResourceModuleVersion);
	if (kNoErr == err)
	{
		mpModule = reinterpret_cast<CResourceModule*>(pModule);
		mId = mpModule->Register();
		mpModule->SetDefaultListener(mId, pListener);
	}
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
		return kMPINotConnectedErr;
	return mpModule->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CResourceMPI::SetDefaultURIPath(const CURI &pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->SetDefaultURIPath(mId, pURIPath);
}

	// Searching for devices
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumDevices(U16 *pCount)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetNumDevices(pCount);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumDevices(U16 *pCount, tDeviceType type)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetNumDevices(pCount, type);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::FindDevice(tDeviceHndl *pHndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindDevice(pHndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindDevice(tDeviceHndl *pHndl, tDeviceType type)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindDevice(pHndl, type);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindNextDevice(tDeviceHndl *pHndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindNextDevice(pHndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::GetDeviceName(const CString **ppName, tDeviceHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetDeviceName(ppName, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetDeviceType(tDeviceType *pType, tDeviceHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetDeviceType(pType, hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->OpenDevice(hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseDevice(tDeviceHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->CloseDevice(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenAllDevices(tOptionFlags openOptions,
										const IEventListener *pListener)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->OpenAllDevices(mId, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseAllDevices()
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->CloseAllDevices();
}

	// Searching for packages
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumRsrcPackages(U32 *pCount, 
									tRsrcPackageType type, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetNumRsrcPackages(pCount, type, pURIPath);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcPackage(tRsrcPackageHndl *pHndl,
									const CURI *pPackageURI,
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindRsrcPackage(pHndl, pPackageURI, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcPackage(tRsrcPackageHndl *pHndl, 
									tRsrcPackageType type,
									const CURI *pURIPath)	
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindRsrcPackage(pHndl, type, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindNextRsrcPackage(tRsrcPackageHndl *pHndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindNextRsrcPackage(pHndl);
}

	// Getting package info
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageURI(const CURI **ppURI, tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageURI(ppURI, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageName(const CString **ppName, tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageName(ppName, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageType(tRsrcPackageType *pType, tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageType(pType, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageVersion(tVersion *pVersion, tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageVersion(pVersion, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageVersionStr(const CString **ppVersionStr,
						tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageVersionStr(ppVersionStr, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageSizeUnpacked(U32 *pSize, tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageSizeUnpacked(pSize, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackageSizePacked(U32 *pSize, tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackageSizePacked(pSize, hndl);
}

	// Opening & closing packages to find resources within them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->OpenRsrcPackage(hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseRsrcPackage(tRsrcPackageHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->CloseRsrcPackage(hndl);
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->LoadRsrcPackage(hndl, loadOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->UnloadRsrcPackage(hndl,unloadOptions, pListener);
}

	// Searching for resources among opened & loaded packages & devices
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumRsrcs(U32 *pCount, 
									const CURI *pURIPath) 	
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetNumRsrcs(mId, pCount, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetNumRsrcs(U32 *pCount, tRsrcType type,
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetNumRsrcs(mId, pCount, type, pURIPath);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrc(tRsrcHndl &hndl, const CURI &pRsrcURI,
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindRsrc(mId, hndl, pRsrcURI, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrc(tRsrcHndl &hndl, tRsrcID rsrcID,
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindRsrc(mId, hndl, rsrcID, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcs(tRsrcHndl &hndl, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindRsrcs(mId, hndl, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindRsrcs(tRsrcHndl &hndl, tRsrcType type, 
									const CURI *pURIPath)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindRsrcs(mId, hndl, type, pURIPath);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::FindNextRsrc(tRsrcHndl &hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->FindNextRsrc(mId, hndl);
}

	// Getting rsrc info
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcURI(ConstPtrCURI &pURI, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcURI(mId, pURI, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcName(ConstPtrCString &pName, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcName(mId, pName, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcID(tRsrcID &id, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcID(id, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcType(tRsrcType &rsrcType, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcType(rsrcType, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcVersion(tVersion &version, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcVersion(version, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcVersionStr(ConstPtrCString &pVersionStr, tRsrcHndl hndl)	
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcVersionStr(pVersionStr, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPackedSize(U32& pSize, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPackedSize(pSize, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcUnpackedSize(U32& pSize, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcUnpackedSize(pSize, hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcPtr(tPtr &pRsrc, tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcPtr(pRsrc, hndl);
}

	// Opening & closing resources without loading them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenRsrc(tRsrcHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->OpenRsrc(mId, hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseRsrc(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->CloseRsrc(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
									U32 *pNumBytesActual,
									tOptionFlags readOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->ReadRsrc(mId, hndl, pBuffer, numBytesRequested, pNumBytesActual,
									readOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->SeekRsrc(hndl, numSeekBytes, seekOptions);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->WriteRsrc(hndl, pBuffer, numBytesRequested,pNumBytesActual,
									writeOptions, pListener);
}

	// Loading & unloading resources
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions,
									const IEventListener *pListener)  
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->LoadRsrc(mId, hndl, loadOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadRsrc(tRsrcHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pListener)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->UnloadRsrc(mId, hndl, unloadOptions, pListener);
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
		return kMPINotConnectedErr;
	return mpModule->AddRsrcRef(hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrcRef(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->DeleteRsrcRef(hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcRefCount(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->GetRsrcRefCount(hndl);
}

	// New rsrc creation/deletion
//----------------------------------------------------------------------------
tErrType CResourceMPI::NewRsrc(tRsrcType rsrcType, void* pRsrc, tRsrcHndl *pHndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->NewRsrc(rsrcType, pRsrc, pHndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrc(tRsrcHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
	return mpModule->DeleteRsrc(hndl);
}
	

// EOF
