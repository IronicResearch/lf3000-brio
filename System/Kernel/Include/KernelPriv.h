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


// Defines
#define ASSERT_POSIX_CALL(err) \
if(err) \
{    int ret = (err); \
        printf("***** POSIX function fails with error (%d). File (%s), Line (%d)\n", \
        err, __FILE__, __LINE__); \
        fflush(stdout); \
        return(ret); \
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
const tVersion kKernelModuleVersion = MakeVersion(0,1);

typedef U32 tKernelID;
typedef U32 tKernelSignature;

//==============================================================================
class CKernelModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// Tasks
	VTABLE_EXPORT tErrType	CreateTask( const CURI* pTaskURI, 
							const tTaskProperties* pProperties, tTaskHndl *pHndl );
    VTABLE_EXPORT tErrType JoiningThreads( tTaskHndl pHndl, void **value_ptr );							
	VTABLE_EXPORT tErrType CancelTask( tTaskHndl hndl );
	VTABLE_EXPORT tTaskHndl GetCurrentTask();
	
	VTABLE_EXPORT tErrType GetTaskPriority( tTaskHndl hndl, int * priority );
	VTABLE_EXPORT tErrType GetTaskSchedulingPolicy( tTaskHndl hndl, int* policy );
	
	VTABLE_EXPORT tErrType TaskSleep( U32 msec );

	// Memory Allocation
	VTABLE_EXPORT tErrType Malloc( U32 size, tPtr ptr );
	VTABLE_EXPORT tErrType Free( tPtr ptr );
	VTABLE_EXPORT tErrType	CreateMessageQueue( const CURI* pQueueURI, 
						const tMessageQueuePropertiesPosix* pQueueProperties,
						tMessageQueueHndl* pHndl );
	VTABLE_EXPORT tErrType	DestroyMessageQueue( tMessageQueueHndl hndl );
	
	VTABLE_EXPORT tErrType	UnlinkMessageQueue( const char *name );

	VTABLE_EXPORT tErrType  ClearMessageQueue( tMessageQueueHndl hndl );

	VTABLE_EXPORT tErrType 	GetMessageQueueNumMessages( tMessageQueueHndl hndl, int *numMsgQueue );
    
	VTABLE_EXPORT tErrType	SendMessage( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize );
	VTABLE_EXPORT tErrType  SendMessageOrWait( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs );

	VTABLE_EXPORT tErrType  	ReceiveMessage( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, unsigned *msg_prio );
	VTABLE_EXPORT tErrType  	ReceiveMessageOrWait( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, U32 timeoutMs = kMaxTimeoutMs );
				
	// Time & Timers
	VTABLE_EXPORT U32		GetElapsedTime( U32* pUs=NULL );	// elapsed time since System startup 
															// in milliseconds (& microseconds)	
    VTABLE_EXPORT tErrType 	CreateTimer( const CURI* pTimerURI,
                          	struct sigevent *se,
                           	tTimerHndl* pHndl );
    
    VTABLE_EXPORT tErrType 	DestroyTimer( tTimerHndl hndl );

	VTABLE_EXPORT tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties* pTimerProperties );

	VTABLE_EXPORT tErrType	StartTimer( tTimerHndl hndl, const struct itimerspec* pValue );
	VTABLE_EXPORT tErrType	StopTimer( tTimerHndl hndl);

	VTABLE_EXPORT tErrType	GetTimerElapsedTime( tTimerHndl hndl, U32* pUs = NULL );		// elapsed time in milliseconds (& microseconds)
	VTABLE_EXPORT tErrType	GetTimerRemainingTime( tTimerHndl hndl, U32* pUs = NULL ); 	// time remaining in milliseconds (& microseconds)

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

