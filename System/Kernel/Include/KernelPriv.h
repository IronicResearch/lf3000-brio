#ifndef LF_BRIO_KERNELPRIVATE_H
#define LF_BRIO_KERNELPRIVATE_H
//==============================================================================
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		KernelPriv.h
//
// Description:
//		Private, hidden data structures used by the KernelMPI
//
//==============================================================================

#include <SystemTypes.h>
#include <KernelTypes.h>
#include <CoreModule.h>
#include <StringTypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

LF_BEGIN_BRIO_NAMESPACE()


inline tErrType AsBrioErr(int err)
{
	// FIXME/tp: Implement, current passthrough may overlap existing Brio errors!
	switch(err)
	{
		case EINVAL:	return kInvalidParamErr;
		case EPERM:		return kPermissionsErr;
		default:		return kUnspecifiedErr;
	}
}

// Defines  
// FIXME/tp: "VALIDATE" rather than "ASSERT"?  We are not aborting execution here.
// FIXME/tp: Use Kernel::Printf for consistency
#define ASSERT_POSIX_CALL(err) \
if(err) \
{ \
	printf("\n***** POSIX function fails with error # (%d). File (%s), Line (%d)\n", \
	(int)err, __FILE__, __LINE__); \
	printf("Error string: %s\n", strerror(err)); \
	fflush(stdout); \
	return(AsBrioErr(err)); \
}
                                                           
#define ASSERT_ERROR(expression,value) 
/*
\
if(!expression) \
{ \
    int ret = ErrorBrio::lookupBrioErrType(value); \
    printf("***** Error (%d). File (%s), Line (%d)\n", \
        ret, __FILE__, __LINE__); \
        fflush(stdout); \
        return(ret); \
}

*/
// Constants
const CURI kKernelModuleName = "Kernel";
const tVersion kKernelModuleVersion = 2;

typedef U32 tKernelID;
typedef U32 tKernelSignature;

//==============================================================================
class CKernelModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*		GetModuleOrigin() const;

	// Tasks
	VTABLE_EXPORT tErrType	CreateTask( tTaskHndl& hndl, 
										const tTaskProperties& properties, 
										const char* pDebugName );
    VTABLE_EXPORT tErrType	JoinTask( tTaskHndl pHndl, tPtr& threadReturnValue );							
	VTABLE_EXPORT tErrType	CancelTask( tTaskHndl hndl );
	
	VTABLE_EXPORT tTaskHndl	GetCurrentTask() const;
	VTABLE_EXPORT int		GetTaskPriority( tTaskHndl hndl ) const;
	VTABLE_EXPORT int		GetTaskSchedulingPolicy( tTaskHndl hndl ) const;
	
	VTABLE_EXPORT void		TaskSleep( U32 msec ) const;

	// Memory Allocation
	VTABLE_EXPORT tPtr		Malloc( U32 size );
	VTABLE_EXPORT tErrType	Free( tPtr ptr );
	
	// IO
	VTABLE_EXPORT void		Printf( const char* format, ... ) const
								__attribute__ ((format (printf, 2, 3)));
	VTABLE_EXPORT void		Printf( const char* format, va_list arguments ) const
								__attribute__ ((format (printf, 2, 0)));
	// Message queues
	VTABLE_EXPORT tErrType	OpenMessageQueue( tMessageQueueHndl& hndl,
										const tMessageQueuePropertiesPosix& properties,
										const char* pDebugName );
	VTABLE_EXPORT tErrType	CloseMessageQueue( tMessageQueueHndl hndl,
										const tMessageQueuePropertiesPosix& properties );
	
#if 0 // FIXME/BSK	
	VTABLE_EXPORT tErrType	UnlinkMessageQueue( const char *name );
