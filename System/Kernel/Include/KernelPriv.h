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
	printf("***** POSIX function fails with error # (%d). File (%s), Line (%d)\n", \
	err, __FILE__, __LINE__); \
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
	VTABLE_EXPORT void		Printf( const char* format, ... ) const;
	VTABLE_EXPORT void		Printf( const char* format, va_list arguments ) const;
	
	// Message queues
	VTABLE_EXPORT tErrType	CreateMessageQueue( tMessageQueueHndl& hndl,
										const tMessageQueuePropertiesPosix& properties,
										const char* pDebugName );
	VTABLE_EXPORT tErrType	DestroyMessageQueue( tMessageQueueHndl hndl );
	
	VTABLE_EXPORT tErrType	UnlinkMessageQueue( const char *name );

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
	VTABLE_EXPORT U32		GetElapsedTime( U32* pUs );		// elapsed time since System startup 
															// in milliseconds (& microseconds)	
	VTABLE_EXPORT tTimerHndl CreateTimer( tCond& condition, 
										const tTimerProperties& props,
										const char* pDebugName );
    
    VTABLE_EXPORT tErrType 	DestroyTimer( tTimerHndl hndl );

	VTABLE_EXPORT tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties& props );

	VTABLE_EXPORT tErrType	StartTimer( tTimerHndl hndl );
	VTABLE_EXPORT tErrType	StopTimer( tTimerHndl hndl);

	VTABLE_EXPORT tErrType	GetTimerElapsedTime( tTimerHndl hndl ) const;		// elapsed time in milliseconds (& microseconds)
	VTABLE_EXPORT tErrType	GetTimerRemainingTime( tTimerHndl hndl ) const; 	// time remaining in milliseconds (& microseconds)

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

