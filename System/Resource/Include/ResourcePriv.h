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
#include <DebugMPI.h>
LF_BEGIN_BRIO_NAMESPACE()


// Constants
const CString	kResourceModuleName	= "Resource";
const tVersion	kResourceModuleVersion	= 2;

//FIXME:tp private?
#define MAX_RSRC_URI_SIZE		128
#define MAX_RSRC_NAME_SIZE		80

struct tRsrcFileDescriptor {
	char 			uri[MAX_RSRC_URI_SIZE];
	char			name[MAX_RSRC_NAME_SIZE];
	tRsrcType		type;
	tVersion		version;
	U32				packedSize;
	U32				unpackedSize;
//	U32				id;					// not currently in use
};

struct tRsrcDescriptor {
	char 			uri[MAX_RSRC_URI_SIZE];
	char			name[MAX_RSRC_NAME_SIZE];
	tRsrcType		type;
	tVersion		version;
	U32				packedSize;
	U32				unpackedSize;
	U32				id;
	tPtr 			pRsrc;
	FILE			*pFile;
	//	need some sort of file pointer (read/seek/write)
	U32				useCount;			// load/unload
};

struct tRsrcDeviceDescriptor {
	char			uriBase[MAX_RSRC_URI_SIZE];
};

union tRsrcSearchPattern {
	char 			uri[MAX_RSRC_URI_SIZE];
	U32				id;
	tRsrcType		type;
	tRsrcHndl 		hndl;
};

struct tRsrcLoadLogEntry {
	tRsrcHndl		hndl;
	U32				taskID;
};


