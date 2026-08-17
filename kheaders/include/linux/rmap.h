/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RMAP_H
#define _LINUX_RMAP_H


#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/rwsem.h>
#include <linux/memcontrol.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/memremap.h>


struct anon_vma {
	struct anon_vma *root;		
	struct rw_semaphore rwsem;	
	
	atomic_t refcount;

	
	unsigned long num_children;
	
	unsigned long num_active_vmas;

	struct anon_vma *parent;	

	

	
	struct rb_root_cached rb_root;
};


struct anon_vma_chain {
	struct vm_area_struct *vma;
	struct anon_vma *anon_vma;
	struct list_head same_vma;   
	struct rb_node rb;			
	unsigned long rb_subtree_last;
#ifdef CONFIG_DEBUG_VM_RB
	unsigned long cached_vma_start, cached_vma_last;
#endif
};

enum ttu_flags {
	TTU_SPLIT_HUGE_PMD	= 0x4,	
	TTU_IGNORE_MLOCK	= 0x8,	
	TTU_SYNC		= 0x10,	
	TTU_HWPOISON		= 0x20,	
	TTU_BATCH_FLUSH		= 0x40,	
	TTU_RMAP_LOCKED		= 0x80,	
};

#ifdef CONFIG_MMU
static inline void get_anon_vma(struct anon_vma *anon_vma)
{
	atomic_inc(&anon_vma->refcount);
}

void __put_anon_vma(struct anon_vma *anon_vma);

static inline void put_anon_vma(struct anon_vma *anon_vma)
{
	if (atomic_dec_and_test(&anon_vma->refcount))
		__put_anon_vma(anon_vma);
}

static inline void anon_vma_lock_write(struct anon_vma *anon_vma)
{
	down_write(&anon_vma->root->rwsem);
}

static inline int anon_vma_trylock_write(struct anon_vma *anon_vma)
{
	return down_write_trylock(&anon_vma->root->rwsem);
}

static inline void anon_vma_unlock_write(struct anon_vma *anon_vma)
{
	up_write(&anon_vma->root->rwsem);
}

static inline void anon_vma_lock_read(struct anon_vma *anon_vma)
{
	down_read(&anon_vma->root->rwsem);
}

static inline int anon_vma_trylock_read(struct anon_vma *anon_vma)
{
	return down_read_trylock(&anon_vma->root->rwsem);
}

static inline void anon_vma_unlock_read(struct anon_vma *anon_vma)
{
	up_read(&anon_vma->root->rwsem);
}



void anon_vma_init(void);	
int  __anon_vma_prepare(struct vm_area_struct *);
void unlink_anon_vmas(struct vm_area_struct *);
int anon_vma_clone(struct vm_area_struct *, struct vm_area_struct *);
int anon_vma_fork(struct vm_area_struct *, struct vm_area_struct *);

static inline int anon_vma_prepare(struct vm_area_struct *vma)
{
	if (likely(vma->anon_vma))
		return 0;

	return __anon_vma_prepare(vma);
}

static inline void anon_vma_merge(struct vm_area_struct *vma,
				  struct vm_area_struct *next)
{
	VM_BUG_ON_VMA(vma->anon_vma != next->anon_vma, vma);
	unlink_anon_vmas(next);
}

struct anon_vma *folio_get_anon_vma(struct folio *folio);


typedef int __bitwise rmap_t;


#define RMAP_NONE		((__force rmap_t)0)


#define RMAP_EXCLUSIVE		((__force rmap_t)BIT(0))


enum rmap_level {
	RMAP_LEVEL_PTE = 0,
	RMAP_LEVEL_PMD,
};

static inline void __folio_rmap_sanity_checks(struct folio *folio,
		struct page *page, int nr_pages, enum rmap_level level)
{
	
	VM_WARN_ON_FOLIO(folio_test_hugetlb(folio), folio);

	
	VM_WARN_ON_FOLIO(is_zero_folio(folio), folio);

	

	VM_WARN_ON_ONCE(nr_pages <= 0);
	VM_WARN_ON_FOLIO(page_folio(page) != folio, folio);
	VM_WARN_ON_FOLIO(page_folio(page + nr_pages - 1) != folio, folio);

	switch (level) {
	case RMAP_LEVEL_PTE:
		break;
	case RMAP_LEVEL_PMD:
		
		VM_WARN_ON_FOLIO(folio_nr_pages(folio) != HPAGE_PMD_NR, folio);
		VM_WARN_ON_FOLIO(nr_pages != HPAGE_PMD_NR, folio);
		break;
	default:
		VM_WARN_ON_ONCE(true);
	}
}


