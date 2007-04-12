#ifndef LF_BRIO_KERNELMPI_H
#define LF_BRIO_KERNELMPI_H
//==============================================================================//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		KernelMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Kernel module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <CoreMPI.h>
#include <KernelTypes.h>
#include <StringTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


class CKernelMPI : public ICoreMPI
{
public:
	// MPI core functions
    virtual Boolean		IsValid() const;	

    virtual tErrType	GetMPIVersion(tVersion &version) const;		   
    virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		

    virtual tErrType	GetModuleVersion(tVersion &version) const;
    virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
    virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;
    
	// class-specific functionality
	CKernelMPI();
	virtual ~CKernelMPI();

	//==============================================================================
	// Tasks
	//==============================================================================
	tErrType	CreateTask( const CURI* pTaskURI, 
					const tTaskProperties* pTaskProperties, tTaskHndl *pHndl );
    
    tErrType	JoiningThreads( tTaskHndl pHndl, void **value_ptr );					
	
	tErrType	CancelTask( tTaskHndl hndl );
	
	tTaskHndl	GetCurrentTask();

	tErrType GetTaskPriority( tTaskHndl hndl, int *priority );
	tErrType GetTaskSchedulingPolicy( tTaskHndl hndl, int *policy );

	tErrType TaskSleep( U32 msec );
	
	//==============================================================================
	// Memory Allocation
	//==============================================================================
	tErrType Malloc( U32 size, tPtr ppAllocatedMemory );
	tErrType Free( tPtr pAllocatedMemory );

	//==============================================================================
	// Message Queues
	//==============================================================================
	tErrType	CreateMessageQueue( const CURI* pQueueURI, 
						const tMessageQueuePropertiesPosix* pQueueProperties,
						tMessageQueueHndl* pHndl );
	tErrType	DestroyMessageQueue( tMessageQueueHndl hndl );
	
	tErrType	UnlinkMessageQueue( const char *name );

	tErrType  	ClearMessageQueue( tMessageQueueHndl hndl );

	tErrType 	GetMessageQueueNumMessages( tMessageQueueHndl hndl, int *numMsgQueue );
    
	tErrType	SendMessage( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize );
	tErrType  	SendMessageOrWait( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs );

	tErrType  	ReceiveMessage( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, unsigned *msg_prio );
	tErrType  	ReceiveMessageOrWait( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, U32 timeoutMs = kMaxTimeoutMs );
				
	//==============================================================================
	// Time & Timers
	//==============================================================================
	U32			GetElapsedTime( U32* pUs=NULL );		// elapsed time since System startup 
														// in milliseconds (& microseconds)	
    tErrType 	CreateTimer( const CURI* pTimerURI,
                          	struct sigevent *se,
                           	tTimerHndl* pHndl );
    
    tErrType 	DestroyTimer( tTimerHndl hndl );

	tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties* pTimerProperties );

	tErrType	StartTimer( tTimerHndl hndl, const struct itimerspec* pValue );
	tErrType	StopTimer( tTimerHndl hndl);

	tErrType	GetTimerElapsedTime( tTimerHndl hndl, U32* pUs = NULL );		// elapsed time in milliseconds (& microseconds)
	tErrType	GetTimerRemainingTime( tTimerHndl hndl, U32* pUs = NULL ); 	// time remaining in milliseconds (& microseconds)

	//==============================================================================
	// Mutexes
	//==============================================================================
    // Initializes a mutex with the attributes specified in the specified mutex attribute object
    tErrType InitMutex( tMutex* pMutex, tMutexAttr* pAttributes );
	
    // Destroys a mutex
    tErrType DeInitMutex( tMutex* pMutex );
	
    // Obtains the priority ceiling of a mutex attribute object
    tErrType GetMutexPriorityCeiling( tMutex* pMutex, S32* pPrioCeiling );
	
    // Sets the priority ceiling attribute of a mutex attribute object
    tErrType SetMutexPriorityCeiling( tMutex* pMutex, S32 prioCeiling, S32* pOldPriority );
	
     // Locks an unlocked mutex
    tErrType LockMutex( tMutex* pMutex );
	
    // Tries to lock a not tested
    tErrType TryLockMutex( tMutex* pMutex );
	
    // Unlocks a mutex
    tErrType UnlockMutex( tMutex* pMutex );
    
	//==============================================================================
	// Conditions
	//==============================================================================
    // Unblocks all threads that are waiting on a condition variable
    tErrType CondBroadcast( tCond* pCond );
    
    // Destroys a condition variable attribute object
    tErrType CondDestroy( tCond* pCond );
    
    // Initializes a condition variable with the attributes specified in the
    // specified condition variable attribute object
    tErrType CondInit( tCond* pCond, const tCondAttr* pAttr );
    
    // Unblocks at least one thread waiting on a condition variable
    tErrType CondSignal( tCond* pCond );
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    tErrType CondTimedWait( tCond* pCond, pthread_mutex_t* pMutex,const tTimeSpec* pAbstime );
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    tErrType CondWait( tCond* pCond, pthread_mutex_t* pMutex );
    
    // Destroys a condition variable
    tErrType CondAttrDestroy( tCondAttr* pAttr );
    
    // Obtains the process-shared setting of a condition variable attribute object
    tErrType GetCondAttrPShared( const tCondAttr* pAttr, int* pShared );
    
    // Initializes a condition variable attribute object    
    tErrType CondAttrInit( tCondAttr* pAttr );
    
    // Sets the process-shared attribute in a condition variable attribute object
    // to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    tErrType SetCondAttrPShared( tCondAttr* pAttr, int shared );

private:
	class CKernelModule *mpModule;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_KERNELMPI_H

// EOF

