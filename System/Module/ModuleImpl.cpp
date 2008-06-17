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

	//--------------------------------------------------------------------------   
	extern "C" void ExitModuleManagerLib(void);
	
	//--------------------------------------------------------------------------   
	class CModuleMgrImpl
	{
	private:
		//----------------------------------------------------------------------
		struct FoundModule {
			CString			name;
			CPath			sopath;
			tVersion		version;
			// initializing ctor
			FoundModule() : version(kUndefinedVersion) {}
		};
		//----------------------------------------------------------------------
		struct ConnectedModule {
			CString			name;
			CPath			sopath;
			tHndl			handle;
			U32				connect_count;
			ICoreModule*	ptr;
			// initializing ctor
			ConnectedModule() : handle(kInvalidHndl), connect_count(0), ptr(NULL) {}
		};
		//----------------------------------------------------------------------
		std::vector<FoundModule>		mFoundModulesList;
		std::vector<ConnectedModule>	mConnectedModulesList;
		
		//----------------------------------------------------------------------
		void AddValidModule( const CPath& path )
		{
			// TODO: Either parse version from file name or load and
			// querry the library (probably the former)
			//
			size_t idx = path.rfind('/');
			CPath name = path.substr(idx+1);
			size_t len = name.size();
			if( len > 6 && name[0] != '.' )
			{
				CString temp = name.substr(3, len-6);
				FoundModule	module;
				module.version = 2;
				module.name = temp;
				module.sopath = path;
				mFoundModulesList.push_back(module);
			}
		}

		//----------------------------------------------------------------------
		FoundModule* FindBestModuleMatch( const CString& name, tVersion version )
		{
			for( int ii = mFoundModulesList.size() - 1; ii >= 0; --ii )
			{
				if( mFoundModulesList[ii].name == name )
				{
					// TODO: Implement more sophisticated version matching scheme:
					//	(use highest version with same major version number)
					//	(if no major version match, match if module version > MPI version)
					if( version == mFoundModulesList[ii].version )
						return &mFoundModulesList[ii];
				}
			}
			return NULL;
		}

		//----------------------------------------------------------------------
		ConnectedModule* FindCachedModule( const CString& name, tVersion /*version*/ )
		{
			for( int ii = mConnectedModulesList.size() - 1; ii >= 0; --ii )
			{
				if( mConnectedModulesList[ii].name == name )
				{
					// TODO: Make sure we have a version match
					return &mConnectedModulesList[ii];
				}
			}
			return NULL;
		}

		//----------------------------------------------------------------------
		ConnectedModule* FindCachedModule( const ICoreModule* ptr )
		{
//			printf("FindCachedModule:: size of connected modules list: %d\n",
//					( int)mConnectedModulesList.size());
			
			for( int ii = mConnectedModulesList.size() - 1; ii >= 0; --ii )
			{
//				printf("FindCachedModule:: looking for ptr: 0x%x, comparing with: 0x%x (module name: %s)\n",
//						(unsigned int)ptr, (unsigned int)mConnectedModulesList[ii].ptr, mConnectedModulesList[ii].name.c_str());
				if( mConnectedModulesList[ii].ptr == ptr )
				{
					// TODO: Make sure we have a version match
					return &mConnectedModulesList[ii];
				}
			}
			return NULL;
		}
		
	public:
		//----------------------------------------------------------------------
		CModuleMgrImpl()
		{
			atexit(ExitModuleManagerLib);
		}
		
		//----------------------------------------------------------------------
		~CModuleMgrImpl()
		{
		}
	
		//----------------------------------------------------------------------
		tErrType FindModules()
		{
			static const char* paths[1];
			CPath path = GetModuleLibraryLocation();
			paths[0] = path.c_str();
						
			// Only need to populate the found module list once
			if (mFoundModulesList.size() > 0)
				return kNoErr;
			
			// CBootSafeKernelMPI	kernel;
			for (size_t ii = 0; ii < ArrayCount(paths); ++ii)
			{
				std::vector<CPath> files = kernel_.GetFilesInDirectory(paths[ii]);
				for (size_t jj = 0; jj < files.size(); ++jj)
					AddValidModule(files[jj]);
			}
			if (mFoundModulesList.size() == 0)
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
			
			tHndl hModule = kernel_.LoadModule(pFound->sopath);				//*3
			if( hModule == kInvalidHndl )
			{
				kernel_.Printf("BOOTFAIL: Failed to load found module at sopath: %s\n", pFound->sopath.c_str());
				kernel_.PowerDown();
			}
			
		    ObjPtrToFunPtrConverter fp;										//*4
		    fp.voidptr = kernel_.RetrieveSymbolFromModule(hModule, kCreateInstanceFnName);
		    if( fp.voidptr == NULL )
		    {
				kernel_.Printf("BOOTFAIL: Failed to find CreateInstance() for: %s\n", pFound->sopath.c_str());
				kernel_.PowerDown();
		    }
		    
			ptr = (*(fp.pfnCreate))(version);								//*5
			if( !ptr )
			{
				kernel_.Printf("BOOTFAIL: CreateInstance() failed for: %s\n", pFound->sopath.c_str());
				kernel_.PowerDown();
			}

			ConnectedModule module;											//*6
			module.handle = hModule;
			module.name = pFound->name;
			module.sopath = pFound->sopath;
			module.connect_count = 1;
			module.ptr = ptr;
			mConnectedModulesList.push_back(module);

			return kNoErr;
		}

		//----------------------------------------------------------------------
		void RemoveCachedModule(ConnectedModule* pModule)
		{
			std::vector<ConnectedModule>::iterator tempIterator;
			
//			printf("RemoveCachedModule:: size of connected modules list: %d\n",
//					(int)mConnectedModulesList.size());

			tempIterator = mConnectedModulesList.begin(); 
			while ( tempIterator != mConnectedModulesList.end() )
				{
//				printf("RemoveCachedModule:: looking for ptr: 0x%x (name: %s), comparing with: 0x%x (name %s)\n",
//							(unsigned int)tempIterator->ptr, tempIterator->name.c_str(), 
//							(unsigned int)pModule->ptr, pModule->name.c_str() );

				if( (ICoreModule*)tempIterator->ptr == (ICoreModule*)pModule->ptr )
					{
//						printf("RemoveCachedModule:: removing: * %s * module from list.\n",
//							tempIterator->name.c_str() );						
						// TODO: Make sure we have a version match
						mConnectedModulesList.erase(tempIterator);
						break;
					}
					tempIterator++;
				}
		}

		//----------------------------------------------------------------------
		void DestroyModuleInstance(ConnectedModule* pModule)
		{
		//	printf("DestroyModuleInstance:: destroying ptr: 0x%x; name: %s)\n",
		//			(unsigned int)pModule->ptr, pModule->name.c_str() );

					ObjPtrToFunPtrConverter fp;
		    fp.voidptr = kernel_.RetrieveSymbolFromModule(pModule->handle, kDestroyInstanceFnName);
		    if( fp.voidptr != NULL )
		    	(*(fp.pfnDestroy))(pModule->ptr);
		    else
		    {
				kernel_.Printf("BOOTFAIL: Failed to find DestroyInstance() for: %s\n", pModule->sopath.c_str());
				kernel_.PowerDown();
		    }
		    kernel_.UnloadModule(pModule->handle);
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
#if 1	// BUGFIX/dm: Unload Brio module as intended
					DestroyModuleInstance(pModule);
					RemoveCachedModule(pModule);
#endif
				}
			}
			return kNoErr;
		}
		
		//----------------------------------------------------------------------
		void DestroyAllModules(void)
		{
			// Destroy all modules remaining in connected list.
			// Called at process exit time via atexit() handler.
			for( int ii = mConnectedModulesList.size() - 1; ii >= 0; --ii )
			{
				DestroyModuleInstance(&mConnectedModulesList[ii]);
			}
			mConnectedModulesList.clear();
		}

		// TODO: unresolved issues:
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

//--------------------------------------------------------------------------   
extern "C" void ExitModuleManagerLib(void)
{
	g_impl.DestroyAllModules();
}

// NOTE: For LF_MONOLITHIC_DEBUG builds, the Module.cpp file never loads this
// library or calls its functions.
	
// eof
