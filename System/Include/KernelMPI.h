#ifndef KERNELMPI_H
#define KERNELMPI_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Kernel module. 
//
//==============================================================================

//#define PATH_BRIO '/mnt/hgfs/win_c/sw/Brio/System/Include'

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <CoreMPI.h>
#include <KernelTypes.h>
#include <StringTypes.h>

#include <mqueue.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

class CKernelMPI : public ICoreMPI
{
public:
	//==============================================================================
	// core functionality
	//==============================================================================
	
    virtual tErrType	Init();			   
	virtual tErrType	DeInit(); 			

	virtual Boolean		IsInited();
    
    virtual Boolean		IsValid() const;	

    virtual tErrType	GetMPIVersion(tVersion &version) const;		   
    virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		

    virtual tErrType	GetModuleVersion(tVersion &version) const;
    virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
    virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;
    
	// class-specific functionality
	CKernelMPI();
	virtual ~CKernelMPI();

	//==============================================================================
	// Tasks
	//==============================================================================

	//------------------------------------------------------------------------------
	// fills in tTaskProperties* with the latest values found in available
	// task properties rsrcs, and returns a mask of which properties were updated
	//------------------------------------------------------------------------------	
	/* FIXME/dg:
	tErrType	GetTaskPropertiesPreferences(const CURI* pTaskURI, 
					tTaskOperatingMode mode, tTaskProperties *pProperties,
					tTaskPropertiesMask *pUpdatedPropertiesMask);
	tErrType	GetTaskPropertiesPreferences(const CURI* pTaskURI, 
					tTaskOperatingMode mode, tTaskProperties *pProperties);
	*/

	//------------------------------------------------------------------------------
	// create/destroy/abort/reset a task using the properties in tTaskProperties
	//------------------------------------------------------------------------------
	tErrType	CreateTask(const CURI* pTaskURI, 
					const tTaskProperties* pTaskProperties, tTaskHndl *pHndl);
	tErrType	DestroyTask(tTaskHndl hndl);

	tErrType	    CancelTask(tTaskHndl hndl);
	tErrType	ResetTask(tTaskHndl hndl, const tTaskProperties* pTaskProperties);

	tTaskHndl	GetCurrentTask();

	U16			GetNumTasks();
	
    /* FIXME/dg: consider iterator scheme
	U16			GetTasks(tTaskHndl* pTasks, U16 maxNumTasks);
	*/

	//------------------------------------------------------------------------------
	// task scheduling
	//------------------------------------------------------------------------------ 
	/*
	tErrType	DisableCurrentTaskPreemption();
	tErrType	EnableCurrentTaskPreemption();

	tErrType	DisableTaskPreemption(tTaskHndl hndl);
	tErrType	EnableTaskPreemption(tTaskHndl hndl);
*/
	tErrType	YieldCurrentTask();
/*
	tErrType	BlockCurrentTask();

	tErrType	WaitForTime(U32 msTimeout, U32 usTimeout=0);	// wait for time in milliseconds (& microseconds)

	tErrType	BlockTask(tTaskHndl hndl);
	tErrType	UnblockTask(tTaskHndl hndl);

	//------------------------------------------------------------------------------
	// task properties for an existing task
	//------------------------------------------------------------------------------
	Boolean		TaskIsAlive(tTaskHndl hndl);
	Boolean		TaskIsActive(tTaskHndl hndl);
	Boolean		TaskIsBlocked(tTaskHndl hndl);
	Boolean		TaskIsAborted(tTaskHndl hndl);

	const CURI*			GetTaskURI(tTaskHndl hndl);
		
	tTaskPriority		GetTaskPriority(tTaskHndl hndl);
	tTaskSchedPolicy	GetTaskSchedulingPolicy(tTaskHndl hndl);
	*/
	tErrType GetTaskPriority(tTaskHndl hndl, int *priority);
	tErrType GetTaskSchedulingPolicy(tTaskHndl hndl, int *policy);
	/*
	U32					GetTaskSchedulingInterval(tTaskHndl hndl);

	tAddr		GetTaskStackStartAddr(tTaskHndl hndl);

	U32			GetTaskStackTotalSize(tTaskHndl hndl);
	U32			GetTaskStackFreeSize(tTaskHndl hndl);
	U32			GetTaskStackMinFreeSize(tTaskHndl hndl);

	tErrType	SetTaskPriority(tTaskHndl hndl, tTaskPriority priority);
	tErrType	SetTaskSchedulingPolicy(tTaskHndl hndl, tTaskSchedPolicy policy);
	tErrType	SetTaskSchedulingInterval(tTaskHndl hndl, U32 intervalTime);

	tErrType	CheckCurrentTaskStack(U8 minStackSizeFreePercent);
	*/

