/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_GFP_TYPES_H
#define __LINUX_GFP_TYPES_H

#include <linux/bits.h>


#if 0

typedef unsigned int __bitwise gfp_t;
#endif



enum {
	___GFP_DMA_BIT,
	___GFP_HIGHMEM_BIT,
	___GFP_DMA32_BIT,
	___GFP_MOVABLE_BIT,
	___GFP_RECLAIMABLE_BIT,
	___GFP_HIGH_BIT,
	___GFP_IO_BIT,
	___GFP_FS_BIT,
	___GFP_ZERO_BIT,
	___GFP_UNUSED_BIT,	
	___GFP_DIRECT_RECLAIM_BIT,
	___GFP_KSWAPD_RECLAIM_BIT,
	___GFP_WRITE_BIT,
	___GFP_NOWARN_BIT,
	___GFP_RETRY_MAYFAIL_BIT,
	___GFP_NOFAIL_BIT,
	___GFP_NORETRY_BIT,
	___GFP_MEMALLOC_BIT,
	___GFP_COMP_BIT,
	___GFP_NOMEMALLOC_BIT,
	___GFP_HARDWALL_BIT,
	___GFP_THISNODE_BIT,
	___GFP_ACCOUNT_BIT,
	___GFP_ZEROTAGS_BIT,
#ifdef CONFIG_KASAN_HW_TAGS
	___GFP_SKIP_ZERO_BIT,
	___GFP_SKIP_KASAN_BIT,
#endif
#ifdef CONFIG_LOCKDEP
	___GFP_NOLOCKDEP_BIT,
#endif
#ifdef CONFIG_SLAB_OBJ_EXT
	___GFP_NO_OBJ_EXT_BIT,
#endif
#ifdef CONFIG_CMA
	___GFP_CMA_BIT,
#endif
	___GFP_LAST_BIT
};


#define ___GFP_DMA		BIT(___GFP_DMA_BIT)
#define ___GFP_HIGHMEM		BIT(___GFP_HIGHMEM_BIT)
#define ___GFP_DMA32		BIT(___GFP_DMA32_BIT)
#define ___GFP_MOVABLE		BIT(___GFP_MOVABLE_BIT)
#define ___GFP_RECLAIMABLE	BIT(___GFP_RECLAIMABLE_BIT)
#define ___GFP_HIGH		BIT(___GFP_HIGH_BIT)
#define ___GFP_IO		BIT(___GFP_IO_BIT)
#define ___GFP_FS		BIT(___GFP_FS_BIT)
#define ___GFP_ZERO		BIT(___GFP_ZERO_BIT)

#define ___GFP_DIRECT_RECLAIM	BIT(___GFP_DIRECT_RECLAIM_BIT)
#define ___GFP_KSWAPD_RECLAIM	BIT(___GFP_KSWAPD_RECLAIM_BIT)
#define ___GFP_WRITE		BIT(___GFP_WRITE_BIT)
#define ___GFP_NOWARN		BIT(___GFP_NOWARN_BIT)
#define ___GFP_RETRY_MAYFAIL	BIT(___GFP_RETRY_MAYFAIL_BIT)
#define ___GFP_NOFAIL		BIT(___GFP_NOFAIL_BIT)
#define ___GFP_NORETRY		BIT(___GFP_NORETRY_BIT)
#define ___GFP_MEMALLOC		BIT(___GFP_MEMALLOC_BIT)
#define ___GFP_COMP		BIT(___GFP_COMP_BIT)
#define ___GFP_NOMEMALLOC	BIT(___GFP_NOMEMALLOC_BIT)
#define ___GFP_HARDWALL		BIT(___GFP_HARDWALL_BIT)
#define ___GFP_THISNODE		BIT(___GFP_THISNODE_BIT)
#define ___GFP_ACCOUNT		BIT(___GFP_ACCOUNT_BIT)
#define ___GFP_ZEROTAGS		BIT(___GFP_ZEROTAGS_BIT)
#ifdef CONFIG_KASAN_HW_TAGS
#define ___GFP_SKIP_ZERO	BIT(___GFP_SKIP_ZERO_BIT)
#define ___GFP_SKIP_KASAN	BIT(___GFP_SKIP_KASAN_BIT)
#else
#define ___GFP_SKIP_ZERO	0
#define ___GFP_SKIP_KASAN	0
#endif
#ifdef CONFIG_LOCKDEP
#define ___GFP_NOLOCKDEP	BIT(___GFP_NOLOCKDEP_BIT)
#else
#define ___GFP_NOLOCKDEP	0
#endif
#ifdef CONFIG_SLAB_OBJ_EXT
#define ___GFP_NO_OBJ_EXT       BIT(___GFP_NO_OBJ_EXT_BIT)
#else
#define ___GFP_NO_OBJ_EXT       0
#endif

