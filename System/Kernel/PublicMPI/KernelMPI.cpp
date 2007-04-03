
//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio Kernel module.
//
//==============================================================================
#include "pthread.h"
#include "sched.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"
#include "errno.h"
#include <unistd.h>
#include <sys/types.h>

#include <StringTypes.h>
#include <SystemErrors.h>

#include <CoreMPI.h>
#include <KernelMPI.h>
#include <Module.h>
#include <KernelPrivate.h>

#if 0 // FIXME/BSK
#include <RsrcMgrMPI.h>
#include <ModuleRsrc.h>
#endif

#include <ErrorBrio.h>
#include <KernelPrivate.h>

const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMPIName = "EventMPI";



// FIXME/dg: hack to get printf

extern "C" void NU_printf(char* fmt, ...);


#define kSystemHeapSize	(0x80000)
#define kRTOSTickMs		10
// BK static NU_MEMORY_POOL systemHeapPool;
static U8 systemHeap[kSystemHeapSize]; 		// FIXME/dg: dummy system heap	

// BK typedef STATUS tNUStatus;

#if 0 // BSK
static tErrType ToErrType(tNUStatus nuStatus)
{
#if 0 // FIXME / BSK
	switch (nuStatus)
	{
		case NU_INVALID_TASK:
		case NU_INVALID_ENTRY:
		case NU_INVALID_MEMORY:
		case NU_INVALID_SIZE:
		case NU_INVALID_PREEMPT:
		case NU_INVALID_START:
		case NU_INVALID_QUEUE:
			return kInvalidParamErr;

		case NU_SUCCESS:
			return kNoErr;

		default:
			return kUnspecifiedErr;
	}
#endif // 
}
#endif


inline U32 ToKernelTime(U32 ticks)
{
#if 0 // FIXME / BSK
	return (ticks * kRTOSTickMs);
#endif	
}


inline U32 ToRTOSTime(U32 mS)
{
#if 0 // FIXME / BSK
	U32 roundUp = (((mS % kRTOSTickMs) != 0) ? 1 : 0);
	 
	// FIXME/dg: verify
	return Max(((mS / kRTOSTickMs) + roundUp), 1);
#endif	
}

inline U32 ToRTOSMessageSize(U32 bytes)
{
#if 0 // FIXME / BSK	
	return (bytes / sizeof(U32));
#endif	
}

inline U32 ToKernelMessageSize(U32 rtosMessageSize)
{
#if 0 // FIXME / BSK	
	return (rtosMessageSize * sizeof(U32));
#endif	
}


static U32 ToRTOSTimeout(U32 timeout) 
{
#if 0 // FIXME / BSK	
	if (timeout == 0)
		return NU_NO_SUSPEND;
	else if (timeout == kMaxTimeoutMs)
		return NU_SUSPEND;
	else
		return ToRTOSTime(timeout);
#endif		
}

CKernelMPI::CKernelMPI() : mpModule(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kKernelModuleName, 
									kKernelModuleVersion);
	mpModule = reinterpret_cast<CKernelModule*>(pModule);
}

CKernelMPI::~CKernelMPI()
{
	Module::Disconnect(mpModule);
}

#if 0 // FIXME / BSK
tErrType CKernelMPI::Init()
{
	CModuleRsrc 		* pRsrc;
	CRsrcMgrMPI 		* pRsrcMgrMPI;
	tRsrcHndl			  hRsrc;
	tErrType 			  err = kNoErr;
	CURI					uri;

/*
	uri = (const char*)kKernelModuleURI;

	if ((pRsrcMgrMPI = new CRsrcMgrMPI) == NULL)
		return kAllocMPIErr;

	pRsrcMgrMPI->Init();

	if((err = pRsrcMgrMPI->FindRsrc(&uri, &hRsrc)) != kNoErr)
		goto ReturnErr;	
	if((err = pRsrcMgrMPI->GetRsrcPtr(hRsrc, (tPtr*)&pRsrc)) != kNoErr)
		goto ReturnErr;									   

	if ((mpImpl = new CKernelMPIImpl) == NULL)
	{
		err = kAllocMPIErr;
		goto ReturnErr;
	}

	// FIXME/dg: enable when ready
// 	pRsrc->GetMPIFcnTable(kKernelMPIFcnTable, (tPtr **)&(mpImpl->mpMPIFcnTable));

	// bump reference count of the rsrc - FIXME: change to smartptr referencing
	pRsrcMgrMPI->AddRsrcRef(hRsrc);



*/
	// FIXME/dg: allocate system heap
	err = ToErrType(NU_Create_Memory_Pool(&systemHeapPool, "<SystemHeap>", ToPtr(systemHeap),
						kSystemHeapSize, 8, NU_FIFO));
	if (err != kNoErr)
		goto ReturnErr;

	return kNoErr;

ReturnErr:
/*	// cleanup
	if (pRsrcMgrMPI != NULL)
	{
		pRsrcMgrMPI->DeInit();
		delete pRsrcMgrMPI;
	}
*/
 	return err;

}
#endif // FIXME/BSK


