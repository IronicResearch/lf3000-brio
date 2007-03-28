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

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <KernelTypes.h>

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

