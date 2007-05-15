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

#include <errno.h>	// for posix calls not yet moved to module...
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <Module.h>
#include <KernelMPI.h>

#include <KernelPriv.h>
#include <StringTypes.h>

#include <pthread.h>
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

//==============================================================================
// Global variables
//==============================================================================

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
	if (!pModule_)
		return kNull;		
	return pModule_->Malloc(size);
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::Free(tPtr ptr)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->Free(ptr);
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

//------------------------------------------------------------------------------
#if 0 // FIXME//BSK 
tErrType CKernelMPI::UnlinkMessageQueue(const char *hndl)
{
 	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnlinkMessageQueue(hndl);  
}
#endif 
//------------------------------------------------------------------------------
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

U32 CKernelMPI::GetElapsedTime( U32* pUs ) const
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->GetElapsedTime(pUs);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CreateTimer(tTimerHndl& hndl, pfnTimerCallback callback,
 						tTimerProperties& props, const char* pDebugName )
{
  	if(!pModule_)
		return kInvalidTimerHndl;
	return pModule_->CreateTimer(hndl, callback, props, pDebugName);  
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
U32 CKernelMPI::GetTimerElapsedTimeInMilliSec(tTimerHndl hndl) const
                                                                    
{
  	if (!pModule_)
		return 0;
	return pModule_->GetTimerElapsedTime(hndl);  
}
	
//------------------------------------------------------------------------------
// time remaining in milliseconds
U32 CKernelMPI::GetTimerRemainingTimeInMilliSec(tTimerHndl hndl) const
{
  	if (!pModule_)
		return 0;
	return pModule_->GetTimerRemainingTime(hndl);  
}

// fixme rdg: move the rest of these into the kernel module at some point
//------------------------------------------------------------------------------
// Mutexes
//------------------------------------------------------------------------------
tErrType CKernelMPI::InitMutex( tMutex& mutex, const tMutexAttr& attributes )
{
    errno = 0;
    pthread_mutex_init(&mutex, &attributes);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}
//------------------------------------------------------------------------------
tErrType CKernelMPI::DeInitMutex( tMutex& mutex )
{
    errno = 0;
    pthread_mutex_destroy(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
S32 CKernelMPI::GetMutexPriorityCeiling(const tMutex& mutex) const
{
   errno = 0;
   int prioCeiling = 0;
// FIXME /BSK NOT DEFINED YET    
//   pthread_mutex_getprioceiling(&mutex, &prioCeiling);
   ASSERT_POSIX_CALL( errno );
   
   return prioCeiling;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority )
{
   errno = 0;

// FIXME /BSK NOT DEFINED YET    
//   pthread_mutex_setprioceiling(&mutex, (int)prioCeiling, (int*)pOldPriority);
   ASSERT_POSIX_CALL( errno );
    
   return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::LockMutex( tMutex& mutex )
{
    errno = 0;
    
    pthread_mutex_lock(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TryLockMutex( tMutex& mutex )
{
    errno = 0;
    
    pthread_mutex_trylock(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::UnlockMutex( tMutex& mutex )
{
    errno = 0;
    
    pthread_mutex_unlock(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
// Conditions 
//------------------------------------------------------------------------------

tErrType CKernelMPI::BroadcastCond( tCond& cond )
{
    errno = 0;

    pthread_cond_broadcast(&cond);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyCond( tCond& cond )
{
    errno = 0;

    pthread_cond_destroy(&cond);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::InitCond( tCond& cond, const tCondAttr& attr )
{
    errno = 0;

    pthread_cond_init(&cond, &attr);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SignalCond( tCond& cond )
{
    errno = 0;

    pthread_cond_signal(&cond);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime )
{
    errno = 0;

    pthread_cond_timedwait(&cond, &mutex, pAbstime );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::WaitOnCond( tCond& cond, tMutex& mutex )
{
    errno = 0;

    pthread_cond_wait(&cond, &mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyCondAttr( tCondAttr& attr )
{
    errno = 0;

    pthread_condattr_destroy( &attr );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::GetCondAttrPShared( const tCondAttr& attr, int* pShared )
{
    errno = 0;
// FIXME/BSK To get the new library
//    pthread_condattr_getpshared( &attr, pShared );
//    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::InitCondAttr( tCondAttr& attr )
{
    errno = 0;

    pthread_condattr_init(&attr);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetCondAttrPShared( tCondAttr* pAttr, int shared )

{
    errno = 0;
// FIXME/BSK To get the new library
//    pthread_condattr_setpshared( pAttr, shared );
//    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

// EOF