	//==============================================================================
	// Dynamic Memory Allocation
	//==============================================================================
	
	//------------------------------------------------------------------------------
	// System heap properties (default memory pool)
	//------------------------------------------------------------------------------
	U32			GetSystemHeapTotalSize();
	U32			GetSystemHeapFreeSize();

	const CURI*	GetSystemHeapURI();
	tAddr		GetSystemHeapStartAddr();

	tMemoryPoolAllocPolicy	GetSystemHeapAllocPolicy();
	U32			GetSystemHeapMinAllocSize();

	tTaskBlockingPolicy	GetSystemHeapTaskBlockingPolicy();
	U16			GetSystemHeapNumBlockedTasks();
	tTaskHndl	GetSystemHeapFirstBlockedTask();

	//------------------------------------------------------------------------------
	// allocating memory from the System heap (default memory pool)
	//------------------------------------------------------------------------------
    tErrType Malloc(U32 size, tPtr ppAllocatedMemory);

#ifdef BK_POSIX
tErrType	Malloc(U32 size, tPtr* ppAllocatedMemory);
#endif
	
    tErrType	MallocOrWait(U32 size, tPtr* ppAllocatedMemory, 
							U32 timeoutMs = kMaxTimeoutMs);
				
	tErrType 	MallocAligned(U32 size, U32 alignment,
							tPtr* ppAllocatedMemory);
	tErrType 	MallocAlignedOrWait(U32 size, U32 alignment, tPtr* ppAllocatedMemory, 
							U32 timeoutMs = kMaxTimeoutMs);

	//------------------------------------------------------------------------------
	// freeing System heap or memory pool memory
	//------------------------------------------------------------------------------
	void Free(tPtr pAllocatedMemory);

#ifdef BK_POSIX
	tErrType 	Free(tPtr pAllocatedMemory);
#endif
	//------------------------------------------------------------------------------
	// create/destroy memory pool
	//------------------------------------------------------------------------------
	tErrType 	CreateMemoryPool(const CURI* pPoolURI, 
			 			const tMemoryPoolProperties* pPoolProperties,
			 			tMemoryPoolHndl* pHndl);
	tErrType 	DestroyMemoryPool(tMemoryPoolHndl hndl);

	/* FIXME/dg: consider iterator scheme
	U16			GetNumMemoryPools();
	U16			GetMemoryPools(tMemoryPoolHndl* pMemoryPools, U16 maxNumPools);
	*/

	//------------------------------------------------------------------------------
	// memory pool properties
	//------------------------------------------------------------------------------
	U32			GetMemoryPoolTotalSize(tMemoryPoolHndl hndl);
	U32			GetMemoryPoolFreeSize(tMemoryPoolHndl hndl);

	const CURI*	GetMemoryPoolURI(tMemoryPoolHndl hndl);
	tAddr		GetMemoryPoolStartAddr(tMemoryPoolHndl hndl);

	tMemoryPoolAllocPolicy	GetMemoryPoolAllocPolicy(tMemoryPoolHndl hndl);
	U32			GetMemoryPoolMinAllocSize(tMemoryPoolHndl hndl);

	tTaskBlockingPolicy	GetMemoryPoolTaskBlockingPolicy(tMemoryPoolHndl hndl);
	U16			GetMemoryPoolNumBlockedTasks(tMemoryPoolHndl hndl);
	tTaskHndl	GetMemoryPoolFirstBlockedTask(tMemoryPoolHndl hndl);

