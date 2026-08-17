/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SWAP_H
#define _LINUX_SWAP_H

#include <linux/spinlock.h>
#include <linux/linkage.h>
#include <linux/mmzone.h>
#include <linux/list.h>
#include <linux/memcontrol.h>
#include <linux/sched.h>
#include <linux/node.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/atomic.h>
#include <linux/page-flags.h>
#include <uapi/linux/mempolicy.h>
#include <asm/page.h>

struct notifier_block;

struct bio;

struct pagevec;

#define SWAP_FLAG_PREFER	0x8000	
#define SWAP_FLAG_PRIO_MASK	0x7fff
#define SWAP_FLAG_PRIO_SHIFT	0
#define SWAP_FLAG_DISCARD	0x10000 
#define SWAP_FLAG_DISCARD_ONCE	0x20000 
#define SWAP_FLAG_DISCARD_PAGES 0x40000 

#define SWAP_FLAGS_VALID	(SWAP_FLAG_PRIO_MASK | SWAP_FLAG_PREFER | \
				 SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE | \
				 SWAP_FLAG_DISCARD_PAGES)
#define SWAP_BATCH 64

static inline int current_is_kswapd(void)
{
	return current->flags & PF_KSWAPD;
}


#define MAX_SWAPFILES_SHIFT	5




#define SWP_PTE_MARKER_NUM 1
#define SWP_PTE_MARKER     (MAX_SWAPFILES + SWP_HWPOISON_NUM + \
			    SWP_MIGRATION_NUM + SWP_DEVICE_NUM)


#ifdef CONFIG_DEVICE_PRIVATE
#define SWP_DEVICE_NUM 4
#define SWP_DEVICE_WRITE (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM)
#define SWP_DEVICE_READ (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+1)
#define SWP_DEVICE_EXCLUSIVE_WRITE (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+2)
#define SWP_DEVICE_EXCLUSIVE_READ (MAX_SWAPFILES+SWP_HWPOISON_NUM+SWP_MIGRATION_NUM+3)
#else
#define SWP_DEVICE_NUM 0
#endif


#ifdef CONFIG_MIGRATION
#define SWP_MIGRATION_NUM 3
#define SWP_MIGRATION_READ (MAX_SWAPFILES + SWP_HWPOISON_NUM)
#define SWP_MIGRATION_READ_EXCLUSIVE (MAX_SWAPFILES + SWP_HWPOISON_NUM + 1)
#define SWP_MIGRATION_WRITE (MAX_SWAPFILES + SWP_HWPOISON_NUM + 2)
#else
#define SWP_MIGRATION_NUM 0
#endif


#ifdef CONFIG_MEMORY_FAILURE
#define SWP_HWPOISON_NUM 1
#define SWP_HWPOISON		MAX_SWAPFILES
#else
#define SWP_HWPOISON_NUM 0
#endif

#define MAX_SWAPFILES \
	((1 << MAX_SWAPFILES_SHIFT) - SWP_DEVICE_NUM - \
	SWP_MIGRATION_NUM - SWP_HWPOISON_NUM - \
	SWP_PTE_MARKER_NUM)


union swap_header {
	struct {
		char reserved[PAGE_SIZE - 10];
		char magic[10];			
	} magic;
	struct {
		char		bootbits[1024];	
		__u32		version;
		__u32		last_page;
		__u32		nr_badpages;
		unsigned char	sws_uuid[16];
		unsigned char	sws_volume[16];
		__u32		padding[117];
		__u32		badpages[1];
	} info;
};


struct reclaim_state {
	
	unsigned long reclaimed;
#ifdef CONFIG_LRU_GEN
	
	struct lru_gen_mm_walk *mm_walk;
#endif
};


static inline void mm_account_reclaimed_pages(unsigned long pages)
{
	if (current->reclaim_state)
		current->reclaim_state->reclaimed += pages;
}

#ifdef __KERNEL__

struct address_space;
struct sysinfo;
struct writeback_control;
struct zone;


