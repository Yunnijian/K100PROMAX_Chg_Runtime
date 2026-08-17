/* SPDX-License-Identifier: GPL-2.0
 *
 * page_pool/helpers.h
 *	Author:	Jesper Dangaard Brouer <netoptimizer@brouer.com>
 *	Copyright (C) 2016 Red Hat, Inc.
 */


#ifndef _NET_PAGE_POOL_HELPERS_H
#define _NET_PAGE_POOL_HELPERS_H

#include <linux/dma-mapping.h>

#include <net/page_pool/types.h>
#include <net/net_debug.h>
#include <net/netmem.h>

#ifdef CONFIG_PAGE_POOL_STATS

int page_pool_ethtool_stats_get_count(void);
u8 *page_pool_ethtool_stats_get_strings(u8 *data);
u64 *page_pool_ethtool_stats_get(u64 *data, const void *stats);

bool page_pool_get_stats(const struct page_pool *pool,
			 struct page_pool_stats *stats);
#else
static inline int page_pool_ethtool_stats_get_count(void)
{
	return 0;
}

static inline u8 *page_pool_ethtool_stats_get_strings(u8 *data)
{
	return data;
}

static inline u64 *page_pool_ethtool_stats_get(u64 *data, const void *stats)
{
	return data;
}
#endif


static inline struct page *page_pool_dev_alloc_pages(struct page_pool *pool)
{
	gfp_t gfp = (GFP_ATOMIC | __GFP_NOWARN);

	return page_pool_alloc_pages(pool, gfp);
}


static inline struct page *page_pool_dev_alloc_frag(struct page_pool *pool,
						    unsigned int *offset,
						    unsigned int size)
{
	gfp_t gfp = (GFP_ATOMIC | __GFP_NOWARN);

	return page_pool_alloc_frag(pool, offset, size, gfp);
}

static inline struct page *page_pool_alloc(struct page_pool *pool,
					   unsigned int *offset,
					   unsigned int *size, gfp_t gfp)
{
	unsigned int max_size = PAGE_SIZE << pool->p.order;
	struct page *page;

	if ((*size << 1) > max_size) {
		*size = max_size;
		*offset = 0;
		return page_pool_alloc_pages(pool, gfp);
	}

	page = page_pool_alloc_frag(pool, offset, *size, gfp);
	if (unlikely(!page))
		return NULL;

	
	if (pool->frag_offset + *size > max_size) {
		*size = max_size - *offset;
		pool->frag_offset = max_size;
	}

	return page;
}


static inline struct page *page_pool_dev_alloc(struct page_pool *pool,
					       unsigned int *offset,
					       unsigned int *size)
{
	gfp_t gfp = (GFP_ATOMIC | __GFP_NOWARN);

	return page_pool_alloc(pool, offset, size, gfp);
}

static inline void *page_pool_alloc_va(struct page_pool *pool,
				       unsigned int *size, gfp_t gfp)
{
	unsigned int offset;
	struct page *page;

	
	page = page_pool_alloc(pool, &offset, size, gfp & ~__GFP_HIGHMEM);
	if (unlikely(!page))
		return NULL;

	return page_address(page) + offset;
}


static inline void *page_pool_dev_alloc_va(struct page_pool *pool,
					   unsigned int *size)
{
	gfp_t gfp = (GFP_ATOMIC | __GFP_NOWARN);

	return page_pool_alloc_va(pool, size, gfp);
}


static inline enum dma_data_direction
page_pool_get_dma_dir(const struct page_pool *pool)
{
	return pool->p.dma_dir;
}

static inline void page_pool_fragment_netmem(netmem_ref netmem, long nr)
{
	atomic_long_set(netmem_get_pp_ref_count_ref(netmem), nr);
}


static inline void page_pool_fragment_page(struct page *page, long nr)
{
	page_pool_fragment_netmem(page_to_netmem(page), nr);
}

