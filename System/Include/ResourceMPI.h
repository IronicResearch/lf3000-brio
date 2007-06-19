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
#include <ResourceTypes.h>
#include <StringTypes.h>
#include <CoreMPI.h>
#include <EventMessage.h>
LF_BEGIN_BRIO_NAMESPACE()

class IEventListener;

	
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
	CResourceMPI(eSynchState block = kBlocking,
				const IEventListener *pListener = NULL);
	virtual ~CResourceMPI();

	// Setting default search path & handlers
	void			SetDefaultURIPath(const CURI &pURIPath);

	// Searching for devices
	U16				GetNumDevices(eDeviceType type = kDeviceTypeAll) const;

	tDeviceHndl		FindFirstDevice(eDeviceType type = kDeviceTypeAll) const;
	tDeviceHndl		FindNextDevice() const;

	const CString*	GetDeviceName(tDeviceHndl hndl) const;
	eDeviceType		GetDeviceType(tDeviceHndl hndl) const;

	tErrType		OpenDevice(tDeviceHndl hndl, 
								tOptionFlags openOptions=kNoOptionFlags,
								const IEventListener *pListener = kNull);  
	tErrType		CloseDevice(tDeviceHndl hndl);

	tErrType		OpenAllDevices(tOptionFlags openOptions=kNoOptionFlags,
								const IEventListener *pListener = kNull);  
	tErrType		CloseAllDevices();

	// Searching for packages
	U32				GetNumPackages(ePackageType type = kPackageTypeAll, 
								const CURI *pURIPath = kNull) const;

	tPackageHndl	FindPackage(const CURI& packageURI,
								const CURI *pURIPath = kNull) const;
	tPackageHndl	FindFirstPackage(ePackageType type = kPackageTypeAll, 
								const CURI *pURIPath = kNull) const;	
	tPackageHndl	FindNextPackage() const;

	// Getting package info
	// FIXME/tp: Need Package Name???
	const CURI*		GetPackageURI(tPackageHndl hndl) const;
	ePackageType	GetPackageType(tPackageHndl hndl) const;
	tVersion		GetPackageVersion(tPackageHndl hndl) const;
//	U32				GetPackageSizeUnpacked(tPackageHndl hndl) const;
//	U32				GetPackageSizePacked(tPackageHndl hndl) const;

	// Opening & closing packages to find resources within them
	tErrType		OpenPackage(tPackageHndl hndl, 
								tOptionFlags openOptions=kNoOptionFlags,
								const IEventListener *pListener = kNull);  
  	tErrType		ClosePackage(tPackageHndl hndl);

	// Loading & unloading packages
	tErrType		LoadPackage(tPackageHndl hndl, 
								tOptionFlags loadOptions = kNoOptionFlags,
								const IEventListener *pListener = kNull);  
	tErrType		UnloadPackage(tPackageHndl hndl, 
								tOptionFlags unloadOptions = kNoOptionFlags,
								const IEventListener *pListener = kNull);  

	// Searching for resources among opened & loaded packages & devices
	U32				GetNumRsrcs(tRsrcType type = kRsrcTypeAll, 
								const CURI *pURIPath = kNull) const;

	tRsrcHndl		FindRsrc(const CURI &pRsrcURI, 
							const CURI *pURIPath = kNull) const;
	tRsrcHndl		FindFirstRsrc(tRsrcType type = kRsrcTypeAll, 
									const CURI *pURIPath = kNull) const;
	tRsrcHndl		FindNextRsrc() const;

	// Getting rsrc info
	const CURI*		GetURI(tRsrcHndl hndl) const;
	tRsrcType		GetType(tRsrcHndl hndl) const;
	tVersion		GetVersion(tRsrcHndl hndl) const;
	U32				GetPackedSize(tRsrcHndl hndl) const;
	U32				GetUnpackedSize(tRsrcHndl hndl) const;
	tPtr			GetPtr(tRsrcHndl hndl) const;

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

	// New rsrc creation/deletion
	tRsrcHndl		NewRsrc(tRsrcType rsrcType, void* pData);
	tErrType		DeleteRsrc(tRsrcHndl hndl);

protected:
	class CResourceModule*	pModule_;
	U32						id_;
};


#endif // LF_BRIO_RESOURCEMPI_H

// eof
