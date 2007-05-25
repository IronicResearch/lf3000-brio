//==============================================================================
//
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
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

#include <errno.h>	// for posix calls not yet moved to module...
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <Module.h>
#include <KernelMPI.h>

#include <KernelPriv.h>
#include <StringTypes.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// MPI Name & Version
//
//		! NOTE: this may not match the MPI name & version implemented by the
//				installed module.  Always use the MPI's core functions to
//				resolve which version of the MPI is installed & available 
//				at runtime.
//==============================================================================
static const CString kMPIName = "KernelMPI";

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CKernelMPI implementation
//==============================================================================
CKernelMPI::CKernelMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kKernelModuleName, 
									kKernelModuleVersion);
	pModule_ = reinterpret_cast<CKernelModule*>(pModule);
}

//----------------------------------------------------------------------------
CKernelMPI::~CKernelMPI()
{
	Module::Disconnect(pModule_);
}

//============================================================================
// Informational functions
//============================================================================
Boolean CKernelMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CKernelMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CKernelMPI::GetModuleVersion() const
{
	if (!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CKernelMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CKernelMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

//==============================================================================
// Tasks
//==============================================================================
tErrType CKernelMPI::CreateTask(tTaskHndl& hndl, const tTaskProperties& props,
								const char* pDebugName)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CreateTask(hndl, props, pDebugName);
}

//----------------------------------------------------------------------------
tErrType CKernelMPI::JoinTask(tTaskHndl pHndl, tPtr& threadReturnValue) 
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->JoinTask(pHndl, threadReturnValue);
}
//----------------------------------------------------------------------------
tErrType CKernelMPI::CancelTask(tTaskHndl hndl)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CancelTask(hndl);
}

//----------------------------------------------------------------------------
// This service returns the currently active task pointer.
tTaskHndl CKernelMPI::GetCurrentTask() const
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetCurrentTask();
}

//----------------------------------------------------------------------------
int CKernelMPI::GetTaskPriority(tTaskHndl hndl) const
{
	if (!pModule_)
		return 0;
	return pModule_->GetTaskPriority(hndl);
}	

//----------------------------------------------------------------------------
int CKernelMPI::GetTaskSchedulingPolicy(tTaskHndl hndl) const
{
	if (!pModule_)
		return 0;	
	return pModule_->GetTaskSchedulingPolicy(hndl);
}	

//----------------------------------------------------------------------------
void CKernelMPI::TaskSleep(U32 msec) const
{
	if (pModule_)
		pModule_->TaskSleep(msec);
}	

//============================================================================
// allocating memory from the System heap
//============================================================================
tPtr CKernelMPI::Malloc(U32 size)
{
	if (!pModule_)
		return kNull;		
	return pModule_->Malloc(size);
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::Free(tPtr ptr)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->Free(ptr);
}

//============================================================================
// Message Queues
//============================================================================
tErrType CKernelMPI::OpenMessageQueue(tMessageQueueHndl& hndl,
								const tMessageQueuePropertiesPosix& properties,
								const char* pDebugName)
{
 	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->OpenMessageQueue(hndl, properties, pDebugName);
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::CloseMessageQueue(tMessageQueueHndl hndl,
								const tMessageQueuePropertiesPosix& properties)
{
 	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->CloseMessageQueue(hndl, properties);  
}

//------------------------------------------------------------------------------
#if 0 // FIXME//BSK 
tErrType CKernelMPI::UnlinkMessageQueue(const char *hndl)
{
 	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnlinkMessageQueue(hndl);  
}
#endif 
//------------------------------------------------------------------------------
tErrType CKernelMPI::ClearMessageQueue(tMessageQueueHndl hndl)
{
  	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ClearMessageQueue(hndl);  
}

//------------------------------------------------------------------------------
int CKernelMPI::GetMessageQueueNumMessages(tMessageQueueHndl hndl) const
{
  	if(!pModule_)
		return 0;
	return pModule_->GetMessageQueueNumMessages(hndl);  
}
    
//------------------------------------------------------------------------------
 tErrType CKernelMPI::SendMessage(tMessageQueueHndl hndl, const CMessage& msg) 
{
  	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SendMessage(hndl, msg);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SendMessageOrWait(tMessageQueueHndl hndl, 
										const CMessage& msg, U32 timeoutMs )
{
  	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SendMessageOrWait(hndl, msg, timeoutMs);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::ReceiveMessage( tMessageQueueHndl hndl, CMessage* msg_ptr, 
									U32 maxMessageSize )
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->ReceiveMessage( hndl, msg_ptr, maxMessageSize );  
}

tErrType CKernelMPI::ReceiveMessageOrWait( tMessageQueueHndl hndl, CMessage* msg_ptr, 
									 U32 maxMessageSize, U32 timeoutMs )
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->ReceiveMessageOrWait( hndl, msg_ptr, maxMessageSize, timeoutMs );  
}
				
//==============================================================================
// Time & Timers
//==============================================================================

U32 CKernelMPI::GetElapsedTime( U32* pUs ) const
{
  	if(!pModule_)
		return kMPINotConnectedErr;
		
	return pModule_->GetElapsedTime(pUs);  
}

//------------------------------------------------------------------------------
//tErrType CKernelMPI::CreateTimer(tTimerHndl& hndl, 
// 						const tTimerProperties& props, const char* pDebugName )

tTimerHndl 	CKernelMPI::CreateTimer( pfnTimerCallback callback, const tTimerProperties& props,
								const char* pDebugName )
{
  	if(!pModule_)
		return kInvalidTimerHndl;
	return pModule_->CreateTimer(callback, props, pDebugName);  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyTimer( tTimerHndl hndl )
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->DestroyTimer(hndl);  
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::ResetTimer( tTimerHndl hndl, const tTimerProperties& props )
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ResetTimer(hndl, props);
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::StartTimer(tTimerHndl hndl, const tTimerProperties& props)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->StartTimer(hndl, props);
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::StopTimer(tTimerHndl hndl)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->StopTimer(hndl);  
}
//------------------------------------------------------------------------------
tErrType CKernelMPI::PauseTimer(tTimerHndl hndl, saveTimerSettings& saveValue)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->PauseTimer(hndl, saveValue );
}
	
