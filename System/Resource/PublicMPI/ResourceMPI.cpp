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
CResourceMPI::CResourceMPI(const IEventListener *pListener) : pModule_(NULL)
{
	tErrType		err;
	
	ICoreModule*	pModule;
	err = Module::Connect(pModule, kResourceModuleName, kResourceModuleVersion);
	if (kNoErr == err)
	{
		pModule_ = reinterpret_cast<CResourceModule*>(pModule);
		id_ = pModule_->Register();
		pModule_->SetDefaultListener(id_, pListener);
	}
}

//----------------------------------------------------------------------------
CResourceMPI::~CResourceMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CResourceMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CResourceMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CResourceMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CResourceMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CResourceMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
void CResourceMPI::SetDefaultURIPath(const CURI &pURIPath)
{
	if(pModule_)
		pModule_->SetDefaultURIPath(id_, pURIPath);
}

	// Searching for devices
//----------------------------------------------------------------------------
U16 CResourceMPI::GetNumDevices() const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumDevices();
}
//----------------------------------------------------------------------------
U16 CResourceMPI::GetNumDevices(tDeviceType type) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumDevices(type);
}

//----------------------------------------------------------------------------
tDeviceHndl CResourceMPI::FindFirstDevice() const
{
	if(!pModule_)
		return kInvalidDeviceHndl;
	return pModule_->FindFirstDevice();
}
//----------------------------------------------------------------------------
tDeviceHndl CResourceMPI::FindFirstDevice(tDeviceType type) const
{
	if(!pModule_)
		return kInvalidDeviceHndl;
	return pModule_->FindFirstDevice(type);
}
//----------------------------------------------------------------------------
tDeviceHndl CResourceMPI::FindNextDevice() const
{
	if(!pModule_)
		return kInvalidDeviceHndl;
	return pModule_->FindNextDevice();
}

