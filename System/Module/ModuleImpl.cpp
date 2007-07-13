//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ModuleImpl.cpp
//
// Description:
//		See the IncludePriv/Module.h file
//
//==============================================================================

#include <SystemTypes.h>
#include <Module.h>
#include <ModulePriv.h>
#include <CoreModule.h>
#include <SystemErrors.h>
#include <BootSafeKernelMPI.h>

LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// CModuleMgrImpl
//==============================================================================	   
namespace
{
	//==========================================================================
	// This union is some syntactic grease that allows us to take the void*
	// returned by dlsym() and "cast" it to an appropriate function pointer
	// when the "-pedantic-error" compiler switch is enabled.
	// Directly casting an object* to a function* causes a compiler error when
	// "-pedantic-error" is enabled.
	//==========================================================================   
	union ObjPtrToFunPtrConverter
	{
		void* 				voidptr;
		pFnCreateInstance	pfnCreate;
		pFnDestroyInstance	pfnDestroy;
	};

	const int kMaxModuleName	= 30;
	const int kMaxPath			= 256;	// FIXME: common handling 
	
	class CModuleMgrImpl
	{
	private:
		//----------------------------------------------------------------------
		struct FoundModule {
			char			name[kMaxModuleName];
			char			sopath[kMaxPath];
			tVersion		version;
		};
		//----------------------------------------------------------------------
		struct ConnectedModule {
			char			name[kMaxModuleName];
			char			sopath[kMaxPath];
			tHndl			handle;
			U32				connect_count;
			ICoreModule*	ptr;
			// FIXME/tp: initializing ctor
		};
		//----------------------------------------------------------------------
		FoundModule		*mpFoundModulesList;
		ConnectedModule	*mpConnectedModulesList;
		U16				mNumFound;
		U16				mNumConnected;
			
		//----------------------------------------------------------------------
		Boolean AddedValidModule( FoundModule* pModule, const CPath& path )
		{
			// TODO: Either parse version from file name or load and
			// querry the library (probably the former)
			//
			size_t idx = path.rfind('/');
			CPath name = path.substr(idx+1);
			size_t len = name.size();
			if( len <= 6 || name[0] == '.' )
				return false;
			CString temp = name.substr(3, len-6);
			pModule->version = 2;
			strncpy(pModule->name, temp.c_str(), kMaxModuleName);
			strncpy(pModule->sopath, path.c_str(), kMaxPath);
//printf("Module: %s  %s\n", pModule->name, pModule->sopath);
			return true;
		}

		//----------------------------------------------------------------------
		FoundModule* FindBestModuleMatch( const CString& name, tVersion version )
		{
			FoundModule* pModule = mpFoundModulesList;
			for( U16 ii = mNumFound; ii > 0; --ii, ++pModule )
			{
				if( pModule->name == name )
				{
					// TODO: Implement more sophisticated version matching scheme:
					//	(use highest version with same major version number)
					//	(if no major version match, match if module version > MPI version)
					if( version == pModule->version )
						return pModule;
				}
			}
			return NULL;
		}

		//----------------------------------------------------------------------
		ConnectedModule* FindCachedModule( const CString& name, tVersion /*version*/ )
		{
			ConnectedModule* pModule = mpConnectedModulesList;
			for( U16 ii = mNumConnected; ii > 0; --ii, ++pModule )
			{
				if( pModule->name == name )
				{
					// TODO: Make sure we have a version match
					return pModule;
				}
			}
			return NULL;
		}

		//----------------------------------------------------------------------
		ConnectedModule* FindCachedModule( const ICoreModule* ptr )
		{
			ConnectedModule* pModule = mpConnectedModulesList;
			for( U16 ii = mNumConnected; ii > 0; --ii, ++pModule )
			{
				if( pModule->ptr == ptr )
				{
					// TODO: Make sure we have a version match
					return pModule;
				}
			}
			return NULL;
		}
		
	public:
		//----------------------------------------------------------------------
		CModuleMgrImpl()
			: mpFoundModulesList(NULL), mpConnectedModulesList(NULL),
			mNumFound(0), mNumConnected(0)
		{
		}
		
		//----------------------------------------------------------------------
		~CModuleMgrImpl()
		{
			// FIXME: Use Kernel::Free()
			free(mpFoundModulesList);
			free(mpConnectedModulesList);
		}
	
		//----------------------------------------------------------------------
		tErrType FindModules()
		{
			static const char* paths[1];
			CPath path = GetModuleLibraryLocation();
			paths[0] = path.c_str();
			
			// FIXME/tp: count first to allocate only enough memory needed
			// FIXME/tp: KernelMPI for malloc
			const int kMaxModuleCount = 100;
			mpFoundModulesList = reinterpret_cast<FoundModule*>(malloc(kMaxModuleCount * sizeof(FoundModule)));
			mpConnectedModulesList = reinterpret_cast<ConnectedModule*>(malloc(kMaxModuleCount * sizeof(ConnectedModule)));
			
			CBootSafeKernelMPI	kernel;
			mNumFound = 0; 
			for (size_t ii = 0; ii < ArrayCount(paths); ++ii)
			{
				std::vector<CPath> files = kernel.GetFilesInDirectory(paths[ii]);
				for (size_t jj = 0; jj < files.size(); ++jj)
				{
					if (AddedValidModule((mpFoundModulesList + mNumFound), files[jj]))
						++mNumFound;
				}
			}
			if (mNumFound == 0)
			{
				kernel_.Printf("BOOTFAIL: No modules found in: %s\n", path.c_str());
				kernel_.PowerDown();
			}
			return kNoErr;
		}

		
		//----------------------------------------------------------------------
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
		
