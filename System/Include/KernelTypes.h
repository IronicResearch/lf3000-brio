#ifndef LF_BRIO_KERNELTYPES_H
#define LF_BRIO_KERNELTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		KernelTypes.h
//
// Description:
//		System Kernel's basic types.
//
//==============================================================================
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================	   
// Kernel errors
//==============================================================================
#define KERNEL_ERRORS											\
			(kMemoryAllocErr) 									\
			(kNoSuchFileOrDirectoryErr)							\
			(kNoSuchProcessErr)									\
			(kInterruptedSystemCallErr)							\
			(kIOErr)											\
			(kNoSuchDeviceOrAddressErr)							\
			(kArgumentListTooLongErr)							\
			(kExecformatErrorErr)								\
			(kBadFileNumberErr)									\
			(kNoChildProcessesErr)								\
			(kTryAgainErr)										\
			(kOutOfMemoryErr)									\
			(kPermissionDeniedErr)								\
			(kBadAddressErr)									\
			(kBlockDeviceRequiredErr)							\
			(kDeviceOrResourceBusyErr)							\
			(kFileExistsErr)									\
			(kCrossDeviceLinkErr)								\
			(kNoSuchDeviceErr)									\
			(kNotDirectoryErr)									\
			(kIsDirectoryErr)									\
			(kFileTableOverflowErr)								\
			(kTooManyOpenFilesErr)								\
			(kNotTypewriterErr)									\
			(kTextFileBusyErr)									\
			(kFileTooLargeErr)									\
			(kNoSpaceLeftOnDevice)								\
			(kIllegalSeekErr)									\
			(kReadOnlyFileSystemErr)							\
			(kTooManyLinksErr)									\
			(kBrokenPipeErr)									\
			(kMathArgumentOutOfDomainOfFuncErr)					\
			(kMathResultNotRepresentableErr)					\
			(kResourceDeadlockSouldOccurErr)					\
			(kFileNameTooLongErr)								\
			(kNoRecordLocksAvailableErr)						\
			(kFunctionNotImplementedErr)						\
			(kDirectoryNotEmptyErr)								\
			(kTooManySymbolicLinksEncounteredErr)				\
			(kNoMessageOfDesiredTypeErr)						\
			(kIdentifierRemovedErr)								\
			(kChannelNumberOutOfRangeErr)						\
			(kLevelTwoNotSynchronizedErr)						\
			(kLevelThreeHaltedErr)								\
			(kLevelThreeResetErr)								\
			(kLinkNumberOutOfRangeErr)							\
			(kProtocolDriverNotAttachedErr)						\
			(kNoCSIStructureAvailableErr)						\
			(kLevelTwoHaltedErr)								\
			(kInvalidExchangeErr)								\
			(kInvalidRequestDescriptorErr)						\
			(kExchangeFullErr)									\
			(kNoAnodeErr)										\
			(kInvalidRequestCodeErr)							\
			(kInvalidSlotErr)									\
			(kResourceDeadlockWouldOccur)						\
			(kBadFontFileFormatErr)								\
			(kDeviceNotAStreamErr)								\
			(kNoDataAvailableErr)								\
			(kTimerExpiredErr)									\
			(kOutOfStreamsResourcesErr)							\
			(kMachineIsNotOnTheNetworkErr)						\
			(kPackageNotInstalledErr)							\
			(kObjectIsRemoteErr)								\
			(kLinkHasBeenSeveredErr)							\
			(kAdvertiseErr)										\
			(kSrmountErr)										\
			(kCommunicationOnSendErr)							\
			(kProtocolErr)										\
			(kMultihopAttemptedErr)								\
			(kRFSSpecificErr)									\
			(kNotADdataMessageErr)								\
			(kValueTooLargeForDefinedDataTypeErr)				\
			(kNameNotUniqueOnNetworkErr)						\
			(kFileDescriptorInBadStateErr)						\
			(kRemoteAddressChangedErr)							\
			(kCanNotAccessANeededSharedLibraryErr)				\
			(kAccessingACorruptedSharedLibraryErr)				\
			(kLibSectionInAOutCorruptedErr)						\
			(kAttemptingToLinkInTooManySharedLibrariesErr)		\
			(kCannotExecASharedLibraryDirectlyErr)				\
			(kIllegalByteSequenceErr)							\
			(kInterruptedSystemCallShouldBeRestartedErr)		\
			(kStreamsPipeErr)									\
			(kTooManyUsersErr)									\
			(kSocketOperationOnNonSocketErr)					\
			(kDestinationAddressRequiredErr)					\
			(kMessageTooLongErr)								\
			(kProtocolSrongTypeForSocketErr)					\
			(kProtocolNotAvailableErr)							\
			(kProtocolNotSupportedErr)							\
			(kSocketTypeNotSupportedErr)						\
			(kOperationNotSupportedOnTransportEndpointErr)		\
			(kProtocolFamilyNotSupportedErr)					\
			(kAddressFamilyNotSupportedByProtocolErr) 			\
			(kAddressAlreadyInUseErr)							\
			(kCannotAssignRequestedAddressErr)					\
			(kNetworkIsDownErr)									\
			(kNetworkIsUnreachableErr)							\
			(kNetworkDroppedConnectionBecauseOfResetErr)		\
			(kSoftwareCausedConnectionAbortErr)					\
			(kConnectionResetByPeerErr)							\
			(kNoBufferSpaceAvailableErr)						\
			(kTransportEndpointIsAlreadyConnectedErr)			\
			(kTransportEndpointIsNotConnectedErr)				\
			(kCannotSendAfterTransportEndpointShutdownErr)		\
			(kTooManyReferencesCannotSpliceErr)					\
			(kConnectionTimedOutErr)							\
			(kConnectionRefusedErr)								\
			(kHostIsDownErr)									\
			(kNoRouteToHostErr)									\
			(kOperationAlreadyInProgressErr)					\
			(kOperationNowInProgressErr)						\
			(kStaleNFSFileHandleErr)							\
			(kStructureNeedsCleaningErr)						\
			(kNotAXENIXNamedTypeFileErr)						\
			(kNoXENIXSemaphoresAvailableErr)					\
			(kIsANamedTypeFileErr)								\
			(kRemoteIOErr)										\
			(kQuotaExceededErr)									\
			(kNoMediumFoundErr)									\
			(kWrongMediumTypeErr)


	BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupKernel), KERNEL_ERRORS)

