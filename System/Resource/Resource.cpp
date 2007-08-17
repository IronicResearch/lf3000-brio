//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		Resource.cpp
//
// Description:
//		Implements the Resource Manager.  This module serves as a singleton.
//
// Design:
//		MPIInstanceState: stores open device, open package, and search/iteration
//						state for a CResourceMPI instance.
//						Each MPI instance invokes this module's Register()
//						function in its ctor, which adds a new MPIInstanceState
//						to a global map and returns a U32 key used to retrieve.
//						that state.  All subsequenc calls to this module use
//						that U32 id as their first parameter.
//						Storing this information in this module rather than
//						in the CResourceMPI class allows us to freely add
//						new features, optimizations and bug fixes without
//						breaking existing code that links against the
//						CResourceMPI class.
//						
//		Resource caching:  The RAM cost of keeping a device open is fairly
//						low, and you can't really do much with the resource
//						manager without having an open device, so we leave
//						cache device state even when the reference count
//						for a device goes to zero.  Open packages require
//						more runtime RAM, and when a client closes a package
//						there is a high likelihood that they will not
//						reopen it (e.g., switching to a new game), so
//						memory is reclaimed when a package reference count
//						goes to zero.
//
//		Iterators		When counting or enumerating resources,
//						we limit the search domain to devices and packages
//						that the client CResourceMPI object has opened.
//						To better encapsulate this iteration this file
//						implements a series of iterator objects, where
//						the package iterator uses the device iterator,
//						and the resource iterator uses the package iterator.
//
//		Handle enumerations  Handles are not stored in resoures, instead
//						each package defines a base resource value and
//						an individual resource's handle is determined by
//						adding this base value to its index in the package's
//						vector of resources.  Dynamically adding or removing
//						resources from a package invalidates any previously
//						retrieved handle values (similar to insertions
//						or erasures invalidating existing STL iterators),
//						so we track whether any handles have been found
//						or enumerated in a package, and if so, we set
//						the base resource value to a new value to invalidate
//						any previously-retrieved handles.
//
// TODO: repeated resources should be searched for when opening packages
//  resrouces vector should be of shared_ptrs?
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>

#include <ResourcePriv.h>
#include <ResourceMPI.h>
#include <EventListener.h>
#include <EventMPI.h>

#include <stdio.h>
#include <string.h> // for strcmp
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <map>
#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
// Constants
//============================================================================
#ifdef LIGHTNING
	const size_t 	kDeviceCount	= 2;
#endif

const tVersion	kModuleVersion		= 2;
const CString	kModuleName			= "Resource";
const CURI		kModuleURI			= "/LF/System/Resource";
const CURI		kNullURI			= "";

extern const char*	kBaseRsrcPath 	= "/Base/rsrc/";
extern const char*	kCart1RsrcPath	= "/Cart1/rsrc/";
const char	kEnumPkgsFile[]			= "EnumPkgs";

const size_t	kMaxBrioPkgLine	= 128;	// FIXME/tp: verify
const U32		kMaxAddNewRsrcs = 26 * 26;


//============================================================================
// Resource state
//============================================================================
struct ResourceDescriptor
{
	CURI 				uri;
	std::string			caseInsensitiveRep;
	tRsrcType			type;
	CPath				path;		// relative path to save space
	U32					size;		// size of file "path"
	U32					usize;		// uncompressed size of reource
	tVersion			version;
	PackageDescriptor*	pPkg;		// containing package (lookup state to avoid duplication)
	U16					refCount;
	int					fd;
	tPtr				ptr;		// managed by LoadRsrc()/UnloadRsrc()
	
	ResourceDescriptor(const char* u, U32 t = kInvalidRsrcType, 
					const CPath& p = CPath(),
					U32 s = 0, U32 us = 0, tVersion v = 1, 
					PackageDescriptor* d = NULL)
		: uri(u), caseInsensitiveRep(uri.casefold_collate_key()), 
		type(t), path(p), size(s), usize(us), 
		version(v), pPkg(d), 
		refCount(0), fd(-1), ptr(0)
	{
	}
	bool operator<(const ResourceDescriptor& rhs) const
	{
		return caseInsensitiveRep < rhs.caseInsensitiveRep;
//		return uri.casefold_collate_key() < rhs.uri.casefold_collate_key();
		/* The basic space vs. speed tradeoff here.  
		 * Either use the caseInsensitiveRep member (more space, less time)
		 * or change this function to generate the casefold_collate_key()
		 * for comparisons (less space, more time)
		 */
	}
};
typedef std::vector<ResourceDescriptor>	ResourceDescriptors;


//============================================================================
// Package state
//============================================================================
struct PackageDescriptor
{
	CURI 				uri;
	std::string			caseInsensitiveRep;
	CPath				path;		// relative path to save space
	CPath				folder;		// parent folder of "path" (heavily used value)
	tVersion			version;
	ePackageType		type;
	tDeviceHndl			deviceHndl;	// handle of the containing device
	tPackageHndl		hndl;		// this package's handle value
	U16					refCount;
	ResourceDescriptors	resources;
	tRsrcHndl			startRsrcHndl;					// base resource handle for the pkg
	bool				dirtyWithRetrievedRsrcHndls;	// reset handle on add/remove rsrc?
	bool				dirtyWithChangedRsrcs;			// serialize out on Close?
	
	PackageDescriptor(const char* u, const CPath& p = CPath(), 
					tVersion v = 1, 
					ePackageType t = kPackageTypeInvalid,
					tDeviceHndl d = kInvalidDeviceHndl)
		: uri(u), caseInsensitiveRep(uri.casefold_collate_key()), 
		path(p), version(v), type(t), deviceHndl(d), 
		hndl(kInvalidPackageHndl), refCount(0), 
		startRsrcHndl(kInvalidRsrcHndl),
		dirtyWithRetrievedRsrcHndls(false),
		dirtyWithChangedRsrcs(false)
	{
		if(!path.empty())
			folder = path.substr(0, path.rfind('/') + 1);
	}
	bool operator<(const PackageDescriptor& rhs) const
	{
		return caseInsensitiveRep < rhs.caseInsensitiveRep;
//		return uri.casefold_collate_key() < rhs.uri.casefold_collate_key();
		/* The basic space vs. speed tradeoff here.  
		 * Either use the caseInsensitiveRep member (more space, less time)
		 * or change this function to generate the casefold_collate_key()
		 * for comparisons (less space, more time)
		 */
	}
	void Flush()
	{
		// If resources were added or removed, rewrite the package file
		// out to disk (called from ClosePackage()).
		//
		if (!dirtyWithChangedRsrcs)
			return;
		FILE* fp = fopen(path.c_str(), "w");
		ResourceDescriptors::iterator it = resources.begin();
		ResourceDescriptors::iterator itEnd = resources.end();
		for ( ; it != itEnd; ++it)
			fprintf(fp, "%s,%d,%s,%d,%d,%d\n", 
					it->uri.c_str(),
					static_cast<int>(it->type), 
					it->path.c_str(), 
					static_cast<int>(it->size),
					static_cast<int>(it->usize), 
					static_cast<int>(it->version));
		fclose(fp);
	}
};
typedef std::vector<PackageDescriptor>	PackageDescriptors;


//============================================================================
// Device state
//============================================================================
struct DeviceDescriptor
{
	// NOTE: If we were unloading cached data when refCount==0, we wouldn't 
	// need "isOpen".  As it stands, we don't really need "refCount"
	//
	U16					refCount;
	Boolean				isOpen;
	CPath				mountPath;
	PackageDescriptors	packages;
	CString				name;
	// Utility ctor to initialize state
	DeviceDescriptor() : refCount(0), isOpen(0) {}
};
	