		tErrType Connect(ICoreModule*& ptr, const CString& name, 
						tVersion version)
		{
			// FIXME: Lots of versioning TODOs in here
			// 1) Check if module is already loaded and return module ptr
			// 2) Otherwise find a module that will work
			// 3) Load the library
			// 4) Resolve the library's CreateInstance() symbol
			// 5) Invoke the library's CreateInstance() function
			// 6) Fill in a new ConnectedModule slot with its info
			//
			ConnectedModule* pModule = FindCachedModule(name, version);		//*1
			if( pModule != NULL )
			{
				++pModule->connect_count;
				ptr = pModule->ptr;
				return kNoErr;
			}
			
			FoundModule* pFound = FindBestModuleMatch(name, version);		//*2
			if( pFound == NULL )
			{
				kernel_.Printf("BOOTFAIL: Failed to find match for module: %s\n", name.c_str());
				kernel_.PowerDown();
			}
			
			tHndl module = kernel_.LoadModule(pFound->sopath);				//*3
			if( module == kInvalidHndl )
			{
				kernel_.Printf("BOOTFAIL: Failed to load found module at sopath: %s\n", pFound->sopath);
				kernel_.PowerDown();
			}
			
		    ObjPtrToFunPtrConverter fp;										//*4
		    fp.voidptr = kernel_.RetrieveSymbolFromModule(module, kCreateInstanceFnName);
		    if( fp.voidptr == NULL )
		    {
				kernel_.Printf("BOOTFAIL: Failed to find CreateInstance() for: %s\n", pFound->sopath);
				kernel_.PowerDown();
		    }
		    
			ptr = (*(fp.pfnCreate))(version);								//*5
			if( !ptr )
			{
				kernel_.Printf("BOOTFAIL: CreateInstance() failed for: %s\n", pFound->sopath);
				kernel_.PowerDown();
			}

			pModule = mpConnectedModulesList + mNumConnected;				//*6
			++mNumConnected;
			pModule->handle = module;
			strcpy(pModule->name, pFound->name);
			strcpy(pModule->sopath, pFound->sopath);
			pModule->connect_count = 1;
			pModule->ptr = ptr;

			return kNoErr;
		}

		//----------------------------------------------------------------------
		tErrType Disconnect(const ICoreModule* ptr)
		{
			ConnectedModule* pModule = FindCachedModule(ptr);
			if( pModule )
			{
				--pModule->connect_count;
				if( pModule->connect_count == 0 )
				{
//					DestroyModuleInstance(pModule);
//					--mNumConnected;
					// FIXME/tp: remove module from list by copying higher entries down
				}
			}
			return kNoErr;
		}
		
		//----------------------------------------------------------------------
		void DestroyModuleInstance(ConnectedModule* pModule)
		{
		    ObjPtrToFunPtrConverter fp;
		    fp.voidptr = kernel_.RetrieveSymbolFromModule(pModule->handle, kDestroyInstanceFnName);
		    if( fp.voidptr != NULL )
		    	(*(fp.pfnDestroy))(pModule->ptr);
		    else
		    {
				kernel_.Printf("BOOTFAIL: Failed to find DestroyInstance() for: %s\n", pModule->sopath);
				kernel_.PowerDown();
		    }
		    kernel_.UnloadModule(pModule->handle);
		}
		
		// unresolved issues:
		//   what to do if two modules with same name and version are found
	private:
		// Disable copy semantics
		CModuleMgrImpl(const CModuleMgrImpl&);
		CModuleMgrImpl& operator=(const CModuleMgrImpl&);
		CBootSafeKernelMPI		kernel_;
	};

	CModuleMgrImpl	g_impl;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Module
//============================================================================
LF_USING_BRIO_NAMESPACE()
//------------------------------------------------------------------------
extern "C" tErrType FindModules()
{
	return g_impl.FindModules();
}
//------------------------------------------------------------------------
extern "C" tErrType Connect(void** pModule, const char* name, tVersion version)
{
	static tErrType init = FindModules();	// FIXME/tp: temp startup, replace with real call from boot module
	init = !!init;		// bogus line to revmove "unused variable 'init'" warning

	ICoreModule* ptr = NULL;
	tErrType status =  g_impl.Connect(ptr, name, version);
	if( status == kNoErr )
		*pModule = ptr;
	return status;
}
//------------------------------------------------------------------------
extern "C" tErrType Disconnect(const ICoreModule* ptr)
{
	return g_impl.Disconnect(ptr);
}

	// NOTE: For LF_MONOLITHIC_DEBUG builds, the Module.cpp file never loads this
	// library or calls its functions.
	
// eof
