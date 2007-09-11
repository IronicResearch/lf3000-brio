//==============================================================================
//
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio Kernel module.
//
//==============================================================================

//#include <stdarg.h>		// for varargs
#include <assert.h>		// for assert()
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>

//#include <SystemTypes.h>
#include <SystemErrors.h>
#include <Module.h>
#include <KernelMPI.h>
#include <BootSafeKernelMPI.h>
#include <KernelPriv.h>
#include <StringTypes.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// MPI Name & Version
//
//		! NOTE: this may not match the MPI name & version implemented by the
//				installed module.  Always use the MPI's core functions to
//				resolve which version of the MPI is installed & available 
//				at runtime.
//==============================================================================
static const CString kMPIName = "KernelMPI";
static const CString kBSMPIName = "BootSafeKernelMPI";


//==============================================================================
// Local implementation functions
//==============================================================================
namespace
{
	//--------------------------------------------------------------------------
	void Printf_Impl( const char * formatString, ... )
				__attribute__ ((format (printf, 1, 2)));
	void VPrintf_Impl( const char * formatString, va_list arguments )
				__attribute__ ((format (printf, 1, 0)));
	
	//--------------------------------------------------------------------------
	void VPrintf_Impl( const char * formatString, va_list arguments )
	{
		vprintf(formatString, arguments);
	}
	
	//--------------------------------------------------------------------------
	void Printf_Impl( const char * formatString, ... )
	{
		va_list arguments;
		va_start(arguments, formatString);
		VPrintf_Impl(formatString, arguments);
		va_end(arguments);
	}

	//--------------------------------------------------------------------------
	void PowerDown_Impl()
	{
		Printf_Impl("\n\n");
		fflush(stdout);
		fflush(stderr);
		assert(!"PowerDown!");
	}
	
	//--------------------------------------------------------------------------
	tPtr Malloc_Impl(U32 size)
	{
		tPtr ptr = malloc( size );
		if (!ptr)
		{
			Printf_Impl("BRIO FATAL: Memory allocation error, shutting down (allocation size was 0x%x\n", 
						static_cast<unsigned int>(size));
			PowerDown_Impl();
		}
		return ptr;
	}
	
	//--------------------------------------------------------------------------
	void Free_Impl(tPtr ptr)
	{
		free(ptr);
	}

	//--------------------------------------------------------------------------
	std::vector<CPath> GetFilesInDirectory_Impl( const CPath& dirIn )
	{
		const CPath	kCurrentDirString = ".";
		const CPath	kParentDirString = "..";
	
		CPath dir = AppendPathSeparator(dirIn);
		std::vector<CPath>	files;
		DIR *dirp = opendir(dirIn.c_str());
		if( dirp == NULL )
			return files;
		
		struct dirent *dp;
		do
		{
			if( (dp = readdir(dirp)) != NULL
					&& dp->d_name != kCurrentDirString
					&& dp->d_name != kParentDirString )
				files.push_back(dir + dp->d_name);
		} while(dp != NULL);
			
		closedir(dirp);
		return files;
	}
	
	//------------------------------------------------------------------------------
	Boolean IsDirectory_Impl( const CPath& dir )
	{
		struct stat filestat;
		if (stat(dir.c_str(), &filestat) == 0
				&& filestat.st_mode & S_IFDIR)
			return true;
		return false;
	}
	
	//------------------------------------------------------------------------------
	tHndl LoadModule_Impl( const CPath& dir )
	{
		void* pLib = dlopen(dir.c_str(), RTLD_LAZY);
		if (pLib == NULL)
		{
			const char *err = dlerror();
			Printf_Impl("BRIO WARNING: unable to open module '%s', %s\n", dir.c_str(), err);
		}
		return reinterpret_cast<tHndl>(pLib);
	}
	
	//------------------------------------------------------------------------------
	void* RetrieveSymbolFromModule_Impl( tHndl module, const CString& symbol )
	{
		if (module == kInvalidHndl)
			return NULL;
	    dlerror();
	    void* pLib = reinterpret_cast<void*>(module);
		void* ptr = dlsym(pLib, symbol.c_str());
		if (ptr == NULL)
		{
			const char *err = dlerror();
			Printf_Impl("BRIO WARNING: unable to retrieve symbol '%s', %s\n", 
					symbol.c_str(), err);
		}
		return ptr;
	}
	
	//------------------------------------------------------------------------------
	void UnloadModule_Impl( tHndl module )
	{
		if (module != kInvalidHndl)
		{
			void* pLib = reinterpret_cast<void*>(module);
			dlclose(pLib);
		}
	}
}


