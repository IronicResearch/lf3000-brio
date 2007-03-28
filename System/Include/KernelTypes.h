#ifndef KERNELTYPES_H
#define KERNELTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelTypes.h
//
// Description:
//		System Kernel's basic types.
//
//==============================================================================
#include <mqueue.h>
#include <sched.h>
#include <ErrorBrio.h>

typedef tHndl	tTaskHndl;
typedef tHndl	tMemoryPoolHndl;
typedef tHndl	tMessageQueueHndl;
typedef tHndl	tTimerHndl;

typedef U8		tTaskPriority;
typedef U8		tMessagePriority;

typedef void 	(*tTaskMainFcn)(U32 argCount, tPtr pArgValues); // Old

typedef void 	*(*tTaskMainFcn_posix)(tPtr pArgValues);

#define kMaxTimeoutMs	((U32)kU32Max)

#define kUndefinedMessageSize		((U16)0)
#define kUndefinedMemoryPoolHndl 	((tMemoryPoolHndl)kUndefinedHndl)
#define kUndefinedTaskHndl 		((tTaskHndl)kUndefinedHndl)

//------------------------------------------------------------------------------
// Task scheduling & blocking
//------------------------------------------------------------------------------


enum {
	kTaskSchedPolicyUndefined 	= 0,
	kTaskSchedPolicyFIFO = SCHED_FIFO,			
	kTaskSchedPolicyRR = SCHED_RR,			
	kTaskSchedPolicyOTHER = SCHED_OTHER,			
//    kTaskSchedPolicyTimeSlice,
//	kTaskSchedPolicyNoPreemption,
	// default
	kTaskSchedPolicyDefault	= kTaskSchedPolicyFIFO,
};

typedef U16	tTaskSchedPolicy;


enum {
	kTaskBlockingPolicyUndefined = 0,
	kTaskBlockingPolicyFIFO,
	kTaskBlockingPolicyPriority,
	// default
	kTaskBlockingPolicyDefault = kTaskBlockingPolicyFIFO,
};

typedef U16 	tTaskBlockingPolicy;

//------------------------------------------------------------------------------
// Task operating modes
//
//	Most tasks are created and run in their default operating mode, but some
//	tasks might have different possible operating modes at creation based
//	on configuration conditions or might have properties that need to be
//	changed at runtime in response to System conditions.
//
//	CKernelMPI::GetTaskPropertiesPreferences(...) returns the preferred 
//	properties based on the mode of operation.
//------------------------------------------------------------------------------

#define kFirstTaskOperatingMode			1

#define kFirstSystemTaskOperatingMode		(0x1000 | kFirstTaskOperatingMode)
#define kFirstProductTaskOperatingMode		(0x2000 | kFirstTaskOperatingMode)
#define kFirstApplicationTaskOperatingMode	(0x8000 | kFirstTaskOperatingMode)
#define kFirstTaskSpecificOperatingMode	(0xF000 | kFirstTaskOperatingMode)

enum {
	kTaskOperatingModeUndefined	= 0,
	kTaskOperatingModeNormal	= kFirstSystemTaskOperatingMode,

	// default
	kTaskOperatingModeDefault 	= kTaskOperatingModeNormal
};

typedef U16	tTaskOperatingMode;

//------------------------------------------------------------------------------
// Task startup modes
//
//	Defines how & when tasks start running when first created
//------------------------------------------------------------------------------

enum {
	kTaskStartupModeUndefined	= 0,
	kTaskStartupModeRun,			// starts up normally
	kTaskStartupModeBlocked,		// starts blocked until UnblockTask called
	kTaskStartupModeBlockCreatorOnSemaphore,	// creating task is blocked until
									// new task releases semaphore

	// default
	kTaskStartupModeDefault	= kTaskStartupModeRun,
};

typedef U16	tTaskStartupMode;

//------------------------------------------------------------------------------
// Task properties
//------------------------------------------------------------------------------

enum {
	kTaskPropertyUndefined	= 0,
	kTaskPropertyPriority,
	kTaskPropertyStackSize,
	kTaskPropertyTaskMainFcn,
	kTaskPropertyTaskMainArgCount,
	kTaskPropertyTaskMainArgValuesPtr,
	kTaskPropertyStartupMode,
	kTaskPropertySchedPolicy,
	kTaskPropertySchedInterval,

	// extents
	kFirstTaskProperty		= kTaskPropertyPriority,
	kLastTaskProperty		= kTaskPropertySchedInterval,
	kNumTaskProperties		= (kLastTaskProperty - kFirstTaskProperty + 1),
};