#if 0 // FIXME/BSK
tErrType CKernelMPI::DeInit()
{
	CRsrcMgrMPI 		* pRsrcMgrMPI;
	tRsrcHndl			  hRsrc;
	tErrType 			  err;
	CURI	   			uri;

/*
	uri = (const char*)kKernelModuleURI;

	if ((pRsrcMgrMPI = new CRsrcMgrMPI) == kNull)
		return kAllocMPIErr;


	pRsrcMgrMPI->Init();
	if((err = pRsrcMgrMPI->FindRsrc(&uri, &hRsrc)) != kNoErr)
		return err;
			
	// bump reference count of the rsrc - FIXME: change to smartptr referencing
	pRsrcMgrMPI->DeleteRsrcRef(hRsrc);

	pRsrcMgrMPI->DeInit();

	delete pRsrcMgrMPI;

	if ( mpImpl != kNull )
	{
		delete mpImpl;
	}
*/
	return kNoErr;
}
#endif // FIXME FIXME / BSK


Boolean CKernelMPI::IsValid() const
{
    return TRUE;
}

#if 0 // FIXME/BSK
Boolean	CKernelMPI::IsInited() 
{
	if ( mpImpl != kNull )
	{
		return true;
	}

	return false;
}
#endif //

tErrType CKernelMPI::GetMPIVersion(tVersion &pVersion) const
{
    pVersion = kMPIVersion;
	return kNoErr;
}
tErrType	CKernelMPI::GetMPIName(ConstPtrCString &pName) const
{
	return kNoImplErr;
}

tErrType CKernelMPI::GetModuleVersion(tVersion &Version) const
{
	Version = kKernelModuleVersion;
	return kNoErr;
}

// FIXME/FIXME / BSK
tErrType	CKernelMPI::GetModuleName(ConstPtrCString &pName) const
{
	return kNoImplErr;
}


// tErrType CKernelMPI::GetModuleOrigin(const CURI **ppURI) // FIXME / BSK old
tErrType CKernelMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	return kNoImplErr;
}

//==============================================================================
// Tasks
//==============================================================================

//------------------------------------------------------------------------------
// fills in tTaskProperties* with the latest values found in available
// task properties rsrcs, and returns a mask of which properties were updated
//------------------------------------------------------------------------------	

/*
tErrType	CKernelMPI::GetTaskPropertiesPreferences(const CURI* pTaskURI, 
					tTaskOperatingMode mode, tTaskProperties *pProperties,
					tTaskPropertiesMask *pUpdatedPropertiesMask)
{
}

tErrType	CKernelMPI::GetTaskPropertiesPreferences(const CURI* pTaskURI, 
					tTaskOperatingMode mode, tTaskProperties *pProperties)
{
}
*/

//------------------------------------------------------------------------------
// create/destroy/abort/reset a task using the properties in tTaskProperties
//------------------------------------------------------------------------------
tErrType	CKernelMPI::CreateTask(const CURI* pTaskURI, 
					const tTaskProperties* pProperties, tTaskHndl *pHndl)
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

#if 0 // FIXME / BSK

tErrType	CKernelMPI::DestroyTask(tTaskHndl hndl)
{
	CKernelTask* pKernelType = CKernelTask::ToKernelType(hndl);
	tNUStatus status;
	tErrType err;
//  This service deletes a previously created application task. The parameter task identifies
//  the task to delete. Note that the specified task must be either in a finished or terminated
//  state prior to calling this service. Additionally, the application must prevent the use of this
//  task during and after deletion
       
    status = NU_Delete_Task(pKernelType->ToRTOSType());
	err = ToErrType(status);

	// FIXME/dg: under what error conditions should we still delete this?
	if (err == kNoErr)        
		delete pKernelType;

ReturnErr:
	return err;
}
#endif


tErrType CKernelMPI::CancelTask(tTaskHndl hndl)
{
// This service terminates the task specified by the task parameter.
// NOTE 1: A terminated task cannot execute again until it is reset.
// NOTE 2: When calling this function from a signal handler, the task
//         whose signal handler is executing cannot be terminated.
	return pthread_cancel(hndl);;
}

#if 0 // FIXME / BSK
tErrType	CKernelMPI::ResetTask(tTaskHndl hndl, const tTaskProperties* pProperties)
{
	// FIXME/dg
	return kNoImplErr;
}
#endif


// This service returns the currently active task pointer.
tTaskHndl	CKernelMPI::GetCurrentTask()
{
/* Old code	
    return CKernelTask::ToOpaqueType(NU_Current_Task_Pointer());
*/
    return pthread_self();
}

#if 0 // I could not find this POSIX function
	U16			CKernelMPI::GetNumTasks()
    {
        return pthread_is_multithreaded_np();
    }
#endif
	
    /* FIXME/dg: consider iterator scheme
	U16			CKernelMPI::GetNumTasks()
	U16			CKernelMPI::GetTasks(tTaskHndl* pTasks, U16 maxNumTasks)
	*/

	//------------------------------------------------------------------------------
	// task scheduling
	//------------------------------------------------------------------------------ 
