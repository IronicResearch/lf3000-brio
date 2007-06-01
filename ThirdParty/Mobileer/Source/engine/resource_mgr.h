#ifndef _RESOURCE_MGR_H
#define _RESOURCE_MGR_H

/* $Id$ */
/**
 *
 * Generic Resource manager. Keep in linked lists.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "dbl_list.h"
#include "spmidi.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define RESOURCE_UNDEFINED_ID   (0)

	typedef spmUInt32 spmResourceToken;

	/* Used to keep track of resources in linked lists.
	 * Resource is identified by a token.
	 * References are counted to prevent deletion of
	 * resources still in use.
	 */
	typedef struct ResourceTracker_s
	{
		DoubleNode      node;
		int             referenceCount;
		spmResourceToken   token;
	}
	ResourceTracker_t;

	void ResourceMgr_InitResource( ResourceTracker_t *resource );

	/** Add in sorted order by token.
	 * Token must not be zero because that is RESOURCE_UNDEFINED_ID
	 */
	SPMIDI_Error ResourceMgr_Add( DoubleList *list, ResourceTracker_t *resource, spmResourceToken token );

	/** Find in list, assuming in sorted order.
	 * @return resource or NULL if not found.
	 */
	ResourceTracker_t *ResourceMgr_Find( DoubleList *list, spmResourceToken token );

	/** Remove from list. */
	SPMIDI_Error ResourceMgr_Remove( ResourceTracker_t *resource );


#ifdef __cplusplus
}
#endif

#endif /* _RESOURCE_MGR_H */