//============================================================================
// MPI state, iterators, and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	class	AttachedPackageIterator;	// forward declaration
	class	AttachedResourceIterator;	// forward declaration
	
	
	//------------------------------------------------------------------------
	U32 GetNextRsrcStartHndl(U32 size)
	{
		static U32	gNextRsrcStartHndl = 1;
		U32 retval = gNextRsrcStartHndl;
		gNextRsrcStartHndl += size + kMaxAddNewRsrcs;
		return retval;
	}
	
	//------------------------------------------------------------------------
	// MPIInstanceState
	//------------------------------------------------------------------------
	struct MPIInstanceState
	{
		//--------------------------------------------------------------------
		// Each MPI interface object has some state (default URI, search 
		// iteration state, etc.).  The Resource module stores this state on
		// behalf of an interface object in this MPIInstanceState class.
		// The CResourceMPI ctor calls CResourceModule::Register() to create
		// an instance of this class that gets added to a global "gMPIMap".
		// Register() returns a U32 key into this map, and every call from
		// the MPI interface object to the CResourceModule passes this U32
		// key so that the operation uses that interface object's state.
		// CResourceMPI instance.
		//--------------------------------------------------------------------
		MPIInstanceState(DeviceDescriptor*	pDevs)
						: pDevices(pDevs),
						isBlocking(kBlocking), 
						pDefaultListener(NULL),
						curDeviceIndex(0),
						curDeviceType(kDeviceTypeInvalid),
						curPkgType(kPackageTypeInvalid),	// FIXME: make "Invalids" consistent
						cachedPkg(NULL),
						curRsrcType(kInvalidRsrcType),
						cachedRsrc(NULL),
						cachedRsrcHndl(kInvalidRsrcHndl)
		{
			for (size_t ii = kDeviceCount; ii > 0; --ii)
				deviceIsAttached[ii-1] = false;
		}
		DeviceDescriptor*		pDevices;			// points to global array of devices
		CURI					defaultURI;
		eSynchState				isBlocking;
		const IEventListener*	pDefaultListener;

		size_t					curDeviceIndex;		// for FindNextDevice iteration
		eDeviceType				curDeviceType;
		
		CURI					curPkgURI;			// for FindNextPackage iteration
		ePackageType			curPkgType;
		boost::shared_ptr<AttachedPackageIterator>	pPkgIter;
		PackageDescriptor*		cachedPkg;// cache last found handle for subsequent operations
				
		CURI					curRsrcURI;			// for FindNextRsrc iteration
		tRsrcType				curRsrcType;
		boost::shared_ptr<AttachedResourceIterator>	pRsrcIter;
		ResourceDescriptor*		cachedRsrc;// cache last found handle for subsequent operations
		tRsrcHndl				cachedRsrcHndl;
		
		//--------------------------------------------------------------------
		Boolean DeviceIsAttached(size_t index) const
		{
			return deviceIsAttached[index] ? true : false;
		}
		
		//--------------------------------------------------------------------
		void AttachDevice(size_t index)
		{
			deviceIsAttached[index] = true;
		}
		
		//--------------------------------------------------------------------
		void DettachDevice(size_t index)
		{
			deviceIsAttached[index] = false;
		}

		//--------------------------------------------------------------------
		Boolean PackageIsAttached(tDeviceHndl hndl) const
		{
			return attachedPackages.count(hndl) ? true : false;
		}
		
		//--------------------------------------------------------------------
		void AttachPackage(tPackageHndl hndl)
		{
			attachedPackages.insert(hndl);
		}
		
		//--------------------------------------------------------------------
		void DettachPackage(tPackageHndl hndl)
		{
			attachedPackages.erase(hndl);
		}

		//--------------------------------------------------------------------
		tPackageHndl GetFirstOpenPackageHandle()
		{
			if (attachedPackages.empty())
				return kInvalidPackageHndl;
			return *attachedPackages.begin();
		}

	private:		
		Boolean					deviceIsAttached[kDeviceCount];
		std::set<tPackageHndl>	attachedPackages;
	};
	typedef std::map<U32, MPIInstanceState>	MPIMap;
	
	MPIMap gMPIMap;
	
	
	//------------------------------------------------------------------------
	// OpenDeviceIterator
	//------------------------------------------------------------------------
	class OpenDeviceIterator
	{
		//--------------------------------------------------------------------
		// Iterate over all the devices which the calling MPI interface object
		// has opened via the "OpenAllDevices" or "OpenDevice" calls.
		//--------------------------------------------------------------------
	public:
		explicit OpenDeviceIterator(const MPIInstanceState& mpiState)
			: mpiState_(mpiState), index_(kDeviceCount-1)
		{
		}
		Boolean GetNext(DeviceDescriptor*& pdd)
		{
			while (index_ >= 0 && !mpiState_.DeviceIsAttached(index_))
				--index_;
			if (index_ < 0)
			{
				pdd = NULL;
				return false;
			}
			pdd = &(mpiState_.pDevices[index_]);
			--index_;
			return true;
		}
	private:
		const MPIInstanceState& mpiState_;
		S16						index_;
	};


	//------------------------------------------------------------------------
	// AttachedPackageIterator
	//------------------------------------------------------------------------
	class AttachedPackageIterator
	{
		//--------------------------------------------------------------------
		// Iterate over all the packages that are present on devices which the 
		// calling MPI interface object has opened via the "OpenAllDevices" or 
		// "OpenDevice" calls.
		//--------------------------------------------------------------------
	public:
		explicit AttachedPackageIterator(const MPIInstanceState& mpiState)
			: itDev_(mpiState)
		{
			if (itDev_.GetNext(pdd_))
			{
				itPkgCur_ = pdd_->packages.begin();
				itPkgEnd_ = pdd_->packages.end();
			}
		}
		Boolean GetNext(PackageDescriptor*& ppd)
		{
			// 1) Loop through all devices (the ctor set us up with the first one) 
			// 2) If we have not iterated through all of the packages in the
			//    current device, return the next one and increment the iterator
			// 3) If done with packages on this device, bump to the next one.
			//
			while (pdd_)													//*1
			{
				if (itPkgCur_ != itPkgEnd_)									//*2
				{
					ppd = &(*itPkgCur_++);
					return true;
				}
				if (itDev_.GetNext(pdd_))									//*3
				{
					itPkgCur_ = pdd_->packages.begin();
					itPkgEnd_ = pdd_->packages.end();
				}
			}
			ppd = NULL;
			return false;
		}
	private:
		PackageDescriptors::iterator itPkgCur_;
		PackageDescriptors::iterator itPkgEnd_;
		OpenDeviceIterator			itDev_;
		DeviceDescriptor*			pdd_;
	};


	//------------------------------------------------------------------------
	// OpenPackageIterator
	//------------------------------------------------------------------------
	class OpenPackageIterator
	{
		//--------------------------------------------------------------------
		// Iterate over all the packages that the calling MPI interface object
		// has opened via the "OpenPackage" or "LoadPackage" calls.
		//--------------------------------------------------------------------
	public:
		explicit OpenPackageIterator(const MPIInstanceState& mpiState)
			: itPkg_(mpiState), mpiState_(mpiState)
		{
		}
		Boolean GetNext(PackageDescriptor*& ppd)
		{
			while (itPkg_.GetNext(ppd))
			{
				if (mpiState_.PackageIsAttached(ppd->hndl))
					return true;
			}
			return false;
		}
	private:
		AttachedPackageIterator		itPkg_;
		const MPIInstanceState& 	mpiState_;
	};


	//------------------------------------------------------------------------
	// AttachedResourceIterator
	//------------------------------------------------------------------------
	class AttachedResourceIterator
	{
		//--------------------------------------------------------------------
		// Iterate over all the resources that are present in packages which 
		// the calling MPI interface object has opened via the "OpenPackage"
		// or "LoadPackage" calls.
		//--------------------------------------------------------------------
	public:
		explicit AttachedResourceIterator(const MPIInstanceState& mpiState)
			: itPkg_(mpiState)
		{
			if (itPkg_.GetNext(ppd_))
			{
				itCur_ = ppd_->resources.begin();
				itEnd_ = ppd_->resources.end();
			}
		}
		Boolean GetNext(ResourceDescriptor*& prd)
		{
			// 1) Loop through all open packages (the ctor set us up with the first one) 
			// 2) If we have not iterated through all of the resources in the
			//    current package, return the next one and increment the iterator
			// 3) If done with resources in this package, bump to the next package.
			//
			while (ppd_)													//*1
			{
				if (itCur_ != itEnd_)										//*2
				{
					prd = &(*itCur_++);
					return true;
				}
				if (itPkg_.GetNext(ppd_))									//*3
				{
					itCur_ = ppd_->resources.begin();
					itEnd_ = ppd_->resources.end();
				}
			}
			prd = NULL;
			return false;
		}
		void MarkPackageDirty()
		{
			ppd_->dirtyWithRetrievedRsrcHndls = true;
		}
		tRsrcHndl CurHandle()
		{
			return ppd_->startRsrcHndl
					+ std::distance(ppd_->resources.begin(), itCur_);
		}
	private:
		ResourceDescriptors::iterator itCur_;
		ResourceDescriptors::iterator itEnd_;
		OpenPackageIterator			itPkg_;
		PackageDescriptor*			ppd_;
	};
	


	//------------------------------------------------------------------------
	// AllPackageIterator
	//------------------------------------------------------------------------
	class AllPackageIterator
	{
		//--------------------------------------------------------------------
		// Iterate over all the packages that are present on devices which the 
		// calling MPI interface object has opened via the "OpenAllDevices" or 
		// "OpenDevice" calls.
		//--------------------------------------------------------------------
	public:
		explicit AllPackageIterator(const MPIInstanceState& mpiState)
			: mpiState_(mpiState), devIndex_(kDeviceCount-1)
		{
			NextDevice();
		}
		Boolean GetNext(PackageDescriptor*& ppd)
		{
			// 1) Loop through all devices (the ctor set us up with the first one) 
			// 2) If we have not iterated through all of the packages in the
			//    current device, return the next one and increment the iterator
			// 3) If done with packages on this device, bump to the next one.
			//
			while (pdd_)													//*1
			{
				if (itPkgCur_ != itPkgEnd_)									//*2
				{
					ppd = &(*itPkgCur_++);
					return true;
				}
				NextDevice();												//*3
			}
			ppd = NULL;
			return false;
		}
	private:
		void NextDevice()
		{
			pdd_ = &(mpiState_.pDevices[devIndex_--]);
			while (!pdd_ || pdd_->packages.empty())
			{
				if (devIndex_ < 0)
				{
					pdd_ = NULL;
					return;
				}
				pdd_ = &(mpiState_.pDevices[devIndex_--]);
			}
			itPkgCur_ = pdd_->packages.begin();
			itPkgEnd_ = pdd_->packages.end();	
		}
		
		PackageDescriptors::iterator itPkgCur_;
		PackageDescriptors::iterator itPkgEnd_;
		const MPIInstanceState& 	mpiState_;
		DeviceDescriptor*			pdd_;
		S16							devIndex_;
	};


	//============================================================================
	// Utility functions
	//============================================================================
	//----------------------------------------------------------------------------
	MPIInstanceState& RetrieveMPIState(U32 id)
	{
		// Register() creates an MPIInstanceState entry in "gMPIMap" and 
		// returns a key/U32 id that the MPI instance uses to identify itself
		// in subsequent calls.
		// This function retrieves the MPIInstanceState for a given key.
		// FIXME/tp: multithreading issues
		//
		MPIMap::iterator it = gMPIMap.find(id);
		if (it != gMPIMap.end())
			return it->second;
		CDebugMPI	dbg(kGroupResource);
		dbg.Assert(false, "CResourceModuleResource::RetrieveMPIState: configuration failure, unregistered MPI id!");
		return it->second;	// dummy return to avoid compiler warning
	}

	//----------------------------------------------------------------------------
	inline size_t Handle2Index(tDeviceHndl hndl)
	{
		// Handles are 1-based but arrays are 0-based so do the translation
		// and validate that the handle is valid.
		//
		size_t tmp = static_cast<size_t>(hndl);
		if (tmp < 1 || tmp > kDeviceCount)
		{
			CDebugMPI	dbg(kGroupResource);
			dbg.Assert(false, "CResourceModule::Handle2Index: Programming Error: Invalid Device handle: %d", tmp);
		}		
		return tmp - 1;
	}

	//--------------------------------------------------------------------------
	void AssignPackageHndl(PackageDescriptor& pd)
	{
		// Guarantee that all tPackageHndls are unique.
		static tPackageHndl s_next = 0;
		pd.hndl = ++s_next;
	}

	//--------------------------------------------------------------------------
	CURI PrepareSearchURI(const CURI& in)
	{
		if (in.empty())
			return in;
		CURI uri = AppendPathSeparator(in);
		if (uri.at(0) == '/')
			uri.erase(0, 1);
		return uri;
	}
	
	//--------------------------------------------------------------------------
	void PostEvent(tEventType type,
					tResourceMsgDat data,
					const MPIInstanceState& mpiState, 
					const IEventListener *pOverrideListener)
	{
		// 1) Only post an event if we are in async/non-blocking mode.
		// 2) For Lightning, we only support blocking operations, so assert out.
		//		Since good work was already done on non-blocking operations,
		//		we allow the unit tests to continue to test them.
		// 3) Post the appropriate event.
		//
		if (mpiState.isBlocking == kBlocking)								//*1
			return;
			
#ifndef UNIT_TESTING
		CDebugMPI	dbg(kGroupResource);									//*2
		dbg.Assert(false, "CResourceModule::PostEvent: Asyncronous/non-blocking Resource Manager calls are not yet supported");
#endif // UNIT_TESTING

		const tEventPriority	kPriorityTBD = 0;							//*3
		CEventMPI	event;
		CResourceEventMessage	msg(type, data);
		const IEventListener *pListener = pOverrideListener
										? pOverrideListener : mpiState.pDefaultListener;
		event.PostEvent(msg, kPriorityTBD, pListener);
	}
}



