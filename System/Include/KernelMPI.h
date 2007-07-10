
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
	tErrType	CreateTask( tTaskHndl& hndl, 		// FIXME/tp: return handle?
							const tTaskProperties& properties,
							const char* pDebugName = NULL );
    tErrType	JoinTask( tTaskHndl hndl, tPtr& threadReturnValue );
	tErrType	CancelTask( tTaskHndl hndl );
	
	tTaskHndl	GetCurrentTask() const;
	int			GetTaskPriority( tTaskHndl hndl ) const;
	int			GetTaskSchedulingPolicy( tTaskHndl hndl ) const;

	void	 	TaskSleep( U32 msec ) const;
	
	//==============================================================================
	// Memory Allocation
	//==============================================================================
	tPtr		Malloc( U32 size );
	void		Free( tPtr pAllocatedMemory );

	//==============================================================================
	// Message Queues
	//==============================================================================
	// FIXME/tp: Non-posix-specific props
	tErrType	OpenMessageQueue( tMessageQueueHndl& hndl,
									const tMessageQueuePropertiesPosix& props,
									const char* pDebugName = NULL ); 
	tErrType	CloseMessageQueue( tMessageQueueHndl hndl,
	 								const tMessageQueuePropertiesPosix& props );
	
	// FIXME/tp: Need to fold unlink into destroy
	// FIXME/BSK Delete
	// tErrType	UnlinkMessageQueue( const char *name );

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
	// Time & TimerstTimerGenHndl
	//==============================================================================
	// Elapsed time since System startup in milliscond
	U32 	GetElapsedTimeAsMSecs() const; 

	// Elapsed time since System startup in microsecond
	U64		GetElapsedTimeAsUSecs() const;	 

// fixme/BSK
	tTimerHndl 	CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName = NULL );
    
    tErrType 	DestroyTimer( tTimerHndl hndl );

    tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties& props );

	tErrType	StartTimer( tTimerHndl hndl, const tTimerProperties& props  );
    tErrType	StopTimer( tTimerHndl hndl );

	tErrType	PauseTimer( tTimerHndl hndl, saveTimerSettings& saveValue );
	tErrType	ResumeTimer( tTimerHndl hndl, saveTimerSettings& saveValue );

	tErrType	GetTimerElapsedTime( tTimerHndl hndl, U32* pMs, U32* pUs=NULL ) const;
	tErrType	GetTimerRemainingTime( tTimerHndl hndl, U32* pMs, U32* pUs=NULL ) const;

	//==============================================================================
	// Mutexes
	//==============================================================================
    // Initialize a mutex Attribute Object 
    tErrType InitMutexAttributeObject( tMutexAttr& mutexAttr );

    // Initializes a mutex with the attributes specified in the specified mutex attribute object
    tErrType InitMutex( tMutex& mutex, const tMutexAttr& mutexAttr );
	
 
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

	//==============================================================================
	// I/O Functions
	//==============================================================================
    // Standard printf syntax to output a string through the serial port
    void	Printf( const char * formatString, ... ) const
						__attribute__ ((format (printf, 2, 3)));

    // Standard vprintf syntax to output a string through the serial port
    void	VPrintf( const char * formatString, va_list arguments ) const
						__attribute__ ((format (printf, 2, 0)));
   
	//==============================================================================
	// Power Control Functions
	//==============================================================================
    void	PowerDown() const;
    
	//==============================================================================
	// File System Functions
	//==============================================================================
    std::vector<CPath>	GetFilesInDirectory( const CPath& dir ) const;
    
    Boolean				IsDirectory( const CPath& dir ) const;
    
	//==============================================================================
	// Code Module Functions
	//==============================================================================
    tHndl	LoadModule( const CPath& dir ) const;
    void*	RetrieveSymbolFromModule( tHndl obj, const CString& symbol ) const;
    void	UnloadModule( tHndl obj ) const;
 
private:
	class CKernelModule *pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_KERNELMPI_H

// EOF