struct swap_extent {
	struct rb_node rb_node;
	pgoff_t start_page;
	pgoff_t nr_pages;
	sector_t start_block;
};


#define MAX_SWAP_BADPAGES \
	((offsetof(union swap_header, magic.magic) - \
	  offsetof(union swap_header, info.badpages)) / sizeof(int))

enum {
	SWP_USED	= (1 << 0),	
	SWP_WRITEOK	= (1 << 1),	
	SWP_DISCARDABLE = (1 << 2),	
	SWP_DISCARDING	= (1 << 3),	
	SWP_SOLIDSTATE	= (1 << 4),	
	SWP_CONTINUED	= (1 << 5),	
	SWP_BLKDEV	= (1 << 6),	
	SWP_ACTIVATED	= (1 << 7),	
	SWP_FS_OPS	= (1 << 8),	
	SWP_AREA_DISCARD = (1 << 9),	
	SWP_PAGE_DISCARD = (1 << 10),	
	SWP_STABLE_WRITES = (1 << 11),	
	__SWP_READ_SYNCHRONOUS_IO = (1 << 12),	
	__SWP_WRITE_SYNCHRONOUS_IO = (1 << 13),	
	SWP_SYNCHRONOUS_IO = (__SWP_READ_SYNCHRONOUS_IO | __SWP_WRITE_SYNCHRONOUS_IO),
					
	SWP_SCANNING	= (1 << 14),	
};

#define SWAP_CLUSTER_MAX 32UL
#define COMPACT_CLUSTER_MAX SWAP_CLUSTER_MAX


#define SWAP_HAS_CACHE	0x40	
#define COUNT_CONTINUED	0x80	


#define SWAP_MAP_MAX	0x3e	
#define SWAP_MAP_BAD	0x3f	
#define SWAP_MAP_SHMEM	0xbf	


#define SWAP_CONT_MAX	0x7f	


struct swap_cluster_info {
	spinlock_t lock;	
	u16 count;
	u8 flags;
	u8 order;
	struct list_head list;
};
#define CLUSTER_FLAG_FREE 1 
#define CLUSTER_FLAG_NONFULL 2 
#define CLUSTER_FLAG_FRAG 4 
#define CLUSTER_FLAG_FULL 8 


#define SWAP_NEXT_INVALID	0

#ifdef CONFIG_THP_SWAP
#define SWAP_NR_ORDERS		(PMD_ORDER + 1)
#else
#define SWAP_NR_ORDERS		1
#endif


struct percpu_cluster {
	unsigned int next[SWAP_NR_ORDERS]; 
};


struct swap_info_struct {
	struct percpu_ref users;	
	unsigned long	flags;		
	signed short	prio;		
	struct plist_node list;		
	signed char	type;		
	unsigned int	max;		
	unsigned char *swap_map;	
	unsigned long *zeromap;		
	struct swap_cluster_info *cluster_info; 
	struct list_head free_clusters; 
	struct list_head full_clusters; 
	struct list_head nonfull_clusters[SWAP_NR_ORDERS];
					
	struct list_head frag_clusters[SWAP_NR_ORDERS];
					
	unsigned int frag_cluster_nr[SWAP_NR_ORDERS];
	unsigned int lowest_bit;	
	unsigned int highest_bit;	
	unsigned int pages;		
	unsigned int inuse_pages;	
	unsigned int cluster_next;	
	unsigned int cluster_nr;	
	unsigned int __percpu *cluster_next_cpu; 
	struct percpu_cluster __percpu *percpu_cluster; 
	struct rb_root swap_extent_root;
	struct block_device *bdev;	
	struct file *swap_file;		
	struct completion comp;		
	spinlock_t lock;		
	spinlock_t cont_lock;		
	struct work_struct discard_work; 
	struct work_struct reclaim_work; 
	struct list_head discard_clusters; 
	struct plist_node avail_lists[]; 
};