//==============================================================================
// Core Module functions
//==============================================================================
//----------------------------------------------------------------------------
CResourceModule::CResourceModule() : dbg_(kGroupResource),
								pDevices_(new DeviceDescriptor[kDeviceCount])
{
	tErrType 			result = kNoErr;
	const tMutexAttr 	attr = {0};
	
	// Initialize the DeviceDescriptor array
	//
	const CString	base = "Base";
	const CString	cart = "Cart";
	MountPoints	mp;
	GetDeviceMountPoints(mp);
	size_t len = mp.size();
	for (size_t ii = 0; ii < len; ++ii)
		pDevices_[ii].mountPath = mp[ii];
	pDevices_[0].name = base;
	for (size_t ii = 1; ii < len; ++ii)
	{
		char buf[4];
		sprintf(buf, "%d", ii);
		pDevices_[ii].name = cart + buf;
	}

	// Init mutex for serialization of MPI calls
	result = kernel_.InitMutex( mpiMutex_, attr );
	dbg_.Assert((kNoErr == result), "CResourceModule::ctor: Couldn't init mutex.\n");
}

//----------------------------------------------------------------------------
CResourceModule::~CResourceModule()
{
	tErrType result = kNoErr;

	result = kernel_.DeInitMutex( mpiMutex_ );
	dbg_.Assert((kNoErr == result), "CResourceModule::dtor --  Couldn't deinit mutex.\n");
}

