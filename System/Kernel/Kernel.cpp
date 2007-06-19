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
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <list>
#include <algorithm>
#include <functional>

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <CoreMPI.h>
#include <KernelMPI.h>
#include <KernelPriv.h>
#include <algorithm>
#include <functional>

//#include <FreeMemory.h>

using namespace std;
extern "C"
{
	void sig_handler(int signal, siginfo_t *psigInfo, void *pFunc);
}

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

const CURI	kModuleURI	= "/LF/System/Kernel";

// single instance of module object.
static CKernelModule*	sinst = NULL;

//==============================================================================
// Definition variables for timer 
//==============================================================================
namespace
{
	int pthreadTimer = 0; 
	pthread_mutex_t mutexValue_1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutexValue_2 = PTHREAD_MUTEX_INITIALIZER;

class ListData
{
	public:
		ListData(U32 ptr, U32 hndl)
		: ptr_(ptr), hndl_(hndl)
		{}

U32 getHndl() const
{
	return hndl_;
}
void setPtr(U32 ptr)
{
	ptr_ = ptr;
}

private:
	U32 ptr_;
	U32 hndl_;
};

struct equal_id : public binary_function<ListData*, int, bool>
{
	bool operator()(const ListData* pp, U32 i) const
{
	return pp->getHndl() == i;
}
};

	static list<ListData *> listMemory;

}
	
//==============================================================================
// Internal untility functions
//==============================================================================
inline tTimerHndl AsBrioTimerHandle(timer_t hndlIn)
{
	return reinterpret_cast<tTimerHndl>(hndlIn) + 1;
}