//==============================================================================	   
// Kernel types
//==============================================================================
typedef tHndl	tTaskHndl;
typedef tHndl	tMemoryPoolHndl;
typedef tHndl	tMessageQueueHndl;
// FIXME // BSK changed declaration
typedef tHndl	tTimerHndl;

typedef U8		tTaskPriority;
typedef U8		tMessagePriority;

typedef void 	(*tTaskMainFcn)(U32 argCount, tPtr pArgValues); // Old

typedef void 	*(*tTaskMainFcn_posix)(tPtr pArgValues);

const U32				kMaxTimeoutMs			 = kU32Max;
const tTaskHndl			kInvalidTaskHndl		 = static_cast<tTaskHndl>(0);
const tMemoryPoolHndl	kInvalidMemoryPoolHndl	 = static_cast<tMemoryPoolHndl>(0);
const tMessageQueueHndl	kInvalidMessageQueueHndl = static_cast<tMessageQueueHndl>(0);
const tTimerHndl		kInvalidTimerHndl		 = static_cast<tTimerHndl>(0);
const tTaskPriority		kInvalidTaskPriorityl	 = static_cast<tTaskPriority>(0);
const tMessagePriority	kInvalidMessagePriority	 = static_cast<tMessagePriority>(0);

#define kUndefinedMessageSize		((U16)0)
#define kUndefinedMemoryPoolHndl 	((tMemoryPoolHndl)kInvalidHndl)
#define kUndefinedTaskHndl 		((tTaskHndl)kInvalidHndl)

//------------------------------------------------------------------------------
// Application exit codes
//------------------------------------------------------------------------------
const int	kKernelExitSuccess 	= 0;	// success 
const int	kKernelExitError 	= 1;	// general error
const int	kKernelExitShutdown	= 0;	// Brio shutdown condition
const int	kKernelExitReset 	= 3;	// Brio reset condition
const int	kKernelExitAssert 	= 4;	// Brio assert condition

//------------------------------------------------------------------------------
// Task scheduling & blocking
//------------------------------------------------------------------------------


enum {
	kTaskSchedPolicyUndefined 	= 0
//	kTaskSchedPolicyFIFO = SCHED_FIFO,			
//	kTaskSchedPolicyRR = SCHED_RR,			
//	kTaskSchedPolicyOTHER = SCHED_OTHER,			
//    kTaskSchedPolicyTimeSlice,
//	kTaskSchedPolicyNoPreemption,
	// default
//	kTaskSchedPolicyDefault	= kTaskSchedPolicyFIFO,
};

typedef U16	tTaskSchedPolicy;


enum {
	kTaskBlockingPolicyUndefined = 0,
	kTaskBlockingPolicyFIFO,
	kTaskBlockingPolicyPriority,
	// default
	kTaskBlockingPolicyDefault = kTaskBlockingPolicyFIFO
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
	kTaskStartupModeDefault	= kTaskStartupModeRun
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
	kNumTaskProperties		= (kLastTaskProperty - kFirstTaskProperty + 1)
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
	S32 				priority;	            // 1
	tAddr				stackAddr;				// 2
	U32 				stackSize;				// 3	
	tTaskMainFcn_posix	TaskMainFcn;            // 4
	U32					taskMainArgCount;       // 5
	tPtr				pTaskMainArgValues;		// 6						
	tTaskStartupMode	startupMode;	        // 7
	tTaskSchedPolicy	schedulingPolicy;		// 8
	U32					schedulingInterval;     // 9
    int                 inheritSched;           // 10
    // Default ctor that initializes all values to defaults
    tTaskProperties() : priority(0), stackAddr(0), stackSize(PTHREAD_STACK_MIN),
    				TaskMainFcn(NULL), taskMainArgCount(0), 
    				pTaskMainArgValues(NULL), startupMode(0),
    				schedulingPolicy(SCHED_OTHER), schedulingInterval(0),
    				inheritSched(0) {}
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
	kMemoryPoolAllocPolicyDefault = kMemoryPoolAllocPolicyFirstFit
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
	kMessageQueueAllocPolicyDefault = kMessageQueueAllocPolicyMessagePtr
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
    wrongQueueFlagParameter = 1001
};

