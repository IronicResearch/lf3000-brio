//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		FreeMem.cpp
//
// Description:
//		Implements Utility functions for Brio.
//
//============================================================================
#include <stdio.h>
#include <sys/sysinfo.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// Returns free memory snapshot in user space
// Note: please see "man sysinfo" for details on what else sysinfo() provides
//----------------------------------------------------------------------------
int GetFreeMem(void)
{
	struct sysinfo info;

	sysinfo(&info); // freeram is in bytes
	return (int)(info.freeram/1024);
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
