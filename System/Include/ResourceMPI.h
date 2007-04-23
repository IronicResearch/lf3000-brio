#ifndef LF_BRIO_RESOURCEMPI_H
#define LF_BRIO_RESOURCEMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ResourceMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Kernel module. 
//
//==============================================================================

#include <SystemTypes.h>
//#include <SystemRsrcTypes.h>
#include <ResourceTypes.h>
#include <StringTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>
#include <EventMessage.h>
LF_BEGIN_BRIO_NAMESPACE()

	
//==============================================================================
// Class:
//		CResourceEventMessage
//
// Description:
//		Class that describes the format of all Resource Event Messages.
//		NOTE: Belongs in ResourceTypes.h, but needs to be elsewhere to avoid
//		a circular reference, since IEventMessage uses a tRsrcHndl
//==============================================================================
class CResourceEventMessage : public IEventMessage 
{
public:
	CResourceEventMessage( tEventType type, const tResourceMsgDat& data );
	virtual U16	GetSizeInBytes() const;

	tResourceMsgDat	resourceMsgData;
};


//==============================================================================
// CResourceMPI
//==============================================================================
class CResourceMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CResourceMPI(const IEventListener *pListener = NULL);
	virtual ~CResourceMPI();

	// Setting default search path & handlers
	void			SetDefaultURIPath(const CURI &pURIPath);

	// Searching for devices
	U16				GetNumDevices() const;
	U16				GetNumDevices(tDeviceType type) const;

	tDeviceHndl		FindFirstDevice() const;
	tDeviceHndl		FindFirstDevice(tDeviceType type) const;
	tDeviceHndl		FindNextDevice() const;

	const CString*	GetDeviceName(tDeviceHndl hndl) const;
	tDeviceType		GetDeviceType(tDeviceHndl hndl) const;

	tErrType		OpenDevice(tDeviceHndl hndl, 
								tOptionFlags openOptions=kNoOptionFlags,
								const IEventListener *pListener = kNull);  
	tErrType		CloseDevice(tDeviceHndl hndl);

	tErrType		OpenAllDevices(tOptionFlags openOptions=kNoOptionFlags,
								const IEventListener *pListener = kNull);  
	tErrType		CloseAllDevices();

	// Searching for packages
	U32				 GetNumRsrcPackages( 
								tRsrcPackageType type = kRsrcPackageTypeUndefined, 
								const CURI *pURIPath = kNull) const;

	tRsrcPackageHndl FindRsrcPackage(const CURI& packageURI,
								const CURI *pURIPath = kNull) const;
	tRsrcPackageHndl FindFirstRsrcPackage(tRsrcPackageType type, 
								const CURI *pURIPath = kNull) const;	
	tRsrcPackageHndl FindNextRsrcPackage() const;

	// Getting package info
	const CURI*		 GetRsrcPackageURI(tRsrcPackageHndl hndl) const;
	const CString*	 GetRsrcPackageName(tRsrcPackageHndl hndl) const;
	tRsrcPackageType GetRsrcPackageType(tRsrcPackageHndl hndl) const;
	tVersion		 GetRsrcPackageVersion(tRsrcPackageHndl hndl) const;
	const CString*	 GetRsrcPackageVersionStr(tRsrcPackageHndl hndl) const;
	U32				 GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl) const;
	U32				 GetRsrcPackageSizePacked(tRsrcPackageHndl hndl) const;

	// Opening & closing packages to find resources within them
	tErrType		OpenRsrcPackage(tRsrcPackageHndl hndl, 
								tOptionFlags openOptions=kNoOptionFlags,
								const IEventListener *pListener = kNull);  
  	tErrType		CloseRsrcPackage(tRsrcPackageHndl hndl);

	// Loading & unloading packages
	tErrType		LoadRsrcPackage(tRsrcPackageHndl hndl, 
								tOptionFlags loadOptions = kNoOptionFlags,
								const IEventListener *pListener = kNull);  
	tErrType		UnloadRsrcPackage(tRsrcPackageHndl hndl, 
								tOptionFlags unloadOptions = kNoOptionFlags,
								const IEventListener *pListener = kNull);  

	// Searching for resources among opened & loaded packages & devices
	void			SetSearchScope(eSearchScope scope); 	
	eSearchScope	GetSearchScope() const; 	
	U32				GetNumRsrcs(const CURI *pURIPath = kNull) const; 	
	U32				GetNumRsrcs(tRsrcType type, const CURI *pURIPath = kNull) const;

	tRsrcHndl		FindRsrc(const CURI &pRsrcURI, 
							const CURI *pURIPath = kNull) const;
	tRsrcHndl		FindFirstRsrc(const CURI *pURIPath = kNull) const;
	tRsrcHndl		FindFirstRsrc(tRsrcType type, const CURI *pURIPath = kNull) const;
	tRsrcHndl		FindNextRsrc() const;

	// Getting rsrc info
	const CURI*		GetRsrcURI(tRsrcHndl hndl) const;
	const CString*	GetRsrcName(tRsrcHndl hndl) const;
	tRsrcType		GetRsrcType(tRsrcHndl hndl) const;
	tVersion		GetRsrcVersion(tRsrcHndl hndl) const;
	const CString*	GetRsrcVersionStr(tRsrcHndl hndl) const;	
	U32				GetRsrcPackedSize(tRsrcHndl hndl) const;
	U32				GetRsrcUnpackedSize(tRsrcHndl hndl) const;
	tPtr			GetRsrcPtr(tRsrcHndl hndl) const;

	// Opening & closing resources without loading them
	tErrType		OpenRsrc(tRsrcHndl hndl, 
							tOptionFlags openOptions = kNoOptionFlags,
							const IEventListener *pListener = kNull);  
	tErrType		CloseRsrc(tRsrcHndl hndl);

	tErrType		ReadRsrc(tRsrcHndl hndl, void* pBuffer, 
							U32 numBytesRequested,
							U32 *pNumBytesActual = kNull,
							tOptionFlags readOptions = kNoOptionFlags,
							const IEventListener *pListener = kNull) const;  
	tErrType		SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
							tOptionFlags seekOptions = kNoOptionFlags) const;
	tErrType		WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
							U32 numBytesRequested, U32 *pNumBytesActual,
							tOptionFlags writeOptions = kNoOptionFlags,
							const IEventListener *pListener = kNull) const;  

// Loading & unloading resources
	tErrType		LoadRsrc(tRsrcHndl hndl, 
							tOptionFlags loadOptions = kNoOptionFlags,
							const IEventListener *pListener = kNull);  
	tErrType		UnloadRsrc(tRsrcHndl hndl, 
							tOptionFlags unloadOptions = kNoOptionFlags,
							const IEventListener *pListener = kNull);

	Boolean			RsrcIsLoaded(tRsrcHndl hndl) const;

	// Rsrc referencing FIXME: move to smartptr hndl class
	tErrType		AddRsrcRef(tRsrcHndl hndl);	
	tErrType		DeleteRsrcRef(tRsrcHndl hndl);
	U32				GetRsrcRefCount(tRsrcHndl hndl);

	// New rsrc creation/deletion
	tRsrcHndl		NewRsrc(tRsrcType rsrcType, void* pData);
	tErrType		DeleteRsrc(tRsrcHndl hndl);

private:
	class CResourceModule*	pModule_;
	U32						id_;
};


#endif // LF_BRIO_RESOURCEMPI_H

// eof
