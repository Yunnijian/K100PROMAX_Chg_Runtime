/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <linux/mm_types_task.h>

#include <linux/auxvec.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rbtree.h>
#include <linux/maple_tree.h>
#include <linux/rwsem.h>
#include <linux/completion.h>
#include <linux/cpumask.h>
#include <linux/uprobes.h>
#include <linux/rcupdate.h>
#include <linux/page-flags-layout.h>
#include <linux/workqueue.h>
#include <linux/seqlock.h>
#include <linux/percpu_counter.h>
#include <linux/types.h>
#include <linux/android_kabi.h>

#include <asm/mmu.h>

#ifndef AT_VECTOR_SIZE_ARCH
#define AT_VECTOR_SIZE_ARCH 0
#endif
#define AT_VECTOR_SIZE (2*(AT_VECTOR_SIZE_ARCH + AT_VECTOR_SIZE_BASE + 1))

#define INIT_PASID	0

struct address_space;
struct mem_cgroup;


#ifdef CONFIG_HAVE_ALIGNED_STRUCT_PAGE
#define _struct_page_alignment	__aligned(2 * sizeof(unsigned long))
#else
#define _struct_page_alignment	__aligned(sizeof(unsigned long))
#endif

struct page {
	unsigned long flags;		
	
	union {
		struct {	
			
			union {
				struct list_head lru;

				
				struct {
					
					void *__filler;
					
					unsigned int mlock_count;
				};

				
				struct list_head buddy_list;
				struct list_head pcp_list;
			};
			
			struct address_space *mapping;
			union {
				pgoff_t index;		
				unsigned long share;	
			};
			
			unsigned long private;
		};
		struct {	
			
			unsigned long pp_magic;
			struct page_pool *pp;
			unsigned long _pp_mapping_pad;
			unsigned long dma_addr;
			atomic_long_t pp_ref_count;
		};
		struct {	
			unsigned long compound_head;	
		};
		struct {	
			
			struct dev_pagemap *pgmap;
			void *zone_device_data;
			
		};

		
		struct rcu_head rcu_head;
	};

	union {		
		
		unsigned int page_type;

		
		atomic_t _mapcount;
	};

	
	atomic_t _refcount;

#ifdef CONFIG_MEMCG
	unsigned long memcg_data;
#elif defined(CONFIG_SLAB_OBJ_EXT)
	unsigned long _unused_slab_obj_exts;
#endif

	
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			
#endif 

#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
	int _last_cpupid;
#endif

#ifdef CONFIG_KMSAN
	
	struct page *kmsan_shadow;
	struct page *kmsan_origin;
#endif
} _struct_page_alignment;


struct encoded_page;

#define ENCODED_PAGE_BITS			3ul


#define ENCODED_PAGE_BIT_DELAY_RMAP		1ul


#define ENCODED_PAGE_BIT_NR_PAGES_NEXT		2ul

static __always_inline struct encoded_page *encode_page(struct page *page, unsigned long flags)
{
	BUILD_BUG_ON(flags > ENCODED_PAGE_BITS);
	return (struct encoded_page *)(flags | (unsigned long)page);
}

static inline unsigned long encoded_page_flags(struct encoded_page *page)
{
	return ENCODED_PAGE_BITS & (unsigned long)page;
}

static inline struct page *encoded_page_ptr(struct encoded_page *page)
{
	return (struct page *)(~ENCODED_PAGE_BITS & (unsigned long)page);
}

static __always_inline struct encoded_page *encode_nr_pages(unsigned long nr)
{
	VM_WARN_ON_ONCE((nr << 2) >> 2 != nr);
	return (struct encoded_page *)(nr << 2);
}

static __always_inline unsigned long encoded_nr_pages(struct encoded_page *page)
{
	return ((unsigned long)page) >> 2;
}


typedef struct {
	unsigned long val;
} swp_entry_t;


struct folio {
	
	union {
		struct {
	
			unsigned long flags;
			union {
				struct list_head lru;
	
				struct {
					void *__filler;
	
