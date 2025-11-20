#ifndef LIB_RESOURCES_H
#define LIB_RESOURCES_H

/*
 * libresources.a
 */

/*
 * Flags
 */

enum
{
	RESF_NONE				= (0),		/* */
	REFS_WALL				= (1 << 0),	/* Create a wall to detect memory writes outside allocation (Disabled for aligned allocations) */
	RESF_CLEANUP			= (1 << 1),	/* Cleanup resources on EndMemTrack() */
	RESF_DAMAGED			= (1 << 2),	/* Accept damaged free pointers (Safe) */
	RESF_ACCEPTMISMATCH	= (1 << 3),	/* Accept mismatched free pointers (Dangerous) */
	RESF_WARNDIRTYPOOL	= (1 << 4), /* Warn in case of delete non empty pool */
	RESF_TASKMISMATCH		= (1 << 5), /* Warn on task mismatch on free/delete operation */
};

/*
 * Allocation Types
 */

enum
{
	REST_ALLOCMEM			=	(1 << 0),	/* AllocMem() */
	REST_ALLOCVEC			=	(1 << 1),	/* AllocVec() */
	REST_ALLOCTASKP		=	(1 << 2),	/* ...TaskPooled() */
	REST_ALIGNED			=	(1 << 3),	/* ...Aligned() */
	REST_DMA					=	(1 << 4),	/* ...DMA() */
	REST_POOLED				=	(1 << 5),	/* ...Pooled() */
	REST_ANSI				=	(1 << 6),	/* malloc and friends */

	REST_CLEAR				=	(1 << 16),	/* Private ! */
};

/*
 * Prototypes
 */

#include "resources_protos.h"

#endif /* LIB_RESOURCES_H */