/*

tErrType	CKernelMPI::DisableCurrentTaskPreemption()
{
	// FIXME/dg
	return kNoImplErr;
}

tErrType	CKernelMPI::EnableCurrentTaskPreemption()
{
	// FIXME/dg
	return kNoImplErr;
}

tErrType	CKernelMPI::DisableTaskPreemption(tTaskHndl hndl)
{
	// FIXME/dg
	return kNoImplErr;
}

tErrType	CKernelMPI::EnableTaskPreemption(tTaskHndl hndl)
{
	// FIXME/dg
	return kNoImplErr;
}
*/
tErrType	CKernelMPI::YieldCurrentTask()
{
	// FIXME/dg
	return kNoImplErr;
}
/*
tErrType	CKernelMPI::BlockCurrentTask()
{
	// FIXME/dg
	return kNoImplErr;
}

tErrType	CKernelMPI::WaitForTime(U32 msTimeout, U32 usTimeout=0)	// wait for time in milliseconds (& microseconds)
{
	// FIXME/dg
	return kNoImplErr;
}

tErrType	CKernelMPI::BlockTask(tTaskHndl hndl)
{
	// FIXME/dg
	return kNoImplErr;
}

tErrType	CKernelMPI::UnblockTask(tTaskHndl hndl)
{
	// FIXME/dg
	return kNoImplErr;
}

//------------------------------------------------------------------------------
// task properties for an existing task
//------------------------------------------------------------------------------
Boolean		CKernelMPI::TaskIsAlive(tTaskHndl hndl)
Boolean		CKernelMPI::TaskIsActive(tTaskHndl hndl)
Boolean		CKernelMPI::TaskIsBlocked(tTaskHndl hndl)
Boolean		CKernelMPI::TaskIsAborted(tTaskHndl hndl)

const CURI*	CKernelMPI::GetTaskURI(tTaskHndl hndl)

tTaskPriority		CKernelMPI::GetTaskPriority(tTaskHndl hndl)
tTaskSchedPolicy	CKernelMPI::GetTaskSchedulingPolicy(tTaskHndl hndl)
*/
tErrType GetTaskPriority(tTaskHndl hndl, int * priority)
{
	int err;
	struct sched_param param;
	int policy;	
	
    err = pthread_getschedparam(hndl, &policy, &param);
	ASSERT_POSIX_CALL(err);

	*priority = param.sched_priority; 			
	
    return kNoErr;
}	

tErrType GetTaskSchedulingPolicy(tTaskHndl hndl, int * policy)
{
	int err;
	struct sched_param param;
    
    err = pthread_getschedparam(hndl, policy, &param);
	ASSERT_POSIX_CALL(err);
    
	return kNoErr;
}	


/*
U32			CKernelMPI::GetTaskSchedulingInterval(tTaskHndl hndl)

tAddr		CKernelMPI::GetTaskStackStartAddr(tTaskHndl hndl)

U32		CKernelMPI::GetTaskStackTotalSize(tTaskHndl hndl)
U32		CKernelMPI::GetTaskStackFreeSize(tTaskHndl hndl)
U32		CKernelMPI::GetTaskStackMinFreeSize(tTaskHndl hndl)

tErrType	CKernelMPI::SetTaskPriority(tTaskHndl hndl, tTaskPriority priority)
tErrType	CKernelMPI::SetTaskSchedulingPolicy(tTaskHndl hndl, tTaskSchedPolicy policy)
tErrType	CKernelMPI::SetTaskSchedulingInterval(tTaskHndl hndl, U32 intervalTime)

tErrType	CKernelMPI::CheckCurrentTaskStack(U8 minStackSizeFreePercent)
*/

	//==============================================================================
	// Dynamic Memory Allocation
	//==============================================================================
	
	//------------------------------------------------------------------------------
	// System heap properties (default memory pool)
	//------------------------------------------------------------------------------
/*
U32			CKernelMPI::GetSystemHeapTotalSize()
{
}

U32			CKernelMPI::GetSystemHeapFreeSize()
{
}

const CURI*	CKernelMPI::GetSystemHeapURI()
{
}

tAddr			CKernelMPI::GetSystemHeapStartAddr()
{
}

tMemoryPoolAllocPolicy	CKernelMPI::GetSystemHeapAllocPolicy()
{
}

U32			CKernelMPI::GetSystemHeapMinAllocSize()
{
}

tTaskBlockingPolicy	CKernelMPI::GetSystemHeapTaskBlockingPolicy()
{
}

U16			CKernelMPI::GetSystemHeapNumBlockedTasks()
{
}

tTaskHndl		CKernelMPI::GetSystemHeapFirstBlockedTask()
{
}
*/

//------------------------------------------------------------------------------
// allocating memory from the System heap (default memory pool)
//------------------------------------------------------------------------------
tErrType CKernelMPI::Malloc(U32 size, tPtr pPtr)
{

	pPtr = (tPtr )malloc( size );
    ASSERT_ERROR(pPtr != 0, kCouldNotAllocateMemory);

	return kNoErr;
}

#if 0 // FIXME / BSK

tErrType	CKernelMPI::MallocOrWait(U32 size, tPtr* pPtr, U32 timeoutMs)
{
	return ToErrType(NU_Allocate_Memory(&systemHeapPool, pPtr, size, ToRTOSTimeout(timeoutMs)));
}
#endif
			
/*
tErrType 	CKernelMPI::MallocAligned(U32 size, U32 alignment,
							tPtr* ppAllocatedMemory)
tErrType 	CKernelMPI::MallocAlignedOrWait(U32 size, U32 alignment, 
							tPtr* ppAllocatedMemory, U32 timeoutMs)
*/

//------------------------------------------------------------------------------
// freeing System heap or memory pool memory
//------------------------------------------------------------------------------
void CKernelMPI::Free(tPtr ptr)
{
	free(ptr);
}

//------------------------------------------------------------------------------
// create/destroy memory pool
//------------------------------------------------------------------------------
#if 0 // FIXME / BSK