typedef U32	tTaskPropertiesMask;

#define MakeTaskPropertyMask(taskProperty)	\
		((tTaskPropertiesMask)(1 << (taskProperty - kFirstTaskProperty)))

#define kTaskPropertiesMaskNull	((tTaskPropertiesMask)0)

#define kTaskPropertiesMaskAll	( \
		MakeTaskPropertyMask(kTaskPropertyPriority) 			|	\
		MakeTaskPropertyMask(kTaskPropertyStackSize) 			|	\
		MakeTaskPropertyMask(kTaskPropertyTaskMainFcn) 			|	\
		MakeTaskPropertyMask(kTaskPropertyTaskMainArgCount) 	|	\
		MakeTaskPropertyMask(kTaskPropertyTaskMainArgValuesPtr) |	\
		MakeTaskPropertyMask(kTaskPropertyStartupMode) 			|	\
		MakeTaskPropertyMask(kTaskPropertySchedPolicy) 			|	\
		MakeTaskPropertyMask(kTaskPropertySchedInterval))
															
#define kTaskPropertiesMaskPriorityAndStackSize (					\
		MakeTaskPropertyMask(kTaskPropertyPriority) 			| 	\
		MakeTaskPropertyMask(kTaskPropertyStackSize))

//------------------------------------------------------------------------------
// tTaskProperties & tTaskPropertiesSet 
//
//	Used at task creation and when changing the properties of a running task.  
//	Callers can query the Kernel for preferred task properties using
//	CKernelMPI::GetTaskPropertiesPreferences(...) and the appropriate
//	tTaskOperatingMode.
//
//	tTaskProperties contains all the task properties values.
//	tTaskPropertiesSet defines a complete or subset group of these properties
//	that apply to a specific task running in a specific operating mode.
//
//                               1. 
//                               2. 
//                               3. 
//                               4. 
//                               5. 
//                               6. 
//                               7. 
//
//                               8. 
// The supported values of policy shall include SCHED_FIFO, SCHED_RR, and SCHED_OTHER,
// which are defined in the <sched.h> header. When threads executing with the scheduling
// policy SCHED_FIFO, SCHED_RR, or SCHED_SPORADIC are waiting on a mutex, they shall acquire
// the mutex in priority order when the mutex is unlocked. 
//
//                               9. 
//
//                               10. 
// The inheriSsched attribute determines how the other scheduling attributes
// of the created thread shall be set:
// 
// PTHREAD_INHERIT_SCHED  - Specifies that the thread scheduling attributes shall be
//                          inherited from the creating thread, and the scheduling
//                          attributes in this attr argument shall be ignored. 
// 
// PTHREAD_EXPLICIT_SCHED  - Specifies that the thread scheduling attributes shall be
//                           set to the corresponding values from this attributes object.
//
//
//
//--------------------------------------------------------------------------------



struct tTaskProperties {
	U32 				priority;	            // 1
	tAddr				stackAddr;				// 2
	U32 				stackSize;				// 3	
	tTaskMainFcn_posix	TaskMainFcn;            // 4
	U32					taskMainArgCount;       // 5
	tPtr				pTaskMainArgValues;		// 6						
	tTaskStartupMode	startupMode;	        // 7
	tTaskSchedPolicy	schedulingPolicy;		// 8
	U32					schedulingInterval;     // 9
    int                 inheritSched;           // 10
};

// FIXME: update to work w/ PID
struct tTaskPropertiesSet {
	S16					taskNameOffset;
	tTaskOperatingMode	operatingMode;
	tTaskPropertiesMask	definedProperties;	// mask of tTaskProperties values 
											// being set by this structure
	tTaskProperties		properties;
};

enum {
	kMemoryPoolAllocPolicyUndefined = 0,
	kMemoryPoolAllocPolicyFirstFit,
	kMemoryPoolAllocPolicyBestFit,
	kMemoryPoolAllocPolicyFixedSize,

	// default
	kMemoryPoolAllocPolicyDefault = kMemoryPoolAllocPolicyFirstFit,
};

typedef U16 tMemoryPoolAllocPolicy;


struct tMemoryPoolProperties {
	tAddr 					startAddr;			
	union {
		U32					totalSize;
		U32					totalCount;
	};
	U32	   					allocSize;
	tMemoryPoolAllocPolicy	allocPolicy;
	tTaskBlockingPolicy		blockingPolicy;	
};


