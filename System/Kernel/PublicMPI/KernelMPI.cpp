
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
static const tVersion kMPIVersion = MakeVersion(0, 1);
static const CString kMPIName = "KernelMPI";

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CKernelMPI implementation
//==============================================================================
CKernelMPI::CKernelMPI() : mpModule(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kKernelModuleName, 
									kKernelModuleVersion);
	mpModule = reinterpret_cast<CKernelModule*>(pModule);
}

//----------------------------------------------------------------------------
CKernelMPI::~CKernelMPI()
{
	Module::Disconnect(mpModule);
}

//============================================================================
// Informational functions
//============================================================================
Boolean CKernelMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CKernelMPI::GetMPIVersion(tVersion &pVersion) const
{
	pVersion = kMPIVersion;
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType	CKernelMPI::GetMPIName(ConstPtrCString &pName) const
{
	pName = &kMPIName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CKernelMPI::GetModuleVersion(tVersion &version) const
{
	if(!mpModule)
		return kMPINotConnectedErr;

	return mpModule->GetModuleVersion( version );
}

//----------------------------------------------------------------------------
tErrType	CKernelMPI::GetModuleName(ConstPtrCString &pName) const
{
	if (!mpModule)
		return kMPINotConnectedErr;

	return mpModule->GetModuleName( pName );
}

//----------------------------------------------------------------------------
tErrType CKernelMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetModuleOrigin( pURI );
}

//==============================================================================
// Tasks
//==============================================================================
tErrType	CKernelMPI::CreateTask(const CURI* pTaskURI, 
					const tTaskProperties* pProperties, tTaskHndl *pHndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->CreateTask( pTaskURI, pProperties, pHndl );
}

//----------------------------------------------------------------------------
tErrType	CKernelMPI::JoiningThreads( tTaskHndl pHndl, void **value_ptr ) 
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->JoiningThreads( pHndl, value_ptr );
}
//----------------------------------------------------------------------------
tErrType CKernelMPI::CancelTask(tTaskHndl hndl)
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->CancelTask( hndl );
}

//----------------------------------------------------------------------------
// This service returns the currently active task pointer.
tTaskHndl	CKernelMPI::GetCurrentTask()
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetCurrentTask();
}

//----------------------------------------------------------------------------
tErrType CKernelMPI::GetTaskPriority( tTaskHndl hndl, int* priority )
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetTaskPriority( hndl, priority );
}	

//----------------------------------------------------------------------------
tErrType CKernelMPI::GetTaskSchedulingPolicy(tTaskHndl hndl, int* policy)
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetTaskSchedulingPolicy( hndl, policy );
}	

//----------------------------------------------------------------------------
tErrType CKernelMPI::TaskSleep( U32 msec )
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->TaskSleep( msec );
}	

//============================================================================
// allocating memory from the System heap
//============================================================================
tErrType CKernelMPI::Malloc( U32 size, tPtr pPtr )
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->Malloc( size, pPtr );
	
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::Free( tPtr ptr )
{
	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->Free( ptr );
}

//============================================================================
// Message Queues
//============================================================================
tErrType	CKernelMPI::CreateMessageQueue( const CURI* pQueueURI, 
						const tMessageQueuePropertiesPosix* pProperties,
						tMessageQueueHndl* pHndl )
{
 	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->CreateMessageQueue( pQueueURI, pProperties, pHndl );  
}

//------------------------------------------------------------------------------
tErrType	CKernelMPI::DestroyMessageQueue( tMessageQueueHndl hndl )
{
 	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->DestroyMessageQueue( hndl );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::UnlinkMessageQueue( const char *hndl )
{
 	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->UnlinkMessageQueue( hndl );  
}

//------------------------------------------------------------------------------
tErrType  	CKernelMPI::ClearMessageQueue( tMessageQueueHndl hndl )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->ClearMessageQueue( hndl );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::GetMessageQueueNumMessages( tMessageQueueHndl hndl, int *numMsgQueue )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetMessageQueueNumMessages( hndl, numMsgQueue );  
}
    
//------------------------------------------------------------------------------
 tErrType	CKernelMPI::SendMessage( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->SendMessage( hndl, pMessage, messageSize );  
}

//------------------------------------------------------------------------------
tErrType  	CKernelMPI::SendMessageOrWait( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize, U32 timeoutMs )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->SendMessageOrWait( hndl, pMessage, messageSize, timeoutMs );  
}

//------------------------------------------------------------------------------
tErrType  	CKernelMPI::ReceiveMessage( tMessageQueueHndl hndl, U32 maxMessageSize, 
									   CMessage* msg_ptr, unsigned *msg_prio )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->ReceiveMessage( hndl, maxMessageSize, msg_ptr, msg_prio );  
}

tErrType  	CKernelMPI::ReceiveMessageOrWait( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* msg_ptr, U32 timeoutMs )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->ReceiveMessageOrWait( hndl, maxMessageSize, msg_ptr, timeoutMs );  
}
				