tErrType 	CKernelMPI::CreateMemoryPool(const CURI* pPoolURI, 
			 			const tMemoryPoolProperties* pProperties,
			 			tMemoryPoolHndl* pHndl)
{

	CKernelMemoryPool		*pMemoryPool = NULL;
	tDataPtr				pPool;
	U32						totalSize, blockingPolicy;
	tErrType				err = kNoErr;

	// convert rest of params to Nucleus version
	pPool 				= ToDataPtr(pProperties->startAddr);
	totalSize 			= pProperties->totalSize;
	blockingPolicy 		= (pProperties->blockingPolicy == kTaskBlockingPolicyPriority) ? 
								NU_PRIORITY : NU_FIFO;

	// create Kernel's version of RTOS struct
	if ((pMemoryPool = new CKernelMemoryPool(totalSize, &pPool)) == NULL)
	{
		err = kMemoryAllocErr;
		goto ReturnErr;
	}

	// create the pool
	// FIXME/dg: need real name & minAlloc size
	err = ToErrType(NU_Create_Memory_Pool(pMemoryPool->ToRTOSType(), "<PoolName>", ToPtr(pPool),
						totalSize, pProperties->allocSize, blockingPolicy));
	if (err != kNoErr)
		goto ReturnErr;

	// good to go
	*pHndl = pMemoryPool->ToOpaqueType();
	return kNoErr;

ReturnErr:
	// cleanup
	if (pMemoryPool != NULL)
		delete pMemoryPool;

	*pHndl = kUndefinedMemoryPoolHndl;
	return err;

}
#endif

#if 0 // FIXME / BSK
tErrType 	CKernelMPI::DestroyMemoryPool(tMemoryPoolHndl hndl)
{
	CKernelMemoryPool* pKernelType	= CKernelMemoryPool::ToKernelType(hndl);
	tNUStatus status;
	tErrType err;

	status = NU_Delete_Memory_Pool(pKernelType->ToRTOSType());
	err = ToErrType(status);

	// FIXME/dg: under what error conditions should we still delete this?
	if (err == kNoErr)
		delete pKernelType;

ReturnErr:
	return err;
}
#endif

/*
*/
	/* FIXME/dg: consider iterator scheme
	U16			CKernelMPI::GetNumMemoryPools()
	U16			CKernelMPI::GetMemoryPools(tMemoryPoolHndl* pMemoryPools, U16 maxNumPools)
	*/

//------------------------------------------------------------------------------
// memory pool properties
//------------------------------------------------------------------------------
/*
U32	CKernelMPI::GetMemoryPoolTotalSize(tMemoryPoolHndl hndl)
{
}

U32			CKernelMPI::GetMemoryPoolFreeSize(tMemoryPoolHndl hndl)

const CURI*	CKernelMPI::GetMemoryPoolURI(tMemoryPoolHndl hndl)
tAddr			CKernelMPI::GetMemoryPoolStartAddr(tMemoryPoolHndl hndl)

tMemoryPoolAllocPolicy	CKernelMPI::GetMemoryPoolAllocPolicy(tMemoryPoolHndl hndl)
U32			CKernelMPI::GetMemoryPoolMinAllocSize(tMemoryPoolHndl hndl)

tTaskBlockingPolicy	GetMemoryPoolTaskBlockingPolicy(tMemoryPoolHndl hndl)
U16			CKernelMPI::GetMemoryPoolNumBlockedTasks(tMemoryPoolHndl hndl)
tTaskHndl		CKernelMPI::GetMemoryPoolFirstBlockedTask(tMemoryPoolHndl hndl)
*/
	//------------------------------------------------------------------------------
	// allocating memory from a pool
	//------------------------------------------------------------------------------
/*
tErrType	CKernelMPI::MallocFromPool(tMemoryPoolHndl hndl, U32 size, 
						tPtr* ppAllocatedMemory)
tErrType	CKernelMPI::MallocFromPoolOrWait(tMemoryPoolHndl hndl, U32 size, 
						tPtr* ppAllocatedMemory, U32 timeoutMs = kMaxTimeoutMs)

tErrType	CKernelMPI::MallocAlignedFromPool(tMemoryPoolHndl hndl, U32 size, U32 alignment,
						tPtr* ppAllocatedMemory)
tErrType	CKernelMPI::MallocAlignedFromPoolOrWait(tMemoryPoolHndl hndl, U32 size, 
						U32 alignment, tPtr* ppAllocatedMemory, 
						U32 timeoutMs = kMaxTimeoutMs)

	//------------------------------------------------------------------------------
	// allocating memory from a fixed-allocation-size pool
	//------------------------------------------------------------------------------
tErrType	CKernelMPI::MallocFromFixedPool(tMemoryPoolHndl hndl, tPtr* ppAllocatedMemory)
tErrType	CKernelMPI::MallocFromFixedPoolOrWait(tMemoryPoolHndl hndl, 
						tPtr* ppAllocatedMemory, U32 timeoutMs = kMaxTimeoutMs)

tErrType	CKernelMPI::MallocAlignedFromFixedPool(tMemoryPoolHndl hndl, U32 alignment,
						tPtr* ppAllocatedMemory)
tErrType	CKernelMPI::MallocAlignedFromFixedPoolOrWait(tMemoryPoolHndl hndl, U32 alignment,
						tPtr* ppAllocatedMemory, U32 timeoutMs = kMaxTimeoutMs)
*/

	//==============================================================================
	// Message Queues
	//==============================================================================

	//------------------------------------------------------------------------------
	// create/destroy/clear message queues
	//------------------------------------------------------------------------------