//==============================================================================
// CKernelMPI implementation
//==============================================================================
CKernelMPI::CKernelMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kKernelModuleName, 
									kKernelModuleVersion);
	pModule_ = reinterpret_cast<CKernelModule*>(pModule);
}

//----------------------------------------------------------------------------
CKernelMPI::~CKernelMPI()
{
	Module::Disconnect(pModule_);
}

//============================================================================
// Informational functions
//============================================================================
Boolean CKernelMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CKernelMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CKernelMPI::GetModuleVersion() const
{
	if (!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CKernelMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CKernelMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

//==============================================================================
// Tasks
//==============================================================================
tErrType CKernelMPI::CreateTask(tTaskHndl& hndl, const tTaskProperties& props,
								const char* pDebugName)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CreateTask(hndl, props, pDebugName);
}

//----------------------------------------------------------------------------
tErrType CKernelMPI::JoinTask(tTaskHndl pHndl, tPtr& threadReturnValue) 
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->JoinTask(pHndl, threadReturnValue);
}
//----------------------------------------------------------------------------
tErrType CKernelMPI::CancelTask(tTaskHndl hndl)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CancelTask(hndl);
}

//----------------------------------------------------------------------------
// This service returns the currently active task pointer.
tTaskHndl CKernelMPI::GetCurrentTask() const
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetCurrentTask();
}

//----------------------------------------------------------------------------
int CKernelMPI::GetTaskPriority(tTaskHndl hndl) const
{
	if (!pModule_)
		return 0;
	return pModule_->GetTaskPriority(hndl);
}	

//----------------------------------------------------------------------------
int CKernelMPI::GetTaskSchedulingPolicy(tTaskHndl hndl) const
{
	if (!pModule_)
		return 0;	
	return pModule_->GetTaskSchedulingPolicy(hndl);
}	

//----------------------------------------------------------------------------
void CKernelMPI::TaskSleep(U32 msec) const
{
	if (pModule_)
		pModule_->TaskSleep(msec);
}	

//============================================================================
// allocating memory from the System heap
//============================================================================
tPtr CKernelMPI::Malloc(U32 size)
{
	return Malloc_Impl(size);
}

//------------------------------------------------------------------------------
void CKernelMPI::Free(tPtr ptr)
{
	Free_Impl(ptr);
}

//============================================================================
// Message Queues
//============================================================================
tErrType CKernelMPI::OpenMessageQueue(tMessageQueueHndl& hndl,
								const tMessageQueuePropertiesPosix& properties,
								const char* pDebugName)
{
 	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenMessageQueue(hndl, properties, pDebugName);
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CloseMessageQueue(tMessageQueueHndl hndl,
								const tMessageQueuePropertiesPosix& properties)
{
 	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseMessageQueue(hndl, properties);  
}

tErrType CKernelMPI::ClearMessageQueue(tMessageQueueHndl hndl)
{
  	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ClearMessageQueue(hndl);  
}

//------------------------------------------------------------------------------
int CKernelMPI::GetMessageQueueNumMessages(tMessageQueueHndl hndl) const
{
  	if(!pModule_)
		return 0;
	return pModule_->GetMessageQueueNumMessages(hndl);  
}
    
//------------------------------------------------------------------------------
 tErrType CKernelMPI::SendMessage(tMessageQueueHndl hndl, const CMessage& msg) 
{
  	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SendMessage(hndl, msg);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SendMessageOrWait(tMessageQueueHndl hndl, 
										const CMessage& msg, U32 timeoutMs )
{
  	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SendMessageOrWait(hndl, msg, timeoutMs);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::ReceiveMessage( tMessageQueueHndl hndl, CMessage* msg_ptr, 
									U32 maxMessageSize )
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->ReceiveMessage( hndl, msg_ptr, maxMessageSize );  
}

tErrType CKernelMPI::ReceiveMessageOrWait( tMessageQueueHndl hndl, CMessage* msg_ptr, 
									 U32 maxMessageSize, U32 timeoutMs )
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->ReceiveMessageOrWait( hndl, msg_ptr, maxMessageSize, timeoutMs );  
}
				
//==============================================================================
// Time & Timers
//==============================================================================
// Elapsed time since System startup in milliscond

U32 CKernelMPI::GetElapsedTimeAsMSecs() const
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->GetElapsedTimeAsMSecs();  
}