					unsigned int mlock_count;
	
				};
	
			};
			struct address_space *mapping;
			pgoff_t index;
			union {
				void *private;
				swp_entry_t swap;
			};
			atomic_t _mapcount;
			atomic_t _refcount;
#ifdef CONFIG_MEMCG
			unsigned long memcg_data;
#elif defined(CONFIG_SLAB_OBJ_EXT)
			unsigned long _unused_slab_obj_exts;
#endif
#if defined(WANT_PAGE_VIRTUAL)
			void *virtual;
#endif
#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
			int _last_cpupid;
#endif
	
		};
		struct page page;
	};
	union {
		struct {
			unsigned long _flags_1;
			unsigned long _head_1;
	
			atomic_t _large_mapcount;
			atomic_t _entire_mapcount;
			atomic_t _nr_pages_mapped;
			atomic_t _pincount;
#ifdef CONFIG_64BIT
			unsigned int _folio_nr_pages;
#endif
	
		};
		struct page __page_1;
	};
	union {
		struct {
			unsigned long _flags_2;
			unsigned long _head_2;
	
			void *_hugetlb_subpool;
			void *_hugetlb_cgroup;
			void *_hugetlb_cgroup_rsvd;
			void *_hugetlb_hwpoison;
	
		};
		struct {
			unsigned long _flags_2a;
			unsigned long _head_2a;
	
			struct list_head _deferred_list;
	
		};
		struct page __page_2;
	};
};

#define FOLIO_MATCH(pg, fl)						\
	static_assert(offsetof(struct page, pg) == offsetof(struct folio, fl))
FOLIO_MATCH(flags, flags);
FOLIO_MATCH(lru, lru);
FOLIO_MATCH(mapping, mapping);
FOLIO_MATCH(compound_head, lru);
FOLIO_MATCH(index, index);
FOLIO_MATCH(private, private);
FOLIO_MATCH(_mapcount, _mapcount);
FOLIO_MATCH(_refcount, _refcount);
#ifdef CONFIG_MEMCG
FOLIO_MATCH(memcg_data, memcg_data);
#endif
#if defined(WANT_PAGE_VIRTUAL)
FOLIO_MATCH(virtual, virtual);
#endif
#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
FOLIO_MATCH(_last_cpupid, _last_cpupid);
#endif
#undef FOLIO_MATCH
#define FOLIO_MATCH(pg, fl)						\
	static_assert(offsetof(struct folio, fl) ==			\
			offsetof(struct page, pg) + sizeof(struct page))
FOLIO_MATCH(flags, _flags_1);
FOLIO_MATCH(compound_head, _head_1);
#undef FOLIO_MATCH
#define FOLIO_MATCH(pg, fl)						\
	static_assert(offsetof(struct folio, fl) ==			\
			offsetof(struct page, pg) + 2 * sizeof(struct page))
FOLIO_MATCH(flags, _flags_2);
FOLIO_MATCH(compound_head, _head_2);
FOLIO_MATCH(flags, _flags_2a);
FOLIO_MATCH(compound_head, _head_2a);
#undef FOLIO_MATCH


struct ptdesc {
	unsigned long __page_flags;

	union {
		struct rcu_head pt_rcu_head;
		struct list_head pt_list;
		struct {
			unsigned long _pt_pad_1;
			pgtable_t pmd_huge_pte;
		};
	};
	unsigned long __page_mapping;

	union {
		pgoff_t pt_index;
		struct mm_struct *pt_mm;
		atomic_t pt_frag_refcount;
#ifdef CONFIG_HUGETLB_PMD_PAGE_TABLE_SHARING
		atomic_t pt_share_count;
#endif
	};

	union {
		unsigned long _pt_pad_2;
#if ALLOC_SPLIT_PTLOCKS
		spinlock_t *ptl;
#else
		spinlock_t ptl;
#endif
	};
	unsigned int __page_type;
	atomic_t __page_refcount;
#ifdef CONFIG_MEMCG
	unsigned long pt_memcg_data;
#endif
};

#define TABLE_MATCH(pg, pt)						\
	static_assert(offsetof(struct page, pg) == offsetof(struct ptdesc, pt))
TABLE_MATCH(flags, __page_flags);
TABLE_MATCH(compound_head, pt_list);
TABLE_MATCH(compound_head, _pt_pad_1);
TABLE_MATCH(mapping, __page_mapping);
TABLE_MATCH(index, pt_index);
TABLE_MATCH(rcu_head, pt_rcu_head);
TABLE_MATCH(page_type, __page_type);
TABLE_MATCH(_refcount, __page_refcount);
#ifdef CONFIG_MEMCG
TABLE_MATCH(memcg_data, pt_memcg_data);
#endif
#undef TABLE_MATCH
static_assert(sizeof(struct ptdesc) <= sizeof(struct page));