#if 0 // FIXME / BSK
tErrType	CKernelMPI::CreateMessageQueue(const CURI* pQueueURI, 
						const tMessageQueueProperties* pProperties,
						tMessageQueueHndl* pHndl)
{

	CKernelMessageQueue		*pKernelType= NULL;
	tDataPtr				pAllocBuffer;
	U32						totalSize, rtosBlockingPolicy, rtosAllocPolicy;
	tErrType				err = kNoErr;

	// convert rest of params to Nucleus version
	pAllocBuffer		= ToDataPtr(pProperties->startAddr);
	totalSize 			= pProperties->totalSize;

	// FIXME/dg: add support for 	kMessageQueueAllocPolicyMessagePtr
	rtosAllocPolicy		= (pProperties->allocPolicy == kMessageQueueAllocPolicyVariableSize) ? 
								NU_VARIABLE_SIZE : NU_FIXED_SIZE;
	rtosBlockingPolicy	= (pProperties->blockingPolicy == kTaskBlockingPolicyPriority) ? 
								NU_PRIORITY : NU_FIFO;

	// create Kernel's version of RTOS struct
	if ((pKernelType = new CKernelMessageQueue(totalSize, &pAllocBuffer)) == NULL)
	{
		err = kMemoryAllocErr;
		goto ReturnErr;
	}

	// FIXME/dg: need real name
	err = ToErrType(NU_Create_Queue(pKernelType->ToRTOSType(), "<QueueName>", 
						ToPtr(pAllocBuffer), ToRTOSMessageSize(totalSize), rtosAllocPolicy,
						ToRTOSMessageSize(pProperties->allocSize), rtosBlockingPolicy));
	if (err != kNoErr)
		goto ReturnErr;

	// good to go
	*pHndl = pKernelType->ToOpaqueType();
	return kNoErr;

ReturnErr:
	// cleanup
	delete pKernelType;

	*pHndl = kUndefinedMemoryPoolHndl;
	return err;
}
#endif

/*---------------------------------------------------------------------------
 * 		POSIX version
 * -------------------------------------------------------------------------*/
tErrType	CKernelMPI::CreateMessageQueue(const CURI* pQueueURI, 
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
            return ErrorBrio::lookupBrioErrType(wrongQueuFlagParameter); 
        }
          
    queuAttr.mq_flags  = 0;
    queuAttr.mq_maxmsg =  pProperties->mq_maxmsg;
    queuAttr.mq_msgsize = pProperties->mq_msgsize;
    queuAttr.mq_curmsgs = pProperties->mq_curmsgs;
    
#if 1 // FIXME / BSK Debug Printing
    printf("THe Input MUEUE parameters are :\n"); 
    printf("name=%s oflag=%d mode=%d mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsgs=%d\n",
    	pProperties->nameQueue, pProperties->oflag,pProperties->mode,
    	queuAttr.mq_flags, pProperties->mq_maxmsg, pProperties->mq_msgsize, pProperties->mq_curmsgs);   
    fflush(stdout);
#endif // FIXME / BSK
    
    errno = 0;
    
    retMq_open = mq_open (pProperties->nameQueue,
                          pProperties->oflag,
                          pProperties->mode,
                          &queuAttr);
    
	ASSERT_POSIX_CALL(err);
    *pHndl = retMq_open;
    
#if 1 // FIXME / BSK Debug Printing
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

tErrType	CKernelMPI::DestroyMessageQueue(tMessageQueueHndl hndl)
{
    errno = 0;

    mq_close(hndl);
    ASSERT_POSIX_CALL(errno);
    
    return kNoErr;
}

tErrType UnlinkMessageQueue(const char *hndl)
{
    errno = 0;
    
    mq_unlink(hndl);
    ASSERT_POSIX_CALL(errno);
    
    return kNoErr;
}