//------------------------------------------------------------------------------
tErrType CKernelMPI::ResumeTimer(tTimerHndl hndl, saveTimerSettings& saveValue)
{
  	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->ResumeTimer(hndl, saveValue);  
}
	
//------------------------------------------------------------------------------
// elapsed time in milliseconds
U32 CKernelMPI::GetTimerElapsedTime(tTimerHndl hndl, U32* pUs) const
                                                                    
{
  	if (!pModule_)
		return 0;
	return pModule_->GetTimerElapsedTime(hndl, pUs );  
}
	
//------------------------------------------------------------------------------
// time remaining in milliseconds
U32 CKernelMPI::GetTimerRemainingTime(tTimerHndl hndl, U32* pUs) const
{
  	if (!pModule_)
		return 0;
	return pModule_->GetTimerRemainingTime(hndl, pUs );  
}

// fixme rdg: move the rest of these into the kernel module at some point
//------------------------------------------------------------------------------
// Mutexes
//------------------------------------------------------------------------------
tErrType CKernelMPI::InitMutexAttributeObject( tMutexAttr& mutexAttr )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitMutexAttributeObject( mutexAttr );  

}	


tErrType CKernelMPI::InitMutex( tMutex& mutex, const tMutexAttr* attributes )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitMutex( mutex, attributes );  
}
//------------------------------------------------------------------------------
tErrType CKernelMPI::DeInitMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->DeInitMutex( mutex );  
}

//------------------------------------------------------------------------------
S32 CKernelMPI::GetMutexPriorityCeiling(const tMutex& mutex) const
{
  	if (!pModule_)
		return 0;
	return pModule_->GetMutexPriorityCeiling( mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority )
{
  	if (!pModule_)
		return 0;
	return pModule_->SetMutexPriorityCeiling( mutex, prioCeiling, pOldPriority );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::LockMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->LockMutex( mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TryLockMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->TryLockMutex( mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::UnlockMutex( tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->UnlockMutex( mutex );  
}

//------------------------------------------------------------------------------
// Conditions 
//------------------------------------------------------------------------------

tErrType CKernelMPI::BroadcastCond( tCond& cond )
{
  	if (!pModule_)
		return 0;
	return pModule_->BroadcastCond( cond );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyCond( tCond& cond )
{
  	if (!pModule_)
		return 0;
	return pModule_->DestroyCond( cond );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::InitCond( tCond& cond, const tCondAttr& attr )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitCond( cond, attr );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SignalCond( tCond& cond )
{
  	if (!pModule_)
		return 0;
	return pModule_->SignalCond( cond );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime )
{
  	if (!pModule_)
		return 0;
	return pModule_->TimedWaitOnCond( cond, mutex, pAbstime );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::WaitOnCond( tCond& cond, tMutex& mutex )
{
  	if (!pModule_)
		return 0;
	return pModule_->WaitOnCond( cond, mutex );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::DestroyCondAttr( tCondAttr& attr )
{
  	if (!pModule_)
		return 0;
	return pModule_->DestroyCondAttr( attr );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::GetCondAttrPShared( const tCondAttr& attr, int* pShared )
{
  	if (!pModule_)
		return 0;
	return pModule_->GetCondAttrPShared( attr, pShared );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::InitCondAttr( tCondAttr& attr )
{
  	if (!pModule_)
		return 0;
	return pModule_->InitCondAttr( attr );  
}

//------------------------------------------------------------------------------
tErrType CKernelMPI::SetCondAttrPShared( tCondAttr* pAttr, int shared )

{
  	if (!pModule_)
		return 0;
	return pModule_->SetCondAttrPShared( pAttr, shared );  
}

// EOF
