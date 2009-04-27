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

#include <DebugMPI.h>
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
		case EINVAL:			return  kInvalidParamErr;
		case EPERM:				return  kPermissionsErr;
		case ENOENT:			return 	kNoSuchFileOrDirectoryErr;
		case ESRCH: 			return 	kNoSuchProcessErr;
		case EINTR:				return 	kInterruptedSystemCallErr;
		case EIO:				return	kIOErr;
		case ENXIO:				return	kNoSuchDeviceOrAddressErr;
		case E2BIG:				return	kArgumentListTooLongErr;
		case ENOEXEC:			return	kExecformatErrorErr;
		case EBADF:				return	kBadFileNumberErr;
		case ECHILD:			return	kNoChildProcessesErr;
		case EAGAIN:			return	kTryAgainErr;
		case ENOMEM:			return	kOutOfMemoryErr;
		case EACCES:			return	kPermissionDeniedErr;
		case EFAULT:			return	kBadAddressErr;
		case ENOTBLK:			return	kBlockDeviceRequiredErr;
		case EBUSY:				return	kDeviceOrResourceBusyErr; 
		case EEXIST:			return	kFileExistsErr; 
		case EXDEV:				return	kCrossDeviceLinkErr;
		case ENODEV:			return	kNoSuchDeviceErr; 
		case ENOTDIR:			return	kNotDirectoryErr; 
		case EISDIR:			return	kIsDirectoryErr; 
//		case EINVAL:			return	kInvalidParamErr; 
		case ENFILE:			return	kFileTableOverflowErr; 
		case EMFILE:			return	kTooManyOpenFilesErr; 
		case ENOTTY:			return	kNotTypewriterErr; 
		case ETXTBSY:			return	kTextFileBusyErr; 
		case EFBIG:				return	kFileTooLargeErr; 
		case ENOSPC:			return	kNoSpaceLeftOnDevice; 
		case ESPIPE:			return	kIllegalSeekErr; 
		case EROFS:				return	kReadOnlyFileSystemErr; 
		case EMLINK:			return	kTooManyLinksErr; 
		case EPIPE:				return	kBrokenPipeErr; 
		case EDOM:				return	kMathArgumentOutOfDomainOfFuncErr;
		case ERANGE:			return	kMathResultNotRepresentableErr; 
		case EDEADLK:			return	kResourceDeadlockSouldOccurErr; 
		case ENAMETOOLONG:		return	kFileNameTooLongErr; 
		case ENOLCK:			return	kNoRecordLocksAvailableErr; 
		case ENOSYS:			return	kFunctionNotImplementedErr; 
		case ENOTEMPTY:			return	kDirectoryNotEmptyErr; 
		case ELOOP:				return	kTooManySymbolicLinksEncounteredErr; 
//		case EWOULDBLOCK:		return  kTryAgainErr; 
		case ENOMSG:			return	kNoMessageOfDesiredTypeErr; 
		case EIDRM:				return	kIdentifierRemovedErr; 
		case ECHRNG:			return	kChannelNumberOutOfRangeErr; 
		case EL2NSYNC:			return	kLevelTwoNotSynchronizedErr; 
		case EL3HLT:			return	kLevelThreeHaltedErr; 
		case EL3RST:			return	kLevelThreeResetErr; 
		case ELNRNG:			return	kLinkNumberOutOfRangeErr; 
		case EUNATCH:			return	kProtocolDriverNotAttachedErr; 
		case ENOCSI:			return	kNoCSIStructureAvailableErr; 
		case EL2HLT:			return	kLevelTwoHaltedErr; 
		case EBADE:				return	kInvalidExchangeErr; 
		case EBADR:				return	kInvalidRequestDescriptorErr; 
		case EXFULL:			return	kExchangeFullErr; 
		case ENOANO:			return	kNoAnodeErr; 
		case EBADRQC:			return	kInvalidRequestCodeErr; 
		case EBADSLT:			return	kInvalidSlotErr; 
