//==============================================================================
//
// File:
//		MemPool.cpp
//
// Description:
//		Implementation for KernelMPI memory pool functions.
//
//		Wrapper for dlmalloc:
//		ftp://gee.cs.oswego.edu/pub/misc/malloc.c
//
//==============================================================================
#include <stdlib.h>
#include <string.h>
#include <KernelMPI.h>
#include <KernelPriv.h>
#include <KernelTypes.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Internal functions
//==============================================================================
namespace
{
	#define MSPACES			1		// memspace support
	#define ONLY_MSPACES	1		// no export for malloc/free overide
	#include "malloc.c"				// Memory allocator implementation itself
}

LF_BEGIN_BRIO_NAMESPACE()
//------------------------------------------------------------------------------
tMemoryPoolHndl CKernelModule::CreateMemPool( U32 size )
{
	return reinterpret_cast<tMemoryPoolHndl>(create_mspace((size_t)size, false));
}

//------------------------------------------------------------------------------
void CKernelModule::DestroyMemPool( tMemoryPoolHndl pool )
{
#ifdef DEBUG
	mDebugMPI.DebugOut(kDbgLvlImportant, "%s: pool=%p, bytes=%d, max=%d\n", __FUNCTION__, (void*)pool, mspace_footprint((mspace)pool), mspace_max_footprint((mspace)pool));
#endif
	destroy_mspace((mspace)pool);
}

//------------------------------------------------------------------------------
tPtr CKernelModule::MemPoolMalloc( tMemoryPoolHndl pool, U32 size )
{
	return mspace_malloc((mspace)pool, (size_t)size);
}

//------------------------------------------------------------------------------
tPtr CKernelModule::MemPoolRealloc( tMemoryPoolHndl pool, tPtr pmem, U32 size )
{
	return mspace_realloc((mspace)pool, pmem, (size_t)size);
}

//------------------------------------------------------------------------------
void CKernelModule::MemPoolFree( tMemoryPoolHndl pool, tPtr pmem )
{
	mspace_free((mspace)pool, pmem);
}

//------------------------------------------------------------------------------
LF_END_BRIO_NAMESPACE()

// EOF