tErrType  	CKernelMPI::ClearMessageQueue(tMessageQueueHndl hndl)
{
    int ret;
//-------------------------------------------------
    struct mq_attr attr;
    ssize_t nr;
    char *buf;
    
    /* Determine the # of messages currently in queue and
    max. msg size; allocate buffer to receive msg */
    
    errno = 0;
    
    mq_getattr(hndl, &attr);
   	ASSERT_POSIX_CALL(errno);

    buf = (char *)malloc(attr.mq_msgsize);
    
    for(int k = 0; k < attr.mq_curmsgs; k++) 
    {
        errno = 0; 

        nr = mq_receive(hndl, buf, attr.mq_msgsize, NULL);
        ASSERT_POSIX_CALL(errno);
    }
    
    free(buf);
    return kNoErr;
}
//-------------------------------------------------


	/* FIXME/dg: consider iterator scheme
	U16			CKernelMPI::GetNumMessageQueues()
	U16			CKernelMPI::GetMessageQueues(tMessageQueueHndl* pMessageQueues, U16 maxNumQueues)
	*/

	//------------------------------------------------------------------------------
	// message queue properties
	//------------------------------------------------------------------------------
	/*
	U32			CKernelMPI::GetMessageQueueTotalSize(tMessageQueueHndl hndl)
	U32			CKernelMPI::GetMessageQueueFreeSize(tMessageQueueHndl hndl)
    */
	
    tErrType CKernelMPI::GetMessageQueueNumMessages(tMessageQueueHndl hndl, int *numMsgQueue)
    {
        struct mq_attr attr = {0};
        mqd_t ret = -1;
        
        errno = 0;
        ret = mq_getattr(hndl, &attr);
        ASSERT_POSIX_CALL(errno);
        
        *numMsgQueue = attr.mq_curmsgs;
        return kNoErr;
    }
    
    /*
	const CURI*	CKernelMPI::GetMessageQueueURI(tMessageQueueHndl hndl)
	tAddr	  	CKernelMPI::GetMessageQueueStartAddr(tMessageQueueHndl hndl)

	tMessageQueueAllocPolicy	CKernelMPI::GetMessageQueueAllocPolicy(tMessageQueueHndl hndl)
	U32			CKernelMPI::GetMessageQueueMaxAllocSize(tMessageQueueHndl hndl)

	tTaskBlockingPolicy	CKernelMPI::GetMessageQueueTaskBlockingPolicy(tMessageQueueHndl hndl)
	U16			CKernelMPI::GetMessageQueueNumBlockedTasks(tMessageQueueHndl hndl)
	tTaskHndl 	CKernelMPI::GetMessageQueueFirstBlockedTask(tMessageQueueHndl hndl)
	*/

	//------------------------------------------------------------------------------
	// send/peek at/receive messages
	//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------
 * 		POSIX version
 * -------------------------------------------------------------------------
*/
 
 tErrType	CKernelMPI::SendMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize)
{
    
#if 0 // FIXME / BSK
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
    
    ASSERT_POSIX_CALL(errno);
    
#if 1 // FIXME / BSK Debugging printing 
    struct mq_attr attr = {0};
    mq_getattr(hndl, &attr);
    printf("\n-----After sending message QUEQUE attributes are: ----\n");
    printf("mq_flags=%d mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
            attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
    fflush(stdout);
#endif // BK

    return kNoErr;
}

tErrType  	CKernelMPI::SendMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize, U32 timeoutMs)
{
    struct timespec tp;

    ASSERT_ERROR(timeoutMs<999,kInvalidFunctionArgument);  

    errno =0;
    
    clock_gettime(CLOCK_REALTIME, &tp);
   	ASSERT_POSIX_CALL(errno);

    tp.tv_sec = tp.tv_sec + timeoutMs / 1000; 
    tp.tv_nsec = tp.tv_nsec + timeoutMs * 1000000; // convert from ms to nanosecond

#if 0 // FIXME / BSK
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
    
   	ASSERT_POSIX_CALL(errno);
    
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

#if 0 // FIXME / BSK
tErrType	CKernelMPI::SendPriorityMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									tMessagePriority priority, U32 messageSize)
{

	// FIXME/dg: doesn't support where messageSize not specified; not dealing w/ priority yet
	tNUStatus status = NU_Send_To_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(messageSize), NU_NO_SUSPEND); 
	return ToErrType(status);
}
#endif

#if 0 // FIXME / BSK
tErrType  	CKernelMPI::SendPriorityMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									tMessagePriority priority,
									U32 messageSize, U32 timeoutMs)
{

	// FIXME/dg: doesn't support where messageSize not specified; not dealing w/ priority yet
	tNUStatus status = NU_Send_To_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(messageSize), ToRTOSTimeout(timeoutMs)); 
	return ToErrType(status);
}
#endif

#if 0 // FIXME / BSK

tErrType	CKernelMPI::SendUrgentMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize)
{

	// FIXME/dg: doesn't support where messageSize not specified
	tNUStatus status = NU_Send_To_Front_Of_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(messageSize), NU_NO_SUSPEND); 
	return ToErrType(status);
}
#endif

#if 0 // FIXME / BSK
tErrType  	CKernelMPI::SendUrgentMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize, U32 timeoutMs)
{
	// FIXME/dg: doesn't support where messageSize not specified
	tNUStatus status = NU_Send_To_Front_Of_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(messageSize), ToRTOSTimeout(timeoutMs)); 

	return ToErrType(status);
}

#endif


tErrType	CKernelMPI::SendBroadcastMessage(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize)
{
}

tErrType  	CKernelMPI::SendBroadcastMessageOrWait(tMessageQueueHndl hndl, CMessage* pMessage, 
									U32 messageSize, U32 timeoutMs)
{
}

/*---------------------------------------------------------------------------
* 		POSIX version
* -------------------------------------------------------------------------*/
tErrType  	CKernelMPI::ReceiveMessage(tMessageQueueHndl hndl, U32 maxMessageSize, 
									   CMessage* msg_ptr, unsigned *msg_prio)
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
   
#if 1 // FIXME / BSK Debug printing
   U8 tmp = msg_ptr->GetMessagePriority();
   mq_getattr(hndl, &attr);
   printf("\n-----CKernelMPI::ReceiveMessage ----\n");
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
         return ErrorBrio::lookupBrioErrType(errno);
     }
#if 1 // FIXME / BSK Debug printing
    printf("CKernelMPI::ReceiveMessage. Received=%d bytes\n", ret_receive);
    fflush(stdout);
#endif // BK
     return kNoErr;