// Elapsed time since System startup in microsecond
U64 CKernelMPI::GetElapsedTimeAsUSecs() const
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->GetElapsedTimeAsUSecs();  
}

	tErrType 	CKernelMPI::GetHRTAsUsec(U32 &uSec) const
	{
	  	if(!pModule_)
			return kMPINotConnectedErr;
		
		return pModule_->GetHRTAsUsec(uSec);  
	
	}	 

	// Elapsed time since System startup in seconds
	tErrType	CKernelMPI::GetElapsedAsSec(U32 &sec) const
	{
  		if(!pModule_)
			return kMPINotConnectedErr;
		
		return pModule_->GetElapsedAsSec(sec);  
	
	}		 

	// Elapsed time since System startup as structure
	tErrType	CKernelMPI::GetElapsedTimeAsStructure(Current_Time &curTime) const
	{
  		if(!pModule_)
			return kMPINotConnectedErr;
		
		return pModule_->GetElapsedTimeAsStructure( curTime );  
	}	

//------------------------------------------------------------------------------
//tErrType CKernelMPI::CreateTimer(tTimerHndl& hndl, 
// 						const tTimerProperties& props, const char* pDebugName )

tTimerHndl 	CKernelMPI::CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName )
{
  	if(!pModule_)
		return kInvalidTimerHndl;
	return pModule_->CreateTimer(callback, props, pDebugName);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyTimer( tTimerHndl hndl )
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->DestroyTimer(hndl);  
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::ResetTimer( tTimerHndl hndl, const tTimerProperties& props )
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ResetTimer(hndl, props);
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::StartTimer(tTimerHndl hndl, const tTimerProperties& props)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->StartTimer(hndl, props);
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::StopTimer(tTimerHndl hndl)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->StopTimer(hndl);  
}
//------------------------------------------------------------------------------
tErrType CKernelMPI::PauseTimer(tTimerHndl hndl, saveTimerSettings& saveValue)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->PauseTimer(hndl, saveValue );
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::ResumeTimer(tTimerHndl hndl, saveTimerSettings& saveValue)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ResumeTimer(hndl, saveValue);  
}
	
//------------------------------------------------------------------------------
// elapsed time in milliseconds
tErrType CKernelMPI::GetTimerElapsedTime(tTimerHndl hndl, U32* pMs, U32* pUs) const
                                                                    
{
  	if (!pModule_)
		return 0;
	return pModule_->GetTimerElapsedTime(hndl, pMs, pUs );  
}
	
//------------------------------------------------------------------------------
// time remaining in milliseconds
tErrType CKernelMPI::GetTimerRemainingTime(tTimerHndl hndl, U32* pMs, U32* pUs) const
{
  	if (!pModule_)
		return 0;
	return pModule_->GetTimerRemainingTime(hndl, pMs, pUs );  
}

// fixme rdg: move the rest of these into the kernel module at some point
//------------------------------------------------------------------------------
// Mutexes
//------------------------------------------------------------------------------
tErrType CKernelMPI::InitMutexAttributeObject( tMutexAttr& mutexAttr )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitMutexAttributeObject( mutexAttr );  

}	


tErrType CKernelMPI::InitMutex( tMutex& mutex, const tMutexAttr& attributes )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitMutex( mutex, attributes );  
}
//------------------------------------------------------------------------------
tErrType CKernelMPI::DeInitMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->DeInitMutex( mutex );  
}