// 
enum{
    TIMER_RELATIVE_SET = 0,
	TIMER_ABSTIME_SET = 1
};    

typedef struct timer_arg
{	
	void (*pfn)(tTimerHndl arg);
	tTimerHndl argFunc;
}callbackData; 

struct tTimerProperties {
	
	int type;			// Absolute or Relative Timer:
						// If the TIMER_ABSTIME_SET flag is set, the timer is set with a specified 
						// starting time (the timer is absolute timer).
						// If the the TIMER_ABSTIME_SET flag is not set, the timer is set relative 
						// to the current time (the timer is a relative timer)

	struct itimerspec timeout;  /* 
									The struct id defined in the the /usr/include/time.h file
								    struct itimerspec { 
  									    struct timespec it_interval;
    									struct timespec it_value;
  									};
  										Where 
  										typedef struct timespec {
  											time_t tv_sec;	
  											long   tv_nsec;  
  								        };
  								*/		
  };

typedef struct current_time
{
        int sec;
        int min;
        int hour;
        int mday;
        int mon;
        int year;
        int wday;
        int yday;
        int isdst;
} Current_Time;

typedef struct itimerspec saveTimerSettings; 

class CObject {
public:

};

// class CMessage : CObject {
class CMessage 
{
public:
	virtual	~CMessage()	{}
	
	U16		GetMessageSize() const { return messageSize; }
	U8		GetMessagePriority() const { return messagePriority; }
//	virtual ~CMessage(){};
    U8      GetMessageReserved(){ return messageReserved;}    
	void	SetMessageSize(U16 size) { messageSize = size; }
  	void	SetMessagePriority(U8 priority) { messagePriority = priority; }	
    void    SetMessageReserved( U8 reserved) {messageReserved = reserved; }

protected:
	U16	   	messageSize;
	U8	   	messagePriority;
	U8	   	messageReserved;
};

class CEventMessage : public CMessage 
{
//	virtual CEventMessage(){};
public:    

   tEventType GetMessageEventType() {return eventType; }
   void       SetMessageEventType(tEventType value) {eventType = value; }

protected:
	tEventType		eventType;
};

// Mutex declarations
typedef pthread_mutex_t     tMutex;
typedef pthread_mutexattr_t tMutexAttr;

// Conditions declarations
typedef pthread_cond_t      tCond;
typedef pthread_condattr_t  tCondAttr;
typedef struct timespec     tTimeSpec;

typedef void (*pfnTimerCallback)(tTimerHndl arg); // FIXME/BSK

#define B_O_ACCMODE          0003
#define B_O_RDONLY             00
#define B_O_WRONLY             01
#define B_O_RDWR               02
#define B_O_CREAT            0100 /* not fcntl */
#define B_O_EXCL             0200 /* not fcntl */
#define B_O_NOCTTY           0400 /* not fcntl */
#define B_O_TRUNC           01000 /* not fcntl */
#define B_O_APPEND          02000
#define B_O_NONBLOCK        04000
#define B_O_NDELAY        	B_O_NONBLOCK
#define B_O_SYNC           010000
#define B_O_FSYNC          B_O_SYNC
#define B_O_ASYNC          020000

#define B_S_IFMT 0170000 	// bitmask for the file type bitfields
#define B_S_IFSOCK 0140000 	// socket
#define B_S_IFLNK 0120000 	// symbolic link
#define B_S_IFREG 0100000 	// regular file
#define B_S_IFBLK 0060000 	// block device
#define B_S_IFDIR 0040000 	// directory
#define B_S_IFCHR 0020000 	// character device
#define B_S_IFIFO 0010000 	// fifo
#define B_S_ISUID 0004000 	// set UID bit
#define B_S_ISGID 0002000 	// set GID bit (see below)
#define B_S_ISVTX 0001000 	// sticky bit (see below)
#define B_S_IRWXU 00700 	// mask for file owner permissions
#define B_S_IRUSR 00400 	// owner has read permission
#define B_S_IWUSR 00200 	// owner has write permission
#define B_S_IXUSR 00100 	// owner has execute permission
#define B_S_IRWXG 00070 	// mask for group permissions
#define B_S_IRGRP 00040 	// group has read permission
#define B_S_IWGRP 00020 	// group has write permission
#define B_S_IXGRP 00010 	// group has execute permission
#define B_S_IRWXO 00007 	// mask for permissions for others (not in group)
#define B_S_IROTH 00004 	// others have read permission
#define B_S_IWOTH 00002 	// others have write permisson
#define B_S_IXOTH 00001 	// others have execute permission


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_KERNELTYPES_H

// EOF