#define ptdesc_page(pt)			(_Generic((pt),			\
	const struct ptdesc *:		(const struct page *)(pt),	\
	struct ptdesc *:		(struct page *)(pt)))

#define ptdesc_folio(pt)		(_Generic((pt),			\
	const struct ptdesc *:		(const struct folio *)(pt),	\
	struct ptdesc *:		(struct folio *)(pt)))

#define page_ptdesc(p)			(_Generic((p),			\
	const struct page *:		(const struct ptdesc *)(p),	\
	struct page *:			(struct ptdesc *)(p)))

#ifdef CONFIG_HUGETLB_PMD_PAGE_TABLE_SHARING
static inline void ptdesc_pmd_pts_init(struct ptdesc *ptdesc)
{
	atomic_set(&ptdesc->pt_share_count, 0);
}

static inline void ptdesc_pmd_pts_inc(struct ptdesc *ptdesc)
{
	atomic_inc(&ptdesc->pt_share_count);
}

static inline void ptdesc_pmd_pts_dec(struct ptdesc *ptdesc)
{
	atomic_dec(&ptdesc->pt_share_count);
}

static inline int ptdesc_pmd_pts_count(struct ptdesc *ptdesc)
{
	return atomic_read(&ptdesc->pt_share_count);
}
#else
static inline void ptdesc_pmd_pts_init(struct ptdesc *ptdesc)
{
}
#endif


#define STRUCT_PAGE_MAX_SHIFT	(order_base_2(sizeof(struct page)))

#define PAGE_FRAG_CACHE_MAX_SIZE	__ALIGN_MASK(32768, ~PAGE_MASK)
#define PAGE_FRAG_CACHE_MAX_ORDER	get_order(PAGE_FRAG_CACHE_MAX_SIZE)


#define page_private(page)		((page)->private)

static inline void set_page_private(struct page *page, unsigned long private)
{
	page->private = private;
}

static inline void *folio_get_private(struct folio *folio)
{
	return folio->private;
}

struct page_frag_cache {
	void * va;
#if (PAGE_SIZE < PAGE_FRAG_CACHE_MAX_SIZE)
	__u16 offset;
	__u16 size;
#else
	__u32 offset;
#endif
	
	unsigned int		pagecnt_bias;
	bool pfmemalloc;
};

typedef unsigned long vm_flags_t;


typedef struct { unsigned long v; } freeptr_t;


struct vm_region {
	struct rb_node	vm_rb;		
	vm_flags_t	vm_flags;	
	unsigned long	vm_start;	
	unsigned long	vm_end;		
	unsigned long	vm_top;		
	unsigned long	vm_pgoff;	
	struct file	*vm_file;	

	int		vm_usage;	
	bool		vm_icache_flushed : 1; 
};

#ifdef CONFIG_USERFAULTFD
#define NULL_VM_UFFD_CTX ((struct vm_userfaultfd_ctx) { NULL, })
struct vm_userfaultfd_ctx {
	struct userfaultfd_ctx *ctx;
};
#else 
#define NULL_VM_UFFD_CTX ((struct vm_userfaultfd_ctx) {})
struct vm_userfaultfd_ctx {};
#endif 

struct anon_vma_name {
	struct kref kref;
	
	char name[];
};

#ifdef CONFIG_ANON_VMA_NAME

struct anon_vma_name *anon_vma_name(struct vm_area_struct *vma);
struct anon_vma_name *anon_vma_name_alloc(const char *name);
void anon_vma_name_free(struct kref *kref);
#else 
static inline struct anon_vma_name *anon_vma_name(struct vm_area_struct *vma)
{
	return NULL;
}

static inline struct anon_vma_name *anon_vma_name_alloc(const char *name)
{
	return NULL;
}
#endif

#define VMA_LOCK_OFFSET	0x40000000
#define VMA_REF_LIMIT	(VMA_LOCK_OFFSET - 1)

struct vma_numab_state {
	
	unsigned long next_scan;

	
	unsigned long pids_active_reset;

	
	unsigned long pids_active[2];

	
	int start_scan_seq;

	
	int prev_scan_seq;
};


