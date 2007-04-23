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
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*		GetModuleOrigin() const;
    
	// class-specific functionality
	CKernelMPI();
	virtual ~CKernelMPI();

// FIXME: Change all pXXXURI to const char* pDebugName = NULL (last param)
	//==============================================================================
	// Tasks
	//==============================================================================
	tErrType	CreateTask( tTaskHndl& hndl, 		// FIXME/tp: return handle?
							const tTaskProperties& properties,
							const char* pDebugName = NULL );
    tErrType	JoinTask( tTaskHndl pHndl, tPtr& threadReturnValue );
	tErrType	CancelTask( tTaskHndl hndl );
	
	tTaskHndl	GetCurrentTask() const;
	int			GetTaskPriority( tTaskHndl hndl ) const;
	int			GetTaskSchedulingPolicy( tTaskHndl hndl ) const;

	void	 	TaskSleep( U32 msec ) const;
	
	//==============================================================================
	// Memory Allocation
	//==============================================================================
	tPtr		Malloc( U32 size );
	tErrType	Free( tPtr pAllocatedMemory );

	//==============================================================================
	// Message Queues
	//==============================================================================
	// FIXME/tp: Non-posix-specific props
	tErrType	CreateMessageQueue( tMessageQueueHndl& hndl,
									const tMessageQueuePropertiesPosix& props,
									const char* pDebugName = NULL ); 
	tErrType	DestroyMessageQueue( tMessageQueueHndl hndl );
	
	// FIXME/tp: Need to fold unlink into destroy
	tErrType	UnlinkMessageQueue( const char *name );

	tErrType  	ClearMessageQueue( tMessageQueueHndl hndl );

	int		 	GetMessageQueueNumMessages( tMessageQueueHndl hndl ) const;
    
	tErrType	SendMessage( tMessageQueueHndl hndl, const CMessage& msg );
	tErrType  	SendMessageOrWait( tMessageQueueHndl hndl, const CMessage& msg, 
									U32 timeoutMs = kMaxTimeoutMs );

	// FIXME/tp: Work these and incorporate with IEventMessage
	// FIXME/tp: Think through order of parameters
	tErrType  	ReceiveMessage( tMessageQueueHndl hndl, CMessage* msg, 
									U32 maxMessageSize );
	tErrType  	ReceiveMessageOrWait( tMessageQueueHndl hndl, CMessage* msg,  
									U32 maxMessageSize, U32 timeoutMs = kMaxTimeoutMs );
				
	//==============================================================================
	// Time & Timers
	//==============================================================================
	U32			GetElapsedTime( U32* pUs=NULL ) const;	// elapsed time since System startup 
														// in milliseconds (& microseconds)	
	tTimerHndl 	CreateTimer( tCond& condition, const tTimerProperties& props,
								const char* pDebugName = NULL );
    
    tErrType 	DestroyTimer( tTimerHndl hndl );

	tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties& props );

	tErrType	StartTimer( tTimerHndl hndl );
	tErrType	StopTimer( tTimerHndl hndl );

	U32			GetTimerElapsedTimeInMilliSec( tTimerHndl hndl ) const;
	U32			GetTimerRemainingTimeInMilliSec( tTimerHndl hndl ) const;

	//==============================================================================
	// Mutexes
	//==============================================================================
    // Initializes a mutex with the attributes specified in the specified mutex attribute object
    tErrType InitMutex( tMutex& mutex, const tMutexAttr& attributes );
	
    // Destroys a mutex
    tErrType DeInitMutex( tMutex& mutex );
	
    // Obtains the priority ceiling of a mutex attribute object
    S32		 GetMutexPriorityCeiling( const tMutex& mutex ) const;
	
    // Sets the priority ceiling attribute of a mutex attribute object
    tErrType SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority = NULL );
	
     // Locks an unlocked mutex
    tErrType LockMutex( tMutex& mutex );
	
    // Tries to lock a not tested
    tErrType TryLockMutex( tMutex& mutex );
	
    // Unlocks a mutex
    tErrType UnlockMutex( tMutex& mutex );
    
	//==============================================================================
	// Conditions
	//==============================================================================
    // Unblocks all threads that are waiting on a condition variable
    tErrType BroadcastCond( tCond& cond );
    
    // Destroys a condition variable attribute object
    tErrType DestroyCond( tCond& cond );
    
    // Initializes a condition variable with the attributes specified in the
    // specified condition variable attribute object
    tErrType InitCond( tCond& cond, const tCondAttr& attr );
    
    // Unblocks at least one thread waiting on a condition variable
    tErrType SignalCond( tCond& cond );
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    // FIXME: pAbstime var
    tErrType TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime );
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    tErrType WaitOnCond( tCond& cond, tMutex& mutex );
    
    // Destroys a condition variable
    tErrType DestroyCondAttr( tCondAttr& attr );
    
    // Obtains the process-shared setting of a condition variable attribute object
    tErrType GetCondAttrPShared( const tCondAttr& attr, int* pShared );
    
    // Initializes a condition variable attribute object    
    tErrType InitCondAttr( tCondAttr& attr );
    
    // Sets the process-shared attribute in a condition variable attribute object
    // to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    tErrType SetCondAttrPShared( tCondAttr* pAttr, int shared );

private:
	class CKernelModule *pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_KERNELMPI_H

// EOF