//----------------------------------------------------------------------------
Boolean	CResourceModule::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
tVersion CResourceModule::GetModuleVersion() const
{
	return kModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CResourceModule::GetModuleName() const
{
	return &kModuleName;
}

//----------------------------------------------------------------------------
const CURI* CResourceModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//==============================================================================
// MPI State Functions
//==============================================================================
//----------------------------------------------------------------------------
U32 CResourceModule::Register( )
{
	tErrType 	result = kNoErr;
	static U32	sNextIndex = 0;

	result = kernel_.LockMutex( mpiMutex_ );
	dbg_.Assert((kNoErr == result), "CResourceModule::Register -- Couldn't lock mutex.\n");

	gMPIMap.insert(MPIMap::value_type(++sNextIndex, MPIInstanceState(pDevices_.get())));

	result = kernel_.UnlockMutex( mpiMutex_ );
	dbg_.Assert((kNoErr == result), "CResourceModule::Register --  Couldn't unlock mutex.\n");

	return sNextIndex;
}

//----------------------------------------------------------------------------
void CResourceModule::Unregister(U32 id)
{
	tErrType 			result = kNoErr;
	MPIMap::iterator 	it = gMPIMap.find(id);

	result = kernel_.LockMutex( mpiMutex_ );
	dbg_.Assert((kNoErr == result), "CResourceModule::Unregister -- Couldn't lock mutex.\n");

	if (it != gMPIMap.end())
	{
		for (tPackageHndl hPkg = it->second.GetFirstOpenPackageHandle();
				hPkg != kInvalidPackageHndl;
				hPkg = it->second.GetFirstOpenPackageHandle())
			ClosePackage(id, hPkg);
		CloseAllDevices(id);
		gMPIMap.erase(it);
	}
	
	result = kernel_.UnlockMutex( mpiMutex_ );
	dbg_.Assert((kNoErr == result), "CAudioModule::Unregister --  Couldn't unlock mutex.\n");
}

//----------------------------------------------------------------------------
void CResourceModule::SetDefaultURIPath(U32 id, const CURI &pURIPath)
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	mpiState.defaultURI = PrepareSearchURI(pURIPath);
}

//----------------------------------------------------------------------------
void CResourceModule::SetSynchronization(U32 id, eSynchState isBlocking)
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	mpiState.isBlocking = isBlocking;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SetDefaultListener(U32 id, const IEventListener *pListener)
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	mpiState.pDefaultListener = pListener;
	return kNoErr;
}


//==============================================================================
// Device Functions
//==============================================================================
//----------------------------------------------------------------------------
U16 CResourceModule::GetNumDevices(eDeviceType type) const
{
	return kDeviceCount; 
}

//----------------------------------------------------------------------------
tDeviceHndl CResourceModule::FindFirstDevice(U32 id, eDeviceType type) const
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	mpiState.curDeviceIndex = 0;
	mpiState.curDeviceType = type;
	return FindNextDevice(id);
}

//----------------------------------------------------------------------------
tDeviceHndl CResourceModule::FindNextDevice(U32 id) const
{
	// TODO: Implement Device types
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	if (mpiState.curDeviceIndex < kDeviceCount)
		return ++mpiState.curDeviceIndex;
	return kInvalidDeviceHndl;
}