#if 0 // BK always. It is Nucleus
    U32 dummyParam;

	// FIXME/dg: doesn't support where messageSize not specified
	tNUStatus status = NU_Receive_From_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(maxMessageSize), &dummyParam, NU_NO_SUSPEND); 
	return ToErrType(status);
#endif
}

tErrType  	CKernelMPI::ReceiveMessageOrWait(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* msg_ptr, U32 timeoutMs)
{
// Retrieve attributes of the messges queque
   struct mq_attr attr = {0};
   
#if 1 // FIXME / BSK Debug printing
   mq_getattr(hndl, &attr);
   printf("\n-----CKernelMPI::ReceiveMessage ----\n");
   printf("mq_flags=%d  mq_maxmsg=%d mq_msgsize=%d mq_curmsq=%d\n",
          attr.mq_flags,attr.mq_maxmsg,attr.mq_msgsize,attr.mq_curmsgs);
#endif // FIXME / BSK
   struct timespec tp;

   ASSERT_ERROR(timeoutMs<999,kInvalidFunctionArgument);  

   errno =0;
   if(clock_gettime(CLOCK_REALTIME, &tp) == -1)
   ASSERT_POSIX_CALL(errno);

   tp.tv_sec = tp.tv_sec + timeoutMs / 1000; 
   tp.tv_nsec = tp.tv_nsec + timeoutMs * 1000000; // convert from ms to nanosecond

   unsigned msg_prio = msg_ptr->GetMessagePriority();	
   errno = 0;
   int ret_receive = mq_timedreceive(hndl,
                                  (char *)msg_ptr,
                                  (size_t )sizeof(CEventMessage),
                                  &msg_prio,
                                  (const struct timespec *)&tp);
   ASSERT_POSIX_CALL(errno);

#if 1 // FIXME / BSK Debug printing
    printf("CKernelMPI::ReceiveMessage. Received=%d bytes\n", ret_receive);
    fflush(stdout);
#endif // FIXME / BSK
     return kNoErr;

#if 0 // FIXME / BSK always. It is Nucleus
    U32 dummyParam;

	// FIXME/dg: doesn't support where messageSize not specified
	tNUStatus status = NU_Receive_From_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(maxMessageSize), &dummyParam, NU_NO_SUSPEND); 
	return ToErrType(status);
#endif

#if 0 // FIXME / BSK always. It is Nucleus
	U32 dummyParam;

	// FIXME/dg: doesn't support where messageSize not specified
	tNUStatus status = NU_Receive_From_Queue(CKernelMessageQueue::ToRTOSType(hndl), ToPtr(pMessage),
									ToRTOSMessageSize(maxMessageSize), &dummyParam, ToRTOSTimeout(timeoutMs)); 

	return ToErrType(status);
#endif
}

/*
tErrType  	CKernelMPI::PeekAtMessage(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage)
{
}

tErrType  	CKernelMPI::PeekAtMessageOrWait(tMessageQueueHndl hndl, U32 maxMessageSize, 
									CMessage* pMessage, U32 timeoutMs)
{
}

*/
				
//==============================================================================
// Time & Timers
//==============================================================================

//------------------------------------------------------------------------------
// system clock status
//------------------------------------------------------------------------------
#if 0 // FIXME / BSK
U32   		CKernelMPI::GetElapsedTime(U32* pUs)
{
	// FIXME/dg: don't support microseconds currently
	if (pUs != NULL)
		*pUs = 0;

	return 	ToKernelTime(NU_Retrieve_Clock());
}
#endif

//------------------------------------------------------------------------------
// create/destroy timers
//------------------------------------------------------------------------------
tErrType 	CKernelMPI::CreateTimer(const CURI* pTimerURI,
                                    struct sigevent *se,
                                     tTimerHndl* pHndl)
{
//	tErrType err = timer_create(CLOCK_REALTIME, NULL, (timer_t *)pHndl);
//	tErrType err = timer_create(CLOCK_MONOTONIC, NULL, (timer_t *)pHndl);
//	tErrType err = timer_create(CLOCK_THREAD_CPUTIME_ID, NULL, (timer_t *)pHndl);
	
    errno = 0;
    timer_create(CLOCK_REALTIME, se, (timer_t *)pHndl);
    
	ASSERT_POSIX_CALL(errno);
    
    return kNoErr; 
}

tErrType 	CKernelMPI::DestroyTimer(tTimerHndl hndl)
{
	
    errno = 0;
    timer_delete((timer_t)hndl );
    
	ASSERT_POSIX_CALL(errno);
    
    return kNoErr; 
}
	
tErrType 	CKernelMPI::ResetTimer(tTimerHndl hndl, const tTimerProperties* pTimerProperties)
{
	struct itimerspec value;
	
    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = (long )pTimerProperties->dummy;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	
    errno = 0;
    timer_settime((timer_t )hndl, 0, &value, NULL);
    
	ASSERT_POSIX_CALL(errno);
    
    return kNoErr; 
}
	
/* FIXME/dg: consider iterator scheme
U16			CKernelMPI::GetNumTimers()
U16			CKernelMPI::GetTimers(tTimerHndl* pTimers, U16 maxNumTimers)
*/

//------------------------------------------------------------------------------
// timer control & status
//------------------------------------------------------------------------------
tErrType	CKernelMPI::StartTimer(tTimerHndl hndl, const struct itimerspec *value)
{

    errno = 0;
    timer_settime((timer_t )hndl, 0, value, NULL);
    
	ASSERT_POSIX_CALL(errno);
    return kNoErr; 
}
	
