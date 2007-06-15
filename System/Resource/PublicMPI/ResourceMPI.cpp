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
CResourceMPI::CResourceMPI(eSynchState block, const IEventListener *pListener)
		 : pModule_(NULL)
{
	tErrType		err;
	
	ICoreModule*	pModule;
	err = Module::Connect(pModule, kResourceModuleName, kResourceModuleVersion);
	if (kNoErr == err)
	{
		pModule_ = reinterpret_cast<CResourceModule*>(pModule);
		id_ = pModule_->Register();
		pModule_->SetSynchronization(id_, block);
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

//----------------------------------------------------------------------------
U16 CResourceMPI::GetNumDevices(eDeviceType type) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumDevices(type);
}

//----------------------------------------------------------------------------
tDeviceHndl CResourceMPI::FindFirstDevice(eDeviceType type) const
{
	if(!pModule_)
		return kInvalidDeviceHndl;
	return pModule_->FindFirstDevice(id_, type);
}
//----------------------------------------------------------------------------
tDeviceHndl CResourceMPI::FindNextDevice() const
{
	if(!pModule_)
		return kInvalidDeviceHndl;
	return pModule_->FindNextDevice(id_);
}

//----------------------------------------------------------------------------
const CString* CResourceMPI::GetDeviceName(tDeviceHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetDeviceName(hndl);
}
//----------------------------------------------------------------------------
eDeviceType CResourceMPI::GetDeviceType(tDeviceHndl hndl) const
{
	if(!pModule_)
		return kRsrcDeviceTypeInvalid;
	return pModule_->GetDeviceType(hndl);
}

//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenDevice(id_, hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::CloseDevice(tDeviceHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseDevice(id_, hndl);
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
	return pModule_->CloseAllDevices(id_);
}


//==============================================================================
// Packages
//==============================================================================
//----------------------------------------------------------------------------
U32 CResourceMPI::GetNumPackages(eRsrcPackageType type, 
									const CURI *pURIPath) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumPackages(id_, type, pURIPath);
}

//----------------------------------------------------------------------------
tPackageHndl CResourceMPI::FindPackage(const CURI& packageURI,
												const CURI *pURIPath) const
{
	if(!pModule_)
		return kInvalidPackageHndl;
	return pModule_->FindPackage(id_, packageURI, pURIPath);
}
//----------------------------------------------------------------------------
tPackageHndl CResourceMPI::FindFirstPackage(eRsrcPackageType type,
												const CURI *pURIPath) const	
{
	if(!pModule_)
		return kInvalidPackageHndl;
	return pModule_->FindFirstPackage(id_, type, pURIPath);
}
//----------------------------------------------------------------------------
tPackageHndl CResourceMPI::FindNextPackage() const
{
	if(!pModule_)
		return kInvalidPackageHndl;
	return pModule_->FindNextPackage(id_);
}

	// Getting package info
//----------------------------------------------------------------------------
const CURI* CResourceMPI::GetPackageURI(tPackageHndl hndl) const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetPackageURI(id_, hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetPackageName(tPackageHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetPackageName(id_, hndl);
}
//----------------------------------------------------------------------------
eRsrcPackageType CResourceMPI::GetPackageType(tPackageHndl hndl) const
{
	if(!pModule_)
		return kRsrcPackageTypeInvalid;
	return pModule_->GetPackageType(id_, hndl);
}
//----------------------------------------------------------------------------
tVersion CResourceMPI::GetPackageVersion(tPackageHndl hndl) const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetPackageVersion(id_, hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetPackageVersionStr(tPackageHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetPackageVersionStr(id_, hndl);
}
/*
//----------------------------------------------------------------------------
U32 CResourceMPI::GetPackageSizeUnpacked(tPackageHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetPackageSizeUnpacked(id_, hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetPackageSizePacked(tPackageHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetPackageSizePacked(id_, hndl);
}
*/

	// Opening & closing packages to find resources within them
//----------------------------------------------------------------------------
tErrType CResourceMPI::OpenPackage(tPackageHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenPackage(id_, hndl, openOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::ClosePackage(tPackageHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ClosePackage(id_, hndl);
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceMPI::LoadPackage(tPackageHndl hndl, 
									tOptionFlags loadOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->LoadPackage(id_, hndl, loadOptions, pListener);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::UnloadPackage(tPackageHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pListener)  
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnloadPackage(id_, hndl, unloadOptions, pListener);
}



//==============================================================================
// Resources
//==============================================================================
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
const CURI* CResourceMPI::GetURI(tRsrcHndl hndl) const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetURI(id_, hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetName(tRsrcHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetName(id_, hndl);
}
//----------------------------------------------------------------------------
tRsrcType CResourceMPI::GetType(tRsrcHndl hndl) const
{
	if(!pModule_)
		return kInvalidRsrcType;
	return pModule_->GetType(id_, hndl);
}
//----------------------------------------------------------------------------
tVersion CResourceMPI::GetVersion(tRsrcHndl hndl) const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetVersion(id_, hndl);
}
//----------------------------------------------------------------------------
const CString* CResourceMPI::GetVersionStr(tRsrcHndl hndl) const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetVersionStr(id_, hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetPackedSize(tRsrcHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetPackedSize(id_, hndl);
}
//----------------------------------------------------------------------------
U32 CResourceMPI::GetUnpackedSize(tRsrcHndl hndl) const
{
	if(!pModule_)
		return 0;
	return pModule_->GetUnpackedSize(id_, hndl);
}
//----------------------------------------------------------------------------
tPtr CResourceMPI::GetPtr(tRsrcHndl hndl) const
{
	if(!pModule_)
		return kNull;
	return pModule_->GetPtr(id_, hndl);
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
	return pModule_->CloseRsrc(id_, hndl);
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
	return pModule_->SeekRsrc(id_, hndl, numSeekBytes, seekOptions);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions,
									const IEventListener *pListener) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->WriteRsrc(id_, hndl, pBuffer, numBytesRequested,pNumBytesActual,
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
	return pModule_->RsrcIsLoaded(id_, hndl);
}


	// New rsrc creation/deletion
//----------------------------------------------------------------------------
tRsrcHndl CResourceMPI::NewRsrc(tRsrcType rsrcType, void* pData)
{
	if(!pModule_)
		return kInvalidRsrcHndl;
	return pModule_->NewRsrc(id_, rsrcType, pData);
}
//----------------------------------------------------------------------------
tErrType CResourceMPI::DeleteRsrc(tRsrcHndl hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->DeleteRsrc(id_, hndl);
}
	

// EOF
