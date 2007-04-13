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
	VTABLE_EXPORT	tErrType		SetDefaultListener(U32 id, const IEventListener *pListener=kNull);

	// Searching for devices
	VTABLE_EXPORT	tErrType		GetNumDevices(U16 *pCount);
	VTABLE_EXPORT	tErrType		GetNumDevices(U16 *pCount, tDeviceType type);

	VTABLE_EXPORT	tErrType		FindDevice(tDeviceHndl *pHndl);
	VTABLE_EXPORT	tErrType		FindDevice(tDeviceHndl *pHndl, tDeviceType type);
	VTABLE_EXPORT	tErrType		FindNextDevice(tDeviceHndl *pHndl);

	VTABLE_EXPORT	tErrType		GetDeviceName(const CString **ppName, tDeviceHndl hndl);
	VTABLE_EXPORT	tErrType		GetDeviceType(tDeviceType *pType, tDeviceHndl hndl);

	VTABLE_EXPORT	tErrType		OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions=kNoOptionFlags,
									const IEventListener *pListener=kNull);  
	VTABLE_EXPORT	tErrType		CloseDevice(tDeviceHndl hndl);

	VTABLE_EXPORT	tErrType		OpenAllDevices(U32 id, tOptionFlags openOptions=kNoOptionFlags,
									const IEventListener *pListener=kNull);  
	VTABLE_EXPORT	tErrType		CloseAllDevices();

	// Searching for packages
	VTABLE_EXPORT	tErrType		GetNumRsrcPackages(U32 *pCount, 
									tRsrcPackageType type=kRsrcPackageTypeUndefined, 
									const CURI *pURIPath=kNull);

	VTABLE_EXPORT	tErrType		FindRsrcPackage(tRsrcPackageHndl *pHndl, const CURI *pPackageURI,
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrcPackage(tRsrcPackageHndl *pHndl, tRsrcPackageType type,
									const CURI *pURIPath=kNull);	
	VTABLE_EXPORT	tErrType		FindNextRsrcPackage(tRsrcPackageHndl *pHndl);

	// Getting package info
	VTABLE_EXPORT	tErrType		GetRsrcPackageURI(const CURI **ppURI, tRsrcPackageHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPackageName(const CString **ppName, tRsrcPackageHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPackageType(tRsrcPackageType *pType, tRsrcPackageHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPackageVersion(tVersion *pVersion, tRsrcPackageHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPackageVersionStr(const CString **ppVersionStr,
										tRsrcPackageHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPackageSizeUnpacked(U32 *pSize, tRsrcPackageHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPackageSizePacked(U32 *pSize, tRsrcPackageHndl hndl);

	// Opening & closing packages to find resources within them
	VTABLE_EXPORT	tErrType		OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions=kNoOptionFlags,
									const IEventListener *pListener=kNull);  
  	VTABLE_EXPORT	tErrType		CloseRsrcPackage(tRsrcPackageHndl hndl);

	// Loading & unloading packages
	VTABLE_EXPORT	tErrType		LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);  
	VTABLE_EXPORT	tErrType		UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);  

	// Searching for resources among opened & loaded packages & devices
	VTABLE_EXPORT	tErrType		GetNumRsrcs(U32 id, U32 *pCount, 
									const CURI *pURIPath=kNull); 	
	VTABLE_EXPORT	tErrType		GetNumRsrcs(U32 id, U32 *pCount, tRsrcType type,
									const CURI *pURIPath=kNull);

	VTABLE_EXPORT	tErrType		FindRsrc(U32 id, tRsrcHndl &hndl, const CURI &pRsrcURI, 
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrc(U32 id, tRsrcHndl &hndl, tRsrcID rsrcID,
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrcs(U32 id, tRsrcHndl &hndl, 
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindRsrcs(U32 id, tRsrcHndl &hndl, tRsrcType type, 
									const CURI *pURIPath=kNull);
	VTABLE_EXPORT	tErrType		FindNextRsrc(U32 id, tRsrcHndl &hndl);

	// Getting rsrc info
	VTABLE_EXPORT	tErrType		GetRsrcURI(U32 id, ConstPtrCURI &pURI, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcName(U32 id, ConstPtrCString &pName, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcID(tRsrcID &id, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcType(tRsrcType &rsrcType, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType 		GetRsrcVersion(tVersion &version, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType 		GetRsrcVersionStr(ConstPtrCString &pVersionStr, tRsrcHndl hndl);	
	VTABLE_EXPORT	tErrType 		GetRsrcPackedSize(U32& pSize, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType 		GetRsrcUnpackedSize(U32& pSize, tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcPtr(tPtr &pRsrc, tRsrcHndl hndl);

	// Opening & closing resources without loading them
	VTABLE_EXPORT	tErrType		OpenRsrc(U32 id, tRsrcHndl hndl, 
									tOptionFlags openOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);  
	VTABLE_EXPORT	tErrType		CloseRsrc(tRsrcHndl hndl);

	VTABLE_EXPORT	tErrType		ReadRsrc(U32 id, tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
									U32 *pNumBytesActual = kNull,
									tOptionFlags readOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);  
	VTABLE_EXPORT	tErrType		SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions = kNoOptionFlags);
	VTABLE_EXPORT	tErrType		WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);  

	// Loading & unloading resources
	VTABLE_EXPORT	tErrType		LoadRsrc(U32 id, tRsrcHndl hndl, tOptionFlags loadOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);  
	VTABLE_EXPORT	tErrType		UnloadRsrc(U32 id, tRsrcHndl hndl, 
									tOptionFlags unloadOptions = kNoOptionFlags,
									const IEventListener *pListener=kNull);

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