//----------------------------------------------------------------------------
const CString* CResourceModule::GetDeviceName(tDeviceHndl hndl) const
{
	size_t index = Handle2Index(hndl);
	return &(pDevices_[index].name);
}
//----------------------------------------------------------------------------
eDeviceType CResourceModule::GetDeviceType(tDeviceHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetDeviceType not implemented");
	return kDeviceTypeInvalid;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::OpenDevice(U32 id, tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	OpenDeviceImpl(id, hndl, openOptions);
	PostEvent(kResourceDeviceOpenedEvent, 0, mpiState, pListener);
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::CloseDevice(U32 id, tDeviceHndl hndl)
{
	// 1) Return immediately if this instance has already closed this device
	// 2) Change MPI state to indicate detached and decrement underlying refCount
	//
	size_t index = Handle2Index(hndl);
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	if (!mpiState.DeviceIsAttached(index))								//*1
		return kNoErr;
		
	mpiState.DettachDevice(index);										//*2
	DeviceDescriptor& device = pDevices_[index];
	--device.refCount;
	
	// TODO/FUTURE: Release memory if refcount goes to zero if need memory.
	// Probably never needed unless you have rarely-opened devices,
	// which is not the case for Lightning.
//	if (device.refCount == 0)
//		free memory
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::OpenAllDevices(U32 id, tOptionFlags openOptions,
										const IEventListener *pListener)
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	for (tDeviceHndl hndl = kDeviceCount; hndl != 0; --hndl)
		OpenDeviceImpl(id, hndl, openOptions);
	PostEvent(kResourceAllDevicesOpenedEvent, 0, mpiState, pListener);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::CloseAllDevices(U32 id)
{
	for (tDeviceHndl hndl = kDeviceCount; hndl != 0; --hndl)
		CloseDevice(id, hndl);
	return kNoErr;
}

//----------------------------------------------------------------------------
void CResourceModule::OpenDeviceImpl(U32 id, tDeviceHndl hndl, 
									tOptionFlags /*openOptions*/)
{
	// 1) If the connected MPI instance has already opened the device,
	//		return immedately with no error.
	// 2) Change MPI state to indicate attached and increment underlying refCount
	// 3) If the device has never been opened, find all the packages on it.
	// 3a) Open the device's rsrc/EnumPkgs file.  It is a CSV file with
	//     the following format (there must be NO whitespace in the line):
	//			URI,filename,version,type
	// 3c) Read the EnumPkgs file line-by-line
	// 4) Right-size the PackageDescriptor vector to free any extra space
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	size_t index = Handle2Index(hndl);
	DeviceDescriptor& device = pDevices_[index];
	if (mpiState.DeviceIsAttached(index))								//*1
		return;
		
	mpiState.AttachDevice(index);										//*2
	++device.refCount;
	
	if (!device.isOpen)													//*3
	{
		device.isOpen = true;
		CPath pkgDesc = device.mountPath + kEnumPkgsFile;				//*3a
		FILE* fp = fopen(pkgDesc.c_str(), "r");
		if (fp == NULL)
		{
			// FIXME/tp: Assert on Base device if not found?
			dbg_.Warn("CResourceModule::OpenDeviceImpl: '%s' file not found on '%s'", 
						kEnumPkgsFile, device.mountPath.c_str());
			return;
		}
		char buf[kMaxBrioPkgLine];
		while (NULL != fgets(buf, kMaxBrioPkgLine, fp))
		{
			char* uri = buf;
			char* file = strchr(uri, ','); *file++ = '\0';
			char* ver = strchr(file, ','); *ver++ = '\0';
			char* type = strchr(ver, ','); *type++ = '\0';
			CPath fullPath = device.mountPath;
			fullPath += file;
			PackageDescriptor pd(uri, fullPath, atoi(ver), kPackageTypeInvalid, hndl);
			device.packages.push_back(pd);
			// FIXME/tp: enumerate valid package types
		}
		fclose(fp);
		
		std::for_each(device.packages.begin(), device.packages.end(),	//*4
						AssignPackageHndl);
		std::vector<PackageDescriptor>	temp(device.packages);		
		device.packages.swap(temp);
	}
}


//==============================================================================
// Package Functions
//==============================================================================
//------------------------------------------------------------------------------
U32 CResourceModule::GetNumPackages(U32 id, ePackageType type, 
										const CURI *pURIPath) const
{
	U32 count = 0;
	tPackageHndl hndl = FindFirstPackage(id, type, pURIPath);
	while (kInvalidPackageHndl != hndl)
	{
		// FIXME/tp: Test by type
		++count;
		hndl = FindNextPackage(id);
	}
	return count;
}

//----------------------------------------------------------------------------
tPackageHndl CResourceModule::FindPackage(U32 id, const CURI& packageURI,
												const CURI *pURIPath) const
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	CURI uri = pURIPath ? PrepareSearchURI(*pURIPath) : mpiState.defaultURI;
	uri += packageURI;
	PackageDescriptor	search(uri.c_str());
	OpenDeviceIterator	iter(mpiState);
	DeviceDescriptor* pdd;
	while (iter.GetNext(pdd))
	{
		typedef std::pair<PackageDescriptors::iterator, 
							PackageDescriptors::iterator> Ret;
		Ret ret = std::equal_range(pdd->packages.begin(), pdd->packages.end(), search);
		if (ret.first != ret.second)
		{
			mpiState.cachedPkg = &(*ret.first);
			return mpiState.cachedPkg->hndl;
		}
		// TODO: Might be a nice feature to reset mpiState.pPkgIter 
		// to the this iterator, so that you could iterate subsequent packages
		// This would require a new AttachedPackageIterator ctor to correctly
		// initialize the iterator's state.		
	}
	return kInvalidPackageHndl;
}

//----------------------------------------------------------------------------
tPackageHndl CResourceModule::FindFirstPackage(U32 id, ePackageType type,
												const CURI *pURIPath) const
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	mpiState.curPkgURI = pURIPath ? PrepareSearchURI(*pURIPath) : mpiState.defaultURI;
	mpiState.curPkgType = type;
	mpiState.pPkgIter.reset(new AttachedPackageIterator(mpiState));
	return FindNextPackage(id);
}

//----------------------------------------------------------------------------
tPackageHndl CResourceModule::FindNextPackage(U32 id) const
{
	// NOTE: Comparing the type field is cheaper than comparing the URI
	// strings, so do the type field comparison first.
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	dbg_.Assert(mpiState.pPkgIter.get() != NULL, 
		"CResourceModule::FindNextPackage: Programming Error: FindNextPackage() called with before initial FindFirstPackage()");	// FIXME/tp: replace with non-assert in release mode
	PackageDescriptor* ppd;
	while (mpiState.pPkgIter->GetNext(ppd))
	{
		if (mpiState.curPkgType != kPackageTypeAll 
				&& mpiState.curPkgType != ppd->type)
			continue;
		//FIXME/tp: Case-insensitive
		if (ppd->uri.find(mpiState.curPkgURI) == 0)
		{
			mpiState.cachedPkg = ppd;
			return mpiState.cachedPkg->hndl;
		}
	}
	return kInvalidPackageHndl;
}

//----------------------------------------------------------------------------
PackageDescriptor* CResourceModule::FindPackagePriv(U32 id, tPackageHndl hndl) const
{
	// At some point we may want to change the Assert to a non-halting
	// operation, so return a pointer rather than a reference to allow
	// an alternate exit mechanism.
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	OpenDeviceIterator	iter(mpiState);
	if (mpiState.cachedPkg->hndl == hndl)
		return mpiState.cachedPkg;
		
	DeviceDescriptor* pdd;
	while (iter.GetNext(pdd))
	{
		if (pdd->packages.empty())
			continue;
		tPackageHndl last = pdd->packages.back().hndl;
		if (hndl > last)
			continue;
		tPackageHndl first = pdd->packages.front().hndl;
		if (hndl >= first)
		{
			mpiState.cachedPkg = &(pdd->packages[hndl - first]);
			return mpiState.cachedPkg;
		}
	}
	dbg_.Assert(false, 
			"CResourceModule::FindPackagePriv: Programming Error: called with invalid handle 0x%x", 
			static_cast<unsigned int>(hndl));
	// FIXME/tp: replace with non-assert in release mode
	return NULL;
}

//----------------------------------------------------------------------------
// NOTE: In development mode, all of the Package accessors assert out if
//		an invalid tPackageHndl is provided.
//----------------------------------------------------------------------------
const CURI* CResourceModule::GetPackageURI(U32 id, tPackageHndl hndl) const
{
	const PackageDescriptor* ppd = FindPackagePriv(id, hndl);
	return ppd ? &(ppd->uri) : &kNullURI;
}
//----------------------------------------------------------------------------
ePackageType CResourceModule::GetPackageType(U32 id, tPackageHndl hndl) const
{
	const PackageDescriptor* ppd = FindPackagePriv(id, hndl);
	return ppd ? (ppd->type) : kPackageTypeInvalid;
}
//----------------------------------------------------------------------------
tVersion CResourceModule::GetPackageVersion(U32 id, tPackageHndl hndl) const
{
	const PackageDescriptor* ppd = FindPackagePriv(id, hndl);
	return ppd ? (ppd->version) : kUndefinedVersion;
}
//----------------------------------------------------------------------------
U32 CResourceModule::GetPackageSizeUnpacked(U32 /*id*/, tPackageHndl /*hndl*/) const
{
	dbg_.DebugOut(kDbgLvlCritical, "ResourceModule::GetPackageSizeUnpacked not implemented");
	return 0;
}
//----------------------------------------------------------------------------
U32 CResourceModule::GetPackageSizePacked(U32 /*id*/, tPackageHndl /*hndl*/) const
{
	dbg_.DebugOut(kDbgLvlCritical, "ResourceModule::GetPackageSizePacked not implemented");
	return 0;
}


//----------------------------------------------------------------------------
tErrType CResourceModule::OpenPackage(U32 id, tPackageHndl hndl, 
									tOptionFlags /*openOptions*/,
									const IEventListener *pListener)  
{
	// 1) If the connected MPI instance has already opened the package,
	//		return immedately with no error.
	// 2) Retrieve the package (error if handle is invalid).
	// 3) Change MPI state to indicate attached and increment underlying refCount
	// 4) If the packages has never been opened, read in all the resource descriptors.
	// 4a) Open the package file.  It is a CSV file with
	//     the following format (there must be NO whitespace in the line):
	//			URI,type,location,size,compressedSize,version
	// 4c) Read the EnumPkgs file line-by-line
	// 5) Right-size the PackageDescriptor vector to free any extra space
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	if (mpiState.PackageIsAttached(hndl))								//*1
		return kNoErr;
		
	PackageDescriptor* ppd = FindPackagePriv(id, hndl);					//*2
	if (ppd == NULL)
		return kResourceInvalidErr;
		
	mpiState.AttachPackage(hndl);										//*3
	++ppd->refCount;
	size_t rootLen = ppd->folder.size();
	
	if (ppd->refCount == 1)												//*4
	{
		FILE* fp = fopen(ppd->path.c_str(), "r");						//*4a
		dbg_.Assert(fp != NULL, "CResourceModule::OpenPackage: file not found '%s'", 
					ppd->path.c_str());
		if (fp == NULL)	return kResourceInvalidErr;

		char buf[kMaxBrioPkgLine];
		while (NULL != fgets(buf, kMaxBrioPkgLine, fp))
		{
			char* uri = buf;
			char* type = strchr(uri, ','); *type++ = '\0';
			char* file = strchr(type, ','); *file++ = '\0';
			char* size = strchr(file, ','); *size++ = '\0';
			char* usize = strchr(size, ','); *usize++ = '\0';
			char* ver = strchr(usize, ','); *ver++ = '\0';
			CPath path(file);
			if (path.find(ppd->folder) == 0)
				path = path.substr(rootLen);
			ResourceDescriptor rd(uri, atoi(type), path, 
								atoi(size), atoi(usize), atoi(ver), ppd);
			ppd->resources.push_back(rd);
		}
		fclose(fp);
		
		std::vector<ResourceDescriptor>	temp(ppd->resources);		//*5
		ppd->resources.swap(temp);
		ppd->startRsrcHndl = GetNextRsrcStartHndl(ppd->resources.size());
	}
	PostEvent(kResourcePackageOpenedEvent, 0, mpiState, pListener);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::ClosePackage(U32 id, tPackageHndl hndl)
{
	// 1) Return immediately if this instance has already closed this package
	// 2) Change MPI state to indicate detached and decrement underlying refCount
	// 3) Free up the resource state:
	// 3a)	Close all contained resources
	// 3b)	Free up all the ResourceDescriptor structures
	// NOTE: The device's list of packages never changes, so we don't have
	// any cleanup to do in the "DeviceDescriptor" state.
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	if (!mpiState.PackageIsAttached(hndl))								//*1
		return kNoErr;
	
	PackageDescriptor* ppd = FindPackagePriv(id, hndl);
	if (ppd == NULL)
		return kResourceInvalidErr;
	ppd->Flush();

	mpiState.DettachPackage(hndl);										//*2
	--ppd->refCount;
	
	if (ppd->refCount == 0)												//*3
	{
		ResourceDescriptors::iterator	it = ppd->resources.begin();
		ResourceDescriptors::iterator	itEnd = ppd->resources.end();
		for ( U32 ii = 0; it != itEnd; ++it, ++ii)						//*3a
			CloseRsrc(id, ppd->startRsrcHndl + ii);
		ppd->resources.clear();											//*3b
	}
		
	return kNoErr;
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceModule::LoadPackage(U32 id, tPackageHndl hndl, 
									tOptionFlags /*loadOptions*/,
									const IEventListener *pListener)  
{
	dbg_.DebugOut(kDbgLvlCritical, "CResourceModule::LoadPackage not implemented");
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::UnloadPackage(U32 id, tPackageHndl hndl, 
									tOptionFlags /*unloadOptions*/,
									const IEventListener *pListener)  
{
	dbg_.DebugOut(kDbgLvlCritical, "CResourceModule::UnloadPackage not implemented");
	return kNoErr;
}


//==============================================================================
// Resources
//==============================================================================
//----------------------------------------------------------------------------
U32 CResourceModule::GetNumRsrcs(U32 id, tRsrcType type, 
								const CURI *pURIPath) const
{
	// Save and restore the iterator state before/after counting
	U32 count = 0;
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	const CURI curRsrcURI = mpiState.curRsrcURI;
	tRsrcType curRsrcType = mpiState.curRsrcType;
	tRsrcHndl curRsrcHndl = mpiState.cachedRsrcHndl;
	tRsrcHndl hndl = FindFirstRsrc(id, type, pURIPath);
	while (kInvalidRsrcHndl != hndl)
	{
		if (type == kRsrcTypeAll || type == GetType(id, hndl))
			++count;
		hndl = FindNextRsrc(id);
	}
	hndl = FindFirstRsrc(id, curRsrcType, &curRsrcURI);
	while (hndl != curRsrcHndl && hndl != kInvalidRsrcHndl)
		hndl = FindNextRsrc(id);
	return count;
}
 
//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindRsrc(U32 id, const CURI &rsrcURI,
									const CURI *pURIPath) const
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	CURI uri = pURIPath ? PrepareSearchURI(*pURIPath) : mpiState.defaultURI;
	uri += rsrcURI;
	ResourceDescriptor	search(uri.c_str());
	OpenPackageIterator	iter(mpiState);
	PackageDescriptor* ppd;
	while (iter.GetNext(ppd))
	{
		typedef std::pair<ResourceDescriptors::iterator, 
							ResourceDescriptors::iterator> Ret;
		Ret ret = std::equal_range(ppd->resources.begin(), ppd->resources.end(), search);
		if (ret.first != ret.second)
		{
			mpiState.cachedRsrc = &(*ret.first);
			mpiState.cachedRsrcHndl = ppd->startRsrcHndl 
							+ std::distance(ppd->resources.begin(), ret.first);
			ppd->dirtyWithRetrievedRsrcHndls = true;
			return mpiState.cachedRsrcHndl;
		}
	}
	return kInvalidRsrcHndl;
}

//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindFirstRsrc(U32 id, tRsrcType type, 
										const CURI *pURIPath) const
{
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	mpiState.curRsrcURI = pURIPath ? PrepareSearchURI(*pURIPath) : mpiState.defaultURI;
	mpiState.curRsrcType = type;
	mpiState.pRsrcIter.reset(new AttachedResourceIterator(mpiState));
	return FindNextRsrc(id);
}
 
//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindNextRsrc(U32 id) const
{
	// NOTE: Comparing the type field is cheaper than comparing the URI
	// strings, so do the type field comparison first.
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	dbg_.Assert(mpiState.pRsrcIter.get() != NULL, 
		"CResourceModule::FindNextRsrc: Programming Error: FindNextRsrc() called with before initial FindFirstRsrc()");	// FIXME/tp: replace with non-assert in release mode
	ResourceDescriptor* prd;
	while (mpiState.pRsrcIter->GetNext(prd))
	{
		if (mpiState.curRsrcType != kRsrcTypeAll 
				&& mpiState.curRsrcType != prd->type)
			continue;
		// FIXME/tp: Case-insensitive
		if (prd->uri.find(mpiState.curRsrcURI) == 0)
		{
			mpiState.pRsrcIter->MarkPackageDirty();
			mpiState.cachedRsrcHndl = mpiState.pRsrcIter->CurHandle();
			mpiState.cachedRsrc = prd;
			return mpiState.cachedRsrcHndl;
		}
	}
	return kInvalidRsrcHndl;
}
 
//----------------------------------------------------------------------------
ResourceDescriptor* CResourceModule::FindRsrcPriv(U32 id, tRsrcHndl hndl) const
{
	// At some point we may want to change the Assert to a non-halting
	// operation, so return a pointer rather than a reference to allow
	// an alternate exit mechanism.
	//
	MPIInstanceState& mpiState = RetrieveMPIState(id);
	AllPackageIterator	iter(mpiState);
	if (hndl != kInvalidRsrcHndl && mpiState.cachedRsrcHndl == hndl)
		return mpiState.cachedRsrc;

	PackageDescriptor* ppd;
	while (iter.GetNext(ppd))
	{
		if (ppd->resources.empty())
			continue;
		tRsrcHndl begin = ppd->startRsrcHndl;
		tRsrcHndl end = begin + ppd->resources.size();
		if (hndl >= end)
			continue;
		if (hndl >= begin)
		{
			mpiState.cachedRsrc = &(ppd->resources[hndl - begin]);
			return mpiState.cachedRsrc;
		}
	}
	dbg_.Assert(false, 
			"CResourceModule::FindRsrcPriv: Programming Error: FindRsrcPriv() called with invalid handle 0x%x", 
			static_cast<unsigned int>(hndl));
	// FIXME/tp: replace with non-assert in release mode
	return NULL;
}

//----------------------------------------------------------------------------
const CURI* CResourceModule::GetURI(U32 id, tRsrcHndl hndl) const
{
	const ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	return prd ? &(prd->uri) : &kNullURI;
}

//----------------------------------------------------------------------------
tRsrcType CResourceModule::GetType(U32 id, tRsrcHndl hndl) const
{
	const ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	return prd ? prd->type : kInvalidRsrcType;
}

//----------------------------------------------------------------------------
tVersion CResourceModule::GetVersion(U32 id, tRsrcHndl hndl) const
{
	const ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	return prd ? prd->version : kUndefinedVersion;
}

//----------------------------------------------------------------------------
U32 CResourceModule::GetPackedSize(U32 id, tRsrcHndl hndl) const
{
	const ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	return prd ? prd->size : 0;
}

//----------------------------------------------------------------------------
U32 CResourceModule::GetUnpackedSize(U32 id, tRsrcHndl hndl) const
{
	const ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	return prd ? prd->usize : 0;
}

//----------------------------------------------------------------------------
tPtr CResourceModule::GetPtr(U32 id, tRsrcHndl hndl) const
{
	const ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetPtr() called with invalid resource handle (0x%x)", 
				static_cast<unsigned int>(hndl));
		return 0;
	}
	
	if (prd->ptr == NULL)
	{
		dbg_.Assert(false, "CResourceModule::GetPtr: GetPtr() called when resource not yet loaded '%s'", prd->uri.c_str());
		return 0;
	}
	return prd->ptr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::OpenRsrc(U32 id, tRsrcHndl hndl, 
								tOptionFlags openOptions,
								const IEventListener *pListener,
								Boolean supressDoneEvent)
{
	// 1) Retrieve resource and error our if invalid
	// 2) If resource is already opened (has a valid file descriptor) 
	//		return immediately
	// 3) Set up open flags
	// 4) Open the file
	// 5) Post an open event
	//
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*1
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CResourceModule::OpenRsrc() called with invalid resource handle (0x%x)", 
					static_cast<unsigned int>(hndl));
		return kResourceInvalidErr;
	}
	
	if (prd->fd != -1)													//*2
		return kNoErr;
		
	int flags = 0;														//*3
	if (!(openOptions & kOpenRsrcOptionWrite))
		flags |= B_O_RDONLY;
	else if (openOptions & kOpenRsrcOptionRead)
		flags |= B_O_RDWR | B_O_APPEND;
	else
		flags |= B_O_WRONLY | B_O_TRUNC | B_O_CREAT;
		
	// FIXME/tp: assumes no '..' in path, validate it	
	CPath fullPath = prd->pPkg->path.substr(0, prd->pPkg->path.rfind('/')+1);//*4
	fullPath += prd->path;						
	prd->fd = open(fullPath.c_str(), flags);
	if (prd->fd == -1)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CResourceModule::OpenRsrc: OpenRsrc() failed for '%s' (%s)",
						prd->uri.c_str(), fullPath.c_str());
		return kResourceInvalidErr;
	}
		
	++prd->refCount;		
	
	if (!supressDoneEvent)												//*5
	{
		MPIInstanceState& mpiState = RetrieveMPIState(id);
		PostEvent(kResourceOpenedEvent, 0, mpiState, pListener);
	}
	
	return kNoErr;
}  