struct vm_area_struct {
	

	union {
		struct {
			
			unsigned long vm_start;
			unsigned long vm_end;
		};
		freeptr_t vm_freeptr; 
	};

	
	struct mm_struct *vm_mm;
	pgprot_t vm_page_prot;          

	
	union {
		const vm_flags_t vm_flags;
		vm_flags_t __private __vm_flags;
	};

#ifdef CONFIG_PER_VMA_LOCK
	
	unsigned int vm_lock_seq;
#endif
	
	struct list_head anon_vma_chain; 
	struct anon_vma *anon_vma;	

	
	const struct vm_operations_struct *vm_ops;

	
	unsigned long vm_pgoff;		
	struct file * vm_file;		
	void * vm_private_data;		

#ifdef CONFIG_SWAP
	atomic_long_t swap_readahead_info;
#endif
#ifndef CONFIG_MMU
	struct vm_region *vm_region;	
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *vm_policy;	
#endif
#ifdef CONFIG_NUMA_BALANCING
	struct vma_numab_state *numab_state;	
#endif
#ifdef CONFIG_PER_VMA_LOCK
	
	refcount_t vm_refcnt ____cacheline_aligned_in_smp;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map vmlock_dep_map;
#endif
#endif
	
	struct {
		struct rb_node rb;
		unsigned long rb_subtree_last;
	} shared;
#ifdef CONFIG_ANON_VMA_NAME
	
	struct anon_vma_name *anon_name;
#endif
	struct vm_userfaultfd_ctx vm_userfaultfd_ctx;

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	ANDROID_KABI_RESERVE(3);
	ANDROID_KABI_RESERVE(4);
} __randomize_layout;

#ifdef CONFIG_NUMA
#define vma_policy(vma) ((vma)->vm_policy)
#else
#define vma_policy(vma) NULL
#endif

#ifdef CONFIG_SCHED_MM_CID
struct mm_cid {
	u64 time;
	int cid;
};
#endif

struct kioctx_table;
struct iommu_mm_data;
struct mm_struct {
	struct {
		
		struct {
			
			atomic_t mm_count;
		} ____cacheline_aligned_in_smp;

		struct maple_tree mm_mt;

		unsigned long mmap_base;	
		unsigned long mmap_legacy_base;	
#ifdef CONFIG_HAVE_ARCH_COMPAT_MMAP_BASES
		
		unsigned long mmap_compat_base;
		unsigned long mmap_compat_legacy_base;
#endif
		unsigned long task_size;	
		pgd_t * pgd;

#ifdef CONFIG_MEMBARRIER
		
		atomic_t membarrier_state;
#endif

		
		atomic_t mm_users;

#ifdef CONFIG_SCHED_MM_CID
		
		struct mm_cid __percpu *pcpu_cid;
		
		unsigned long mm_cid_next_scan;
#endif
#ifdef CONFIG_MMU
		atomic_long_t pgtables_bytes;	
#endif
		int map_count;			

		spinlock_t page_table_lock; 
		
		struct rw_semaphore mmap_lock;

		struct list_head mmlist; 
#ifdef CONFIG_PER_VMA_LOCK
		struct rcuwait vma_writer_wait;
		
		seqcount_t mm_lock_seq;
#endif


		unsigned long hiwater_rss; 
		unsigned long hiwater_vm;  

		unsigned long total_vm;	   
		unsigned long locked_vm;   
		atomic64_t    pinned_vm;   
		unsigned long data_vm;	   
		unsigned long exec_vm;	   
		unsigned long stack_vm;	   
		unsigned long def_flags;

		
		seqcount_t write_protect_seq;

		spinlock_t arg_lock; 

		unsigned long start_code, end_code, start_data, end_data;
		unsigned long start_brk, brk, start_stack;
		unsigned long arg_start, arg_end, env_start, env_end;

		unsigned long saved_auxv[AT_VECTOR_SIZE]; 

		struct percpu_counter rss_stat[NR_MM_COUNTERS];

		struct linux_binfmt *binfmt;

		
		mm_context_t context;

		unsigned long flags; 

#ifdef CONFIG_AIO
		spinlock_t			ioctx_lock;
		struct kioctx_table __rcu	*ioctx_table;
#endif
#ifdef CONFIG_MEMCG
		