static inline swp_entry_t page_swap_entry(struct page *page)
{
	struct folio *folio = page_folio(page);
	swp_entry_t entry = folio->swap;

	entry.val += folio_page_idx(folio, page);
	return entry;
}


bool workingset_test_recent(void *shadow, bool file, bool *workingset,
				bool flush);
void workingset_age_nonresident(struct lruvec *lruvec, unsigned long nr_pages);
void *workingset_eviction(struct folio *folio, struct mem_cgroup *target_memcg);
void workingset_refault(struct folio *folio, void *shadow);
void workingset_activation(struct folio *folio);


extern unsigned long totalreserve_pages;


#define nr_free_pages() global_zone_page_state(NR_FREE_PAGES)



void lru_note_cost(struct lruvec *lruvec, bool file,
		   unsigned int nr_io, unsigned int nr_rotated);
void lru_note_cost_refault(struct folio *);
void folio_add_lru(struct folio *);
void folio_add_lru_vma(struct folio *, struct vm_area_struct *);
void mark_page_accessed(struct page *);
void folio_mark_accessed(struct folio *);

static inline bool folio_may_be_lru_cached(struct folio *folio)
{
	
	return !folio_test_large(folio);
}

extern atomic_t lru_disable_count;

static inline bool lru_cache_disabled(void)
{
	return atomic_read(&lru_disable_count);
}

static inline void lru_cache_enable(void)
{
	atomic_dec(&lru_disable_count);
}

extern void lru_cache_disable(void);
extern void lru_add_drain(void);
extern void lru_add_drain_cpu(int cpu);
extern void lru_add_drain_cpu_zone(struct zone *zone);
extern void lru_add_drain_all(void);
void folio_deactivate(struct folio *folio);
void folio_mark_lazyfree(struct folio *folio);
extern void swap_setup(void);


extern unsigned long zone_reclaimable_pages(struct zone *zone);
extern unsigned long try_to_free_pages(struct zonelist *zonelist, int order,
					gfp_t gfp_mask, nodemask_t *mask);

#define MEMCG_RECLAIM_MAY_SWAP (1 << 1)
#define MEMCG_RECLAIM_PROACTIVE (1 << 2)
#define MIN_SWAPPINESS 0
#define MAX_SWAPPINESS 200
extern unsigned long try_to_free_mem_cgroup_pages(struct mem_cgroup *memcg,
						  unsigned long nr_pages,
						  gfp_t gfp_mask,
						  unsigned int reclaim_options,
						  int *swappiness);
extern unsigned long mem_cgroup_shrink_node(struct mem_cgroup *mem,
						gfp_t gfp_mask, bool noswap,
						pg_data_t *pgdat,
						unsigned long *nr_scanned);
extern unsigned long shrink_all_memory(unsigned long nr_pages);
extern int vm_swappiness;
long remove_mapping(struct address_space *mapping, struct folio *folio);

extern unsigned long reclaim_pages(struct list_head *folio_list);
extern unsigned long __reclaim_pages(struct list_head *folio_list,
				     void *private);
#ifdef CONFIG_NUMA
extern int node_reclaim_mode;
extern int sysctl_min_unmapped_ratio;
extern int sysctl_min_slab_ratio;
#else
#define node_reclaim_mode 0
#endif

static inline bool node_reclaim_enabled(void)
{
	
	return node_reclaim_mode & (RECLAIM_ZONE|RECLAIM_WRITE|RECLAIM_UNMAP);
}

void check_move_unevictable_folios(struct folio_batch *fbatch);

extern void __meminit kswapd_run(int nid);
extern void __meminit kswapd_stop(int nid);

#ifdef CONFIG_SWAP

int add_swap_extent(struct swap_info_struct *sis, unsigned long start_page,
		unsigned long nr_pages, sector_t start_block);
int generic_swapfile_activate(struct swap_info_struct *, struct file *,
		sector_t *);

static inline unsigned long total_swapcache_pages(void)
{
	return global_node_page_state(NR_SWAPCACHE);
}

