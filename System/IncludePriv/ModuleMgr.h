#ifndef LF_BRIO_MODULEMGR_H
#define LF_BRIO_MODULEMGR_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ModuleMgr.h
//
// Description:
//		The boot system's ModuleMgr class is responsible for the following:
//
//		* Finding all system modules on devices
//		* Maintaining a table of found modules and their versions
//		* Managing module lifetimes using a lazy-loading policy
//		* Maintaining a policy of one instanciated module per subsystem
//		* Connecting module interface objects (ICoreMPI-derived classes)
//			to modules through Connect()/Disconnect() member functions
//		* Enforce versioning constraints between interface objects
//			and modules
//
//		One key distinction between system module objects (ICoreModule-derived)
//		and lightweight module interface objects (ICoreMPI-derived) is that
//		the former need to be replacable, so they are dynamically linked,
//		whereas the latter are statically linked.  Interface objects provide
//		the subsystem API to the application code that uses them, so it doesn't
//		make sense to update that interface for an existing application.
//
//==============================================================================
// NOTE: There are many subtle issues with module versioning that are not yet worked out.

#include <CoreModule.h>
#include <StringTypes.h>

class CModuleMgr {
public:
	static CModuleMgr* Instance()
	{
		if( mpinst == NULL )
			mpinst = new CModuleMgr;
		return mpinst;
	}
	tErrType	FindModules(ConstPtrCURI pURI);
	tErrType	Connect(ICoreModule*& ptr, const CString& name, 
						tVersion version);
	tErrType	Disconnect(const CString& name);

private:
	CModuleMgr();
	~CModuleMgr();
	static CModuleMgr 	*mpinst;
};


/*
//==============================================================================
// Pseudocode for ModlueMgr.cpp file implementation
//==============================================================================
CModuleMgr* CModuleMgr::mpinst = NULL;
class CModuleMgrImpl {
public:
	CModuleMgrImpl()	{}
	~CModuleMgrImpl()	{}

	tErrType	FindModules(ConstPtrCURI pURI);	// populates mFoundModulesList
	{
		if (GetMajorVersion(version) > GetMajorVersion(kEventMgrModuleVersion))
			return NULL;	// expecting newer version than we can provide
	}
	
	// Connect() either uses existing instance,
	// or loads the shared object path, gets its "Instance()" function pointer
	// and creates a new instance.
	//
	// It is an error if a request for moduleV2 comes when moduleV1 is loaded
	// and has a non-zero connect_count.  If moduleV1 it has a zero connect
	// count, the existing modulev1 object is destroyed and a new modulev2 is
	// created in its place.
	//
	// The "version" parameter passed by the ICoreMPI-derived object
	// is the version of the module against which that object was built.
	// The three cases are
	//		ICoreMPI-provided verson == module version: great, succeed
	//		ICoreMPI-provided verson <  module version: if module maintains backwards compatability, succeed else fail
	//		ICoreMPI-provided verson >  bad, fail.  Interface object may try to invoke non-existant functionality.
	
	tErrType	Connect(ICoreModulePtr& ptr, ConstPtrCURI pURI, tVersion version)
	{
		// Lots of versioning TODOs in here
		FoundModule* pFound = find(pURI, version);
		ConnectedModule* pModule = find(pFound);
		if( pModule != NULL )
		{
			++pModule->connect_count;
			ptr = pModule->ptr;
			return kNoErr;
		}
		libinst = OSCallToGetLibraryInstance(pModule->sopath);
		funptr = OSCallToGetFunctionPtr(libinst, "CreateInstance");
		ptr = (*funptr)();
		pModule = new ConnectedModule;
		pModule->name = pModule->name;
		pModule->sopath = pModule->sopath;
		pModule->context_count = 1;
		pModule->ptr = ptr;
		// fix up list by chaining "next" ptrs
		return kNoErr;
	}
	tErrType	Disconnect(ConstPtrCURI pURI);
	{
		ConnectedModule* pModule = find(pURI);
		--pModule->connect_count;
		if( pModule->connect_count == 0 )
		{
			// optionally unload the module
			libinst = OSCallToGetLibraryInstance(pModule->sopath);
			funptr = OSCallToGetFunctionPtr(libinst, "DestroyInstance");
			(*funptr)();
			delete pModule;
			// fix up list by chaining "next" ptrs
		}
		return kNoErr;
	}
	
	struct FoundModule {
		CURI			name;
		tVersion		version;
		CPath			sopath;
		FoundModule 	*next;
	};
	struct ConnectedModule {
		CURI			name;
		CPath			sopath;
		U32				connect_count;
		ICoreModulePtr	ptr;
		ConnectedModule *next
	};
	FoundModule		*mFoundModulesList;
	ConnectedModule	*mConnectedModulesList;
	
	// unresolved issues:
	//   what to do if two modules with same name and version are found
};

static CModuleMgrImpl	g_impl;

//==============================================================================
// Forward ModlueMgr members to the implementation class
//==============================================================================
CModuleMgr::CModuleMgr()	: mpimpl(new CModuleMgrImp)	{}
CModuleMgr::~CModuleMgr()	{ delete mpimpl; }
tErrType CModuleMgr::FindModules(ConstPtrCURI pURI)
{
	return g_impl.FindModlues(pURI);
}
tErrType CModuleMgr::Connect(ICoreModulePtr& ptr, ConstPtrCURI pURI, tVersion version)
{
	return g_impl.Connect(ptr, pURI, version);
}
tErrType CModuleMgr::Disonnect(ConstPtrCURI pURI)
{
	return g_impl.Disconnect(pURI);
}


//==============================================================================
// Pseudocode for CCoreMPI-derived class ModuleMgr usage
//==============================================================================
class CMyMPI : public CCoreMPI
{
	CMyModule		*mpinst;
	const CURL		kMyMPIName = "...";
	const tVersion	kMyMPIBoundVersion = MakeVersion(1,0);
public:
	CMyMPI::CMyMPI()
	{
		CMyModule* mpinst = reinterpret_cast<CMyModule*>
				(CModuleMgr::Instance()->Connect(kMyMPIName, kMyMPIBoundVersion));
	}
	CMyMPI::CMyMPI()
	{
		CModuleMgr::Instance()->Disonnect(
	}
}
*/

#endif // LF_BRIO_MODULEMGR_H

// eof