		struct task_struct __rcu *owner;
#endif
		struct user_namespace *user_ns;

		
		struct file __rcu *exe_file;
#ifdef CONFIG_MMU_NOTIFIER
		struct mmu_notifier_subscriptions *notifier_subscriptions;
#endif
#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && !defined(CONFIG_SPLIT_PMD_PTLOCKS)
		pgtable_t pmd_huge_pte; 
#endif
#ifdef CONFIG_NUMA_BALANCING
		
		unsigned long numa_next_scan;

		
		unsigned long numa_scan_offset;

		
		int numa_scan_seq;
#endif
		
		atomic_t tlb_flush_pending;
#ifdef CONFIG_ARCH_WANT_BATCHED_UNMAP_TLB_FLUSH
		
		atomic_t tlb_flush_batched;
#endif
		struct uprobes_state uprobes_state;
#ifdef CONFIG_PREEMPT_RT
		struct rcu_head delayed_drop;
#endif
#ifdef CONFIG_HUGETLB_PAGE
		atomic_long_t hugetlb_usage;
#endif
		struct work_struct async_put_work;

#ifdef CONFIG_IOMMU_MM_DATA
		struct iommu_mm_data *iommu_mm;
#endif
#ifdef CONFIG_KSM
		
		unsigned long ksm_merging_pages;
		
		unsigned long ksm_rmap_items;
		
		atomic_long_t ksm_zero_pages;
#endif 
#ifdef CONFIG_LRU_GEN_WALKS_MMU
		struct {
			
			struct list_head list;
			
			unsigned long bitmap;
#ifdef CONFIG_MEMCG
			
			struct mem_cgroup *memcg;
#endif
		} lru_gen;
#endif 

		ANDROID_KABI_USE(1, struct task_dma_buf_info *dmabuf_info);
	} __randomize_layout;

	
	unsigned long cpu_bitmap[];
};

#define MM_MT_FLAGS	(MT_FLAGS_ALLOC_RANGE | MT_FLAGS_LOCK_EXTERN | \
			 MT_FLAGS_USE_RCU)
extern struct mm_struct init_mm;


static inline void mm_init_cpumask(struct mm_struct *mm)
{
	unsigned long cpu_bitmap = (unsigned long)mm;

	cpu_bitmap += offsetof(struct mm_struct, cpu_bitmap);
	cpumask_clear((struct cpumask *)cpu_bitmap);
}


static inline cpumask_t *mm_cpumask(struct mm_struct *mm)
{
	return (struct cpumask *)&mm->cpu_bitmap;
}

#ifdef CONFIG_LRU_GEN

struct lru_gen_mm_list {
	
	struct list_head fifo;
	
	spinlock_t lock;
};

#endif 

#ifdef CONFIG_LRU_GEN_WALKS_MMU

void lru_gen_add_mm(struct mm_struct *mm);
void lru_gen_del_mm(struct mm_struct *mm);
void lru_gen_migrate_mm(struct mm_struct *mm);

static inline void lru_gen_init_mm(struct mm_struct *mm)
{
	INIT_LIST_HEAD(&mm->lru_gen.list);
	mm->lru_gen.bitmap = 0;
#ifdef CONFIG_MEMCG
	mm->lru_gen.memcg = NULL;
#endif
}

static inline void lru_gen_use_mm(struct mm_struct *mm)
{
	
	WRITE_ONCE(mm->lru_gen.bitmap, -1);
}

#else 

static inline void lru_gen_add_mm(struct mm_struct *mm)
{
}

static inline void lru_gen_del_mm(struct mm_struct *mm)
{
}

static inline void lru_gen_migrate_mm(struct mm_struct *mm)
{
}

static inline void lru_gen_init_mm(struct mm_struct *mm)
{
}

static inline void lru_gen_use_mm(struct mm_struct *mm)
{
}

#endif 

struct vma_iterator {
	struct ma_state mas;
};

#define VMA_ITERATOR(name, __mm, __addr)				\
	struct vma_iterator name = {					\
		.mas = {						\
			.tree = &(__mm)->mm_mt,				\
			.index = __addr,				\
			.node = NULL,					\
			.status = ma_start,				\
		},							\
	}

static inline void vma_iter_init(struct vma_iterator *vmi,
		struct mm_struct *mm, unsigned long addr)
{
	mas_init(&vmi->mas, &mm->mm_mt, addr);
}

