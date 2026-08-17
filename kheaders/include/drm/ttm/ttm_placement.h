


#ifndef _TTM_PLACEMENT_H_
#define _TTM_PLACEMENT_H_

#include <linux/types.h>



#define TTM_PL_SYSTEM           0
#define TTM_PL_TT               1
#define TTM_PL_VRAM             2
#define TTM_PL_PRIV             3



#define TTM_PL_FLAG_CONTIGUOUS  (1 << 0)
#define TTM_PL_FLAG_TOPDOWN     (1 << 1)


#define TTM_PL_FLAG_TEMPORARY   (1 << 2)


#define TTM_PL_FLAG_DESIRED	(1 << 3)


#define TTM_PL_FLAG_FALLBACK	(1 << 4)


struct ttm_place {
	unsigned	fpfn;
	unsigned	lpfn;
	uint32_t	mem_type;
	uint32_t	flags;
};


struct ttm_placement {
	unsigned		num_placement;
	const struct ttm_place	*placement;
};

#endif
