/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _NET_PAGE_POOL_TYPES_H
#define _NET_PAGE_POOL_TYPES_H

#include <linux/dma-direction.h>
#include <linux/ptr_ring.h>
#include <linux/types.h>
#include <linux/xarray.h>
#include <net/netmem.h>
#include <linux/android_kabi.h>

#define PP_FLAG_DMA_MAP		BIT(0) 
#define PP_FLAG_DMA_SYNC_DEV	BIT(1) 
#define PP_FLAG_SYSTEM_POOL	BIT(2) 


#define PP_FLAG_ALLOW_UNREADABLE_NETMEM BIT(3)

#define PP_FLAG_ALL		(PP_FLAG_DMA_MAP | PP_FLAG_DMA_SYNC_DEV | \
				 PP_FLAG_SYSTEM_POOL | PP_FLAG_ALLOW_UNREADABLE_NETMEM)


#define PP_DMA_INDEX_LIMIT XA_LIMIT(1, BIT(PP_DMA_INDEX_BITS) - 1)


#define PP_ALLOC_CACHE_SIZE	128
#define PP_ALLOC_CACHE_REFILL	64
struct pp_alloc_cache {
	u32 count;
	netmem_ref cache[PP_ALLOC_CACHE_SIZE];
};


struct page_pool_params {
	struct_group_tagged(page_pool_params_fast, fast,
		unsigned int	order;
		unsigned int	pool_size;
		int		nid;
		struct device	*dev;
		struct napi_struct *napi;
		enum dma_data_direction dma_dir;
		unsigned int	max_len;
		unsigned int	offset;
	);
	struct_group_tagged(page_pool_params_slow, slow,
		struct net_device *netdev;
		unsigned int queue_idx;
		unsigned int	flags;

		void (*init_callback)(netmem_ref netmem, void *arg);
		void *init_arg;
	);
};

#ifdef CONFIG_PAGE_POOL_STATS

struct page_pool_alloc_stats {
	u64 fast;
	u64 slow;
	u64 slow_high_order;
	u64 empty;
	u64 refill;
	u64 waive;
};


struct page_pool_recycle_stats {
	u64 cached;
	u64 cache_full;
	u64 ring;
	u64 ring_full;
	u64 released_refcnt;
};


struct page_pool_stats {
	struct page_pool_alloc_stats alloc_stats;
	struct page_pool_recycle_stats recycle_stats;
};
#endif


#define PAGE_POOL_FRAG_GROUP_ALIGN	(4 * sizeof(long))

struct pp_memory_provider_params {
	void *mp_priv;
};

struct page_pool {
	struct page_pool_params_fast p;

	int cpuid;
	u32 pages_state_hold_cnt;

	bool has_init_callback:1;	
	bool dma_map:1;			
	bool dma_sync:1;		
#ifdef CONFIG_PAGE_POOL_STATS
	bool system:1;			
#endif

	__cacheline_group_begin_aligned(frag, PAGE_POOL_FRAG_GROUP_ALIGN);
	long frag_users;
	netmem_ref frag_page;
	unsigned int frag_offset;
	__cacheline_group_end_aligned(frag, PAGE_POOL_FRAG_GROUP_ALIGN);

	struct delayed_work release_dw;
	void (*disconnect)(void *pool);
	unsigned long defer_start;
	unsigned long defer_warn;

#ifdef CONFIG_PAGE_POOL_STATS
	
	struct page_pool_alloc_stats alloc_stats;
#endif
	u32 xdp_mem_id;

	
	struct pp_alloc_cache alloc ____cacheline_aligned_in_smp;

	
	struct ptr_ring ring;

	void *mp_priv;

	struct xarray dma_mapped;

#ifdef CONFIG_PAGE_POOL_STATS
	
	struct page_pool_recycle_stats __percpu *recycle_stats;
#endif
	atomic_t pages_state_release_cnt;

	
	refcount_t user_cnt;

	u64 destroy_cnt;

	
	struct page_pool_params_slow slow;
	
	struct {
		struct hlist_node list;
		u64 detach_time;
		u32 napi_id;
		u32 id;
	} user;
	ANDROID_KABI_RESERVE(1);
};

struct page *page_pool_alloc_pages(struct page_pool *pool, gfp_t gfp);
netmem_ref page_pool_alloc_netmem(struct page_pool *pool, gfp_t gfp);
struct page *page_pool_alloc_frag(struct page_pool *pool, unsigned int *offset,
				  unsigned int size, gfp_t gfp);
netmem_ref page_pool_alloc_frag_netmem(struct page_pool *pool,
				       unsigned int *offset, unsigned int size,
				       gfp_t gfp);
struct page_pool *page_pool_create(const struct page_pool_params *params);
struct page_pool *page_pool_create_percpu(const struct page_pool_params *params,
					  int cpuid);

struct xdp_mem_info;

#ifdef CONFIG_PAGE_POOL
void page_pool_disable_direct_recycling(struct page_pool *pool);
void page_pool_destroy(struct page_pool *pool);
void page_pool_use_xdp_mem(struct page_pool *pool, void (*disconnect)(void *),
			   const struct xdp_mem_info *mem);
void page_pool_put_page_bulk(struct page_pool *pool, void **data,
			     int count);
#else
static inline void page_pool_destroy(struct page_pool *pool)
{
}

static inline void page_pool_use_xdp_mem(struct page_pool *pool,
					 void (*disconnect)(void *),
					 const struct xdp_mem_info *mem)
{
}

static inline void page_pool_put_page_bulk(struct page_pool *pool, void **data,
					   int count)
{
}
#endif

void page_pool_put_unrefed_netmem(struct page_pool *pool, netmem_ref netmem,
				  unsigned int dma_sync_size,
				  bool allow_direct);
void page_pool_put_unrefed_page(struct page_pool *pool, struct page *page,
				unsigned int dma_sync_size,
				bool allow_direct);

static inline bool is_page_pool_compiled_in(void)
{
#ifdef CONFIG_PAGE_POOL
	return true;
#else
	return false;
#endif
}


void page_pool_update_nid(struct page_pool *pool, int new_nid);

#endif 