#ifdef CONFIG_SCHED_MM_CID

enum mm_cid_state {
	MM_CID_UNSET = -1U,		
	MM_CID_LAZY_PUT = (1U << 31),
};

static inline bool mm_cid_is_unset(int cid)
{
	return cid == MM_CID_UNSET;
}

static inline bool mm_cid_is_lazy_put(int cid)
{
	return !mm_cid_is_unset(cid) && (cid & MM_CID_LAZY_PUT);
}

static inline bool mm_cid_is_valid(int cid)
{
	return !(cid & MM_CID_LAZY_PUT);
}

static inline int mm_cid_set_lazy_put(int cid)
{
	return cid | MM_CID_LAZY_PUT;
}

static inline int mm_cid_clear_lazy_put(int cid)
{
	return cid & ~MM_CID_LAZY_PUT;
}


static inline cpumask_t *mm_cidmask(struct mm_struct *mm)
{
	unsigned long cid_bitmap = (unsigned long)mm;

	cid_bitmap += offsetof(struct mm_struct, cpu_bitmap);
	
	cid_bitmap += cpumask_size();
	return (struct cpumask *)cid_bitmap;
}

static inline void mm_init_cid(struct mm_struct *mm)
{
	int i;

	for_each_possible_cpu(i) {
		struct mm_cid *pcpu_cid = per_cpu_ptr(mm->pcpu_cid, i);

		pcpu_cid->cid = MM_CID_UNSET;
		pcpu_cid->time = 0;
	}
	cpumask_clear(mm_cidmask(mm));
}

static inline int mm_alloc_cid_noprof(struct mm_struct *mm)
{
	mm->pcpu_cid = alloc_percpu_noprof(struct mm_cid);
	if (!mm->pcpu_cid)
		return -ENOMEM;
	mm_init_cid(mm);
	return 0;
}
#define mm_alloc_cid(...)	alloc_hooks(mm_alloc_cid_noprof(__VA_ARGS__))

static inline void mm_destroy_cid(struct mm_struct *mm)
{
	free_percpu(mm->pcpu_cid);
	mm->pcpu_cid = NULL;
}

static inline unsigned int mm_cid_size(void)
{
	return cpumask_size();
}
#else 
static inline void mm_init_cid(struct mm_struct *mm) { }
static inline int mm_alloc_cid(struct mm_struct *mm) { return 0; }
static inline void mm_destroy_cid(struct mm_struct *mm) { }
static inline unsigned int mm_cid_size(void)
{
	return 0;
}
#endif 

struct mmu_gather;
extern void tlb_gather_mmu(struct mmu_gather *tlb, struct mm_struct *mm);
extern void tlb_gather_mmu_fullmm(struct mmu_gather *tlb, struct mm_struct *mm);
extern void tlb_finish_mmu(struct mmu_gather *tlb);

struct vm_fault;


typedef __bitwise unsigned int vm_fault_t;


enum vm_fault_reason {
	VM_FAULT_OOM            = (__force vm_fault_t)0x000001,
	VM_FAULT_SIGBUS         = (__force vm_fault_t)0x000002,
	VM_FAULT_MAJOR          = (__force vm_fault_t)0x000004,
	VM_FAULT_HWPOISON       = (__force vm_fault_t)0x000010,
	VM_FAULT_HWPOISON_LARGE = (__force vm_fault_t)0x000020,
	VM_FAULT_SIGSEGV        = (__force vm_fault_t)0x000040,
	VM_FAULT_NOPAGE         = (__force vm_fault_t)0x000100,
	VM_FAULT_LOCKED         = (__force vm_fault_t)0x000200,
	VM_FAULT_RETRY          = (__force vm_fault_t)0x000400,
	VM_FAULT_FALLBACK       = (__force vm_fault_t)0x000800,
	VM_FAULT_DONE_COW       = (__force vm_fault_t)0x001000,
	VM_FAULT_NEEDDSYNC      = (__force vm_fault_t)0x002000,
	VM_FAULT_COMPLETED      = (__force vm_fault_t)0x004000,
	VM_FAULT_HINDEX_MASK    = (__force vm_fault_t)0x0f0000,
};


