#ifndef ERRORBRIO_H
#define ERRORBRIO_H
						    
//==============================================================================
// $Source: $
//
// Copyright (c) 2002,2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		ErrorBrio.h
//
// Description:
//		Brio error procesing
//
//==============================================================================

#include </mnt/hgfs/win_c/sw/Brio/System/include/CoreTypes.h>
#include </mnt/hgfs/win_c/sw/Brio/System/include/SystemTypes.h>
#include </mnt/hgfs/win_c/sw/Brio/System/include/KernelTypes.h>

// Temporary error definition

enum{
    kInitialExtraErrorCode=2000,
    kInvalidFunctionArgument=kInitialExtraErrorCode,
    kCouldNotAllocateMemory,
    kFinalExtraErrorCode
};

class ErrorBrio
{
    public:
        static tErrType lookupBrioErrType(int errFunc);
	
};
#endif // #ifndef KERNELBRIO_H

// EOF

