
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

	//==============================================================================
	// Tasks
	//==============================================================================
	// Creates a new thread which starts executing at the routine function
	tErrType	CreateTask( tTaskHndl& hndl, 		// FIXME/tp: return handle?
							const tTaskProperties& properties,
							const char* pDebugName = NULL );
	
	// Suspends execution of the calling thread until the target thread
	// completes its execution.
    tErrType	JoinTask( tTaskHndl hndl, tPtr& threadReturnValue );
	
	// Requests that thread be canceled
	tErrType	CancelTask( tTaskHndl hndl );
	
	// Returns the thread ID of the calling thread
	tTaskHndl	GetCurrentTask() const;
	
	// Gets the task priority of the thread given by thread.
	int			GetTaskPriority( tTaskHndl hndl ) const;
	
	// Gets the scheduling parameter of the thread given by thread.
	int			GetTaskSchedulingPolicy( tTaskHndl hndl ) const;

	// Causes the caller to sleep for at least the amount of time specified in duration
	void	 	TaskSleep( U32 msec ) const;
	
	//==============================================================================
	// Memory Allocation
	//==============================================================================
	// Allocates size bytes and returns a pointer to the allocated memory
	tPtr		Malloc( U32 size );
	
	// Frees the memory space pointed to by pAllocatedMemory, which has been returned
	// by a previous call to Malloc
	tErrType	Free( tPtr pAllocatedMemory );

	//==============================================================================
	// Message Queues
	//==============================================================================
	// FIXME/tp: Non-posix-specific props
	// Associates a message queue descriptor with the message queue specified by name. name
	// uniquely identifies the message queue across the system.
	tErrType	OpenMessageQueue( tMessageQueueHndl& hndl,
									const tMessageQueuePropertiesPosix& props,
									const char* pDebugName = NULL ); 
	
	// Closes the message queue, invalidating hndl
	tErrType	CloseMessageQueue( tMessageQueueHndl hndl,
	 								const tMessageQueuePropertiesPosix& props );
	
	// Clears message Queue
	tErrType  	ClearMessageQueue( tMessageQueueHndl hndl );

	// Gets the the current number of messages.
	int		 	GetMessageQueueNumMessages( tMessageQueueHndl hndl ) const;
    
	// Adds the message pointed to by hndl to the message queue
	tErrType	SendMessage( tMessageQueueHndl hndl, const CMessage& msg );
	
	// behaves just like SendMessage(), except that if the queue is full and the O_NONBLOCK flag
	// is not enabled for the message queue description, then timeoutMs specifies a ceiling on the time
	// for which the call will block
	tErrType  	SendMessageOrWait( tMessageQueueHndl hndl, const CMessage& msg, 
									U32 timeoutMs = kMaxTimeoutMs );

	// FIXME/tp: Work these and incorporate with IEventMessage
	// FIXME/tp: Think through order of parameters
	// behaves just like ReceiveMessage(), except that if the queue is empty and the O_NONBLOCK flag
	// is not enabled for the message queue description, then timeoutMs specifies a ceiling on the time
	// for which the call will block
	tErrType  	ReceiveMessageOrWait( tMessageQueueHndl hndl, CMessage* msg,  
									U32 maxMessageSize, U32 timeoutMs = kMaxTimeoutMs );
				
	//==============================================================================
	// Time & TimerstTimerGenHndl
	//==============================================================================
	// Returns time as millisecond and microseconds since the Epoch
	U32 	GetElapsedTime( U32* pUs=NULL ) const;	// elapsed time since System startup 
	
	// Creates a per-process timer using CLOCK_REALTIME as a base.
	tTimerHndl 	CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName = NULL );
    
	// Deletes the timer created by CreateTimer()
    tErrType 	DestroyTimer( tTimerHndl hndl );

	// Resets timer
	tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties& props );

	// Start the timer with parameters defined in the tTimerProperties property
	tErrType	StartTimer( tTimerHndl hndl, const tTimerProperties& props  );
	
	// Turns off the timer
	tErrType	StopTimer( tTimerHndl hndl );

	// Pauses the timer
	tErrType	PauseTimer( tTimerHndl hndl, saveTimerSettings& saveValue );
	
	// Resume the timer
	ttErrType	ResumeTimer( tTimerHndl hndl, saveTimerSettings& saveValue );

	// Returns the amount of time after the last timer expiration as millisecond and microseconds
	U32			GetTimerElapsedTime( tTimerHndl hndl, U32* pUs=NULL ) const;
	
	// Returns the amount of time before the timer expires as millisecond and microseconds
	U32			GetTimerRemainingTime( tTimerHndl hndl, U32* pUs=NULL ) const;

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