static inline long page_pool_unref_netmem(netmem_ref netmem, long nr)
{
	atomic_long_t *pp_ref_count = netmem_get_pp_ref_count_ref(netmem);
	long ret;

	
	if (atomic_long_read(pp_ref_count) == nr) {
		
		BUILD_BUG_ON(__builtin_constant_p(nr) && nr != 1);
		if (!__builtin_constant_p(nr))
			atomic_long_set(pp_ref_count, 1);

		return 0;
	}

	ret = atomic_long_sub_return(nr, pp_ref_count);
	WARN_ON(ret < 0);

	
	if (unlikely(!ret))
		atomic_long_set(pp_ref_count, 1);

	return ret;
}

static inline long page_pool_unref_page(struct page *page, long nr)
{
	return page_pool_unref_netmem(page_to_netmem(page), nr);
}

static inline void page_pool_ref_netmem(netmem_ref netmem)
{
	atomic_long_inc(netmem_get_pp_ref_count_ref(netmem));
}

static inline void page_pool_ref_page(struct page *page)
{
	page_pool_ref_netmem(page_to_netmem(page));
}

static inline bool page_pool_is_last_ref(netmem_ref netmem)
{
	
	return page_pool_unref_netmem(netmem, 1) == 0;
}

static inline void page_pool_put_netmem(struct page_pool *pool,
					netmem_ref netmem,
					unsigned int dma_sync_size,
					bool allow_direct)
{
	
#ifdef CONFIG_PAGE_POOL
	if (!page_pool_is_last_ref(netmem))
		return;

	page_pool_put_unrefed_netmem(pool, netmem, dma_sync_size, allow_direct);
#endif
}


static inline void page_pool_put_page(struct page_pool *pool,
				      struct page *page,
				      unsigned int dma_sync_size,
				      bool allow_direct)
{
	page_pool_put_netmem(pool, page_to_netmem(page), dma_sync_size,
			     allow_direct);
}

static inline void page_pool_put_full_netmem(struct page_pool *pool,
					     netmem_ref netmem,
					     bool allow_direct)
{
	page_pool_put_netmem(pool, netmem, -1, allow_direct);
}


static inline void page_pool_put_full_page(struct page_pool *pool,
					   struct page *page, bool allow_direct)
{
	page_pool_put_netmem(pool, page_to_netmem(page), -1, allow_direct);
}


static inline void page_pool_recycle_direct(struct page_pool *pool,
					    struct page *page)
{
	page_pool_put_full_page(pool, page, true);
}

#define PAGE_POOL_32BIT_ARCH_WITH_64BIT_DMA	\
		(sizeof(dma_addr_t) > sizeof(unsigned long))


static inline void page_pool_free_va(struct page_pool *pool, void *va,
				     bool allow_direct)
{
	page_pool_put_page(pool, virt_to_head_page(va), -1, allow_direct);
}

static inline dma_addr_t page_pool_get_dma_addr_netmem(netmem_ref netmem)
{
	dma_addr_t ret = netmem_get_dma_addr(netmem);

	if (PAGE_POOL_32BIT_ARCH_WITH_64BIT_DMA)
		ret <<= PAGE_SHIFT;

	return ret;
}


static inline dma_addr_t page_pool_get_dma_addr(const struct page *page)
{
	return page_pool_get_dma_addr_netmem(page_to_netmem((struct page *)page));
}


static inline void page_pool_dma_sync_for_cpu(const struct page_pool *pool,
					      const struct page *page,
					      u32 offset, u32 dma_sync_size)
{
	dma_sync_single_range_for_cpu(pool->p.dev,
				      page_pool_get_dma_addr(page),
				      offset + pool->p.offset, dma_sync_size,
				      page_pool_get_dma_dir(pool));
}

static inline bool page_pool_put(struct page_pool *pool)
{
	return refcount_dec_and_test(&pool->user_cnt);
}

static inline void page_pool_nid_changed(struct page_pool *pool, int new_nid)
{
	if (unlikely(pool->p.nid != new_nid))
		page_pool_update_nid(pool, new_nid);
}

#endif 