	//------------------------------------------------------------------------------
	// allocating memory from a pool
	//------------------------------------------------------------------------------
	tErrType	MallocFromPool(tMemoryPoolHndl hndl, U32 size, 
						tPtr* ppAllocatedMemory);
	tErrType	MallocFromPoolOrWait(tMemoryPoolHndl hndl, U32 size, 
						tPtr* ppAllocatedMemory, U32 timeoutMs = kMaxTimeoutMs);

	tErrType	MallocAlignedFromPool(tMemoryPoolHndl hndl, U32 size, U32 alignment,
						tPtr* ppAllocatedMemory);
	tErrType	MallocAlignedFromPoolOrWait(tMemoryPoolHndl hndl, U32 size, 
						U32 alignment, tPtr* ppAllocatedMemory, 
						U32 timeoutMs = kMaxTimeoutMs);

	//------------------------------------------------------------------------------
	// allocating memory from a fixed-allocation-size pool
	//------------------------------------------------------------------------------
	tErrType	MallocFromFixedPool(tMemoryPoolHndl hndl, tPtr* ppAllocatedMemory);
	tErrType	MallocFromFixedPoolOrWait(tMemoryPoolHndl hndl, 
						tPtr* ppAllocatedMemory, U32 timeoutMs = kMaxTimeoutMs);

	tErrType	MallocAlignedFromFixedPool(tMemoryPoolHndl hndl, U32 alignment,
						tPtr* ppAllocatedMemory);
	tErrType	MallocAlignedFromFixedPoolOrWait(tMemoryPoolHndl hndl, U32 alignment,
						tPtr* ppAllocatedMemory, U32 timeoutMs = kMaxTimeoutMs);

	//==============================================================================
	// Message Queues
	//==============================================================================

	//------------------------------------------------------------------------------
	// create/destroy/clear message queues
	//------------------------------------------------------------------------------
	tErrType	CreateMessageQueue(const CURI* pQueueURI, 
						const tMessageQueuePropertiesPosix* pQueueProperties,
						tMessageQueueHndl* pHndl);
	tErrType	DestroyMessageQueue(tMessageQueueHndl hndl);
	
	tErrType	UnlinkMessageQueue(const char *name);

	tErrType  	ClearMessageQueue(tMessageQueueHndl hndl);

	/* FIXME/dg: consider iterator scheme
	U16			GetNumMessageQueues();
	U16			GetMessageQueues(tMessageQueueHndl* pMessageQueues, U16 maxNumQueues);
	*/

	//------------------------------------------------------------------------------
	// message queue properties
	//------------------------------------------------------------------------------
	/*
	U32			GetMessageQueueTotalSize(tMessageQueueHndl hndl);
	U32			GetMessageQueueFreeSize(tMessageQueueHndl hndl);
    */
	
    tErrType GetMessageQueueNumMessages(tMessageQueueHndl hndl, int *numMsgQueue);
    
    /*
	const CURI*	GetMessageQueueURI(tMessageQueueHndl hndl);
	tAddr	  	GetMessageQueueStartAddr(tMessageQueueHndl hndl);

	tMessageQueueAllocPolicy	GetMessageQueueAllocPolicy(tMessageQueueHndl hndl);
	U32			GetMessageQueueMaxAllocSize(tMessageQueueHndl hndl);

	tTaskBlockingPolicy	GetMessageQueueTaskBlockingPolicy(tMessageQueueHndl hndl);
	U16			GetMessageQueueNumBlockedTasks(tMessageQueueHndl hndl);
	tTaskHndl 	GetMessageQueueFirstBlockedTask(tMessageQueueHndl hndl);
	*/