#define VM_FAULT_SET_HINDEX(x) ((__force vm_fault_t)((x) << 16))
#define VM_FAULT_GET_HINDEX(x) (((__force unsigned int)(x) >> 16) & 0xf)

#define VM_FAULT_ERROR (VM_FAULT_OOM | VM_FAULT_SIGBUS |	\
			VM_FAULT_SIGSEGV | VM_FAULT_HWPOISON |	\
			VM_FAULT_HWPOISON_LARGE | VM_FAULT_FALLBACK)

#define VM_FAULT_RESULT_TRACE \
	{ VM_FAULT_OOM,                 "OOM" },	\
	{ VM_FAULT_SIGBUS,              "SIGBUS" },	\
	{ VM_FAULT_MAJOR,               "MAJOR" },	\
	{ VM_FAULT_HWPOISON,            "HWPOISON" },	\
	{ VM_FAULT_HWPOISON_LARGE,      "HWPOISON_LARGE" },	\
	{ VM_FAULT_SIGSEGV,             "SIGSEGV" },	\
	{ VM_FAULT_NOPAGE,              "NOPAGE" },	\
	{ VM_FAULT_LOCKED,              "LOCKED" },	\
	{ VM_FAULT_RETRY,               "RETRY" },	\
	{ VM_FAULT_FALLBACK,            "FALLBACK" },	\
	{ VM_FAULT_DONE_COW,            "DONE_COW" },	\
	{ VM_FAULT_NEEDDSYNC,           "NEEDDSYNC" },	\
	{ VM_FAULT_COMPLETED,           "COMPLETED" }

struct vm_special_mapping {
	const char *name;	

	
	struct page **pages;

	
	vm_fault_t (*fault)(const struct vm_special_mapping *sm,
				struct vm_area_struct *vma,
				struct vm_fault *vmf);

	int (*mremap)(const struct vm_special_mapping *sm,
		     struct vm_area_struct *new_vma);

	void (*close)(const struct vm_special_mapping *sm,
		      struct vm_area_struct *vma);
};

enum tlb_flush_reason {
	TLB_FLUSH_ON_TASK_SWITCH,
	TLB_REMOTE_SHOOTDOWN,
	TLB_LOCAL_SHOOTDOWN,
	TLB_LOCAL_MM_SHOOTDOWN,
	TLB_REMOTE_SEND_IPI,
	NR_TLB_FLUSH_REASONS,
};


enum fault_flag {
	FAULT_FLAG_WRITE =		1 << 0,
	FAULT_FLAG_MKWRITE =		1 << 1,
	FAULT_FLAG_ALLOW_RETRY =	1 << 2,
	FAULT_FLAG_RETRY_NOWAIT = 	1 << 3,
	FAULT_FLAG_KILLABLE =		1 << 4,
	FAULT_FLAG_TRIED = 		1 << 5,
	FAULT_FLAG_USER =		1 << 6,
	FAULT_FLAG_REMOTE =		1 << 7,
	FAULT_FLAG_INSTRUCTION =	1 << 8,
	FAULT_FLAG_INTERRUPTIBLE =	1 << 9,
	FAULT_FLAG_UNSHARE =		1 << 10,
	FAULT_FLAG_ORIG_PTE_VALID =	1 << 11,
	FAULT_FLAG_VMA_LOCK =		1 << 12,
};

typedef unsigned int __bitwise zap_flags_t;


typedef int __bitwise cydp_t;


#define CYDP_CLEAR_YOUNG		((__force cydp_t)BIT(0))


#define CYDP_CLEAR_DIRTY		((__force cydp_t)BIT(1))



enum {
	
	FOLL_WRITE = 1 << 0,
	
	FOLL_GET = 1 << 1,
	
	FOLL_DUMP = 1 << 2,
	
	FOLL_FORCE = 1 << 3,
	
	FOLL_NOWAIT = 1 << 4,
	
	FOLL_NOFAULT = 1 << 5,
	
	FOLL_HWPOISON = 1 << 6,
	
	FOLL_ANON = 1 << 7,
	
	FOLL_LONGTERM = 1 << 8,
	
	FOLL_SPLIT_PMD = 1 << 9,
	
	FOLL_PCI_P2PDMA = 1 << 10,
	
	FOLL_INTERRUPTIBLE = 1 << 11,
	
	FOLL_HONOR_NUMA_FAULT = 1 << 12,

	
};

#endif 
