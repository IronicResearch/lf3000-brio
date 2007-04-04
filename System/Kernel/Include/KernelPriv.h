#ifndef LF_BRIO_KERNELPRIVATE_H
#define LF_BRIO_KERNELPRIVATE_H
						    
//==============================================================================
//
// Copyright (c) 2002,2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelPriv.h
//
// Description:
//		Private, hidden data structures used by the KernelMPI
//
//==============================================================================

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <KernelTypes.h>
#include <CoreModule.h>

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

	// class-specific functionality
	CKernelModule();
	virtual ~CKernelModule();

	// Tasks
	virtual	tErrType	CreateTask( const CURI* pTaskURI, 
							const tTaskProperties* pProperties, tTaskHndl *pHndl );
    virtual tErrType JoiningThreads( tTaskHndl pHndl, void **value_ptr );							
	virtual tErrType CancelTask( tTaskHndl hndl );
	virtual tTaskHndl GetCurrentTask();
	
	virtual tErrType GetTaskPriority( tTaskHndl hndl, int * priority );
	virtual tErrType GetTaskSchedulingPolicy( tTaskHndl hndl, int* policy );
	
	// Memory Allocation
	virtual tErrType Malloc( U32 size, tPtr ptr );
	virtual tErrType Free( tPtr ptr );
	virtual tErrType	CreateMessageQueue( const CURI* pQueueURI, 
						const tMessageQueuePropertiesPosix* pQueueProperties,
						tMessageQueueHndl* pHndl );
	virtual tErrType	DestroyMessageQueue( tMessageQueueHndl hndl );
	
	virtual tErrType	UnlinkMessageQueue( const char *name );

	virtual tErrType  	ClearMessageQueue( tMessageQueueHndl hndl );

	virtual tErrType 	GetMessageQueueNumMessages( tMessageQueueHndl hndl, int *numMsgQueue );
    
	virtual tErrType	SendMessage( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize );
	virtual tErrType  	SendMessageOrWait( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs );

	virtual tErrType  	ReceiveMessage( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, unsigned *msg_prio );
	virtual tErrType  	ReceiveMessageOrWait( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, U32 timeoutMs = kMaxTimeoutMs );
				
	// Time & Timers
	virtual U32			GetElapsedTime( U32* pUs=NULL );	// elapsed time since System startup 
															// in milliseconds (& microseconds)	
    virtual tErrType 	CreateTimer( const CURI* pTimerURI,
                          	struct sigevent *se,
                           	tTimerHndl* pHndl );
    
    virtual tErrType 	DestroyTimer( tTimerHndl hndl );

	virtual tErrType 	ResetTimer( tTimerHndl hndl, const tTimerProperties* pTimerProperties );

	virtual tErrType	StartTimer( tTimerHndl hndl, const struct itimerspec* pValue );
	virtual tErrType	StopTimer( tTimerHndl hndl);

	virtual tErrType	GetTimerElapsedTime( tTimerHndl hndl, U32* pUs = NULL );		// elapsed time in milliseconds (& microseconds)
	virtual tErrType	GetTimerRemainingTime( tTimerHndl hndl, U32* pUs = NULL ); 	// time remaining in milliseconds (& microseconds)
private:

};

#endif		// LF_BRIO_KERNELPRIVATE_H

// EOF