	//------------------------------------------------------------------------------
	// send/peek at/receive messages
	//------------------------------------------------------------------------------
	tErrType	SendMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize);
	tErrType  	SendMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs);

	tErrType	SendPriorityMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									tMessagePriority priority,
									U32 messageSize = kUndefinedMessageSize);
	tErrType  	SendPriorityMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									tMessagePriority priority,
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs);

	tErrType	SendUrgentMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize);
	tErrType  	SendUrgentMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs);

	tErrType	SendBroadcastMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize);
	tErrType  	SendBroadcastMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize = kUndefinedMessageSize,
									U32 timeoutMs = kMaxTimeoutMs);

	tErrType  	ReceiveMessage(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, unsigned *msg_prio);
	tErrType  	ReceiveMessageOrWait(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, U32 timeoutMs = kMaxTimeoutMs);

	tErrType  	PeekAtMessage(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage);
	tErrType  	PeekAtMessageOrWait(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, U32 timeoutMs = kMaxTimeoutMs);
				
	//==============================================================================
	// Time & Timers
	//==============================================================================

	//------------------------------------------------------------------------------
	// system clock status
	//------------------------------------------------------------------------------
	U32			GetElapsedTime(U32* pUs=NULL);		// elapsed time since System startup 
													// in milliseconds (& microseconds)	

	//------------------------------------------------------------------------------
	// create/destroy timers
	//------------------------------------------------------------------------------
	/* Old declaration
    tErrType 	CreateTimer(const CURI* pTimerURI, 
						const tTimerProperties* pTimerProperties, tTimerHndl* pHndl);
	*/ 
   
    tErrType 	CreateTimer(const CURI* pTimerURI,
                          	struct sigevent *se,
                           	tTimerHndl* pHndl);
    
    tErrType 	DestroyTimer(tTimerHndl hndl);

	tErrType 	ResetTimer(tTimerHndl hndl, const tTimerProperties* pTimerProperties);

	/* FIXME/dg: consider iterator scheme
	U16			GetNumTimers();
	U16			GetTimers(tTimerHndl* pTimers, U16 maxNumTimers);
	*/

	//------------------------------------------------------------------------------
	// timer control & status
	//------------------------------------------------------------------------------
	tErrType	StartTimer(tTimerHndl hndl, const struct itimerspec *value);
	tErrType	StopTimer(tTimerHndl hndl);

	tErrType	GetTimerElapsedTime(tTimerHndl hndl, U32* pUs = NULL);		// elapsed time in milliseconds (& microseconds)
	tErrType	GetTimerRemainingTime(tTimerHndl hndl, U32* pUs = NULL); 	// time remaining in milliseconds (& microseconds)

	//------------------------------------------------------------------------------
	// Mutexes
    //------------------------------------------------------------------------------
	
    // Initializes a mutex with the attributes specified in the specified mutex attribute object
    tErrType InitMutex(tMutex* pMutex, tMutexAttr* pAttributes);
	
    // Destroys a mutex
    tErrType DeInitMutex(tMutex* pMutex);
	
    // Obtains the priority ceiling of a mutex attribute object
    tErrType GetMutexPriorityCeiling(tMutex* pMutex, S32* priority);
	
    // Sets the priority ceiling attribute of a mutex attribute object
    tErrType SetMutexPriorityCeiling(tMutex* pMutex, S32 priority, S32 *oldPriority);
	
    // FIXME/BSK Body function is empty
    tErrType GetMutexWaitCount(tMutex* pMutex);
	
    // Locks an unlocked mutex
    tErrType LockMutex(tMutex* pMutex);
	
    // Tries to lock a not tested
    tErrType TryLockMutex(tMutex* pMutex);
	
    // Unlocks a mutex
    tErrType UnlockMutex(tMutex* pMutex);
    