void folio_move_anon_rmap(struct folio *, struct vm_area_struct *);
void folio_add_anon_rmap_ptes(struct folio *, struct page *, int nr_pages,
		struct vm_area_struct *, unsigned long address, rmap_t flags);
#define folio_add_anon_rmap_pte(folio, page, vma, address, flags) \
	folio_add_anon_rmap_ptes(folio, page, 1, vma, address, flags)
void folio_add_anon_rmap_pmd(struct folio *, struct page *,
		struct vm_area_struct *, unsigned long address, rmap_t flags);
void folio_add_new_anon_rmap(struct folio *, struct vm_area_struct *,
		unsigned long address, rmap_t flags);
void folio_add_file_rmap_ptes(struct folio *, struct page *, int nr_pages,
		struct vm_area_struct *);
#define folio_add_file_rmap_pte(folio, page, vma) \
	folio_add_file_rmap_ptes(folio, page, 1, vma)
void folio_add_file_rmap_pmd(struct folio *, struct page *,
		struct vm_area_struct *);
void folio_remove_rmap_ptes(struct folio *, struct page *, int nr_pages,
		struct vm_area_struct *);
#define folio_remove_rmap_pte(folio, page, vma) \
	folio_remove_rmap_ptes(folio, page, 1, vma)
void folio_remove_rmap_pmd(struct folio *, struct page *,
		struct vm_area_struct *);

void hugetlb_add_anon_rmap(struct folio *, struct vm_area_struct *,
		unsigned long address, rmap_t flags);
void hugetlb_add_new_anon_rmap(struct folio *, struct vm_area_struct *,
		unsigned long address);


static inline int hugetlb_try_dup_anon_rmap(struct folio *folio,
		struct vm_area_struct *vma)
{
	VM_WARN_ON_FOLIO(!folio_test_hugetlb(folio), folio);
	VM_WARN_ON_FOLIO(!folio_test_anon(folio), folio);

	if (PageAnonExclusive(&folio->page)) {
		if (unlikely(folio_needs_cow_for_dma(vma, folio)))
			return -EBUSY;
		ClearPageAnonExclusive(&folio->page);
	}
	atomic_inc(&folio->_entire_mapcount);
	atomic_inc(&folio->_large_mapcount);
	return 0;
}


static inline int hugetlb_try_share_anon_rmap(struct folio *folio)
{
	VM_WARN_ON_FOLIO(!folio_test_hugetlb(folio), folio);
	VM_WARN_ON_FOLIO(!folio_test_anon(folio), folio);
	VM_WARN_ON_FOLIO(!PageAnonExclusive(&folio->page), folio);

	
	if (IS_ENABLED(CONFIG_HAVE_GUP_FAST))
		smp_mb();

	if (unlikely(folio_maybe_dma_pinned(folio)))
		return -EBUSY;
	ClearPageAnonExclusive(&folio->page);

	
	if (IS_ENABLED(CONFIG_HAVE_GUP_FAST))
		smp_mb__after_atomic();
	return 0;
}

static inline void hugetlb_add_file_rmap(struct folio *folio)
{
	VM_WARN_ON_FOLIO(!folio_test_hugetlb(folio), folio);
	VM_WARN_ON_FOLIO(folio_test_anon(folio), folio);

	atomic_inc(&folio->_entire_mapcount);
	atomic_inc(&folio->_large_mapcount);
}

static inline void hugetlb_remove_rmap(struct folio *folio)
{
	VM_WARN_ON_FOLIO(!folio_test_hugetlb(folio), folio);

	atomic_dec(&folio->_entire_mapcount);
	atomic_dec(&folio->_large_mapcount);
}

static __always_inline void __folio_dup_file_rmap(struct folio *folio,
		struct page *page, int nr_pages, enum rmap_level level)
{
	const int orig_nr_pages = nr_pages;

	__folio_rmap_sanity_checks(folio, page, nr_pages, level);

	switch (level) {
	case RMAP_LEVEL_PTE:
		if (!folio_test_large(folio)) {
			atomic_inc(&folio->_mapcount);
			break;
		}

		do {
			atomic_inc(&page->_mapcount);
		} while (page++, --nr_pages > 0);
		atomic_add(orig_nr_pages, &folio->_large_mapcount);
		break;
	case RMAP_LEVEL_PMD:
		atomic_inc(&folio->_entire_mapcount);
		atomic_inc(&folio->_large_mapcount);
		break;
	}
}


