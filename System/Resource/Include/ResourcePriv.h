#ifndef LF_BRIO_RESOURCEMGRMODULEPRIV_H
#define LF_BRIO_RESOURCEMGRMODULEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ResourcePriv.h
//
// Description:
//		Defines the interface for the private underlying Resource module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <CoreModule.h>
//#include "ResourceMPI.h"	// for tResourceRegistrationFlags
LF_BEGIN_BRIO_NAMESPACE()


// Constants
const CString	kResourceModuleName	= "Resource";
const tVersion	kResourceModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CResourceModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	VTABLE_EXPORT	tErrType		SetDefaultURIPath(U32 id, const CURI &pURIPath);
	VTABLE_EXPORT	tErrType		SetDefaultEventHandler(U32 id, const IEventListener *pEventHandler=kNull,
									tEventContext callerContext=kEventContextUndefined);

	// Searching for devices
	VTABLE_EXPORT	tErrType		GetNumDevices(U16 *pCount);
	VTABLE_EXPORT	tErrType		GetNumDevices(tDeviceType type, U16 *pCount);

	VTABLE_EXPORT	tErrType		FindDevice(tDeviceHndl *pHndl);
	VTABLE_EXPORT	tErrType		FindDevice(tDeviceType type, tDeviceHndl *pHndl);
	VTABLE_EXPORT	tErrType		FindNextDevice(tDeviceHndl *pHndl);

	VTABLE_EXPORT	tErrType		GetDeviceName(tDeviceHndl hndl, const CString **ppName);
	VTABLE_EXPORT	tErrType		GetDeviceType(tDeviceHndl hndl, tDeviceType *pType);

	VTABLE_EXPORT	tErrType		OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions=kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
	VTABLE_EXPORT	tErrType		CloseDevice(tDeviceHndl hndl);

	VTABLE_EXPORT	tErrType		OpenAllDevices(tOptionFlags openOptions=kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
	VTABLE_EXPORT	tErrType		CloseAllDevices();

	// Searching for packages
	VTABLE_EXPORT	tErrType		GetNumRsrcPackages(U32 *pCount, 
									tRsrcPackageType type=kRsrcPackageTypeUndefined, 
									const CURI *pURIPath=kNull);

	VTABLE_EXPORT	tErrType		FindRsrcPackage(const CURI *pPackageURI, tRsrcPackageHndl *pHndl,
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrcPackages(tRsrcPackageType type, tRsrcPackageHndl *pHndl, 
									const CURI *pURIPath=kNull);	
	VTABLE_EXPORT	tErrType		FindNextRsrcPackage(tRsrcPackageHndl *pHndl);

	// Getting package info
	VTABLE_EXPORT	tErrType		GetRsrcPackageURI(tRsrcPackageHndl hndl, const CURI **ppURI);
	VTABLE_EXPORT	tErrType		GetRsrcPackageName(tRsrcPackageHndl hndl, const CString **ppName);
	VTABLE_EXPORT	tErrType		GetRsrcPackageType(tRsrcPackageHndl hndl, tRsrcPackageType *pType);
	VTABLE_EXPORT	tErrType		GetRsrcPackageVersion(tRsrcPackageHndl hndl, tVersion *pVersion);
	VTABLE_EXPORT	tErrType		GetRsrcPackageVersionStr(tRsrcPackageHndl hndl, 
									const CString **ppVersionStr);
	VTABLE_EXPORT	tErrType		GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl, U32 *pSize);
	VTABLE_EXPORT	tErrType		GetRsrcPackageSizePacked(tRsrcPackageHndl hndl, U32 *pSize);

	// Opening & closing packages to find resources within them
	VTABLE_EXPORT	tErrType		OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions=kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
  	VTABLE_EXPORT	tErrType		CloseRsrcPackage(tRsrcPackageHndl hndl);

	// Loading & unloading packages
	VTABLE_EXPORT	tErrType		LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
	VTABLE_EXPORT	tErrType		UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  

	// Searching for resources among opened & loaded packages & devices
	VTABLE_EXPORT	tErrType		GetNumRsrcs(U32 id, U32 *pCount, 
									const CURI *pURIPath=kNull); 	
	VTABLE_EXPORT	tErrType		GetNumRsrcs(U32 id, tRsrcType type, U32 *pCount, 
									const CURI *pURIPath=kNull);

	VTABLE_EXPORT	tErrType		FindRsrc(U32 id, const CURI &pRsrcURI, tRsrcHndl &hndl, 
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrc(U32 id, tRsrcID rsrcID, tRsrcHndl &hndl,
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrcs(U32 id, tRsrcHndl &hndl, 
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrcs(U32 id, tRsrcType type, tRsrcHndl &hndl, 
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindNextRsrc(U32 id, tRsrcHndl &hndl);

	// Getting rsrc info
	VTABLE_EXPORT	tErrType		GetRsrcURI(U32 id, tRsrcHndl hndl, ConstPtrCURI &pURI);
	VTABLE_EXPORT	tErrType		GetRsrcName(U32 id, tRsrcHndl hndl, ConstPtrCString &pName);
	VTABLE_EXPORT	tErrType		GetRsrcID(tRsrcHndl hndl, tRsrcID &id);
	VTABLE_EXPORT	tErrType		GetRsrcType(tRsrcHndl hndl, tRsrcType &rsrcType);
	VTABLE_EXPORT	tErrType 		GetRsrcVersion(tRsrcHndl hndl, tVersion &version);
	VTABLE_EXPORT	tErrType 		GetRsrcVersionStr(tRsrcHndl hndl, ConstPtrCString &pVersionStr);	
	VTABLE_EXPORT	tErrType 		GetRsrcPackedSize(tRsrcHndl hndl, U32& pSize);
	VTABLE_EXPORT	tErrType 		GetRsrcUnpackedSize(tRsrcHndl hndl, U32& pSize);
	VTABLE_EXPORT	tErrType		GetRsrcPtr(tRsrcHndl hndl, tPtr &pRsrc);

	// Opening & closing resources without loading them
	VTABLE_EXPORT	tErrType		OpenRsrc(tRsrcHndl hndl, 
									tOptionFlags openOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
	VTABLE_EXPORT	tErrType		CloseRsrc(tRsrcHndl hndl);

	VTABLE_EXPORT	tErrType		ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
									U32 *pNumBytesActual = kNull,
									tOptionFlags readOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
	VTABLE_EXPORT	tErrType		SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions = kNoOptionFlags);
	VTABLE_EXPORT	tErrType		WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  

	// Loading & unloading resources
	VTABLE_EXPORT	tErrType		LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);  
	VTABLE_EXPORT	tErrType		UnloadRsrc(tRsrcHndl hndl, 
									tOptionFlags unloadOptions = kNoOptionFlags,
									const IEventListener *pEventHandler=kNull,
									tEventContext eventContext=kEventContextUndefined);

	VTABLE_EXPORT	Boolean			RsrcIsLoaded(tRsrcHndl hndl);

	// Rsrc referencing FIXME: move to smartptr hndl class
	VTABLE_EXPORT	tErrType		AddRsrcRef(tRsrcHndl hndl);	
	VTABLE_EXPORT	tErrType		DeleteRsrcRef(tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcRefCount(tRsrcHndl hndl);

	// New rsrc creation/deletion
	VTABLE_EXPORT	tErrType 		NewRsrc(tRsrcType rsrcType, void* pRsrc, tRsrcHndl *pHndl);
	VTABLE_EXPORT	tErrType		DeleteRsrc(tRsrcHndl hndl);
	
	VTABLE_EXPORT	U32				Register( );
	
private:
	// Limit object creation to the Module Manager interface functions
	CResourceModule();
	virtual ~CResourceModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