void delete_from_swap_cache(struct folio *folio);
void free_swap_cache(struct folio *folio);
void free_page_and_swap_cache(struct page *);
void free_pages_and_swap_cache(struct encoded_page **, int);

extern atomic_long_t nr_swap_pages;
extern long total_swap_pages;
extern atomic_t nr_rotate_swap;
extern bool has_usable_swap(void);


static inline bool vm_swap_full(void)
{
	return atomic_long_read(&nr_swap_pages) * 2 < total_swap_pages;
}

static inline long get_nr_swap_pages(void)
{
	return atomic_long_read(&nr_swap_pages);
}

extern void si_swapinfo(struct sysinfo *);
swp_entry_t folio_alloc_swap(struct folio *folio);
bool folio_free_swap(struct folio *folio);
void put_swap_folio(struct folio *folio, swp_entry_t entry);
extern swp_entry_t get_swap_page_of_type(int);
extern int get_swap_pages(int n, swp_entry_t swp_entries[], int order);
extern int add_swap_count_continuation(swp_entry_t, gfp_t);
extern void swap_shmem_alloc(swp_entry_t, int);
extern int swap_duplicate(swp_entry_t);
extern int swapcache_prepare(swp_entry_t entry, int nr);
extern void swap_free_nr(swp_entry_t entry, int nr_pages);
extern void swapcache_free_entries(swp_entry_t *entries, int n);
extern void free_swap_and_cache_nr(swp_entry_t entry, int nr);
int swap_type_of(dev_t device, sector_t offset);
int find_first_swap(dev_t *device);
extern unsigned int count_swap_pages(int, int);
extern sector_t swapdev_block(int, pgoff_t);
extern int __swap_count(swp_entry_t entry);
extern int swap_swapcount(struct swap_info_struct *si, swp_entry_t entry);
extern int swp_swapcount(swp_entry_t entry);
struct swap_info_struct *swp_swap_info(swp_entry_t entry);
struct backing_dev_info;
extern int init_swap_address_space(unsigned int type, unsigned long nr_pages);
extern void exit_swap_address_space(unsigned int type);
extern struct swap_info_struct *get_swap_device(swp_entry_t entry);
sector_t swap_folio_sector(struct folio *folio);
extern sector_t alloc_swapdev_block(int swap);

static inline void put_swap_device(struct swap_info_struct *si)
{
	percpu_ref_put(&si->users);
}

#else 
static inline struct swap_info_struct *swp_swap_info(swp_entry_t entry)
{
	return NULL;
}

static inline struct swap_info_struct *get_swap_device(swp_entry_t entry)
{
	return NULL;
}

static inline void put_swap_device(struct swap_info_struct *si)
{
}

#define get_nr_swap_pages()			0L
#define total_swap_pages			0L
#define total_swapcache_pages()			0UL
#define vm_swap_full()				0

#define si_swapinfo(val) \
	do { (val)->freeswap = (val)->totalswap = 0; } while (0)

#define free_page_and_swap_cache(page) \
	put_page(page)
#define free_pages_and_swap_cache(pages, nr) \
	release_pages((pages), (nr));

static inline void free_swap_and_cache_nr(swp_entry_t entry, int nr)
{
}

static inline void free_swap_cache(struct folio *folio)
{
}

static inline int add_swap_count_continuation(swp_entry_t swp, gfp_t gfp_mask)
{
	return 0;
}

static inline void swap_shmem_alloc(swp_entry_t swp, int nr)
{
}

static inline int swap_duplicate(swp_entry_t swp)
{
	return 0;
}

static inline int swapcache_prepare(swp_entry_t swp, int nr)
{
	return 0;
}

static inline void swap_free_nr(swp_entry_t entry, int nr_pages)
{
}

static inline void put_swap_folio(struct folio *folio, swp_entry_t swp)
{
}

static inline int __swap_count(swp_entry_t entry)
{
	return 0;
}