//  FIXME/BSK POSIX POSIX mutex list are not implemented (?):
// pthread_mutexattr_destroy Destroys a mutex attribute object 
// pthread_mutexattr_getprotocol Obtains protocol of a mutex attribute object 
// pthread_mutexattr_getpshared  Obtains a process-shared setting of a mutex attribute object 
// pthread_mutexattr_setprotocol Sets the protocol attribute of a mutex attribute object 
// pthread_mutexattr_setpshared  Sets the process-shared attribute of a mutex attribute
//                               object to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE 


	//------------------------------------------------------------------------------
	// Conditions
	//------------------------------------------------------------------------------
    // Unblocks all threads that are waiting on a condition variable
    tErrType CondBroadcast(tCond *cond);
    
    // Destroys a condition variable attribute object
    tErrType CondDestroy(tCond *cond);
    
    // Initializes a condition variable with the attributes specified in the
    // specified condition variable attribute object
    tErrType CondInit(tCond *cond, const tCondAttr *attr);
    
    // Unblocks at least one thread waiting on a condition variable
    tErrType CondSignal(tCond *cond);
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    tErrType CondTimedWait(tCond *cond, pthread_mutex_t *mutex,const tTimeSpec *abstime);
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    tErrType CondWait(tCond *cond, pthread_mutex_t *mutex);
    
    // Destroys a condition variable
    tErrType CondAttrDestroy(tCondAttr *attr);
    
    // Obtains the process-shared setting of a condition variable attribute object
    tErrType GetCondAttrPShared(const tCondAttr *attr, int *pshared);
    
    // Initializes a condition variable attribute object    
    tErrType CondAttrInit(tCondAttr *attr);
    
    // Sets the process-shared attribute in a condition variable attribute object
    // to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    tErrType SetCondAttrPShared(tCondAttr *attr, int pshared);
    
	
    //------------------------------------------------------------------------------
    // mailboxes
	// NU_Broadcast_To_Mailbox
	// NU_Create_Mailbox
	// NU_Delete_Mailbox
	// NU_Established_Mailboxes
	// NU_Mailbox_Information
	// NU_Mailbox_Pointers
	// NU_Receive_From_Mailbox
	// NU_Reset_Mailbox
	// NU_Send_To_Mailbox

	// pipes
	// NU_Broadcast_To_Pipe
	// NU_Create_Pipe
	// NU_Delete_Pipe
	// NU_Established_Pipes
	// NU_Pipe_Information
	// NU_Pipe_Pointers
	// NU_Receive_From_Pipe
	// NU_Reset_Pipe
	// NU_Send_To_Front_Of_Pipe
	// NU_Send_To_Pipe

	// semaphores
	// NU_Create_Semaphore
	// NU_Delete_Semaphore
	// NU_Established_Semaphores
	// NU_Obtain_Semaphore
	// NU_Release_Semaphore
	// NU_Reset_Semaphore
	// NU_Semaphore_Information
	// NU_Semaphore_Pointers

	// event groups
	// NU_Create_Event_Group
	// NU_Delete_Event_Group
	// NU_Established_Event_Groups
	// NU_Event_Group_Information
	// NU_Event_Group_Pointers
	// NU_Retrieve_Events
	// NU_Set_Events

	// signals
	// NU_Control_Signals
	// NU_Receive_Signals
	// NU_Register_Signal_Handler
	// NU_Send_Signals

	// interrupts
	// NU_Activiate_HISR
	// NU_Control_Interrupts
	// NU_Create_HISR
	// NU_Current_HISR_Pointer
	// NU_Delete_HISR
	// NU_Established_HISRs
	// NU_HISR_Information
	// NU_HISR_Pointers
	// NU_Local_Control_Interrupts
	// NU_Register_LISR
	// NU_Setup_Vector

	// diagnostics
	// NU_ASSERT
	// NU_CHECK
	// NU_Disable_History_Saving
	// NU_Enable_History_Saving
	// NU_License_Information
	// NU_Make_History_Entry
	// NU_Release_Information
	// NU_Retrieve_History_Entry

	// drivers
	// NU_Create_Driver
	// NU_Delete_Driver
	// NU_Driver_Pointers
	// NU_Established_Drivers
	// NU_Protect
	// NU_Request_Driver
	// NU_Resume_Driver
	// NU_Suspend_Driver
	// NU_Unprotect

private:
	class CKernelMPIImpl *mpImpl;
};


#endif // KERNELMPI_H

// EOF