//----------------------------------------------------------------------------
const CString* CResourceMPI::GetDeviceName(tDeviceHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetDeviceName(hndl);
}
//----------------------------------------------------------------------------
tDeviceType CResourceMPI::GetDeviceType(tDeviceHndl hndl) const
{
	if(!pModule_)
		return kRsrcDeviceTypeUndefined;
	return pModule_->GetDeviceType(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenDevice(hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseDevice(tDeviceHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseDevice(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenAllDevices(tOptionFlags openOptions,
										const IEventListener *pListener)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenAllDevices(id_, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseAllDevices()
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseAllDevices();
}

	// Searching for packages
//----------------------------------------------------------------------------
U32 CResourceMPI::GetNumRsrcPackages(tRsrcPackageType type, 
									const CURI *pURIPath) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumRsrcPackages(type, pURIPath);
}

//----------------------------------------------------------------------------
tRsrcPackageHndl CResourceMPI::FindRsrcPackage(const CURI& packageURI,
												const CURI *pURIPath) const
{
	if(!pModule_)
		return kInvalidRsrcPackageHndl;
	return pModule_->FindRsrcPackage(packageURI, pURIPath);
}
//----------------------------------------------------------------------------
tRsrcPackageHndl CResourceMPI::FindFirstRsrcPackage(tRsrcPackageType type,
												const CURI *pURIPath) const	
{
	if(!pModule_)
		return kInvalidRsrcPackageHndl;
	return pModule_->FindFirstRsrcPackage(type, pURIPath);
}
//----------------------------------------------------------------------------
tRsrcPackageHndl CResourceMPI::FindNextRsrcPackage() const
{
	if(!pModule_)
		return kInvalidRsrcPackageHndl;
	return pModule_->FindNextRsrcPackage();
}

	// Getting package info
//----------------------------------------------------------------------------
const CURI* CResourceMPI::GetRsrcPackageURI(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetRsrcPackageURI(hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetRsrcPackageName(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetRsrcPackageName(hndl);
}
//----------------------------------------------------------------------------
tRsrcPackageType CResourceMPI::GetRsrcPackageType(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return kRsrcPackageTypeUndefined;
	return pModule_->GetRsrcPackageType(hndl);
}
//----------------------------------------------------------------------------
tVersion CResourceMPI::GetRsrcPackageVersion(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetRsrcPackageVersion(hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetRsrcPackageVersionStr(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetRsrcPackageVersionStr(hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetRsrcPackageSizeUnpacked(hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetRsrcPackageSizePacked(tRsrcPackageHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetRsrcPackageSizePacked(hndl);
}

	// Opening & closing packages to find resources within them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenRsrcPackage(hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseRsrcPackage(tRsrcPackageHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseRsrcPackage(hndl);
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->LoadRsrcPackage(hndl, loadOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnloadRsrcPackage(hndl,unloadOptions, pListener);
}

	// Searching for resources among opened & loaded packages & devices
//----------------------------------------------------------------------------
void CResourceMPI::SetSearchScope(eSearchScope scope)
{
	if(pModule_)
		return pModule_->SetSearchScope(id_, scope);
}
//----------------------------------------------------------------------------
eSearchScope CResourceMPI::GetSearchScope() const
{
	if(!pModule_)
		return kUndefinedSearchScope;
	return pModule_->GetSearchScope(id_);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetNumRsrcs(const CURI *pURIPath) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumRsrcs(id_, pURIPath);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetNumRsrcs(tRsrcType type, const CURI *pURIPath) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumRsrcs(id_, type, pURIPath);
}

//----------------------------------------------------------------------------
tRsrcHndl CResourceMPI::FindRsrc(const CURI &pRsrcURI, const CURI *pURIPath) const
{
	if(!pModule_)
		return kInvalidRsrcHndl;
	return pModule_->FindRsrc(id_, pRsrcURI, pURIPath);
}
//----------------------------------------------------------------------------
tRsrcHndl CResourceMPI::FindFirstRsrc(const CURI *pURIPath) const
{
	if(!pModule_)
		return kInvalidRsrcHndl;
	return pModule_->FindFirstRsrc(id_, pURIPath);
}
//----------------------------------------------------------------------------
tRsrcHndl CResourceMPI::FindFirstRsrc(tRsrcType type, const CURI *pURIPath) const
{
	if(!pModule_)
		return kInvalidRsrcHndl;
	return pModule_->FindFirstRsrc(id_, type, pURIPath);
}
//----------------------------------------------------------------------------
tRsrcHndl CResourceMPI::FindNextRsrc() const
{
	if(!pModule_)
		return kInvalidRsrcHndl;
	return pModule_->FindNextRsrc(id_);
}

	// Getting rsrc info
//----------------------------------------------------------------------------
const CURI* CResourceMPI::GetRsrcURI(tRsrcHndl hndl) const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetRsrcURI(id_, hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetRsrcName(tRsrcHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetRsrcName(id_, hndl);
}
//----------------------------------------------------------------------------
tRsrcType CResourceMPI::GetRsrcType(tRsrcHndl hndl) const
{
	if(!pModule_)
		return kInvalidRsrcType;
	return pModule_->GetRsrcType(hndl);
}
//----------------------------------------------------------------------------
tVersion CResourceMPI::GetRsrcVersion(tRsrcHndl hndl) const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetRsrcVersion(hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetRsrcVersionStr(tRsrcHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetRsrcVersionStr(hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetRsrcPackedSize(tRsrcHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetRsrcPackedSize(hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetRsrcUnpackedSize(tRsrcHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetRsrcUnpackedSize(hndl);
}
//----------------------------------------------------------------------------
tPtr CResourceMPI::GetRsrcPtr(tRsrcHndl hndl) const
{
	if(!pModule_)
		return kNull;
	return pModule_->GetRsrcPtr(hndl);
}

	// Opening & closing resources without loading them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenRsrc(tRsrcHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenRsrc(id_, hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseRsrc(tRsrcHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseRsrc(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
									U32 *pNumBytesActual,
									tOptionFlags readOptions,
									const IEventListener *pListener) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ReadRsrc(id_, hndl, pBuffer, numBytesRequested, pNumBytesActual,
									readOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SeekRsrc(hndl, numSeekBytes, seekOptions);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions,
									const IEventListener *pListener) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->WriteRsrc(hndl, pBuffer, numBytesRequested,pNumBytesActual,
									writeOptions, pListener);
}

	// Loading & unloading resources
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->LoadRsrc(id_, hndl, loadOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadRsrc(tRsrcHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pListener)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnloadRsrc(id_, hndl, unloadOptions, pListener);
}

//----------------------------------------------------------------------------
Boolean CResourceMPI::RsrcIsLoaded(tRsrcHndl hndl) const
{
	if(!pModule_)
		return false;
	return pModule_->RsrcIsLoaded(hndl);
}

	// Rsrc referencing FIXME: move to smartptr hndl class
//----------------------------------------------------------------------------
tErrType CResourceMPI::AddRsrcRef(tRsrcHndl hndl)	
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->AddRsrcRef(hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrcRef(tRsrcHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->DeleteRsrcRef(hndl);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::GetRsrcRefCount(tRsrcHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetRsrcRefCount(hndl);
}

	// New rsrc creation/deletion
//----------------------------------------------------------------------------
tRsrcHndl CResourceMPI::NewRsrc(tRsrcType rsrcType, void* pData)
{
	if(!pModule_)
		return kInvalidRsrcHndl;
	return pModule_->NewRsrc(rsrcType, pData);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrc(tRsrcHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->DeleteRsrc(hndl);
}
	

// EOF
