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

#include <vector>
#include <SystemTypes.h>
#include <CoreMPI.h>
#include <KernelTypes.h>
#include <StringTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

#define MAX_LOGGING_MSG_LEN	 256

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
	/// Tasks
	//==============================================================================
	/// Creates a new thread which starts executing at the routine function
	tErrType	CreateTask( tTaskHndl& hndl, 		// FIXME/tp: return handle?
							const tTaskProperties& properties,
							const char* pDebugName = NULL );
	/// Suspends execution of the calling thread until the target thread
	/// completes its execution.
    tErrType	JoinTask( tTaskHndl hndl, tPtr& threadReturnValue );

    /// Requests that thread be canceled
   tErrType	CancelTask( tTaskHndl hndl );
	
   /// Returns the thread ID of the calling thread
	tTaskHndl	GetCurrentTask() const;

	/// Gets the task priority of the thread given by thread.
	int			GetTaskPriority( tTaskHndl hndl ) const;

	/// Gets the scheduling parameter of the thread given by thread.
	int			GetTaskSchedulingPolicy( tTaskHndl hndl ) const;

	/// Causes the caller to sleep for at least the amount of time specified
	/// in duration
	void	 	TaskSleep( U32 msec ) const;
	
	//==============================================================================
	/// Memory Allocation
	//==============================================================================
	/// Allocates size bytes and returns a pointer to the allocated memory
	tPtr		Malloc( U32 size );

	/// Frees the memory space pointed to by pAllocatedMemory, which has been
	/// returned by a previous call to Malloc
	void		Free( tPtr pAllocatedMemory );

	///==============================================================================
	/// Message Queues
	///==============================================================================
	// FIXME/tp: Non-posix-specific props
	/// Associates a message queue descriptor with the message queue specified by name.
	/// name uniquely identifies the message queue across the system.
	tErrType	OpenMessageQueue( tMessageQueueHndl& hndl,
									const tMessageQueuePropertiesPosix& props,
									const char* pDebugName = NULL ); 
	/// Closes the message queue, invalidating hndl
	tErrType	CloseMessageQueue( tMessageQueueHndl hndl,
	 								const tMessageQueuePropertiesPosix& props );
	
	// FIXME/tp: Need to fold unlink into destroy
	// FIXME/BSK Delete
	// tErrType	UnlinkMessageQueue( const char *name );

	/// Clears message Queue
	tErrType  	ClearMessageQueue( tMessageQueueHndl hndl );

	/// Gets the the current number of messages.
	int		 	GetMessageQueueNumMessages( tMessageQueueHndl hndl ) const;
    
	/// Adds the message pointed to by hndl to the message queue
	tErrType	SendMessage( tMessageQueueHndl hndl, const CMessage& msg );

	/// behaves just like SendMessage(), except that if the queue is full and
	/// the O_NONBLOCK flag is not enabled for the message queue description,
	/// then timeoutMs specifies a ceiling on the time for which the call will block
	tErrType  	SendMessageOrWait( tMessageQueueHndl hndl, const CMessage& msg, 
									U32 timeoutMs = kMaxTimeoutMs );

	// FIXME/tp: Work these and incorporate with IEventMessage
	// FIXME/tp: Think through order of parameters
	// FIXME/dg: ReceiveMessage's maxMessageSize param MUST be the same as
	//           the max message size you pass in to the OpenMessageQueue's props.
	//           It would be better if the KernelMPI remembered this for you so
	//           we could leave it out of the API.
	/// Copies the oldest of the highest priority messages into the buffer pointed
	/// to by msg_ptr and removes it from the queue
	tErrType  	ReceiveMessage( tMessageQueueHndl hndl, CMessage* msg, 
									U32 maxMessageSize );

	/// behaves just like ReceiveMessage(), except that if the queue is empty and
	/// the O_NONBLOCK flag is not enabled for the message queue description, then
	/// timeoutMs specifies a ceiling on the time for which the call will block
	tErrType  	ReceiveMessageOrWait( tMessageQueueHndl hndl, CMessage* msg,  
									U32 maxMessageSize, U32 timeoutMs = kMaxTimeoutMs );
				
	///==============================================================================
	/// Time & Date
	///==============================================================================

	/// Elapsed time in milliseconds since the Epoch (1970). (32-bit wrap-around) 
	U32 	GetElapsedTimeAsMSecs() const; 

	/// Elapsed time in microseconds since the Epoch.
	U64		GetElapsedTimeAsUSecs() const;	 

	/// Get time as microseconds from High Resolution Timer (HRT)
	tErrType 	GetHRTAsUsec(U32 &uSec) const; 

	/// Get Elapsed time in seconds.
	tErrType	GetElapsedAsSec(U32 &sec) const;	 

	/// Get Elapsed time as \ref Current_Time structure.
	tErrType	GetElapsedTimeAsStructure(Current_Time &curTime) const;

	///======================================================
	/// Timer
	///======================================================
	/// Creates a per-process timer using CLOCK_REALTIME as a base.
	tTimerHndl 	CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName = NULL );
    
	/// Deletes the timer created by CreateTimer()
    tErrType 	DestroyTimer( tTimerHndl hndl );

    /// Resets timer
    tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties& props );

    /// Start the timer with parameters defined in the tTimerProperties property
	tErrType	StartTimer( tTimerHndl hndl, const tTimerProperties& props  );

	/// Turns off the timer
	tErrType	StopTimer( tTimerHndl hndl );

	/// Pauses the timer
	tErrType	PauseTimer( tTimerHndl hndl, saveTimerSettings& saveValue );

	/// Resume the timer/// Returns the amount of time before the timer expires as millisecond and
	/// microseconds

	tErrType	ResumeTimer( tTimerHndl hndl, saveTimerSettings& saveValue );

	/// Returns the amount of time after the last timer expiration as millisecond
	/// and microseconds
	tErrType	GetTimerElapsedTime( tTimerHndl hndl, U32* pMs, U32* pUs=NULL ) const;

	/// Returns the amount of time before the timer expires as millisecond and
	/// microseconds
	tErrType	GetTimerRemainingTime( tTimerHndl hndl, U32* pMs, U32* pUs=NULL ) const;

	///==============================================================================
	/// Watchdog Timer
	///==============================================================================
	// Start Watchdog Timer
	tErrType StartWatchdog( U32 seconds ) const;

	// Stop Watchdog Timer
	tErrType StopWatchdog( void ) const;

	// KeepAlive request to Watchdog Timer
	tErrType KeepWatchdogAlive( void ) const;

	///==============================================================================
	/// Mutexes
	///==============================================================================
    /// Initialize a mutex Attribute Object 
    tErrType InitMutexAttributeObject( tMutexAttr& mutexAttr );

    /// Initializes a mutex with the attributes specified in the specified mutex attribute object
    tErrType InitMutex( tMutex& mutex, const tMutexAttr& mutexAttr );
	
 
    /// Destroys a mutex
    tErrType DeInitMutex( tMutex& mutex );
	
    /// Obtains the priority ceiling of a mutex attribute object
    S32		 GetMutexPriorityCeiling( const tMutex& mutex ) const;
	
    /// Sets the priority ceiling attribute of a mutex attribute object
    tErrType SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority = NULL );
	
     /// Locks an unlocked mutex
    tErrType LockMutex( tMutex& mutex );
	
    /// Tries to lock a not tested
    tErrType TryLockMutex( tMutex& mutex );
	
    /// Unlocks a mutex
    tErrType UnlockMutex( tMutex& mutex );
    
	///==============================================================================
	/// Conditions
	///==============================================================================
    /// Unblocks all threads that are waiting on a condition variable
    tErrType BroadcastCond( tCond& cond );
    
    /// Destroys a condition variable attribute object
    tErrType DestroyCond( tCond& cond );
    
    /// Initializes a condition variable with the attributes specified in the
    /// specified condition variable attribute object
    tErrType InitCond( tCond& cond, const tCondAttr& attr );
    
    /// Unblocks at least one thread waiting on a condition variable
    tErrType SignalCond( tCond& cond );
    
    /// Automatically unlocks the specified mutex, and places the calling thread into a wait state
    /// FIXME: pAbstime var
    tErrType TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime );
    
    /// Automatically unlocks the specified mutex, and places the calling thread into a wait state
    tErrType WaitOnCond( tCond& cond, tMutex& mutex );
    
    /// Destroys a condition variable
    tErrType DestroyCondAttr( tCondAttr& attr );
    
    /// Obtains the process-shared setting of a condition variable attribute object
    tErrType GetCondAttrPShared( const tCondAttr& attr, int* pShared );
    
    /// Initializes a condition variable attribute object    
    tErrType InitCondAttr( tCondAttr& attr );
    
    /// Sets the process-shared attribute in a condition variable attribute object
    /// to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    tErrType SetCondAttrPShared( tCondAttr* pAttr, int shared );

	///==============================================================================
	/// I/O Functions
	///==============================================================================
    /// Standard printf syntax to output a string through the serial port
    void	Printf( const char * formatString, ... ) const
						__attribute__ ((format (printf, 2, 3)));

    /// Standard vprintf syntax to output a string through the serial port
    void	VPrintf( const char * formatString, va_list arguments ) const
						__attribute__ ((format (printf, 2, 0)));
   
	///==============================================================================
	/// Power Control Functions
	///==============================================================================
    void	PowerDown() const;
    
	///==============================================================================
	/// File System Functions
	///==============================================================================
    std::vector<CPath>	GetFilesInDirectory( const CPath& dir ) const;
    
    Boolean				IsDirectory( const CPath& dir ) const;
    
	///==============================================================================
	/// Code Module Functions
	///==============================================================================
    tHndl	LoadModule( const CPath& dir ) const;
    void*	RetrieveSymbolFromModule( tHndl obj, const CString& symbol ) const;
    void	UnloadModule( tHndl obj ) const;

	//==============================================================================
	/// Memory Pool Allocation functions
	//==============================================================================
	/// Create memory pool of size bytes and returns a pointer to the memory pool
    /// \param 	size 	initial size of memory pool in bytes
    /// \return			memory pool handle
    tMemoryPoolHndl		CreateMemPool( U32 size );

	/// Destroys memory pool and releases all associated memory for that pool
    /// \param	pool	memory pool handle
	void		DestroyMemPool( tMemoryPoolHndl pool );

	/// Allocates size bytes from pool and returns a pointer to the allocated memory
    /// \param	pool	memory pool handle
    /// \param 	size 	size of allocation in bytes
    /// \return			pointer to memory allocated within pool
	tPtr		MemPoolMalloc( tMemoryPoolHndl pool, U32 size );

	/// Re-Allocates size bytes from pool and returns a pointer to the allocated memory
    /// \param	pool	memory pool handle
	/// \param	pmem	pointer to memory returned by MemPoolMalloc()
    /// \param 	size 	size of allocation in bytes
    /// \return			pointer to memory allocated within pool
	tPtr		MemPoolRealloc( tMemoryPoolHndl pool, tPtr pmem, U32 size );

	/// Frees the memory pointed to by pAllocatedMemory allocated from pool
    /// \param	pool	memory pool handle
	/// \param	pmem	pointer to memory returned by MemPoolMalloc() or MemPoolRealloc()
	void		MemPoolFree( tMemoryPoolHndl pool, tPtr pmem );

private:
	class CKernelModule *pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif /// LF_BRIO_KERNELMPI_H

// EOF