static inline void folio_dup_file_rmap_ptes(struct folio *folio,
		struct page *page, int nr_pages)
{
	__folio_dup_file_rmap(folio, page, nr_pages, RMAP_LEVEL_PTE);
}

static __always_inline void folio_dup_file_rmap_pte(struct folio *folio,
		struct page *page)
{
	__folio_dup_file_rmap(folio, page, 1, RMAP_LEVEL_PTE);
}


static inline void folio_dup_file_rmap_pmd(struct folio *folio,
		struct page *page)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	__folio_dup_file_rmap(folio, page, HPAGE_PMD_NR, RMAP_LEVEL_PTE);
#else
	WARN_ON_ONCE(true);
#endif
}

static __always_inline int __folio_try_dup_anon_rmap(struct folio *folio,
		struct page *page, int nr_pages, struct vm_area_struct *src_vma,
		enum rmap_level level)
{
	const int orig_nr_pages = nr_pages;
	bool maybe_pinned;
	int i;

	VM_WARN_ON_FOLIO(!folio_test_anon(folio), folio);
	__folio_rmap_sanity_checks(folio, page, nr_pages, level);

	
	maybe_pinned = likely(!folio_is_device_private(folio)) &&
		       unlikely(folio_needs_cow_for_dma(src_vma, folio));

	
	switch (level) {
	case RMAP_LEVEL_PTE:
		if (unlikely(maybe_pinned)) {
			for (i = 0; i < nr_pages; i++)
				if (PageAnonExclusive(page + i))
					return -EBUSY;
		}

		if (!folio_test_large(folio)) {
			if (PageAnonExclusive(page))
				ClearPageAnonExclusive(page);
			atomic_inc(&folio->_mapcount);
			break;
		}

		do {
			if (PageAnonExclusive(page))
				ClearPageAnonExclusive(page);
			atomic_inc(&page->_mapcount);
		} while (page++, --nr_pages > 0);
		atomic_add(orig_nr_pages, &folio->_large_mapcount);
		break;
	case RMAP_LEVEL_PMD:
		if (PageAnonExclusive(page)) {
			if (unlikely(maybe_pinned))
				return -EBUSY;
			ClearPageAnonExclusive(page);
		}
		atomic_inc(&folio->_entire_mapcount);
		atomic_inc(&folio->_large_mapcount);
		break;
	}
	return 0;
}


static inline int folio_try_dup_anon_rmap_ptes(struct folio *folio,
		struct page *page, int nr_pages, struct vm_area_struct *src_vma)
{
	return __folio_try_dup_anon_rmap(folio, page, nr_pages, src_vma,
					 RMAP_LEVEL_PTE);
}

static __always_inline int folio_try_dup_anon_rmap_pte(struct folio *folio,
		struct page *page, struct vm_area_struct *src_vma)
{
	return __folio_try_dup_anon_rmap(folio, page, 1, src_vma,
					 RMAP_LEVEL_PTE);
}


static inline int folio_try_dup_anon_rmap_pmd(struct folio *folio,
		struct page *page, struct vm_area_struct *src_vma)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	return __folio_try_dup_anon_rmap(folio, page, HPAGE_PMD_NR, src_vma,
					 RMAP_LEVEL_PMD);
#else
	WARN_ON_ONCE(true);
	return -EBUSY;
#endif
}

static __always_inline int __folio_try_share_anon_rmap(struct folio *folio,
		struct page *page, int nr_pages, enum rmap_level level)
{
	VM_WARN_ON_FOLIO(!folio_test_anon(folio), folio);
	VM_WARN_ON_FOLIO(!PageAnonExclusive(page), folio);
	__folio_rmap_sanity_checks(folio, page, nr_pages, level);

	
	if (unlikely(folio_is_device_private(folio))) {
		ClearPageAnonExclusive(page);
		return 0;
	}

	

	
	if (IS_ENABLED(CONFIG_HAVE_GUP_FAST))
		smp_mb();

	if (unlikely(folio_maybe_dma_pinned(folio)))
		return -EBUSY;
	ClearPageAnonExclusive(page);

	
	if (IS_ENABLED(CONFIG_HAVE_GUP_FAST))
		smp_mb__after_atomic();
	return 0;
}


