//==============================================================================
//
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		Kernel.cpp
//
// Description:
//		Underlying implementation code used by the KernelMPI
//
//==============================================================================

#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>	// for gettimeofday()
#include <unistd.h>
#include <sys/types.h>


#include <SystemTypes.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <CoreMPI.h>
#include <KernelMPI.h>
#include <KernelPriv.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

const CURI	kModuleURI	= "/Somewhere/KernelModule";

// single instance of module object.
static CKernelModule*	sinst = NULL;

//==============================================================================
//==============================================================================
CKernelModule::CKernelModule()
{
}

CKernelModule::~CKernelModule()
{
}

//============================================================================
// Informational functions
//============================================================================
Boolean CKernelModule::IsValid() const
{
	return (sinst != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
tErrType CKernelModule::GetModuleVersion(tVersion &version) const
{
	version = kKernelModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType	CKernelModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kKernelModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CKernelModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}

//==============================================================================
// Tasks
//==============================================================================
//------------------------------------------------------------------------------
tErrType	CKernelModule::CreateTask( const CURI* pTaskURI, 
					const tTaskProperties* pProperties, tTaskHndl *pHndl )
{
	pthread_attr_t tattr;
	sched_param param;
	tErrType err = kNoErr;

 /*  Application program can define property using the tTaskProperties structure
  struct tTaskProperties {
	U32 				priority;				// 1	
	tAddr				stackAddr;				// 2
	U32 				stackSize;				// 3	
	tTaskMainFcn_posix	TaskMainFcn;			// 4
	U32					taskMainArgCount;		// 5
	tPtr				pTaskMainArgValues;		// 6						
	tTaskStartupMode	startupMode;			// 7
	tTaskSchedPolicy	schedulingPolicy;		// 8
	U32					schedulingInterval;		// 9
};
*/    
	/* initialized 'tattr' object with with default attributes */
	err = pthread_attr_init(&tattr);
	ASSERT_POSIX_CALL(err);

	/* Setting the thread scheduling attributes */
  
	/* 1. set the thread's set scope		*/
	err = pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
	ASSERT_POSIX_CALL(err);
	
	/* 2. set scheduling policy */
	err = pthread_attr_setschedpolicy(&tattr, pProperties->schedulingPolicy);
	ASSERT_POSIX_CALL(err);

	/* 3. set the priority */
    param.sched_priority = pProperties->priority;
	int sched_priority_min = sched_get_priority_min(pProperties->schedulingPolicy);
	int sched_priority_max = sched_get_priority_max(pProperties->schedulingPolicy);
	if( pProperties->priority < sched_priority_min )
	{	
		param.sched_priority = sched_priority_min;
	}	 
		 
	if( pProperties->priority > sched_priority_max )
	{	
		param.sched_priority = sched_priority_max;
	}	 

	err = pthread_attr_setschedparam (&tattr, &param);
	ASSERT_POSIX_CALL(err);

    //------------------------------------------------
    /* Address and size stack specified */
    //------------------------------------------------
	if(pProperties->stackSize)
	{
		err = pthread_attr_setstacksize(&tattr, pProperties->stackSize);
        ASSERT_POSIX_CALL(err);
	}	

	if( pProperties->stackAddr )
	{
		err = pthread_attr_setstack(&tattr, (void *)pProperties->stackAddr, pProperties->stackSize);
        ASSERT_POSIX_CALL(err);
	}
    
	pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
	ASSERT_POSIX_CALL(err);
	
    err = pthread_create((pthread_t *)pHndl,
                     &tattr,
                     (void*(*)(void*))pProperties->TaskMainFcn,
                     (void *)pProperties->pTaskMainArgValues);
    ASSERT_POSIX_CALL(err);

	err = pthread_attr_destroy(&tattr);
	ASSERT_POSIX_CALL(err);
	
	// good to go
//	*pHndl = pTask->ToOpaqueType();
//	return kNoErr;
	// cleanup
//	if (pTask != NULL)
//		delete pTask;

//	*pHndl = kUndefinedTaskHndl;
    return kNoErr;
}

tErrType CKernelModule::JoiningThreads( tTaskHndl pHndl, void **value_ptr )
{
	return pthread_join( pHndl, value_ptr );  
}
//------------------------------------------------------------------------------
tErrType CKernelModule::CancelTask( tTaskHndl hndl )
{
// This service terminates the task specified by the task parameter.
// NOTE 1: A terminated task cannot execute again until it is reset.
// NOTE 2: When calling this function from a signal handler, the task
//         whose signal handler is executing cannot be terminated.
	return pthread_cancel( hndl );
}



// This service returns the currently active task pointer.
tTaskHndl	CKernelModule::GetCurrentTask()
{
    return pthread_self();
}

tErrType CKernelModule::GetTaskPriority( tTaskHndl hndl, int* priority )
{
	int err;
	struct sched_param param;
	int policy;	
	
    err = pthread_getschedparam(hndl, &policy, &param);
	ASSERT_POSIX_CALL(err);

	*priority = param.sched_priority; 			
	
    return kNoErr;
}	

tErrType CKernelModule::GetTaskSchedulingPolicy( tTaskHndl hndl, int* policy )
{
	int err;
	struct sched_param param;
    
    err = pthread_getschedparam(hndl, policy, &param);
	ASSERT_POSIX_CALL(err);
    
	return kNoErr;
}	

tErrType CKernelModule::TaskSleep( U32 msec )
{
    while( msec > 999 )     /* For OpenBSD and IRIX, argument */
        {                   /* to usleep must be < 1000000.   */
        usleep( 999000 );
        msec -= 999;
        }
    usleep( msec * 1000 );

	return kNoErr;
}

//============================================================================
// Allocating memory from the System heap
//============================================================================
tErrType CKernelModule::Malloc( U32 size, tPtr pPtr )
{
	pPtr = (tPtr)malloc( size );
    ASSERT_ERROR(pPtr != 0, kCouldNotAllocateMemory);

	return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::Free( tPtr ptr )
{
	free(ptr);
	
	return kNoErr;
}

//==============================================================================
// Message Queues
//==============================================================================
tErrType	CKernelModule::CreateMessageQueue(const CURI* pQueueURI, 
						const tMessageQueuePropertiesPosix* pProperties,
						tMessageQueueHndl* pHndl)
{
    mqd_t retMq_open = -1;
    tErrType err = 0;   

    mq_attr queuAttr = {0};
   
    if (pProperties->mq_flags == 0 || pProperties->mq_flags == O_NONBLOCK)
    {
        queuAttr.mq_flags = pProperties->mq_flags;
    }
    else{
//            return ErrorBrio::lookupBrioErrType(wrongQueuFlagParameter); 
 			return -1; // rdg fixme!
        }
          
    queuAttr.mq_flags  = 0;
    queuAttr.mq_maxmsg =  pProperties->mq_maxmsg;
    queuAttr.mq_msgsize = pProperties->mq_msgsize;
    queuAttr.mq_curmsgs = pProperties->mq_curmsgs;
    
#if 1 // BSK Debug Printing
    printf("THe Input MUEUE parameters are :\n"); 
    printf("name=%s oflag=%d mode=%d mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsgs=%d\n",
    	pProperties->nameQueue, pProperties->oflag,pProperties->mode,
    	queuAttr.mq_flags, pProperties->mq_maxmsg, pProperties->mq_msgsize, pProperties->mq_curmsgs);   
    fflush(stdout);
#endif // BSK
    
    errno = 0;
    
    retMq_open = mq_open (pProperties->nameQueue,
                          pProperties->oflag,
                          pProperties->mode,
                          &queuAttr);
    
	ASSERT_POSIX_CALL(err);
    *pHndl = retMq_open;
    
#if 1 // BSK Debug Printing
    printf("Return value of mq_open function is %d ERRNO=%d\n", retMq_open, errno);
    fflush(stdout);;
    
    struct mq_attr attr = {0};
    mq_getattr(retMq_open, &attr);
   	printf("\n-----After opening QUEQUE attributes are: ----\n");
   	printf("mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
	attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
    fflush(stdout);
   
    printf("retMq_open=%u \n", retMq_open);
    fflush(stdout);

#endif // BK

    return kNoErr;

}

//------------------------------------------------------------------------------
tErrType	CKernelModule::DestroyMessageQueue(tMessageQueueHndl hndl)
{
    errno = 0;

    mq_close(hndl);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::UnlinkMessageQueue( const char *hndl )
{
    errno = 0;
    
    mq_unlink(hndl);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType  	CKernelModule::ClearMessageQueue( tMessageQueueHndl hndl )
{
    int ret;
    struct mq_attr attr;
    ssize_t nr;
    char *buf;
    
    /* Determine the # of messages currently in queue and
    max. msg size; allocate buffer to receive msg */
    
    errno = 0;
    
    mq_getattr(hndl, &attr);
   	ASSERT_POSIX_CALL( errno );

    buf = (char *)malloc(attr.mq_msgsize);
    
    for(int k = 0; k < attr.mq_curmsgs; k++) 
    {
        errno = 0; 

        nr = mq_receive(hndl, buf, attr.mq_msgsize, NULL);
        ASSERT_POSIX_CALL( errno );
    }
    
    free(buf);
    return kNoErr;
}
	
//------------------------------------------------------------------------------
tErrType CKernelModule::GetMessageQueueNumMessages( tMessageQueueHndl hndl, int *numMsgQueue )
{
    struct mq_attr attr = {0};
    mqd_t ret = -1;
    
    errno = 0;
    ret = mq_getattr(hndl, &attr);
    ASSERT_POSIX_CALL( errno );
    
    *numMsgQueue = attr.mq_curmsgs;
    return kNoErr;
}
 
//------------------------------------------------------------------------------
 tErrType	CKernelModule::SendMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize)
{
    
#if 0 // BSK
    printf("pMessage->GetMessageSize() = %d\n", pMessage->GetMessageSize());
    printf("pMessage.GetMessageEventType() =%d\n",
            ((CEventMessage *)pMessage)->GetMessageEventType()); 
    fflush(stdout);

#endif
    errno = 0;
    int err = mq_send( hndl,
                   (const char *)pMessage,
                   (size_t )pMessage->GetMessageSize(),
                   (unsigned )pMessage->GetMessagePriority() );
    
    ASSERT_POSIX_CALL( errno );
    
#if 1 // BSK Debugging printing 
    struct mq_attr attr = {0};
    mq_getattr(hndl, &attr);
    printf("\n-----After sending message QUEQUE attributes are: ----\n");
    printf("mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
            attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
    fflush(stdout);
#endif // BK

    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType  	CKernelModule::SendMessageOrWait( tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize, U32 timeoutMs )
{
    struct timespec tp;

    ASSERT_ERROR(timeoutMs<999,kInvalidFunctionArgument);  

    errno =0;
    
    clock_gettime(CLOCK_REALTIME, &tp);
   	ASSERT_POSIX_CALL( errno );

    tp.tv_sec = tp.tv_sec + timeoutMs / 1000; 
    tp.tv_nsec = tp.tv_nsec + timeoutMs * 1000000; // convert from ms to nanosecond

#if 0 // BSK
    printf("pMessage->GetMessageSize() = %d\n", pMessage->GetMessageSize());
    printf("pMessage.GetMessageEventType() =%d\n",
            ((CEventMessage *)pMessage)->GetMessageEventType()); 
    fflush(stdout);

#endif
    errno = 0;
    int err = mq_timedsend( hndl, 
                   (const char *)pMessage,
                   (size_t )pMessage->GetMessageSize(),
                   (unsigned )pMessage->GetMessagePriority(),
                   (const struct timespec *)&tp);
    
   	ASSERT_POSIX_CALL( errno );
    
#if 1 // Debugging printing 
    struct mq_attr attr = {0};
    mq_getattr(hndl, &attr);
    printf("\n-----After sending message QUEQUE attributes are: ----\n");
    printf("mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
            attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
    fflush(stdout);
#endif // BK

    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType  	CKernelModule::ReceiveMessage( tMessageQueueHndl hndl, U32 maxMessageSize, 
									   CMessage* msg_ptr, unsigned *msg_prio )
{
/*-----------------------------------------------------------
    U16		GetMessageSize() { return messageSize; }
    U8		GetMessagePriority() { return messagePriority; }
	U16	   	messageSize;
	U8	   	messagePriority;
	U8	   	reserved;
--------------------------------------------------------- */
    
// Retrieve attributes of the messges queque
   struct mq_attr attr = {0};
   
#if 1 // BSK Debug printing
   U8 tmp = msg_ptr->GetMessagePriority();
   mq_getattr(hndl, &attr);
   printf("\n-----CKernelModule::ReceiveMessage ----\n");
   printf("mq_flags=%d  mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
          attr.mq_flags,attr.mq_maxmsg,attr.mq_msgsize,attr.mq_curmsgs);
#endif // BK

    errno = 0;
    int ret_receive = mq_receive( hndl,
                                  (char *)msg_ptr,
                                  (size_t )sizeof(CEventMessage),
                                  msg_prio);
     if (ret_receive == -1)
     {
         //return ErrorBrio::lookupBrioErrType(errno);
		return -1;  // rdg fixme
     }
#if 1 // BSK Debug printing
    printf("CKernelModule::ReceiveMessage. Received=%d bytes\n", ret_receive);
    fflush(stdout);
#endif // BK
     return kNoErr;
}

//------------------------------------------------------------------------------
tErrType  	CKernelModule::ReceiveMessageOrWait( tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* msg_ptr, U32 timeoutMs )
{
// Retrieve attributes of the messges queque
   struct mq_attr attr = {0};
   
#if 1 // BSK Debug printing
   mq_getattr(hndl, &attr);
   printf("\n-----CKernelModule::ReceiveMessage ----\n");
   printf("mq_flags=%d  mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
          attr.mq_flags,attr.mq_maxmsg,attr.mq_msgsize,attr.mq_curmsgs);
#endif // BSK
   struct timespec tp;

   ASSERT_ERROR(timeoutMs<999,kInvalidFunctionArgument);  

   errno =0;
   if(clock_gettime(CLOCK_REALTIME, &tp) == -1)
   ASSERT_POSIX_CALL( errno );

   tp.tv_sec = tp.tv_sec + timeoutMs / 1000; 
   tp.tv_nsec = tp.tv_nsec + timeoutMs * 1000000; // convert from ms to nanosecond

   unsigned msg_prio = msg_ptr->GetMessagePriority();	
   errno = 0;
   int ret_receive = mq_timedreceive(hndl,
                                  (char *)msg_ptr,
                                  (size_t )sizeof(CEventMessage),
                                  &msg_prio,
                                  (const struct timespec *)&tp);
   ASSERT_POSIX_CALL( errno );

#if 1 // BSK Debug printing
    printf("CKernelModule::ReceiveMessage. Received=%d bytes\n", ret_receive);
    fflush(stdout);
#endif // BSK
     return kNoErr;

}
				
//==============================================================================
// Time & Timers
//==============================================================================
U32   		CKernelModule::GetElapsedTime( U32* pUs )
{
    timeval time;

	gettimeofday( &time, NULL );
	*pUs = time.tv_usec;
	return (time.tv_sec * 1000 );
}

//------------------------------------------------------------------------------
tErrType 	CKernelModule::CreateTimer( const CURI* pTimerURI,
                                    struct sigevent *se, tTimerHndl* pHndl )
{
//	tErrType err = timer_create(CLOCK_REALTIME, NULL, (timer_t *)pHndl);
//	tErrType err = timer_create(CLOCK_MONOTONIC, NULL, (timer_t *)pHndl);
//	tErrType err = timer_create(CLOCK_THREAD_CPUTIME_ID, NULL, (timer_t *)pHndl);
	
    errno = 0;
    timer_create( CLOCK_REALTIME, se, (timer_t *)pHndl );
    
	ASSERT_POSIX_CALL( errno );
    
    return kNoErr; 
}

//------------------------------------------------------------------------------
tErrType 	CKernelModule::DestroyTimer( tTimerHndl hndl )
{
	
    errno = 0;
    timer_delete( (timer_t)hndl );
    
	ASSERT_POSIX_CALL( errno );
    
    return kNoErr; 
}
	
//------------------------------------------------------------------------------
tErrType 	CKernelModule::ResetTimer( tTimerHndl hndl, const tTimerProperties* pTimerProperties)
{ 
	struct itimerspec value;
	
    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = (long )pTimerProperties->dummy;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	
    errno = 0;
    timer_settime( (timer_t )hndl, 0, &value, NULL );
    
	ASSERT_POSIX_CALL( errno );
    
    return kNoErr; 
}
	
//------------------------------------------------------------------------------
tErrType	CKernelModule::StartTimer( tTimerHndl hndl, const struct itimerspec* value )
{

    errno = 0;
    timer_settime( (timer_t )hndl, 0, value, NULL );
    
	ASSERT_POSIX_CALL( errno );
    return kNoErr; 
}
	
//------------------------------------------------------------------------------
tErrType	CKernelModule::StopTimer( tTimerHndl hndl )
{
	struct itimerspec value;
	
    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	
    errno = 0;
    timer_settime( (timer_t)hndl, 0, &value, NULL );	
	
    ASSERT_POSIX_CALL( errno );

    return kNoErr;

}

//------------------------------------------------------------------------------
// elapsed time in milliseconds // (& microseconds)	
tErrType CKernelModule::GetTimerElapsedTime( tTimerHndl hndl, U32* pUs ) 
{
	struct itimerspec value;
 //	printf("hndl=%d    12hndl=%d\n", hndl, (timer_t )hndl);
//	fflush(stdout);
	
    errno = 0;
    int err = timer_gettime( (timer_t )hndl, &value );
	
	ASSERT_POSIX_CALL( errno );
	
    *pUs = (value.it_interval.tv_sec -  value.it_value.tv_sec) * 1000 + 
           (value.it_interval.tv_nsec - value.it_value.tv_nsec) / 1000000;

#if 1 // BSK  Debug printing 
    printf("CKernelModule::GetTimerElapsedTime:\n value.it_interval.tv_sec=%ld\n value.it_value.tv_sec=%ld\n \
            value.it_interval.tv_nsec=%ld\n value.it_value.tv_nsec=%ld\n",
      value.it_interval.tv_sec, value.it_value.tv_sec, value.it_interval.tv_nsec,value.it_value.tv_nsec);       
#endif	

    return kNoErr;
}
	
//------------------------------------------------------------------------------
// time remaining in milliseconds (& microseconds)
tErrType CKernelModule::GetTimerRemainingTime( tTimerHndl hndl, U32* pUs ) 
{
	struct itimerspec value;
	
    errno = 0;
    timer_gettime( (timer_t )hndl, &value );
	
    ASSERT_POSIX_CALL( errno );
	
    *pUs = value.it_value.tv_sec * 1000 + value.it_value.tv_nsec / 1000000;
	
    return kNoErr;	
}

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
extern "C"
{
	//------------------------------------------------------------------------
	tVersion ReportVersion()
	{
		// TBD: ask modules for this or incorporate verion into so file name
		return kKernelModuleVersion;
	}
	
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance( tVersion version )
	{
		if (sinst == NULL)
			sinst = new CKernelModule;
		return sinst;
	}
	
	//------------------------------------------------------------------------
	void DestroyInstance( ICoreModule* ptr )
	{
//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}


// EOF

