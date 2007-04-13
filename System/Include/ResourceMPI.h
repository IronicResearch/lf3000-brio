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

#define MAX_RSRC_URI_SIZE		128
#define MAX_RSRC_NAME_SIZE		80

#include <SystemTypes.h>
//#include <SystemRsrcTypes.h>
#include <ResourceTypes.h>
#include <StringTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>
#include <EventMessage.h>
LF_BEGIN_BRIO_NAMESPACE()


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
// Class:
//		CResourceEventMessage
//
// Description:
//		Class that describes the format of all Resource Event Messages. 
//==============================================================================

class CResourceEventMessage : public IEventMessage 
{
public:
	CResourceEventMessage( tEventType type, const tResourceMsgDat& data );
	virtual U16	GetSizeInBytes() const;

	tResourceMsgDat	resourceMsgData;
};


class CResourceMPI : public ICoreMPI {
public:	

	// core functionality
	virtual	Boolean		IsValid() const;
	virtual tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CResourceMPI(const IEventListener *pListener = NULL);
	virtual ~CResourceMPI();

	// Setting default search path & handlers
	//@FIXME/tp: Original was to SetDefaultURIPath via string pointer???
	tErrType		SetDefaultURIPath(const CURI &pURIPath);

	// Searching for devices
	tErrType		GetNumDevices(U16 *pCount);
	tErrType		GetNumDevices(U16 *pCount, tDeviceType type);

	tErrType		FindDevice(tDeviceHndl *pHndl);
	tErrType		FindDevice(tDeviceHndl *pHndl, tDeviceType type);
	tErrType		FindNextDevice(tDeviceHndl *pHndl);

	tErrType		GetDeviceName(const CString **ppName, tDeviceHndl hndl);
	tErrType		GetDeviceType(tDeviceType *pType, tDeviceHndl hndl);

	tErrType		OpenDevice(tDeviceHndl hndl, 
						tOptionFlags openOptions=kNoOptionFlags,
						const IEventListener *pListener=kNull);  
	tErrType		CloseDevice(tDeviceHndl hndl);

	tErrType		OpenAllDevices(tOptionFlags openOptions=kNoOptionFlags,
						const IEventListener *pListener=kNull);  
	tErrType		CloseAllDevices();

	// Searching for packages
	tErrType		GetNumRsrcPackages(U32 *pCount, 
						tRsrcPackageType type=kRsrcPackageTypeUndefined, 
						const CURI *pURIPath=kNull);

	tErrType		FindRsrcPackage(tRsrcPackageHndl *pHndl, const CURI *pPackageURI,
							const CURI *pURIPath=kNull);
	tErrType		FindRsrcPackage(tRsrcPackageHndl *pHndl, tRsrcPackageType type, 
							const CURI *pURIPath=kNull);	
	tErrType		FindNextRsrcPackage(tRsrcPackageHndl *pHndl);

	// Getting package info
	tErrType		GetRsrcPackageURI(const CURI **ppURI, tRsrcPackageHndl hndl);
	tErrType		GetRsrcPackageName(const CString **ppName, tRsrcPackageHndl hndl);
	tErrType		GetRsrcPackageType(tRsrcPackageType *pType, tRsrcPackageHndl hndl);
	tErrType		GetRsrcPackageVersion(tVersion *pVersion, tRsrcPackageHndl hndl);
	tErrType		GetRsrcPackageVersionStr(const CString **ppVersionStr, 
						tRsrcPackageHndl hndl);
	tErrType		GetRsrcPackageSizeUnpacked(U32 *pSize, tRsrcPackageHndl hndl);
	tErrType		GetRsrcPackageSizePacked(U32 *pSize, tRsrcPackageHndl hndl);

	// Opening & closing packages to find resources within them
	tErrType		OpenRsrcPackage(tRsrcPackageHndl hndl, 
						tOptionFlags openOptions=kNoOptionFlags,
						const IEventListener *pListener=kNull);  
  	tErrType		CloseRsrcPackage(tRsrcPackageHndl hndl);

	// Loading & unloading packages
	tErrType		LoadRsrcPackage(tRsrcPackageHndl hndl, 
						tOptionFlags loadOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);  
	tErrType		UnloadRsrcPackage(tRsrcPackageHndl hndl, 
						tOptionFlags unloadOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);  

	// Searching for resources among opened & loaded packages & devices
	tErrType		GetNumRsrcs(U32 *pCount, 
						const CURI *pURIPath=kNull); 	
	tErrType		GetNumRsrcs(U32 *pCount, tRsrcType type,
						const CURI *pURIPath=kNull);

	tErrType		FindRsrc(tRsrcHndl &hndl, const CURI &pRsrcURI, 
						const CURI *pURIPath=kNull);
	tErrType		FindRsrc(tRsrcHndl &hndl, tRsrcID rsrcID,
						const CURI *pURIPath=kNull);
	tErrType		FindRsrcs(tRsrcHndl &hndl, 
						const CURI *pURIPath=kNull);
	tErrType		FindRsrcs(tRsrcHndl &hndl, tRsrcType type,
						const CURI *pURIPath=kNull);
	tErrType		FindNextRsrc(tRsrcHndl &hndl);

	// Getting rsrc info
	tErrType		GetRsrcURI(ConstPtrCURI &pURI, tRsrcHndl hndl);
	tErrType		GetRsrcName(ConstPtrCString &pName, tRsrcHndl hndl);
	tErrType		GetRsrcID(tRsrcID &id, tRsrcHndl hndl);
	tErrType		GetRsrcType(tRsrcType &rsrcType, tRsrcHndl hndl);
	tErrType 		GetRsrcVersion(tVersion &version, tRsrcHndl hndl);
	tErrType 		GetRsrcVersionStr(ConstPtrCString &pVersionStr, tRsrcHndl hndl);	
	tErrType 		GetRsrcPackedSize(U32& pSize, tRsrcHndl hndl);
	tErrType 		GetRsrcUnpackedSize(U32& pSize, tRsrcHndl hndl);
	tErrType		GetRsrcPtr(tPtr &pRsrc, tRsrcHndl hndl);

	// Opening & closing resources without loading them
	tErrType		OpenRsrc(tRsrcHndl hndl, 
						tOptionFlags openOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);  
	tErrType		CloseRsrc(tRsrcHndl hndl);

	tErrType		ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
						U32 *pNumBytesActual = kNull,
						tOptionFlags readOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);  
	tErrType		SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
						tOptionFlags seekOptions = kNoOptionFlags);
	tErrType		WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
						U32 numBytesRequested, U32 *pNumBytesActual,
						tOptionFlags writeOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);  

	// Loading & unloading resources
	tErrType		LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);  
	tErrType		UnloadRsrc(tRsrcHndl hndl, 
						tOptionFlags unloadOptions = kNoOptionFlags,
						const IEventListener *pListener=kNull);

	Boolean			RsrcIsLoaded(tRsrcHndl hndl);

	// Rsrc referencing FIXME: move to smartptr hndl class
	tErrType		AddRsrcRef(tRsrcHndl hndl);	
	tErrType		DeleteRsrcRef(tRsrcHndl hndl);
	U32				GetRsrcRefCount(tRsrcHndl hndl);

	// New rsrc creation/deletion
	tErrType 		NewRsrc(tRsrcType rsrcType, void* pRsrc, tRsrcHndl *pHndl);
	tErrType		DeleteRsrc(tRsrcHndl hndl);

private:
	class CResourceModule *mpModule;
	U32				mId;
};


#endif // LF_BRIO_RESOURCEMPI_H

// eof