//----------------------------------------------------------------------------
tErrType CResourceModule::CloseRsrc(U32 id, tRsrcHndl hndl)
{
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CResourceModule::CloseRsrc(): called with invalid resource handle (0x%x)",
						static_cast<unsigned int>(hndl));
		return kResourceInvalidErr;
	}
	
	--prd->refCount;
	if (prd->refCount == 0)
	{
		close(prd->fd);
		prd->fd = -1;
	}
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::ReadRsrc(U32 id, tRsrcHndl hndl, 
									void* pBuffer, 
									U32 numBytesRequested,
									U32 *pNumBytesActual,
									tOptionFlags /*readOptions*/,
									const IEventListener *pListener,
									Boolean suppressDoneEvent) const
{
	// 1) Check parameters for early returns
	// 2) Find the resource and validate it is open
	// 3) Read the data
	// 4) Post the read done event
	//	
	if (numBytesRequested == 0)											//*1
		return kNoErr;	
		
	if (pBuffer == NULL)
		return kInvalidParamErr;
		
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*2
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CResourceModule::ReadRsrc: called with invalid resource handle (0x%x)", 
						static_cast<unsigned int>(hndl));
		return kResourceInvalidErr;
	}

	if (prd->fd == -1)
	{
		dbg_.Assert(false, "CResourceModule::ReadRsrc: called with non-open resource '%s'", prd->uri.c_str());
		return kResourceNotOpenErr;
	}
		
	U32 countRead = read(prd->fd, pBuffer, numBytesRequested);			//*3
	if (pNumBytesActual != NULL)
		*pNumBytesActual = countRead;
	
	if (!suppressDoneEvent)												//*4
	{
		MPIInstanceState& mpiState = RetrieveMPIState(id);
		PostEvent(kResourceReadDoneEvent, 0, mpiState, pListener);
	}
	
	return kNoErr;
}  