#endif
	VTABLE_EXPORT tErrType  ClearMessageQueue( tMessageQueueHndl hndl );

	VTABLE_EXPORT int	 	GetMessageQueueNumMessages( tMessageQueueHndl hndl ) const;
    
	VTABLE_EXPORT tErrType	SendMessage( tMessageQueueHndl hndl, const CMessage& message );
	VTABLE_EXPORT tErrType  SendMessageOrWait( tMessageQueueHndl hndl, 
										const CMessage& message, 
										U32 timeoutMs = kMaxTimeoutMs );

	VTABLE_EXPORT tErrType  ReceiveMessage( tMessageQueueHndl hndl,   
										CMessage* pMessage, U32 maxMessageSize );
	VTABLE_EXPORT tErrType  ReceiveMessageOrWait( tMessageQueueHndl hndl, 
										CMessage* pMessage, U32 maxMessageSize, 
										U32 timeoutMs = kMaxTimeoutMs );
				
	// Time & Timers
	VTABLE_EXPORT U32	GetElapsedTimeAsMSecs(); // elapsed time since System startup 
												// in milliseconds	

	VTABLE_EXPORT U64	GetElapsedTimeAsUSecs();	// elapsed time since System startup 
												// in microseconds	
    
	VTABLE_EXPORT tTimerHndl CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName = NULL );

    VTABLE_EXPORT tErrType 	DestroyTimer( tTimerHndl hndl );

	VTABLE_EXPORT tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties& props );

	VTABLE_EXPORT tErrType	StartTimer( tTimerHndl hndl, const tTimerProperties& props );
	VTABLE_EXPORT tErrType	StopTimer( tTimerHndl hndl);

	VTABLE_EXPORT tErrType	PauseTimer( tTimerHndl hndl, saveTimerSettings& saveValue );
	VTABLE_EXPORT tErrType	ResumeTimer( tTimerHndl hndl, saveTimerSettings& saveValue);

	VTABLE_EXPORT tErrType	GetTimerElapsedTime( tTimerHndl hndl, U32* pMs,U32* pUs = NULL ) const;		// elapsed time in milliseconds (& microseconds)
	VTABLE_EXPORT tErrType	GetTimerRemainingTime( tTimerHndl hndl, U32* pMs, U32* pUs = NULL ) const; 	// time remaining in milliseconds (& microseconds)

//------------------------------------------------------------------------------
// Mutexes
//------------------------------------------------------------------------------
    // Initialize a mutex Attribute Object 
    VTABLE_EXPORT tErrType InitMutexAttributeObject( tMutexAttr& mutexattr );

    // Initializes a mutex with the attributes specified in the specified mutex attribute object
    VTABLE_EXPORT tErrType InitMutex( tMutex& mutex, const tMutexAttr& mutexattr );
	
    // Destroys a mutex
    VTABLE_EXPORT tErrType DeInitMutex( tMutex& mutex );
	
    // Obtains the priority ceiling of a mutex attribute object
    VTABLE_EXPORT S32		 GetMutexPriorityCeiling( const tMutex& mutex ) const;
	
    // Sets the priority ceiling attribute of a mutex attribute object
    VTABLE_EXPORT tErrType SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority = NULL );
	
     // Locks an unlocked mutex
    VTABLE_EXPORT tErrType LockMutex( tMutex& mutex );
	
    // Tries to lock a not tested
    VTABLE_EXPORT tErrType TryLockMutex( tMutex& mutex );
	
    // Unlocks a mutex
    VTABLE_EXPORT tErrType UnlockMutex( tMutex& mutex );
    
	//==============================================================================
	// Conditions
	//==============================================================================
    // Unblocks all threads that are waiting on a condition variable
    VTABLE_EXPORT tErrType BroadcastCond( tCond& cond );
    
    // Destroys a condition variable attribute object
    VTABLE_EXPORT tErrType DestroyCond( tCond& cond );
    
    // Initializes a condition variable with the attributes specified in the
    // specified condition variable attribute object
    VTABLE_EXPORT tErrType InitCond( tCond& cond, const tCondAttr& attr );
    
    // Unblocks at least one thread waiting on a condition variable
    VTABLE_EXPORT tErrType SignalCond( tCond& cond );
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    // FIXME: pAbstime var
    VTABLE_EXPORT tErrType TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime );
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    VTABLE_EXPORT tErrType WaitOnCond( tCond& cond, tMutex& mutex );
    
    // Destroys a condition variable
    VTABLE_EXPORT tErrType DestroyCondAttr( tCondAttr& attr );
    
    // Obtains the process-shared setting of a condition variable attribute object
    VTABLE_EXPORT tErrType GetCondAttrPShared( const tCondAttr& attr, int* pShared );
    
    // Initializes a condition variable attribute object    
    VTABLE_EXPORT tErrType InitCondAttr( tCondAttr& attr );
    
    // Sets the process-shared attribute in a condition variable attribute object
    // to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    VTABLE_EXPORT tErrType SetCondAttrPShared( tCondAttr* pAttr, int shared );

private:
	// Limit object creation to the Module Manager interface functions
	CKernelModule();
	virtual ~CKernelModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_KERNELPRIVATE_H

// EOF

