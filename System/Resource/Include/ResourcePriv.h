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
#include <KernelMPI.h>
#include <ResourceTypes.h>

#include <vector>
#include <boost/scoped_array.hpp>

LF_BEGIN_BRIO_NAMESPACE()


//----------------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------------
class IEventListener;
struct DeviceDescriptor;
struct PackageDescriptor;
struct ResourceDescriptor;


//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
const CString	kResourceModuleName	= "Resource";
const tVersion	kResourceModuleVersion	= 2;

extern const char*	kBaseRsrcPath;
extern const char*	kCart1RsrcPath;



//----------------------------------------------------------------------------
// Platform-specific helper functions
//----------------------------------------------------------------------------
typedef std::vector<CPath>	MountPoints;
void GetDeviceMountPoints(MountPoints& mp);



//==============================================================================
class CResourceModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// MPI state storage
	VTABLE_EXPORT	U32				Register( );
	VTABLE_EXPORT	void			Unregister(U32 id);
	VTABLE_EXPORT	void			SetDefaultURIPath(U32 id, const CURI &pURIPath);
	VTABLE_EXPORT	tErrType		SetDefaultListener(U32 id, //FIXME/tp check interface
												const IEventListener *pListener);
	VTABLE_EXPORT	void			SetSynchronization(U32 id, eSynchState block);

	// Searching for devices
	VTABLE_EXPORT	U16				GetNumDevices(eDeviceType type) const;

	VTABLE_EXPORT	tDeviceHndl		FindFirstDevice(U32 id, eDeviceType type) const;
	VTABLE_EXPORT	tDeviceHndl		FindNextDevice(U32 id) const;

	VTABLE_EXPORT	const CString*	GetDeviceName(tDeviceHndl hndl) const;
	VTABLE_EXPORT	eDeviceType		GetDeviceType(tDeviceHndl hndl) const;

	VTABLE_EXPORT	tErrType		OpenDevice(U32 id, tDeviceHndl hndl, 
												tOptionFlags openOptions,
												const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		CloseDevice(U32 id, tDeviceHndl hndl);

	VTABLE_EXPORT	tErrType		OpenAllDevices(U32 id, tOptionFlags openOptions,
												const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		CloseAllDevices(U32 id);

	// Searching for packages
	VTABLE_EXPORT	U32				GetNumPackages(U32 id, ePackageType type, 
												const CURI *pURIPath) const;

	VTABLE_EXPORT	tPackageHndl	FindPackage(U32 id, const CURI& packageURI,
												const CURI *pURIPath) const;
	VTABLE_EXPORT	tPackageHndl	FindFirstPackage(U32 id, ePackageType type,
												const CURI *pURIPath) const;	
	VTABLE_EXPORT	tPackageHndl	FindNextPackage(U32 id) const;

	// Getting package info
	VTABLE_EXPORT	const CURI*		GetPackageURI(U32 id, tPackageHndl hndl) const;
	VTABLE_EXPORT	const CPath*	GetPackagePath(U32 id, tPackageHndl hndl) const;
	VTABLE_EXPORT	ePackageType	GetPackageType(U32 id, tPackageHndl hndl) const;
	VTABLE_EXPORT	tVersion		GetPackageVersion(U32 id, tPackageHndl hndl) const;
	VTABLE_EXPORT	U32				GetPackageSizeUnpacked(U32 id, tPackageHndl hndl) const;
	VTABLE_EXPORT	U32				GetPackageSizePacked(U32 id, tPackageHndl hndl) const;

	// Opening & closing packages to find resources within them
	VTABLE_EXPORT	tErrType		OpenPackage(U32 id, tPackageHndl hndl, 
													tOptionFlags openOptions,
													const IEventListener *pListener);  
  	VTABLE_EXPORT	tErrType		ClosePackage(U32 id, tPackageHndl hndl);

	// Loading & unloading packages
	VTABLE_EXPORT	tErrType		LoadPackage(U32 id, tPackageHndl hndl, 
													tOptionFlags loadOptions,
													const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		UnloadPackage(U32 id, tPackageHndl hndl, 
													tOptionFlags unloadOptions,
													const IEventListener *pListener);  

	// Searching for resources among opened & loaded packages & devices
	VTABLE_EXPORT	U32				GetNumRsrcs(U32 id, tRsrcType type,
												const CURI *pURIPath) const;

	VTABLE_EXPORT	tRsrcHndl		FindRsrc(U32 id, const CURI &pRsrcURI, 
												const CURI *pURIPath) const;
	VTABLE_EXPORT	tRsrcHndl		FindFirstRsrc(U32 id, tRsrcType type, 
												const CURI *pURIPath) const;
	VTABLE_EXPORT	tRsrcHndl		FindNextRsrc(U32 id) const;

	// Getting rsrc info
	VTABLE_EXPORT	const CURI*		GetURI(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	const CPath*	GetPath(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	tRsrcType		GetType(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	tVersion		GetVersion(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	U32				GetPackedSize(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	U32				GetUnpackedSize(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	tPtr			GetPtr(U32 id, tRsrcHndl hndl) const;

	// Opening & closing resources without loading them
	VTABLE_EXPORT	tErrType		OpenRsrc(U32 id, tRsrcHndl hndl, 
											tOptionFlags openOptions,
											const IEventListener *pListener,
											Boolean suppressDoneEvent = false);  
	VTABLE_EXPORT	tErrType		CloseRsrc(U32 id, tRsrcHndl hndl);

	VTABLE_EXPORT	tErrType		ReadRsrc(U32 id, tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
											U32 *pNumBytesActual,
											tOptionFlags readOptions,
											const IEventListener *pListener,
											Boolean suppressDoneEvent = false) const;  
	VTABLE_EXPORT	tErrType		SeekRsrc(U32 id, tRsrcHndl hndl, U32 numSeekBytes, 
											tOptionFlags seekOptions) const;
	VTABLE_EXPORT	U32				TellRsrc(U32 id, tRsrcHndl hndl) const;
	VTABLE_EXPORT	tErrType		WriteRsrc(U32 id, tRsrcHndl hndl, const void *pBuffer, 
											U32 numBytesRequested, U32 *pNumBytesActual,
											tOptionFlags writeOptions,
											const IEventListener *pListener) const;  

	// Loading & unloading resources
	VTABLE_EXPORT	tErrType		LoadRsrc(U32 id, tRsrcHndl hndl, tOptionFlags loadOptions,
											const IEventListener *pListener);  
	VTABLE_EXPORT	tErrType		UnloadRsrc(U32 id, tRsrcHndl hndl, 
											tOptionFlags unloadOptions,
											const IEventListener *pListener);

	VTABLE_EXPORT	Boolean			RsrcIsLoaded(U32 id, tRsrcHndl hndl) const;

	// New rsrc creation/deletion
	VTABLE_EXPORT	tRsrcHndl		AddNewRsrcToPackage( U32 id, tPackageHndl hPkg, 
										const CURI& fullRsrcURI, 
										tRsrcType rsrcType );
	  
	VTABLE_EXPORT	tRsrcHndl		AddRsrcToPackageFromFile( U32 id, tPackageHndl hPkg, 
											const CURI& fullRsrcURI, 
											tRsrcType rsrcType,
											const CPath& fullPath );

	VTABLE_EXPORT	tErrType		RemoveRsrcFromPackage( U32 id, tPackageHndl hPkg,
			 								const CURI& fullURI, 
			 								bool deleteRsrc);

	
private:
	void 	OpenDeviceImpl(U32 id, tDeviceHndl hndl, tOptionFlags openOptions);
	PackageDescriptor*	FindPackagePriv(U32 id, tPackageHndl hndl) const;
	ResourceDescriptor*	FindRsrcPriv(U32 id, tRsrcHndl hndl) const	;
	CDebugMPI			dbg_;
	CKernelMPI 			kernel_;
	tMutex     			mpiMutex_;
	boost::scoped_array<DeviceDescriptor>	pDevices_;

	// Limit object creation to the Module Manager interface functions
	CResourceModule();
	virtual ~CResourceModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};



LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