//----------------------------------------------------------------------------
tErrType CResourceModule::LoadRsrc(U32 id, tRsrcHndl hndl, 
									tOptionFlags /*loadOptions*/,
									const IEventListener *pListener)
{
	// 1) Open the resource
	// 2) Allocate memory
	// 3) Read the resource
	// 4) Close the resource
	// 5) Post an event
	//
	tErrType stat = OpenRsrc(id, hndl, kOpenRsrcOptionRead, NULL, true);//*1
	if (stat != kNoErr)
		return stat;
		
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*2
	prd->ptr = kernel_.Malloc(prd->size);	// TODO/tp: cusom allocator?
	if (!prd->ptr)
	{
		dbg_.DebugOut(kDbgLvlCritical, 
			"CResourceModule::LoadRsrc: memory allocation failure '%s'", prd->uri.c_str());
		return kMemoryAllocationErr;
	}
	
	U32 countRead;														//*3
	stat = ReadRsrc(id, hndl, prd->ptr, prd->size, &countRead, 0, NULL, true);		//*1
	if (stat != kNoErr)
		return stat;
	
	stat = CloseRsrc(id, hndl);											//*4
	
	MPIInstanceState& mpiState = RetrieveMPIState(id);					//*5
	PostEvent(kResourceLoadedEvent, 0, mpiState, pListener);
	
	return stat;	
}  

//----------------------------------------------------------------------------
tErrType CResourceModule::UnloadRsrc(U32 id, tRsrcHndl hndl, 
									tOptionFlags /*unloadOptions*/,
									const IEventListener *pListener)
{
	// 1) Free memory
	// 2) Post the event
	//
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*1
	// TODO/tp: Add custom allocator for resources?
	if (prd->ptr)
		kernel_.Free(prd->ptr);
	prd->ptr = 0;
	
	MPIInstanceState& mpiState = RetrieveMPIState(id);					//*2
	PostEvent(kResourceUnloadedEvent, 0, mpiState, pListener);

	return kNoErr;
}

//----------------------------------------------------------------------------
Boolean	CResourceModule::RsrcIsLoaded(U32 id, tRsrcHndl hndl) const
{
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);
	return (prd->ptr != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SeekRsrc(U32 id, tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions) const
{
	// 1) Find the resource and validate it is open
	// 2) Seek
	//	
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*1
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "SeekRsrc() called with invalid resource handle (0x%x)", 
						static_cast<unsigned int>(hndl));
		return kResourceInvalidErr;
	}

	if (prd->fd == -1)
	{
		dbg_.Assert(false, "CResourceModule::SeekRsrc: called with non-open resource '%s'", prd->uri.c_str());
		return kResourceNotOpenErr;
	}
		
	lseek(prd->fd, numSeekBytes, seekOptions);							//*2

	return kNoErr;
}

//----------------------------------------------------------------------------
U32 CResourceModule::TellRsrc(U32 id, tRsrcHndl hndl) const
{
	// 1) Find the resource and validate it is open
	// 2) Tell
	//	
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*1
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "SeekRsrc() called with invalid resource handle (0x%x)", 
						static_cast<unsigned int>(hndl));
		return kResourceInvalidErr;
	}

	if (prd->fd == -1)
	{
		dbg_.Assert(false, "CResourceModule::SeekRsrc: called with non-open resource '%s'", prd->uri.c_str());
		return kResourceNotOpenErr;
	}
	return lseek(prd->fd, 0, SEEK_CUR);									//*2
}

