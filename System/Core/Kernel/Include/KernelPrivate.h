#ifndef KERNELPRIVATE_H
#define KERNELPRIVATE_H
						    
//==============================================================================
// $Source: $
//
// Copyright (c) 2002,2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		KernelPrivate.h
//
// Description:
//		Private, hidden data structures used by the KernelMPI
//
//==============================================================================

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <KernelTypes.h>

// nucleus includes
// BK #include <nucleus.h>

const CURI kKernelModuleURI = "/LF/Brio/System/Module/Kernel";
const CURI kKernelMPIURI = "/LF/Brio/System/MPI/KernelMPI";

const tVersion kKernelMPIVersion = MakeVersion(0,1);

typedef U32 tKernelID;
typedef U32 tKernelSignature;

struct tKernelMPIFcnTable
{
	tErrType (*pGetModuleVersion)(tVersion *pVersion);
	tErrType (*pGetModuleName)(const CString **ppName);
	tErrType (*pGetModuleOrigin)(const CURI **ppURI);
	void	 (*pFirstKernelFunction)();
};


// context information for each KernelMPI object created
class CKernelMPIImpl {
public:
//	CDataFmtVersion		fmtVersion;
	tKernelMPIFcnTable	* mpMPIFcnTable;
	CURI				* mpModuleURI;	
//	tKernelSignature	signature;
//	tKernelID			id;
};

#define DERIVE_CLASS_FROM_RTOS_STRUCT(kernelType, rtosType, opaqueType) 		\
			class kernelType : public rtosType {								\
			public:																\
				rtosType*	ToRTOSType() 										\
								{ return (rtosType*)(this); }					\
				opaqueType	ToOpaqueType()										\
								{ return reinterpret_cast<opaqueType>(this); }	\
				static opaqueType ToOpaqueType(rtosType* t) 					\
								{ return reinterpret_cast<opaqueType>(t); }		\
				static rtosType* ToRTOSType(opaqueType t) 						\
								{ return reinterpret_cast<rtosType*>(t); }		\
				static kernelType* ToKernelType(opaqueType t) 					\
								{ return reinterpret_cast<kernelType*>(t); }	\
																				\
			kernelType()														\
			{																	\
				mpExtraMemory = NULL;											\
			}																	\
																				\
			kernelType(U32 extraMemorySize, tDataPtr* ppExtraMemory)			\
			{																	\
				mpExtraMemory = (extraMemorySize != 0) ? new U8[extraMemorySize] : NULL;	\
				if (ppExtraMemory != NULL)										\
					*ppExtraMemory = mpExtraMemory;								\
			}																	\
																				\
			~kernelType()														\
			{																	\
				if (mpExtraMemory != NULL) 										\
					delete [] mpExtraMemory; 									\
			}																	\
																				\
			protected:															\
				tDataPtr	mpExtraMemory;										
			// end of class

#ifdef BK_LINUX
DERIVE_CLASS_FROM_RTOS_STRUCT(CKernelTask, NU_TASK, tTaskHndl) // {
public:
};

DERIVE_CLASS_FROM_RTOS_STRUCT(CKernelMemoryPool, NU_MEMORY_POOL, tMemoryPoolHndl) //{
public:
};

DERIVE_CLASS_FROM_RTOS_STRUCT(CKernelMessageQueue, NU_QUEUE, tMemoryPoolHndl)// {
public:
};

DERIVE_CLASS_FROM_RTOS_STRUCT(CKernelTimer, NU_TIMER, tTimerHndl) // {
public:
};
#endif



#endif // #ifndef KERNELPRIVATE_H

// EOF