#ifdef CONFIG_CMA
#define ___GFP_CMA		BIT(___GFP_CMA_BIT)
#else
#define ___GFP_CMA		0
#endif


#define __GFP_DMA	((__force gfp_t)___GFP_DMA)
#define __GFP_HIGHMEM	((__force gfp_t)___GFP_HIGHMEM)
#define __GFP_DMA32	((__force gfp_t)___GFP_DMA32)
#define __GFP_MOVABLE	((__force gfp_t)___GFP_MOVABLE)  
#define __GFP_CMA	((__force gfp_t)___GFP_CMA)
#define GFP_ZONEMASK	(__GFP_DMA|__GFP_HIGHMEM|__GFP_DMA32|__GFP_MOVABLE)


#define __GFP_RECLAIMABLE ((__force gfp_t)___GFP_RECLAIMABLE)
#define __GFP_WRITE	((__force gfp_t)___GFP_WRITE)
#define __GFP_HARDWALL   ((__force gfp_t)___GFP_HARDWALL)
#define __GFP_THISNODE	((__force gfp_t)___GFP_THISNODE)
#define __GFP_ACCOUNT	((__force gfp_t)___GFP_ACCOUNT)
#define __GFP_NO_OBJ_EXT   ((__force gfp_t)___GFP_NO_OBJ_EXT)


#define __GFP_HIGH	((__force gfp_t)___GFP_HIGH)
#define __GFP_MEMALLOC	((__force gfp_t)___GFP_MEMALLOC)
#define __GFP_NOMEMALLOC ((__force gfp_t)___GFP_NOMEMALLOC)


#define __GFP_IO	((__force gfp_t)___GFP_IO)
#define __GFP_FS	((__force gfp_t)___GFP_FS)
#define __GFP_DIRECT_RECLAIM	((__force gfp_t)___GFP_DIRECT_RECLAIM) 
#define __GFP_KSWAPD_RECLAIM	((__force gfp_t)___GFP_KSWAPD_RECLAIM) 
#define __GFP_RECLAIM ((__force gfp_t)(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM))
#define __GFP_RETRY_MAYFAIL	((__force gfp_t)___GFP_RETRY_MAYFAIL)
#define __GFP_NOFAIL	((__force gfp_t)___GFP_NOFAIL)
#define __GFP_NORETRY	((__force gfp_t)___GFP_NORETRY)


#define __GFP_NOWARN	((__force gfp_t)___GFP_NOWARN)
#define __GFP_COMP	((__force gfp_t)___GFP_COMP)
#define __GFP_ZERO	((__force gfp_t)___GFP_ZERO)
#define __GFP_ZEROTAGS	((__force gfp_t)___GFP_ZEROTAGS)
#define __GFP_SKIP_ZERO ((__force gfp_t)___GFP_SKIP_ZERO)
#define __GFP_SKIP_KASAN ((__force gfp_t)___GFP_SKIP_KASAN)


#define __GFP_NOLOCKDEP ((__force gfp_t)___GFP_NOLOCKDEP)


#define __GFP_BITS_SHIFT ___GFP_LAST_BIT
#define __GFP_BITS_MASK ((__force gfp_t)((1 << __GFP_BITS_SHIFT) - 1))


#define GFP_ATOMIC	(__GFP_HIGH|__GFP_KSWAPD_RECLAIM)
#define GFP_KERNEL	(__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define GFP_KERNEL_ACCOUNT (GFP_KERNEL | __GFP_ACCOUNT)
#define GFP_NOWAIT	(__GFP_KSWAPD_RECLAIM | __GFP_NOWARN)
#define GFP_NOIO	(__GFP_RECLAIM)
#define GFP_NOFS	(__GFP_RECLAIM | __GFP_IO)
#define GFP_USER	(__GFP_RECLAIM | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
#define GFP_DMA		__GFP_DMA
#define GFP_DMA32	__GFP_DMA32
#define GFP_HIGHUSER	(GFP_USER | __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE	(GFP_HIGHUSER | __GFP_MOVABLE | __GFP_SKIP_KASAN)
#define GFP_TRANSHUGE_LIGHT	((GFP_HIGHUSER_MOVABLE | __GFP_COMP | \
			 __GFP_NOMEMALLOC | __GFP_NOWARN) & ~__GFP_RECLAIM)
#define GFP_TRANSHUGE	(GFP_TRANSHUGE_LIGHT | __GFP_DIRECT_RECLAIM)

#endif 