static inline int swap_swapcount(struct swap_info_struct *si, swp_entry_t entry)
{
	return 0;
}

static inline int swp_swapcount(swp_entry_t entry)
{
	return 0;
}

static inline swp_entry_t folio_alloc_swap(struct folio *folio)
{
	swp_entry_t entry;
	entry.val = 0;
	return entry;
}

static inline bool folio_free_swap(struct folio *folio)
{
	return false;
}

static inline int add_swap_extent(struct swap_info_struct *sis,
				  unsigned long start_page,
				  unsigned long nr_pages, sector_t start_block)
{
	return -EINVAL;
}
#endif 

static inline void free_swap_and_cache(swp_entry_t entry)
{
	free_swap_and_cache_nr(entry, 1);
}

static inline void swap_free(swp_entry_t entry)
{
	swap_free_nr(entry, 1);
}

#ifdef CONFIG_MEMCG
extern void _trace_android_vh_use_vm_swappiness(bool *use_vm_swappiness);

static inline int mem_cgroup_swappiness(struct mem_cgroup *memcg)
{
	bool use_vm_swappiness = false;

	_trace_android_vh_use_vm_swappiness(&use_vm_swappiness);
	if (use_vm_swappiness)
		return READ_ONCE(vm_swappiness);

	
	if (cgroup_subsys_on_dfl(memory_cgrp_subsys))
		return READ_ONCE(vm_swappiness);

	
	if (mem_cgroup_disabled() || mem_cgroup_is_root(memcg))
		return READ_ONCE(vm_swappiness);

	return READ_ONCE(memcg->swappiness);
}
#else
static inline int mem_cgroup_swappiness(struct mem_cgroup *mem)
{
	return READ_ONCE(vm_swappiness);
}
#endif

#if defined(CONFIG_SWAP) && defined(CONFIG_MEMCG) && defined(CONFIG_BLK_CGROUP)
void __folio_throttle_swaprate(struct folio *folio, gfp_t gfp);
static inline void folio_throttle_swaprate(struct folio *folio, gfp_t gfp)
{
	if (mem_cgroup_disabled())
		return;
	__folio_throttle_swaprate(folio, gfp);
}
#else
static inline void folio_throttle_swaprate(struct folio *folio, gfp_t gfp)
{
}
#endif

#if defined(CONFIG_MEMCG) && defined(CONFIG_SWAP)
void mem_cgroup_swapout(struct folio *folio, swp_entry_t entry);
int __mem_cgroup_try_charge_swap(struct folio *folio, swp_entry_t entry);
static inline int mem_cgroup_try_charge_swap(struct folio *folio,
		swp_entry_t entry)
{
	if (mem_cgroup_disabled())
		return 0;
	return __mem_cgroup_try_charge_swap(folio, entry);
}

extern void __mem_cgroup_uncharge_swap(swp_entry_t entry, unsigned int nr_pages);
static inline void mem_cgroup_uncharge_swap(swp_entry_t entry, unsigned int nr_pages)
{
	if (mem_cgroup_disabled())
		return;
	__mem_cgroup_uncharge_swap(entry, nr_pages);
}

extern long mem_cgroup_get_nr_swap_pages(struct mem_cgroup *memcg);
extern bool mem_cgroup_swap_full(struct folio *folio);
#else
static inline void mem_cgroup_swapout(struct folio *folio, swp_entry_t entry)
{
}

static inline int mem_cgroup_try_charge_swap(struct folio *folio,
					     swp_entry_t entry)
{
	return 0;
}

static inline void mem_cgroup_uncharge_swap(swp_entry_t entry,
					    unsigned int nr_pages)
{
}

static inline long mem_cgroup_get_nr_swap_pages(struct mem_cgroup *memcg)
{
	return get_nr_swap_pages();
}

static inline bool mem_cgroup_swap_full(struct folio *folio)
{
	return vm_swap_full();
}
#endif

#endif 
#endif 