//		case EDEADLOCK:			return  EDEADLK;
		case EBFONT:			return	kBadFontFileFormatErr; 
		case ENOSTR:			return	kDeviceNotAStreamErr; 
		case ENODATA:			return	kNoDataAvailableErr; 
		case ETIME:				return	kTimerExpiredErr; 
		case ENOSR:				return	kOutOfStreamsResourcesErr; 
		case ENONET:			return	kMachineIsNotOnTheNetworkErr; 
		case ENOPKG:			return	kPackageNotInstalledErr;
		case EREMOTE:			return	kObjectIsRemoteErr; 
		case ENOLINK:			return	kLinkHasBeenSeveredErr;
		case EADV:				return	kAdvertiseErr; 
		case ESRMNT:			return	kSrmountErr; 
		case ECOMM:				return	kCommunicationOnSendErr; 
		case EPROTO:			return	kProtocolErr; 
		case EMULTIHOP:			return	kMultihopAttemptedErr; 
		case EDOTDOT:			return	kRFSSpecificErr; 
		case EBADMSG:			return	kNotADdataMessageErr; 
		case EOVERFLOW:			return	kValueTooLargeForDefinedDataTypeErr; 
		case ENOTUNIQ:			return	kNameNotUniqueOnNetworkErr; 
		case EBADFD:			return	kFileDescriptorInBadStateErr; 
		case EREMCHG:			return	kRemoteAddressChangedErr; 
		case ELIBACC:			return	kCanNotAccessANeededSharedLibraryErr; 
		case ELIBBAD:			return	kAccessingACorruptedSharedLibraryErr; 
		case ELIBSCN:			return	kLibSectionInAOutCorruptedErr; 
		case ELIBMAX:			return	kAttemptingToLinkInTooManySharedLibrariesErr; 
		case ELIBEXEC:			return	kCannotExecASharedLibraryDirectlyErr; 
		case EILSEQ:			return	kIllegalByteSequenceErr; 
		case ERESTART:			return	kInterruptedSystemCallShouldBeRestartedErr; 
		case ESTRPIPE:			return	kStreamsPipeErr; 
		case EUSERS:			return	kTooManyUsersErr; 
		case ENOTSOCK:			return	kSocketOperationOnNonSocketErr; 
		case EDESTADDRREQ:		return	kDestinationAddressRequiredErr; 
		case EMSGSIZE:			return	kMessageTooLongErr; 
		case EPROTOTYPE:		return	kProtocolSrongTypeForSocketErr;
		case ENOPROTOOPT:		return	kProtocolNotAvailableErr; 
		case EPROTONOSUPPORT:	return	kProtocolNotSupportedErr; 
		case ESOCKTNOSUPPORT:	return	kSocketTypeNotSupportedErr; 
		case EOPNOTSUPP:		return	kOperationNotSupportedOnTransportEndpointErr; 
		case EPFNOSUPPORT:		return	kProtocolFamilyNotSupportedErr; 
		case EAFNOSUPPORT:		return	kAddressFamilyNotSupportedByProtocolErr; 
		case EADDRINUSE:		return	kAddressAlreadyInUseErr; 
		case EADDRNOTAVAIL:		return	kCannotAssignRequestedAddressErr; 
		case ENETDOWN:			return	kNetworkIsDownErr; 
		case ENETUNREACH:		return	kNetworkIsUnreachableErr; 
		case ENETRESET:			return	kNetworkDroppedConnectionBecauseOfResetErr; 
		case ECONNABORTED:		return	kSoftwareCausedConnectionAbortErr;
		case ECONNRESET:		return	kConnectionResetByPeerErr; 
		case ENOBUFS:			return	kNoBufferSpaceAvailableErr; 
		case EISCONN:			return	kTransportEndpointIsAlreadyConnectedErr; 
		case ENOTCONN:			return	kTransportEndpointIsNotConnectedErr; 
		case ESHUTDOWN:			return	kCannotSendAfterTransportEndpointShutdownErr; 
		case ETOOMANYREFS:		return	kTooManyReferencesCannotSpliceErr; 
		case ETIMEDOUT:			return	kConnectionTimedOutErr; 
		case ECONNREFUSED:		return	kConnectionRefusedErr; 
		case EHOSTDOWN:			return	kHostIsDownErr; 
		case EHOSTUNREACH:		return	kNoRouteToHostErr; 
		case EALREADY:			return	kOperationAlreadyInProgressErr;
		case EINPROGRESS:		return	kOperationNowInProgressErr; 
		case ESTALE:			return	kStaleNFSFileHandleErr; 
		case EUCLEAN:			return	kStructureNeedsCleaningErr; 
		case ENOTNAM:			return	kNotAXENIXNamedTypeFileErr; 
		case ENAVAIL:			return	kNoXENIXSemaphoresAvailableErr; 
		case EISNAM:			return	kIsANamedTypeFileErr; 
		case EREMOTEIO:			return	kRemoteIOErr; 
		case EDQUOT:			return	kQuotaExceededErr; 
		case ENOMEDIUM:			return	kNoMediumFoundErr; 
		case EMEDIUMTYPE:		return	kWrongMediumTypeErr; 
		default:				return  kUnspecifiedErr;
	}
}

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
	VTABLE_EXPORT U32	GetElapsedTimeAsMSecs(); 	// elapsed time since System startup 
													// in milliseconds	from Linux

	VTABLE_EXPORT U64	GetElapsedTimeAsUSecs();	// elapsed time since System startup 
													// in microseconds from Linux	

	VTABLE_EXPORT tErrType 	GetHRTAsUsec(U32 &uSec) const; 

	// Elapsed time since System startup in seconds
	VTABLE_EXPORT tErrType	GetElapsedAsSec(U32 &sec) const;	 

	// Elapsed time since System startup as structure
	VTABLE_EXPORT tErrType	GetElapsedTimeAsStructure(Current_Time &curTime) const;
	
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

//==============================================================================
// Watchdog Timer
//==============================================================================

	// Start Watchdog Timer
	VTABLE_EXPORT tErrType StartWatchdog( U32 seconds ) const;

	// Stop Watchdog Timer
	VTABLE_EXPORT tErrType StopWatchdog( void ) const;

	// KeepAlive request to Watchdog Timer
	VTABLE_EXPORT tErrType KeepWatchdogAlive( void ) const;
	
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
	LeapFrog::Brio::CDebugMPI mDebugMPI;
	CKernelModule();
	virtual ~CKernelModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_KERNELPRIVATE_H

// EOF