//------------------------------------------------------------------------------
S32 CKernelMPI::GetMutexPriorityCeiling(const tMutex& mutex) const
{
  	if (!pModule_)
		return 0;
	return pModule_->GetMutexPriorityCeiling( mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority )
{
  	if (!pModule_)
		return 0;
	return pModule_->SetMutexPriorityCeiling( mutex, prioCeiling, pOldPriority );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::LockMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->LockMutex( mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TryLockMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->TryLockMutex( mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::UnlockMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->UnlockMutex( mutex );  
}

//------------------------------------------------------------------------------
// Conditions 
//------------------------------------------------------------------------------

tErrType CKernelMPI::BroadcastCond( tCond& cond )
{
  	if (!pModule_)
		return 0;
	return pModule_->BroadcastCond( cond );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyCond( tCond& cond )
{
  	if (!pModule_)
		return 0;
	return pModule_->DestroyCond( cond );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::InitCond( tCond& cond, const tCondAttr& attr )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitCond( cond, attr );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SignalCond( tCond& cond )
{
  	if (!pModule_)
		return 0;
	return pModule_->SignalCond( cond );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime )
{
  	if (!pModule_)
		return 0;
	return pModule_->TimedWaitOnCond( cond, mutex, pAbstime );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::WaitOnCond( tCond& cond, tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->WaitOnCond( cond, mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyCondAttr( tCondAttr& attr )
{
  	if (!pModule_)
		return 0;
	return pModule_->DestroyCondAttr( attr );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::GetCondAttrPShared( const tCondAttr& attr, int* pShared )
{
  	if (!pModule_)
		return 0;
	return pModule_->GetCondAttrPShared( attr, pShared );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::InitCondAttr( tCondAttr& attr )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitCondAttr( attr );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetCondAttrPShared( tCondAttr* pAttr, int shared )

{
  	if (!pModule_)
		return 0;
	return pModule_->SetCondAttrPShared( pAttr, shared );  
}



//----------------------------------------------------------------------------
void CKernelMPI::Printf( const char * formatString, ... ) const
{
	va_list arguments;
	va_start(arguments, formatString);
	VPrintf_Impl(formatString, arguments);
	va_end(arguments);
}

//----------------------------------------------------------------------------
void CKernelMPI::VPrintf( const char * formatString, va_list arguments ) const
{
	VPrintf_Impl(formatString, arguments);
}

//----------------------------------------------------------------------------
void CKernelMPI::PowerDown() const
{
	PowerDown_Impl();
}

//------------------------------------------------------------------------------
std::vector<CPath> CKernelMPI::GetFilesInDirectory( const CPath& dir ) const
{
	return GetFilesInDirectory_Impl(dir);
}

//------------------------------------------------------------------------------
Boolean CKernelMPI::IsDirectory( const CPath& dir ) const
{
	return IsDirectory_Impl(dir);
}

//------------------------------------------------------------------------------
tHndl CKernelMPI::LoadModule( const CPath& dir ) const
{
	return LoadModule_Impl(dir);
}

//------------------------------------------------------------------------------
void* CKernelMPI::RetrieveSymbolFromModule( tHndl module, const CString& symbol ) const
{
	return RetrieveSymbolFromModule_Impl(module, symbol);
}

//------------------------------------------------------------------------------
void CKernelMPI::UnloadModule( tHndl module ) const
{
	UnloadModule_Impl(module);
}


//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************

//==============================================================================
// CBootSafeKernelMPI implementation
//==============================================================================
CBootSafeKernelMPI::CBootSafeKernelMPI()
{
}


//============================================================================
// Informational functions
//============================================================================
Boolean CBootSafeKernelMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CBootSafeKernelMPI::GetMPIName() const
{
	return &kBSMPIName;
}

//----------------------------------------------------------------------------
tVersion CBootSafeKernelMPI::GetModuleVersion() const
{
	return 1;
}

//----------------------------------------------------------------------------
const CString* CBootSafeKernelMPI::GetModuleName() const
{
	return &kBSMPIName;
}

//----------------------------------------------------------------------------
const CURI* CBootSafeKernelMPI::GetModuleOrigin() const
{
	return &kNullURI;
}

//------------------------------------------------------------------------------
tPtr CBootSafeKernelMPI::Malloc(U32 size)
{
	return Malloc_Impl(size);
}

//------------------------------------------------------------------------------
void CBootSafeKernelMPI::Free(tPtr ptr)
{
	Free_Impl(ptr);
}

//----------------------------------------------------------------------------
void CBootSafeKernelMPI::Printf( const char * formatString, ... ) const
{
	va_list arguments;
	va_start(arguments, formatString);
	VPrintf_Impl(formatString, arguments);
	va_end(arguments);
}

//----------------------------------------------------------------------------
void CBootSafeKernelMPI::VPrintf( const char * formatString, va_list arguments ) const
{
	VPrintf_Impl(formatString, arguments);
}

//----------------------------------------------------------------------------
void CBootSafeKernelMPI::PowerDown() const
{
	PowerDown_Impl();
}

//------------------------------------------------------------------------------
std::vector<CPath> CBootSafeKernelMPI::GetFilesInDirectory( const CPath& dir ) const
{
	return GetFilesInDirectory_Impl(dir);
}

//------------------------------------------------------------------------------
Boolean CBootSafeKernelMPI::IsDirectory( const CPath& dir ) const
{
	return IsDirectory_Impl(dir);
}

//------------------------------------------------------------------------------
tHndl CBootSafeKernelMPI::LoadModule( const CPath& dir ) const
{
	return LoadModule_Impl(dir);
}

//------------------------------------------------------------------------------
void* CBootSafeKernelMPI::RetrieveSymbolFromModule( tHndl module, const CString& symbol ) const
{
	return RetrieveSymbolFromModule_Impl(module, symbol);
}

//------------------------------------------------------------------------------
void CBootSafeKernelMPI::UnloadModule( tHndl module ) const
{
	UnloadModule_Impl(module);
}

LF_END_BRIO_NAMESPACE()
// EOF