inline timer_t AsPosixTimerHandle(tTimerHndl hndlIn)
{
	return reinterpret_cast<timer_t>(hndlIn - 1);
}


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
tVersion CKernelModule::GetModuleVersion() const
{
	return kKernelModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CKernelModule::GetModuleName() const
{
	return &kKernelModuleName;
}

//----------------------------------------------------------------------------
const CURI* CKernelModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//==============================================================================
// Tasks
//==============================================================================
//------------------------------------------------------------------------------
tErrType CKernelModule::CreateTask(tTaskHndl& hndl, 
									const tTaskProperties& props, 
									const char* /*pDebugName*/)
{
	// NOTE: pTaskURI is used in task creation of some RTOSs and is propgated
	// in debug information, but not in POSIX pthreads.
	//
	pthread_attr_t tattr;
	sched_param param;
	tErrType err = kNoErr;
	hndl = kInvalidTaskHndl;

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
	err = pthread_attr_setschedpolicy(&tattr, props.schedulingPolicy);
	ASSERT_POSIX_CALL(err);

	/* 3. set the priority */
    param.sched_priority = props.priority;
	unsigned int sched_priority_min = 
		sched_get_priority_min(props.schedulingPolicy);
	unsigned int sched_priority_max = 
		sched_get_priority_max(props.schedulingPolicy);
	if( props.priority < sched_priority_min )
	{	
		param.sched_priority = sched_priority_min;
	}	 
		 
	if( props.priority > sched_priority_max )
	{	
		param.sched_priority = sched_priority_max;
	}	 

	err = pthread_attr_setschedparam (&tattr, &param);
	ASSERT_POSIX_CALL(err);

    //------------------------------------------------
    /* Address and size stack specified */
    //------------------------------------------------
	if(props.stackSize)
	{
		err = pthread_attr_setstacksize(&tattr, props.stackSize);
        ASSERT_POSIX_CALL(err);
	}	

	if( props.stackAddr )
	{
		err = pthread_attr_setstack(&tattr, (void *)props.stackAddr, props.stackSize);
        ASSERT_POSIX_CALL(err);
	}
    
	pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
	ASSERT_POSIX_CALL(err);
	
	pthread_t	pthread;
   	err = pthread_create(&pthread,
                     &tattr,
                     (void*(*)(void*))props.TaskMainFcn,
                     (void *)props.pTaskMainArgValues);

    ASSERT_POSIX_CALL(err);
    hndl = static_cast<tTaskHndl>(pthread);
    // FIXME/tp: Determine if 0 is a valid value for pthread, and hav the "hndl" always
    // represent the pthread+1 if so.

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

//------------------------------------------------------------------------------
tErrType CKernelModule::JoinTask( tTaskHndl pHndl, tPtr& threadReturnValue )
{
//	return AsBrioErr(pthread_join(pHndl, &threadReturnValue));
	tErrType err = kNoErr;
	
//	errno = 0;
	err = pthread_join(pHndl, &threadReturnValue);
	ASSERT_POSIX_CALL(err);
	
    return kNoErr;
  
}

//------------------------------------------------------------------------------
tErrType CKernelModule::CancelTask( tTaskHndl hndl )
{
// This service terminates the task specified by the task parameter.
// NOTE 1: A terminated task cannot execute again until it is reset.
// NOTE 2: When calling this function from a signal handler, the task
//         whose signal handler is executing cannot be terminated.
//	return AsBrioErr(pthread_cancel(hndl));
	tErrType err = kNoErr;
	
	errno = 0;
	err = pthread_cancel(hndl);
	if ( err > 0 && err != ESRCH )
	{
		ASSERT_POSIX_CALL(err);
	}
    return kNoErr;
}
//------------------------------------------------------------------------------
// This service returns the currently active task pointer.
tTaskHndl CKernelModule::GetCurrentTask() const
{
    return pthread_self();
}

//------------------------------------------------------------------------------
int CKernelModule::GetTaskPriority(tTaskHndl hndl) const
{
	int err;
	struct sched_param param;
	int priority = 0;
	int policy;
	
    if ((err = pthread_getschedparam(hndl, &policy, &param)) != 0)
    {
		ASSERT_POSIX_CALL(err);
    }
	else
		priority = param.sched_priority;
	return priority;			
}	

//------------------------------------------------------------------------------
int CKernelModule::GetTaskSchedulingPolicy(tTaskHndl hndl) const 
{
	int err;
	struct sched_param param;
	int policy = kTaskSchedPolicyUndefined;
    
    if ((err = pthread_getschedparam(hndl, &policy, &param)) != 0)
    {
		ASSERT_POSIX_CALL(err);
    }
	return policy;
}	

//------------------------------------------------------------------------------
void CKernelModule::TaskSleep(U32 msec) const
{
 //   while( msec > 999 )     /* For OpenBSD and IRIX, argument */
 //   {                       /* to usleep must be < 1000000.   */
 //       usleep( 999000 );
 //       msec -= 999;
//	}
//    usleep( msec * 1000 ); This function is obsolete. Need to use nanosleep instead.
	struct timespec sleeptime;
	sleeptime.tv_sec = msec /1000;
	sleeptime.tv_nsec = (msec % 1000) * 1000000;
	nanosleep( &sleeptime, NULL ); 
}

//============================================================================
// Allocating memory from the System heap
//============================================================================
tPtr CKernelModule::Malloc(U32 size)
{
	// FIXME/tp: True assertion here if fail to allocate memory, or just warning?
	tPtr ptr = malloc( size );
    ASSERT_ERROR(pPtr != 0, kCouldNotAllocateMemory);
	return ptr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::Free(tPtr ptr)
{
	free(ptr);
	return kNoErr;
}

//============================================================================
// Terminal/debug IO
//============================================================================
//------------------------------------------------------------------------------
void CKernelModule::Printf( const char* format, ... ) const
{
	va_list arguments;
	va_start(arguments, format);
	vprintf(format, arguments);
	va_end(arguments);
}

//------------------------------------------------------------------------------
void CKernelModule::Printf( const char* format, va_list arguments ) const
{
	vprintf(format, arguments);
}

//==============================================================================
// Message Queues
//==============================================================================
tErrType CKernelModule::OpenMessageQueue(tMessageQueueHndl& hndl,
									const tMessageQueuePropertiesPosix& props,
									const char* /*pDebugName*/)
{
    mqd_t retMq_open = -1;
//    tErrType err = 0;   

    mq_attr queuAttr = {0};
   
    if (props.mq_flags != 0 && props.mq_flags != O_NONBLOCK)
		return kInvalidParamErr;
	queuAttr.mq_flags	= props.mq_flags;
    queuAttr.mq_flags	= 0;
    queuAttr.mq_maxmsg	= props.mq_maxmsg;
    queuAttr.mq_msgsize	= props.mq_msgsize;
    queuAttr.mq_curmsgs	= props.mq_curmsgs;
    
#if 0 // BSK Debug Printing
    printf("THe Input MUEUE parameters are :\n"); 
    printf("name=%s oflag=%d mode=%d \n mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsgs=%d\n",
    	props.nameQueue, props.oflag, props.mode,
    	queuAttr.mq_flags, props.mq_maxmsg, props.mq_msgsize, props.mq_curmsgs);   
    fflush(stdout);
#endif // BSK
    
    errno = 0;
    retMq_open = mq_open (props.nameQueue, // /test_q
                          props.oflag,     // 578
                          props.mode,      // 448
                          &queuAttr);
    
	ASSERT_POSIX_CALL(errno);
    hndl = retMq_open;
    
#if 0 // BSK Debug Printing
    printf("Return value of mq_open function is %d ERRNO=%d\n", retMq_open, errno);
    fflush(stdout);;
//         struct mq_attr {
//							long mq_flags;       /* Flags: 0 or O_NONBLOCK */
//							long mq_maxmsg;      /* Max. # of messages on queue */
//	            			long mq_msgsize;     /* Max. message size (bytes) */
//             				long mq_curmsgs;     /* # of messages currently in queue */
//         				  }
    
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
tErrType CKernelModule::CloseMessageQueue(tMessageQueueHndl hndl,
							const tMessageQueuePropertiesPosix& props)
{
	// FIXME/tp: This just closes it, not destroy.
	// FIXME/tp: Need ot fold UnlinkMessageQueue() into this function to truly
	// destroy the queue.  This will require tracking its name through its handle.
    errno = 0;
    if( hndl != 0 )
    {
    	mq_close(hndl);
    	ASSERT_POSIX_CALL( errno );
    	
    	errno = 0;
    	mq_unlink(props.nameQueue);
    	ASSERT_POSIX_CALL( errno );
    }
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::ClearMessageQueue(tMessageQueueHndl hndl)
{

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
int CKernelModule::GetMessageQueueNumMessages(tMessageQueueHndl hndl) const
{
    struct mq_attr attr = {0};
    mqd_t ret = -1;
    
    errno = 0;
    ret = mq_getattr(hndl, &attr);
    ASSERT_POSIX_CALL( errno );
    
    return attr.mq_curmsgs;
}
 
//------------------------------------------------------------------------------
tErrType CKernelModule::SendMessage(tMessageQueueHndl hndl, const CMessage& msg) 
{
#if 0 // BSK
    printf("pMessage->GetMessageSize() = %d\n", pMessage->GetMessageSize());
    printf("pMessage.GetMessageEventType() =%d\n",
            ((CEventMessage *)pMessage)->GetMessageEventType()); 
    fflush(stdout);

#endif
    errno = 0;
    mq_send(hndl,
        reinterpret_cast<const char *>(&msg),
        msg.GetMessageSize(),
        msg.GetMessagePriority());
    
    ASSERT_POSIX_CALL( errno );
    
#if 0 // BSK Debugging printing 
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
tErrType CKernelModule::SendMessageOrWait(tMessageQueueHndl hndl, 
											const CMessage& msg, U32 timeoutMs)
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
    mq_timedsend(hndl, 
          reinterpret_cast<const char *>(&msg),
          msg.GetMessageSize(),
          msg.GetMessagePriority(), &tp);
    
   	ASSERT_POSIX_CALL( errno );
    
#if 0 // Debugging printing 
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
tErrType CKernelModule::ReceiveMessage( tMessageQueueHndl hndl, 
									   CMessage* msg_ptr, U32 maxMessageSize )
{
/*-----------------------------------------------------------
    U16		GetMessageSize() { return messageSize; }
    U8		GetMessagePriority() { return messagePriority; }
	U16	   	messageSize;
	U8	   	messagePriority;
	U8	   	reserved;
--------------------------------------------------------- */
    
// Retrieve attributes of the messges queque
//   struct mq_attr attr = {0};
   unsigned int  msg_prio;
   
#if 0 // BSK Debug printing
   U8 tmp = msg_ptr->GetMessagePriority();
   mq_getattr(hndl, &attr);
   printf("\n-----CKernelModule::ReceiveMessage ----\n");
   printf("mq_flags=%d  mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
          attr.mq_flags,attr.mq_maxmsg,attr.mq_msgsize,attr.mq_curmsgs);
#endif // BK

    errno = 0;
    int ret_receive = mq_receive( hndl,
                                  reinterpret_cast<char *>(msg_ptr),
                                  maxMessageSize, 
                                  &msg_prio);
 
    if (ret_receive == -1) {
     	printf("mq_receive returned an error!  errno = %d, errstr = %s\n", errno, strerror(errno));
     	return AsBrioErr(errno);
    }	
    assert(msg_prio == msg_ptr->GetMessagePriority());
#if 1 // BSK Debug printing
    printf("CKernelModule::ReceiveMessage. Received=%d bytes\n", ret_receive);
    fflush(stdout);
#endif // BK
     return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::ReceiveMessageOrWait( tMessageQueueHndl hndl, 
									CMessage* msg_ptr, U32 maxMessageSize, 
									U32 timeoutMs )
{
// Retrieve attributes of the messges queque
//   struct mq_attr attr = {0};
   
#if 0 // BSK Debug printing
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

   unsigned int  msg_prio;
   errno = 0;
   int ret_receive = mq_timedreceive(hndl,
                                  (char *)msg_ptr,
                                  (size_t )sizeof(CEventMessage),
                                  &msg_prio,
                                  (const struct timespec *)&tp);
	ASSERT_POSIX_CALL( errno );
    assert(msg_prio == msg_ptr->GetMessagePriority());

#if 1 // BSK Debug printing
    printf("CKernelModule::ReceiveMessage. Received=%d bytes\n", ret_receive);
    fflush(stdout);
#endif // BSK
     return kNoErr;

}
				
//==============================================================================
// Time & Timers
//==============================================================================
U32	CKernelModule::GetElapsedTimeAsMSecs()
{
    timeval time;

	gettimeofday( &time, NULL );
	
	return ( time.tv_sec * 1000 + time.tv_usec / 1000);
}

U64	CKernelModule::GetElapsedTimeAsUSecs()
{
    timeval time;

	errno = 0;
	gettimeofday( &time, NULL );
    ASSERT_POSIX_CALL( errno );

	return ( ((U64 )time.tv_sec) * 1000000 + time.tv_usec);
}
//------------------------------------------------------------------------------
tTimerHndl 	CKernelModule::CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName )
{
//	tErrType err = kNoErr;
//    sigset_t signal_set;
	tErrType err = kNoErr;
    int signum = SIGRTMAX;     

//	clockid_t clockid;     Now, it is used CLOCK_REALTIME
						// There are the following possible values:
						//	CLOCK_MONOTONIC
						//	CLOCK_THREAD_CPUTIME_ID
	clockid_t clockid = CLOCK_REALTIME;
//	clockid = CLOCK_REALTIME;
						// struct sigevent se;
						// Event structure
	                    // The  evp  argument,  if  non-NULL,  points  to  a sigevent structure.
	                    // This structure, allocated by the application,
       					// defines the asynchronous notification to occur as specified  in  Signal  Generation
       					// and  Delivery  when  the  timer expires.  If the evp argument is NULL, the effect
       					// is as if the evp argument pointed to a sigevent structure with the sigev_notify
       					// member having the value  SIGEV_SIGNAL,  the  sigev_signo  having  a  default
       					// signal  number,  and  the sigev_value member having the value of the timer ID.
               		    //--------------------------------------------------------------------------------
               		    //     	union sigval
               		    //		 {    /* Data passed with notification */
             			//				int     sival_int;   /* Integer value */
             			//				void   *sival_ptr;   /* Pointer value */
         				//		  };
                        // 
         				//		struct sigevent 
         				//		{
             			//			int    sigev_notify;      /* Notification method */
             			//			int    sigev_signo;       /* Notification signal */
             			//			union sigval sigev_value; /* Data passed with notification */
             			//			void (*sigev_notify_function) (union sigval);
                        //       							/* Function for thread notification */
             			//			void  *sigev_notify_attributes;	/* Thread function attributes */
         				//		};

	pthread_mutex_lock( &mutexValue_1);
	pthreadTimer++;
	pthread_mutex_unlock( &mutexValue_1);

    // Initialize the sigaction structure for handler 
	if( pthreadTimer >1 )
	{
    	/* Setup signal to repond to handler */
    	struct sigaction act;
//   	sigemptyset( &act.sa_mask );
    	sigfillset( &act.sa_mask );
    	sigaddset( &act.sa_mask, SIGRTMAX ); 
    	act.sa_flags = SA_SIGINFO; //SA_RESTART| // 
    	act.sa_sigaction = sig_handler;
    	sigaction( signum, &act, NULL ); 
	}	
    
	// Set up timer
    struct sigevent se;
    memset(&se, 0, sizeof(se)); 
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = signum;
	
	callbackData *ptrData = 
		(callbackData *)(malloc(sizeof(callbackData)));
	ASSERT_POSIX_CALL( !ptrData );
	se.sigev_value.sival_ptr = (void *)ptrData;

	timer_t	posixHndl;
    errno = 0;
    timer_create(clockid, &se, &posixHndl); // BSK
	ASSERT_POSIX_CALL( errno );

	tTimerHndl hndl = AsBrioTimerHandle(posixHndl);
 	ptrData->pfn = callback;
	ptrData->argFunc = hndl;
	
	ListData *ptrList = new ListData((U32 )callback, (U32 )hndl);

	pthread_mutex_lock( &mutexValue_1);
	listMemory.push_back( ptrList );
	pthread_mutex_unlock( &mutexValue_1);
#if 0 // FIXME/BSK
		printf("CreateTimer tTimerHndl=0x%x \n", hndl);
		fflush(stdout);  		
#endif
	
    return hndl;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::DestroyTimer( tTimerHndl hndl )
{
	tErrType err = kNoErr;
    errno = 0;

#if 0
	printf("DestroyTimer Before Num elements = %d\n", listMemory.size() );
	fflush(stdout);
#endif

    timer_delete(AsPosixTimerHandle( hndl ));
	ASSERT_POSIX_CALL( errno );

	list<ListData*>::iterator p = find_if(listMemory.begin(), listMemory.end(),
	bind2nd(equal_id(), hndl));

	assert((p != listMemory.end()) && ((*p)->getHndl() == hndl));
	(*p)->setPtr( 0 );
	err = pthread_mutex_lock( &mutexValue_2);
	ASSERT_POSIX_CALL( err );
    listMemory.erase( p ); 
	err = pthread_mutex_unlock( &mutexValue_2);
	ASSERT_POSIX_CALL( err );

#if 0
	printf("DestroyTimer After Num elements = %d\n", listMemory.size() );
	fflush(stdout);
#endif

    return kNoErr; 
}
	
//------------------------------------------------------------------------------
tErrType CKernelModule::ResetTimer(tTimerHndl hndl, const tTimerProperties& props)
{ 
	struct itimerspec value;
	
    value.it_interval.tv_sec = props.timeout.it_interval.tv_sec;
	value.it_interval.tv_nsec = props.timeout.it_interval.tv_nsec;
	value.it_value.tv_sec = props.timeout.it_value.tv_sec;
	value.it_value.tv_nsec = props.timeout.it_value.tv_nsec;
	
    errno = 0;
    timer_settime(AsPosixTimerHandle( hndl ), props.type, &value, NULL );
	ASSERT_POSIX_CALL( errno );
	// FIXME/tp: Need to internally track the sigevent and clock the condition
    
    return kNoErr; 
}
	
//------------------------------------------------------------------------------
tErrType CKernelModule::StartTimer( tTimerHndl hndl, const tTimerProperties& props )
{
	struct itimerspec value;
	// FIXME/tp: Lookup timer properties stored in ResetTimer and initialize "value"

    value.it_interval.tv_sec = props.timeout.it_interval.tv_sec;
	value.it_interval.tv_nsec = props.timeout.it_interval.tv_nsec;
	value.it_value.tv_sec = props.timeout.it_value.tv_sec;
	value.it_value.tv_nsec = props.timeout.it_value.tv_nsec;

    errno = 0;
    timer_settime(AsPosixTimerHandle( hndl ), 0, &value, NULL );
	ASSERT_POSIX_CALL( errno );

    return kNoErr; 
}
	
//------------------------------------------------------------------------------
tErrType CKernelModule::StopTimer( tTimerHndl hndl )
{
	struct itimerspec value;

    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;

// FIXME/BSK	
//	U32 remaining = GetTimerRemainingTime(hndl);
	
    errno = 0;
    timer_settime(AsPosixTimerHandle( hndl ), 0, &value, NULL );	
    ASSERT_POSIX_CALL( errno );

    return kNoErr;
}
//------------------------------------------------------------------------------
// FIXME/BSK
tErrType CKernelModule::PauseTimer( tTimerHndl hndl, saveTimerSettings& saveValue )
{
	struct itimerspec value;

	errno=0;

	timer_gettime( AsPosixTimerHandle( hndl ), &value ); 
    ASSERT_POSIX_CALL( errno );

	saveValue = value;     
#if 0 // FIXME/BSK    
    printf(" Pause values:\n %d %d | %d %d || %d %d | %d %d \n",  
    	saveValue.it_value.tv_sec, value.it_value.tv_sec,
		saveValue.it_value.tv_nsec, value.it_value.tv_nsec,
		saveValue.it_interval.tv_sec,  value.it_interval.tv_sec,
		saveValue.it_interval.tv_nsec, value.it_interval.tv_nsec);
		fflush(stdout);
#endif

    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;

// FIXME/BSK
//	U32 remaining = GetTimerRemainingTime(hndl);
	
    errno = 0;
    timer_settime(AsPosixTimerHandle( hndl ), 0, &value, NULL );	
	
    ASSERT_POSIX_CALL( errno );

    return kNoErr;

}
// FIXME/BSK
//------------------------------------------------------------------------------
tErrType CKernelModule::ResumeTimer( tTimerHndl hndl, saveTimerSettings& saveValue )
{
	struct itimerspec value;
	// FIXME/tp: Lookup timer properties stored in ResetTimer and initialize "value"
//	value = saveTimer;
     value = saveValue;

#if 0 // FIXME/BSK    
    printf("Resume values:\n %d %d | %d %d || %d %d | %d %d \n",  
    	saveValue.it_value.tv_sec, value.it_value.tv_sec,
		saveValue.it_value.tv_nsec, value.it_value.tv_nsec,
		saveValue.it_interval.tv_sec,  value.it_interval.tv_sec,
		saveValue.it_interval.tv_nsec, value.it_interval.tv_nsec);
		fflush(stdout);

#endif
	errno = 0;
    timer_settime(AsPosixTimerHandle( hndl ), 0, &value, NULL );
	ASSERT_POSIX_CALL( errno );
    return kNoErr; 
}
	
//------------------------------------------------------------------------------
// elapsed time in milliseconds // (& microseconds)	
U32 CKernelModule::GetTimerElapsedTime( tTimerHndl hndl, U32* pUs ) const
{
	struct itimerspec value;
 //	printf("hndl=%d    12hndl=%d\n", hndl, (timer_t )hndl);
//	fflush(stdout);
	
    errno = 0;
    timer_gettime( AsPosixTimerHandle( hndl ), &value );
	ASSERT_POSIX_CALL( errno );
	
    U32 milliSeconds = (value.it_interval.tv_sec -  value.it_value.tv_sec) * 1000 + 
          			 (value.it_interval.tv_nsec - value.it_value.tv_nsec) / 1000000;

		if( pUs )
			*pUs =  (value.it_interval.tv_sec -  value.it_value.tv_sec) * 1000000 + 
          			(value.it_interval.tv_nsec - value.it_value.tv_nsec) / 1000;
			
#if 0 // BSK  Debug printing 
    printf("CKernelModule::GetTimerElapsedTime:\n value.it_interval.tv_sec=%ld\n value.it_value.tv_sec=%ld\n \
            value.it_interval.tv_nsec=%ld\n value.it_value.tv_nsec=%ld\n",
      value.it_interval.tv_sec, value.it_value.tv_sec, value.it_interval.tv_nsec,value.it_value.tv_nsec);       
#endif	

    return milliSeconds;
}
	
//------------------------------------------------------------------------------
// time remaining in milliseconds (& microseconds)
U32 CKernelModule::GetTimerRemainingTime( tTimerHndl hndl, U32* pUs ) const
{
	struct itimerspec value;
	
    errno = 0;
    timer_gettime( AsPosixTimerHandle( hndl ), &value );
    ASSERT_POSIX_CALL( errno );
	
    U32 milliSeconds = value.it_value.tv_sec * 1000 
    				+ value.it_value.tv_nsec / 1000000;
	
	if( pUs )
			*pUs = value.it_value.tv_sec * 1000000 
    				+ value.it_value.tv_nsec / 1000; 
    return milliSeconds;	
}

//------------------------------------------------------------------------------
// Mutexes
//------------------------------------------------------------------------------
    // Initialize a mutex Attribute Object 
tErrType CKernelModule::InitMutexAttributeObject( tMutexAttr& mutexAttr )
{
    errno = 0;
    tErrType err = kNoErr;
    
    tMutexAttr mutexAttrTmp;
    pthread_mutexattr_init( &mutexAttrTmp );
    ASSERT_POSIX_CALL( err );
   
    mutexAttr = static_cast<tMutexAttr>(mutexAttrTmp);
    
    return kNoErr;

}	


tErrType CKernelModule::InitMutex( tMutex& mutex, const tMutexAttr& mutexAttr )
{
    errno = 0;
    tErrType err = kNoErr;
    
    tMutex  mutexTmp;
    pthread_mutex_init(&mutexTmp, &mutexAttr);
    ASSERT_POSIX_CALL( err );

    mutex = static_cast<tMutex>(mutexTmp);

    return kNoErr;
}
//------------------------------------------------------------------------------
tErrType CKernelModule::DeInitMutex( tMutex& mutex )
{
    errno = 0;
    pthread_mutex_destroy(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
S32 CKernelModule::GetMutexPriorityCeiling(const tMutex& mutex) const
{
   errno = 0;
   int prioCeiling = 0;
// FIXME /BSK NOT DEFINED YET    
//   pthread_mutex_getprioceiling(&mutex, &prioCeiling);
   ASSERT_POSIX_CALL( errno );
   
   return prioCeiling;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority )
{
   errno = 0;

// FIXME /BSK NOT DEFINED YET    
//   pthread_mutex_setprioceiling(&mutex, (int)prioCeiling, (int*)pOldPriority);
   ASSERT_POSIX_CALL( errno );
    
   return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::LockMutex( tMutex& mutex )
{
    errno = 0;
    
    pthread_mutex_lock(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::TryLockMutex( tMutex& mutex )
{
    errno = 0;
    
    pthread_mutex_trylock(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::UnlockMutex( tMutex& mutex )
{
    errno = 0;
    
    pthread_mutex_unlock(&mutex);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
// Conditions 
//------------------------------------------------------------------------------

tErrType CKernelModule::BroadcastCond( tCond& cond )
{
    errno = 0;

    pthread_cond_broadcast(&cond);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::DestroyCond( tCond& cond )
{
    errno = 0;

    pthread_cond_destroy(&cond);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::InitCond( tCond& cond, const tCondAttr& attr )
{
    errno = 0;

    pthread_cond_init(&cond, &attr);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::SignalCond( tCond& cond )
{
    errno = 0;

    pthread_cond_signal(&cond);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime )
{
	tErrType err = kNoErr;
    errno = 0;

    err = pthread_cond_timedwait(&cond, &mutex, pAbstime );

    if( err == ETIMEDOUT )
    {
    	return err;
    }

    if( err > 0 && err != ETIMEDOUT )
    {
    	ASSERT_POSIX_CALL( err );
    }


    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::WaitOnCond( tCond& cond, tMutex& mutex )
{
	tErrType err = kNoErr;
    errno = 0;

    err = pthread_cond_wait(&cond, &mutex);
    ASSERT_POSIX_CALL( err );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::DestroyCondAttr( tCondAttr& attr )
{
    errno = 0;

    pthread_condattr_destroy( &attr );
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::GetCondAttrPShared( const tCondAttr& attr, int* pShared )
{
    errno = 0;
// FIXME/BSK To get the new library
//    pthread_condattr_getpshared( &attr, pShared );
//    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::InitCondAttr( tCondAttr& attr )
{
    errno = 0;

    pthread_condattr_init(&attr);
    ASSERT_POSIX_CALL( errno );
    
    return kNoErr;
}

//------------------------------------------------------------------------------
tErrType CKernelModule::SetCondAttrPShared( tCondAttr* pAttr, int shared )

{
    errno = 0;
// FIXME/BSK To get the new library
//    pthread_condattr_setpshared( pAttr, shared );
//    ASSERT_POSIX_CALL( errno );
    
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

void sig_handler( int signal, siginfo_t *psigInfo, void *pFunc)
{
		callbackData *ta = (callbackData *)psigInfo->si_value.sival_ptr;

#if 0 // FIXME/BSK
	printf("HANDLER CALLBACK = 0x%x, ARG =0x%x\n", *(ta->pfn), (tTimerHndl )ta->argFunc);
	fflush(stdout);
#endif
		((ta->pfn))((tTimerHndl )ta->argFunc);
}

} // extern "C"

// EOF