//==============================================================================
class CResourceModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT	void			SetDefaultURIPath(U32 id, const CURI &pURIPath);
	VTABLE_EXPORT	tErrType		SetDefaultListener(U32 id, //FIXME/tp check interface
												const IEventListener *pListener);

	// Searching for devices
	VTABLE_EXPORT	U16				GetNumDevices() const;
	VTABLE_EXPORT	U16				GetNumDevices(tDeviceType type) const;

	VTABLE_EXPORT	tDeviceHndl		FindFirstDevice() const;
	VTABLE_EXPORT	tDeviceHndl		FindFirstDevice(tDeviceType type) const;
	VTABLE_EXPORT	tDeviceHndl		FindNextDevice() const;

	VTABLE_EXPORT	const CString*	GetDeviceName(tDeviceHndl hndl) const;
	VTABLE_EXPORT	tDeviceType		GetDeviceType(tDeviceHndl hndl) const;

	VTABLE_EXPORT	tErrType		OpenDevice(tDeviceHndl hndl, 
												tOptionFlags openOptions,
												const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		CloseDevice(tDeviceHndl hndl);

	VTABLE_EXPORT	tErrType		OpenAllDevices(U32 id, tOptionFlags openOptions,
												const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		CloseAllDevices();

	// Searching for packages
	VTABLE_EXPORT	U32				GetNumRsrcPackages(tRsrcPackageType type, 
												const CURI *pURIPath) const;

	VTABLE_EXPORT	tRsrcPackageHndl FindRsrcPackage(const CURI& packageURI,
												const CURI *pURIPath) const;
	VTABLE_EXPORT	tRsrcPackageHndl FindFirstRsrcPackage(tRsrcPackageType type,
												const CURI *pURIPath) const;	
	VTABLE_EXPORT	tRsrcPackageHndl FindNextRsrcPackage() const;

	// Getting package info
	VTABLE_EXPORT	const CURI*		 GetRsrcPackageURI(tRsrcPackageHndl hndl) const;
	VTABLE_EXPORT	const CString*	 GetRsrcPackageName(tRsrcPackageHndl hndl) const;
	VTABLE_EXPORT	tRsrcPackageType GetRsrcPackageType(tRsrcPackageHndl hndl) const;
	VTABLE_EXPORT	tVersion		 GetRsrcPackageVersion(tRsrcPackageHndl hndl) const;
	VTABLE_EXPORT	const CString*	 GetRsrcPackageVersionStr(tRsrcPackageHndl hndl) const;
	VTABLE_EXPORT	U32				 GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl) const;
	VTABLE_EXPORT	U32				 GetRsrcPackageSizePacked(tRsrcPackageHndl hndl) const;

	// Opening & closing packages to find resources within them
	VTABLE_EXPORT	tErrType		OpenRsrcPackage(tRsrcPackageHndl hndl, 
													tOptionFlags openOptions,
													const IEventListener *pListener);  
  	VTABLE_EXPORT	tErrType		CloseRsrcPackage(tRsrcPackageHndl hndl);

	// Loading & unloading packages
	VTABLE_EXPORT	tErrType		LoadRsrcPackage(tRsrcPackageHndl hndl, 
													tOptionFlags loadOptions,
													const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		UnloadRsrcPackage(tRsrcPackageHndl hndl, 
													tOptionFlags unloadOptions,
													const IEventListener *pListener);  

	// Searching for resources among opened & loaded packages & devices
	VTABLE_EXPORT	void			SetSearchScope(U32 id, eSearchScope scope); 	
	VTABLE_EXPORT	eSearchScope	GetSearchScope(U32 id) const; 	
	VTABLE_EXPORT	U32				GetNumRsrcs(U32 id, const CURI *pURIPath) const; 	
	VTABLE_EXPORT	U32				GetNumRsrcs(U32 id, tRsrcType type,
												const CURI *pURIPath) const;

	VTABLE_EXPORT	tRsrcHndl		FindRsrc(U32 id, const CURI &pRsrcURI, 
												const CURI *pURIPath) const;
	VTABLE_EXPORT	tRsrcHndl		FindFirstRsrc(U32 id, const CURI *pURIPath) const;
	VTABLE_EXPORT	tRsrcHndl		FindFirstRsrc(U32 id, tRsrcType type, 
												const CURI *pURIPath) const;
	VTABLE_EXPORT	tRsrcHndl		FindNextRsrc(U32 id) const;

	// Getting rsrc info
	VTABLE_EXPORT	const CURI*		GetRsrcURI(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	const CString*	GetRsrcName(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	tRsrcType		GetRsrcType(tRsrcHndl hndl) const;
	VTABLE_EXPORT	tVersion		GetRsrcVersion(tRsrcHndl hndl) const;
	VTABLE_EXPORT	const CString*	GetRsrcVersionStr(tRsrcHndl hndl) const;	
	VTABLE_EXPORT	U32				GetRsrcPackedSize(tRsrcHndl hndl) const;
	VTABLE_EXPORT	U32				GetRsrcUnpackedSize(tRsrcHndl hndl) const;
	VTABLE_EXPORT	tPtr			GetRsrcPtr(tRsrcHndl hndl) const;

	// Opening & closing resources without loading them
	VTABLE_EXPORT	tErrType		OpenRsrc(U32 id, tRsrcHndl hndl, 
											tOptionFlags openOptions,
											const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		CloseRsrc(tRsrcHndl hndl);

	VTABLE_EXPORT	tErrType		ReadRsrc(U32 id, tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
											U32 *pNumBytesActual,
											tOptionFlags readOptions,
											const IEventListener *pListener) const;  
	VTABLE_EXPORT	tErrType		SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
											tOptionFlags seekOptions) const;
	VTABLE_EXPORT	tErrType		WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
											U32 numBytesRequested, U32 *pNumBytesActual,
											tOptionFlags writeOptions,
											const IEventListener *pListener) const;  

	// Loading & unloading resources
	VTABLE_EXPORT	tErrType		LoadRsrc(U32 id, tRsrcHndl hndl, tOptionFlags loadOptions,
											const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		UnloadRsrc(U32 id, tRsrcHndl hndl, 
											tOptionFlags unloadOptions,
											const IEventListener *pListener);

	VTABLE_EXPORT	Boolean			RsrcIsLoaded(tRsrcHndl hndl) const;

	// Rsrc referencing FIXME: move to smartptr hndl class
	VTABLE_EXPORT	tErrType		AddRsrcRef(tRsrcHndl hndl);	
	VTABLE_EXPORT	tErrType		DeleteRsrcRef(tRsrcHndl hndl);
	VTABLE_EXPORT	tErrType		GetRsrcRefCount(tRsrcHndl hndl);

	// New rsrc creation/deletion
	VTABLE_EXPORT	tRsrcHndl 		NewRsrc(tRsrcType rsrcType, void* pData);
	VTABLE_EXPORT	tErrType		DeleteRsrc(tRsrcHndl hndl);
	
	VTABLE_EXPORT	U32				Register( );
	
private:
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CResourceModule();
	virtual ~CResourceModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};



LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