//----------------------------------------------------------------------------
tErrType CResourceModule::WriteRsrc(U32 id, tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags /*writeOptions*/,
									const IEventListener *pListener) const
{
	// 1) Check parameters for early returns
	// 2) Find the resource and validate it is open
	// 3) Write the data
	// 4) Post the read done event
	//	
	if (numBytesRequested == 0)											//*1
		return kNoErr;	
		
	if (pBuffer == NULL)
		return kInvalidParamErr;
		
	ResourceDescriptor* prd = FindRsrcPriv(id, hndl);					//*2
	if (prd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "ReadRsrc() called with invalid resource handle (0x%x)",
						static_cast<unsigned int>(hndl));
		return kResourceInvalidErr;
	}

	if (prd->fd == -1)
	{
		dbg_.Assert(false, "CResourceModule::WriteRsrc: called with non-open resource '%s'", prd->uri.c_str());
		return kResourceNotOpenErr;
	}
		
	U32 countWritten = write(prd->fd, pBuffer, numBytesRequested);		//*3
	if (pNumBytesActual != NULL)
		*pNumBytesActual = countWritten;
	
	MPIInstanceState& mpiState = RetrieveMPIState(id);					//*4
	PostEvent(kResourceWriteDoneEvent, 0, mpiState, pListener);

	return kNoErr;
}


//----------------------------------------------------------------------------
// New rsrc creation/deletion
//----------------------------------------------------------------------------
inline bool FileExists( const CPath& path )
{
	struct stat filestat;
	return stat(path.c_str(), &filestat) == 0;
}

//----------------------------------------------------------------------------
inline U32 FileSize( const CPath& path )
{
	struct stat filestat;
	int status = stat(path.c_str(), &filestat);
	return (status == 0) ? filestat.st_size : 0;
}

//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::AddNewRsrcToPackage(U32 id, tPackageHndl hPkg, 
									const CURI& fullRsrcURI, 
									tRsrcType rsrcType)
{
	// 1) Validate input package exists
	// 2) Find the next available filename, create the empty file
	// 3) Delegate the rest to AddRsrcToPackageFromFile()
	//
	PackageDescriptor* ppd = FindPackagePriv(id, hPkg);					//*1
	if (ppd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "AddNewRsrcToPackage() called with invalid package handle (0x%x)\n",
						static_cast<unsigned int>(hPkg));
		return kInvalidRsrcHndl;
	}
	
	CPath dir = ppd->folder;											//*2
	char f1[] = "A";
	char f2[] = "A";
	CPath f3 = "_";
	CPath file = CPath(f1) + f2 + f3;
	while (FileExists(dir + file))
	{
		if (f2[0] != 'Z')
			++f2[0];
		else if (f1[0] != 'Z')
		{
			f2[0] = 'A';
			++f1[0];
		}
		else
		{
			dbg_.DebugOut(kDbgLvlCritical, "AddNewRsrcToPackage() more than %d directory entries!\n", 26*26);
			return kInvalidRsrcHndl;		
		}
		file = CPath(f1) + f2 + f3;
	}
	CPath fullPath = dir + file;
	
	return AddRsrcToPackageFromFile(id, hPkg, fullRsrcURI, rsrcType, fullPath);
}

//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::AddRsrcToPackageFromFile(U32 id, tPackageHndl hPkg, 
									const CURI& fullRsrcURI, 
									tRsrcType rsrcType,
									const CPath& fullPath)
{
	// 1) Validate input package exists
	// 2) Verify package does not already contain new URI
	// 3) Open or create the file
	// 4) Create the resource and update the package with it
	// 5) Protect against unexpected behavior on app-cached handles by
	//    invalidating the existing package handles and assigning new ones
	// 6) Mark the package as dirty (closing it will flush it to disk)
	//    and return the new resource handle value
	//
	PackageDescriptor* ppd = FindPackagePriv(id, hPkg);					//*1
	if (ppd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, 
				"AddRsrcToPackageFromFile() called with invalid package handle (0x%x)",
				static_cast<unsigned int>(hPkg));
		return kInvalidPackageHndl;
	}

	tRsrcHndl hndl = FindRsrc(id, fullRsrcURI, &kNullURI);				//*2
	if (hndl != kInvalidRsrcHndl)
	{
		dbg_.DebugOut(kDbgLvlCritical, 
				"AddNewRsrcToPackage/AddRsrcToPackageFromFile() called with existing URI '%s'\n",
				fullRsrcURI.c_str());
		return kInvalidRsrcHndl;		
	}

	U32 flags = FileExists(fullPath) ? B_O_RDWR : (B_O_RDWR | B_O_CREAT);		//*3
	int fd = open(fullPath.c_str(), flags);
	if (fd == -1)
	{
		dbg_.DebugOut(kDbgLvlCritical, 
				"AddNewRsrcToPackage/AddRsrcToPackageFromFile() failed to open/create file for '%s'!\n", 
				fullRsrcURI.c_str());
		return kInvalidRsrcHndl;		
	}
	lseek(fd, 0, SEEK_END);
	
	tVersion ver = 1; 													//*4
	U32 size = FileSize(fullPath);
	CPath file = (fullPath.find(ppd->folder) == CPath::npos)
				? fullPath : fullPath.substr(ppd->folder.size());
	ResourceDescriptor res(fullRsrcURI.c_str(), rsrcType, file,
							size, size, ver, ppd);
	res.fd = fd;
	ResourceDescriptors::iterator it = 
		std::lower_bound(ppd->resources.begin(), ppd->resources.end(), res);
	it = ppd->resources.insert(it, res);
	
	if (ppd->dirtyWithRetrievedRsrcHndls)								//*5
	{
		ppd->startRsrcHndl = GetNextRsrcStartHndl(ppd->resources.size());
		ppd->dirtyWithRetrievedRsrcHndls = false;
	}
	
	ppd->dirtyWithChangedRsrcs = true;									//*6
	return ppd->startRsrcHndl + std::distance(ppd->resources.begin(), it);
}

//----------------------------------------------------------------------------
tErrType CResourceModule::RemoveRsrcFromPackage(U32 id, tPackageHndl hPkg,
		 						const CURI& fullURI, 
		 						bool deleteRsrc)
{
	// 1) Validate input package exists
	// 2) Verify package contains the resource
	// 3) Optionally delete the file that is backing the resource
	// 4) Remove the resource from the package file
	//
	PackageDescriptor* ppd = FindPackagePriv(id, hPkg);					//*1
	if (ppd == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "RemoveRsrcFromPackage() called with invalid package handle (0x%x)\n",
						static_cast<unsigned int>(hPkg));
		return kResourceInvalidErr;
	}
	
	typedef std::pair<ResourceDescriptors::iterator, 					//*2
						ResourceDescriptors::iterator> Ret;
	ResourceDescriptor	search(fullURI.c_str());
	Ret ret = std::equal_range(ppd->resources.begin(), ppd->resources.end(), search);
	if (ret.first == ret.second)
	{
		dbg_.DebugOut(kDbgLvlCritical, "RemoveRsrcFromPackage() called with non-existing URI '%s'\n",
						fullURI.c_str());
		return kResourceNotFoundErr;
	}
	
	if (deleteRsrc)														//*3
	{
		CPath fullPath = (ret.first->path[0] == '/')
				? ret.first->path
				: ppd->folder + ret.first->path;
		remove(fullPath.c_str());
	}

	ppd->resources.erase(ret.first);									//*4
	ppd->dirtyWithChangedRsrcs = true;
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()



//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CResourceModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if (sinst == NULL)
			sinst = new CResourceModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG

// EOF