enum {
	kMessageQueueAllocPolicyUndefined = 0,
	kMessageQueueAllocPolicyMessagePtr,
	kMessageQueueAllocPolicyVariableSize,
	kMessageQueueAllocPolicyFixedSize,

	// default
	kMessageQueueAllocPolicyDefault = kMessageQueueAllocPolicyMessagePtr,
};

typedef U16 tMessageQueueAllocPolicy;

struct tMessageQueueProperties {
	tAddr 						startAddr;			
	union {		
		U32	 					totalSize;
		U32	 					totalCount;
	};
	U32	   	 					allocSize;
	tMessageQueueAllocPolicy	allocPolicy;
	tTaskBlockingPolicy			blockingPolicy;	
    
};                                              

struct tMessageQueuePropertiesPosix{
   tTaskBlockingPolicy blockingPolicy;  // Only flags: 0 or NONBLOCK
                                        // 0 - A sending message sleeps
                                        // NONBLOCK - the mq_send function returns 
                                        //  immediately with an error status
    char*   nameQueue;
    mode_t  mode;
    long    oflag;
    U32     priority;
    long	mq_flags;	/* message queue flags			*/
    long	mq_maxmsg;	/* maximum number of messages		*/
    long	mq_msgsize;	/* maximum message size			*/
    long	mq_curmsgs;	/* number of messages currently queued	*/
};

enum{
    wrongQueuFlagParameter = 1001,
};


struct tTimerProperties {
	U32 dummy;
};


class CObject {
public:

/*
	virtual tObjTypeID		GetObjType() = 0;
	virtual const CUTI*		GetObjUTI() = 0;
//	e.g. DEFINE_OBJTYPE_SIGNATURE(CEventMessage, kEventMessageObj, "/LF/Brio/System/Message/EventMessage");
//	e.g. DEFINE_OBJTYPE_SIGNATURE(CEventMessage, kEventMessageObj, "/LF/Brio/System/Message/EventMessage");
*/
};

// class CMessage : CObject {
class CMessage 
{
public:
	U16		GetMessageSize() { return messageSize; }
	U8		GetMessagePriority() { return messagePriority; }
//	virtual ~CMessage(){};
#if 1 // BK
    U8      GetMessageReserved(){ return messageReserved;}    
#endif

	void	SetMessageSize(U16 size) { messageSize = size; }
  	void	SetMessagePriority(U8 priority) { messagePriority = priority; }	
                                                                               
#if 1 // BK
    void    SetMessageReserved( U8 reserved) {messageReserved = reserved; }
#endif

protected:
	U16	   	messageSize;
	U8	   	messagePriority;
	U8	   	messageReserved;
};

class CEventMessage : public CMessage 
{
//	virtual CEventMessage(){};
public:    

#if 1 // BK
   tEventType GetMessageEventType() {return eventType; }
   void       SetMessageEventType(tEventType value) {eventType = value; }
#endif

protected:
	tEventType		eventType;
};

/*
class CReturnReceiptEventMessage : public CEventMessage {
protected:
	IEventHandler*	returnEventHandler;

};


class CAddressedEventMessage :  public CEventMessage {
public:
	tRsrcHndl		GetSenderRsrcHndl() { return senderRsrcHndl; }
	tRsrcHndl		GetReceiverRsrcHndl() { return receiverRsrcHndl; }

protected:
	tRsrcHndl		senderRsrcHndl;
	tRsrcHndl		receiverRsrcHndl;
};
*/

// Mutex declarations
typedef pthread_mutex_t     tMutex;
typedef pthread_mutexattr_t tMutexAttr;

// Conditions declarations
typedef pthread_cond_t      tCond;
typedef pthread_condattr_t  tCondAttr;
typedef struct timespec     tTimeSpec;


#define ASSERT_POSIX_CALL(err) \
if(err) \
{ \
    int ret = ErrorBrio::lookupBrioErrType(err); \
        printf("***** POSIX function fails with error (%d). File (%s), Line (%d)\n", \
        err, __FILE__, __LINE__); \
        fflush(stdout); \
        return(ret); \
}
                                                                               
#define ASSERT_ERROR(expression,value) \
if(!expression) \
{ \
    int ret = ErrorBrio::lookupBrioErrType(value); \
    printf("***** Error (%d). File (%s), Line (%d)\n", \
        ret, __FILE__, __LINE__); \
        fflush(stdout); \
        return(ret); \
}

#endif // KERNELTYPES_H

// EOF