//==============================================================================
// Time & Timers
//==============================================================================

U32   		CKernelMPI::GetElapsedTime( U32* pUs )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetElapsedTime( pUs );  
}

//------------------------------------------------------------------------------
tErrType 	CKernelMPI::CreateTimer(const CURI* pTimerURI,
                                    struct sigevent* se, tTimerHndl* pHndl)
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->CreateTimer( pTimerURI, se, pHndl );  
}

//------------------------------------------------------------------------------
tErrType 	CKernelMPI::DestroyTimer( tTimerHndl hndl )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->DestroyTimer( hndl );  
}
	
//------------------------------------------------------------------------------
tErrType 	CKernelMPI::ResetTimer( tTimerHndl hndl, const tTimerProperties* pTimerProperties )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->ResetTimer( hndl, pTimerProperties );  
}
	
//------------------------------------------------------------------------------
tErrType	CKernelMPI::StartTimer( tTimerHndl hndl, const struct itimerspec* value )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->StartTimer( hndl, value );  
}
	
//------------------------------------------------------------------------------
tErrType	CKernelMPI::StopTimer( tTimerHndl hndl )
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->StopTimer( hndl );  
}
	
//------------------------------------------------------------------------------
// elapsed time in milliseconds, (& microseconds)
tErrType CKernelMPI::GetTimerElapsedTime( tTimerHndl hndl, U32* pUs ) 
                                                                    
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetTimerElapsedTime( hndl, pUs );  
}
	
//------------------------------------------------------------------------------
// time remaining in milliseconds (& microseconds)
tErrType CKernelMPI::GetTimerRemainingTime( tTimerHndl hndl, U32* pUs ) 
{
  	if(!mpModule)
		return kMPINotConnectedErr;
		
	return mpModule->GetTimerRemainingTime( hndl, pUs );  
}

// fixme rdg: move the rest of these into the kernel module at some point
//------------------------------------------------------------------------------
// Mutexes
//------------------------------------------------------------------------------
tErrType CKernelMPI::InitMutex( tMutex* pMutex, tMutexAttr* pAttributes )
{
    errno = 0;
    pthread_mutex_init( pMutex, pAttributes );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DeInitMutex( tMutex* pMutex )
{
    errno = 0;
    pthread_mutex_destroy( pMutex );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::GetMutexPriorityCeiling( tMutex* pMutex, S32* pPrioCeiling )
{
   errno = 0;
   
   pthread_mutex_getprioceiling( pMutex, (int*)pPrioCeiling );
   ASSERT_POSIX_CALL( errno );
   
   return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetMutexPriorityCeiling( tMutex* pMutex, S32 prioCeiling, S32* pOldPriority )
{
   errno = 0;

   pthread_mutex_setprioceiling( pMutex, (int)prioCeiling, (int*)pOldPriority);
   ASSERT_POSIX_CALL( errno );
    
   return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::LockMutex( tMutex* pMutex )
{
    errno = 0;
    
    pthread_mutex_lock( pMutex );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TryLockMutex( tMutex* pMutex )
{
    errno = 0;
    
    pthread_mutex_trylock( pMutex );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::UnlockMutex( tMutex* pMutex )
{
    errno = 0;
    
    pthread_mutex_unlock( pMutex );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
// Conditions 
//------------------------------------------------------------------------------

tErrType CKernelMPI::CondBroadcast( tCond* pCond )
{
    errno = 0;

    pthread_cond_broadcast( pCond );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondDestroy( tCond* pCond )
{
    errno = 0;

    pthread_cond_destroy( pCond );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondInit( tCond* pCond, const tCondAttr* pAttr )
{
    errno = 0;

    pthread_cond_init( pCond, pAttr );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondSignal( tCond* pCond )
{
    errno = 0;

    pthread_cond_signal( pCond );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondTimedWait( tCond* pCond, pthread_mutex_t* pMutex,const tTimeSpec* pAbstime )
{
    errno = 0;

    pthread_cond_timedwait( pCond, pMutex, pAbstime );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondWait( tCond* pCond, pthread_mutex_t* pMutex)
{
    errno = 0;

    pthread_cond_wait( pCond, pMutex );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondAttrDestroy( tCondAttr* pAttr )
{
    errno = 0;

    pthread_condattr_destroy( pAttr );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::GetCondAttrPShared( const tCondAttr* pAttr, int* pShared )
{
    errno = 0;

    pthread_condattr_getpshared( pAttr, pShared );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CondAttrInit( tCondAttr* pAttr )
{
    errno = 0;

    pthread_condattr_init( pAttr );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetCondAttrPShared( tCondAttr* pAttr, int shared )

{
    errno = 0;

    pthread_condattr_setpshared( pAttr, shared );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

// EOF