tErrType	CKernelMPI::StopTimer(tTimerHndl hndl)
{
	struct itimerspec value;
	
    value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	
    errno = 0;
    timer_settime((timer_t)hndl, 0, &value, NULL);	
	
    ASSERT_POSIX_CALL(errno);

    return kNoErr;

}
	
tErrType CKernelMPI::GetTimerElapsedTime(tTimerHndl hndl, U32* pUs) // elapsed time in milliseconds
                                                                    // (& microseconds)
{
	struct itimerspec value;
 //	printf("hndl=%d    12hndl=%d\n", hndl, (timer_t )hndl);
//	fflush(stdout);
	
    errno = 0;
    int err = timer_gettime((timer_t )hndl, &value);
	
	ASSERT_POSIX_CALL(errno);
	
    *pUs = (value.it_interval.tv_sec -  value.it_value.tv_sec) * 1000 + 
           (value.it_interval.tv_nsec - value.it_value.tv_nsec) / 1000000;

#if 1 // FIXME / BSK  Debug printing 
    printf("CKernelMPI::GetTimerElapsedTime:\n value.it_interval.tv_sec=%ld\n value.it_value.tv_sec=%ld\n \
            value.it_interval.tv_nsec=%ld\n value.it_value.tv_nsec=%ld\n",
      value.it_interval.tv_sec, value.it_value.tv_sec, value.it_interval.tv_nsec,value.it_value.tv_nsec);       
#endif	

    return kNoErr;
}
	
tErrType CKernelMPI::GetTimerRemainingTime(tTimerHndl hndl, U32* pUs) // time remaining in milliseconds (& microseconds)
{
	struct itimerspec value;
	
    errno = 0;
    timer_gettime((timer_t )hndl, &value);
	
    ASSERT_POSIX_CALL(errno);
	
    *pUs = value.it_value.tv_sec * 1000 + value.it_value.tv_nsec / 1000000;
	
    return kNoErr;	
}
// EOF
	//------------------------------------------------------------------------------
	// Mutexes
	//------------------------------------------------------------------------------
	tErrType CKernelMPI::InitMutex(tMutex* pMutex, tMutexAttr* pAttributes)
    {
        errno = 0;
        pthread_mutex_init(pMutex, pAttributes);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
	
    tErrType CKernelMPI::DeInitMutex(tMutex* pMutex)
    {
        errno = 0;
        pthread_mutex_destroy(pMutex);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
	
    tErrType CKernelMPI::GetMutexPriorityCeiling(tMutex* pMutex, S32 *prioceiling)
    {
       errno = 0;
       
       pthread_mutex_getprioceiling(pMutex, (int *)prioceiling);
       ASSERT_POSIX_CALL(errno);
       
       return kNoErr;
    }
	
    tErrType CKernelMPI::SetMutexPriorityCeiling(tMutex* pMutex, S32 prioceiling, S32 *oldPriority)
    {
       errno = 0;

       pthread_mutex_setprioceiling(pMutex, (int )prioceiling, (int *)oldPriority);
       ASSERT_POSIX_CALL(errno);
        
       return kNoErr;
    }
	
    tErrType CKernelMPI::GetMutexWaitCount(tMutex* pMutex)
    {
        return kNoErr;
    }
	
    tErrType CKernelMPI::LockMutex(tMutex* pMutex)
    {
        errno = 0;
        
        pthread_mutex_lock(pMutex);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
	
    tErrType CKernelMPI::TryLockMutex(tMutex* pMutex)
    {
        errno = 0;
        
        pthread_mutex_trylock(pMutex);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
	
    tErrType CKernelMPI::UnlockMutex(tMutex* pMutex)
	{
        errno = 0;
        
        pthread_mutex_unlock(pMutex);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    
	//------------------------------------------------------------------------------
	// Conditions 
	//------------------------------------------------------------------------------
    
    tErrType CKernelMPI::CondBroadcast(tCond *cond)
    {
        errno = 0;

        pthread_cond_broadcast(cond);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondDestroy(tCond *cond)
    {
        errno = 0;

        pthread_cond_destroy(cond);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondInit(tCond *cond, const tCondAttr *attr)
    {
        errno = 0;

        pthread_cond_init(cond, attr);;
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondSignal(tCond *cond)
    {
        errno = 0;

        pthread_cond_signal(cond);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondTimedWait(tCond *cond, pthread_mutex_t *mutex,const tTimeSpec *abstime)
    {
        errno = 0;

        pthread_cond_timedwait(cond, mutex, abstime);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondWait(tCond *cond, pthread_mutex_t *mutex)
    {
        errno = 0;

        pthread_cond_wait(cond, mutex);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondAttrDestroy(tCondAttr *attr)
    {
        errno = 0;

        pthread_condattr_destroy(attr);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::GetCondAttrPShared(const tCondAttr *attr, int *pshared)
    {
        errno = 0;

        pthread_condattr_getpshared(attr, pshared);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::CondAttrInit(tCondAttr *attr)
    {
        errno = 0;

        pthread_condattr_init(attr);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }
    tErrType CKernelMPI::SetCondAttrPShared(tCondAttr *attr, int pshared)
    
    {
        errno = 0;

        pthread_condattr_setpshared(attr, pshared);
        ASSERT_POSIX_CALL(errno);
        
        return kNoErr;
    }

