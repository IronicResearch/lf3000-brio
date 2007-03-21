#ifndef RSRCMGRMPI_H
#define RSRCMGRMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		RsrcMgrMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Kernel module. 
//
//==============================================================================

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemRsrcTypes.h>
#include <ResourceTypes.h>
#include <StringTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>

// @FIXME/dg: temp "backstore" for rsrcs during bringup
struct tRsrcDescriptor {
	char* 			uri;
	char*			name;
	U32				id;
	tRsrcType		type;
	tVersion		version;
	U32				packedSize;
	U32				unpackedSize;
	tPtr 			pRsrc;
};

// @FIXME/tp: Remove event context stuff?
typedef U32	tEventContext;
const tEventContext	kEventContextUndefined = 0;

 
class CRsrcMgrMPI : public ICoreMPI {
public:	
	// @FIXME/dg: temp bringup functionality to create "backstore"
	void	InstallArrayOfDescriptorPtrs(tRsrcDescriptor** pPtrArray, U32 numDescriptors);
	void	UninstallArrayOfDescriptorPtrs();

	// core functionality
	virtual	Boolean		IsValid() const;
	virtual tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CRsrcMgrMPI();
	virtual ~CRsrcMgrMPI();

	// Setting default search path & handlers
	//@FIXME/tp: Original was to SetDefaultURIPath via string pointer???
	tErrType		SetDefaultURIPath(const CURI &pURIPath);
	tErrType		SetDefaultEventHandler(const IEventListener *pEventHandler=kNull,
						tEventContext callerContext=kEventContextUndefined);

	// Searching for devices
	tErrType		GetNumDevices(U16 *pCount);
	tErrType		GetNumDevices(tDeviceType type, U16 *pCount);

	tErrType		FindDevice(tDeviceHndl *pHndl);
	tErrType		FindDevice(tDeviceType type, tDeviceHndl *pHndl);
	tErrType		FindNextDevice(tDeviceHndl *pHndl);

	tErrType		GetDeviceName(tDeviceHndl hndl, const CString **ppName);
	tErrType		GetDeviceType(tDeviceHndl hndl, tDeviceType *pType);

	tErrType		OpenDevice(tDeviceHndl hndl, 
						tOptionFlags openOptions=kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
	tErrType		CloseDevice(tDeviceHndl hndl);

	tErrType		OpenAllDevices(tOptionFlags openOptions=kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
	tErrType		CloseAllDevices();

	// Searching for packages
	tErrType		GetNumRsrcPackages(U32 *pCount, 
						tRsrcPackageType type=kRsrcPackageTypeUndefined, 
						const CURI *pURIPath=kNull);

	tErrType		FindRsrcPackage(const CURI *pPackageURI, tRsrcPackageHndl *pHndl,
							const CURI *pURIPath=kNull);
	tErrType		FindRsrcPackages(tRsrcPackageType type, tRsrcPackageHndl *pHndl, 
							const CURI *pURIPath=kNull);	
	tErrType		FindNextRsrcPackage(tRsrcPackageHndl *pHndl);

	// Getting package info
	tErrType		GetRsrcPackageURI(tRsrcPackageHndl hndl, const CURI **ppURI);
	tErrType		GetRsrcPackageName(tRsrcPackageHndl hndl, const CString **ppName);
	tErrType		GetRsrcPackageType(tRsrcPackageHndl hndl, tRsrcPackageType *pType);
	tErrType		GetRsrcPackageVersion(tRsrcPackageHndl hndl, tVersion *pVersion);
	tErrType		GetRsrcPackageVersionStr(tRsrcPackageHndl hndl, 
						const CString **ppVersionStr);
	tErrType		GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl, U32 *pSize);
	tErrType		GetRsrcPackageSizePacked(tRsrcPackageHndl hndl, U32 *pSize);

	// Opening & closing packages to find resources within them
	tErrType		OpenRsrcPackage(tRsrcPackageHndl hndl, 
						tOptionFlags openOptions=kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
  	tErrType		CloseRsrcPackage(tRsrcPackageHndl hndl);

	// Loading & unloading packages
	tErrType		LoadRsrcPackage(tRsrcPackageHndl hndl, 
						tOptionFlags loadOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
	tErrType		UnloadRsrcPackage(tRsrcPackageHndl hndl, 
						tOptionFlags unloadOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  

	// Searching for resources among opened & loaded packages & devices
	tErrType		GetNumRsrcs(U32 *pCount, 
						const CURI *pURIPath=kNull); 	
	tErrType		GetNumRsrcs(tRsrcType type, U32 *pCount, 
						const CURI *pURIPath=kNull);

	tErrType		FindRsrc(const CURI &pRsrcURI, tRsrcHndl &hndl, 
						const CURI *pURIPath=kNull);
	tErrType		FindRsrc(tRsrcID rsrcID, tRsrcHndl &hndl,
						const CURI *pURIPath=kNull);
	tErrType		FindRsrcs(tRsrcHndl &hndl, 
						const CURI *pURIPath=kNull);
	tErrType		FindRsrcs(tRsrcType type, tRsrcHndl &hndl, 
						const CURI *pURIPath=kNull);
	tErrType		FindNextRsrc(tRsrcHndl &hndl);

	// Getting rsrc info
	tErrType		GetRsrcURI(tRsrcHndl hndl, ConstPtrCURI &pURI);
	tErrType		GetRsrcName(tRsrcHndl hndl, ConstPtrCString &pName);
	tErrType		GetRsrcID(tRsrcHndl hndl, tRsrcID &id);
	tErrType		GetRsrcType(tRsrcHndl hndl, tRsrcType &rsrcType);
	tErrType 		GetRsrcVersion(tRsrcHndl hndl, tVersion &version);
	tErrType 		GetRsrcVersionStr(tRsrcHndl hndl, ConstPtrCString &pVersionStr);	
	tErrType 		GetRsrcPackedSize(tRsrcHndl hndl, U32& pSize);
	tErrType 		GetRsrcUnpackedSize(tRsrcHndl hndl, U32& pSize);
	tErrType		GetRsrcPtr(tRsrcHndl hndl, tPtr &pRsrc);

	// Opening & closing resources without loading them
	tErrType		OpenRsrc(tRsrcHndl hndl, 
						tOptionFlags openOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
	tErrType		CloseRsrc(tRsrcHndl hndl);

	tErrType		ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
						U32 *pNumBytesActual = kNull,
						tOptionFlags readOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
	tErrType		SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
						tOptionFlags seekOptions = kNoOptionFlags);
	tErrType		WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
						U32 numBytesRequested, U32 *pNumBytesActual,
						tOptionFlags writeOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  

	// Loading & unloading resources
	tErrType		LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);  
	tErrType		UnloadRsrc(tRsrcHndl hndl, 
						tOptionFlags unloadOptions = kNoOptionFlags,
						const IEventListener *pEventHandler=kNull,
						tEventContext eventContext=kEventContextUndefined);

	Boolean			RsrcIsLoaded(tRsrcHndl hndl);

	// Rsrc referencing FIXME: move to smartptr hndl class
	tErrType		AddRsrcRef(tRsrcHndl hndl);	
	tErrType		DeleteRsrcRef(tRsrcHndl hndl);
	U32				GetRsrcRefCount(tRsrcHndl hndl);

	// New rsrc creation/deletion
	tErrType 		NewRsrc(tRsrcType rsrcType, void* pRsrc, tRsrcHndl *pHndl);
	tErrType		DeleteRsrc(tRsrcHndl hndl);

private:
	class CRsrcMgrMPIImpl *mpImpl;
};


#endif // RSRCMGRMPI_H

// eof