static inline int folio_try_share_anon_rmap_pte(struct folio *folio,
		struct page *page)
{
	return __folio_try_share_anon_rmap(folio, page, 1, RMAP_LEVEL_PTE);
}


static inline int folio_try_share_anon_rmap_pmd(struct folio *folio,
		struct page *page)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	return __folio_try_share_anon_rmap(folio, page, HPAGE_PMD_NR,
					   RMAP_LEVEL_PMD);
#else
	WARN_ON_ONCE(true);
	return -EBUSY;
#endif
}


int folio_referenced(struct folio *, int is_locked,
			struct mem_cgroup *memcg, unsigned long *vm_flags);

void try_to_migrate(struct folio *folio, enum ttu_flags flags);
void try_to_unmap(struct folio *, enum ttu_flags flags);

int make_device_exclusive_range(struct mm_struct *mm, unsigned long start,
				unsigned long end, struct page **pages,
				void *arg);


#define PVMW_SYNC		(1 << 0)

#define PVMW_MIGRATION		(1 << 1)




#define PVMW_PGTABLE_CROSSED	(1 << 16)

struct page_vma_mapped_walk {
	unsigned long pfn;
	unsigned long nr_pages;
	pgoff_t pgoff;
	struct vm_area_struct *vma;
	unsigned long address;
	pmd_t *pmd;
	pte_t *pte;
	spinlock_t *ptl;
	unsigned int flags;
};

#define DEFINE_FOLIO_VMA_WALK(name, _folio, _vma, _address, _flags)	\
	struct page_vma_mapped_walk name = {				\
		.pfn = folio_pfn(_folio),				\
		.nr_pages = folio_nr_pages(_folio),			\
		.pgoff = folio_pgoff(_folio),				\
		.vma = _vma,						\
		.address = _address,					\
		.flags = _flags,					\
	}

static inline void page_vma_mapped_walk_done(struct page_vma_mapped_walk *pvmw)
{
	
	if (pvmw->pte && !is_vm_hugetlb_page(pvmw->vma))
		pte_unmap(pvmw->pte);
	if (pvmw->ptl)
		spin_unlock(pvmw->ptl);
}


static inline void
page_vma_mapped_walk_restart(struct page_vma_mapped_walk *pvmw)
{
	WARN_ON_ONCE(!pvmw->pmd && !pvmw->pte);

	if (likely(pvmw->ptl))
		spin_unlock(pvmw->ptl);
	else
		WARN_ON_ONCE(1);

	pvmw->ptl = NULL;
	pvmw->pmd = NULL;
	pvmw->pte = NULL;
}

bool page_vma_mapped_walk(struct page_vma_mapped_walk *pvmw);


unsigned long page_address_in_vma(struct page *, struct vm_area_struct *);


int folio_mkclean(struct folio *);

int pfn_mkclean_range(unsigned long pfn, unsigned long nr_pages, pgoff_t pgoff,
		      struct vm_area_struct *vma);

enum rmp_flags {
	RMP_LOCKED		= 1 << 0,
	RMP_USE_SHARED_ZEROPAGE	= 1 << 1,
};

void remove_migration_ptes(struct folio *src, struct folio *dst, int flags);


struct rmap_walk_control {
	void *arg;
	bool try_lock;
	bool contended;
	
	bool (*rmap_one)(struct folio *folio, struct vm_area_struct *vma,
					unsigned long addr, void *arg);
	int (*done)(struct folio *folio);
	struct anon_vma *(*anon_lock)(struct folio *folio,
				      struct rmap_walk_control *rwc);
	bool (*invalid_vma)(struct vm_area_struct *vma, void *arg);
};

void rmap_walk(struct folio *folio, struct rmap_walk_control *rwc);
void rmap_walk_locked(struct folio *folio, struct rmap_walk_control *rwc);
struct anon_vma *folio_lock_anon_vma_read(struct folio *folio,
					  struct rmap_walk_control *rwc);

#else	

#define anon_vma_init()		do {} while (0)
#define anon_vma_prepare(vma)	(0)

static inline int folio_referenced(struct folio *folio, int is_locked,
				  struct mem_cgroup *memcg,
				  unsigned long *vm_flags)
{
	*vm_flags = 0;
	return 0;
}

static inline void try_to_unmap(struct folio *folio, enum ttu_flags flags)
{
}

static inline int folio_mkclean(struct folio *folio)
{
	return 0;
}
#endif	

#endif	
