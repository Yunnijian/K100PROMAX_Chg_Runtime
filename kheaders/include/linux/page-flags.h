/* SPDX-License-Identifier: GPL-2.0 */


#ifndef PAGE_FLAGS_H
#define PAGE_FLAGS_H

#include <linux/types.h>
#include <linux/bug.h>
#include <linux/mmdebug.h>
#ifndef __GENERATING_BOUNDS_H
#include <linux/mm_types.h>
#include <generated/bounds.h>
#endif 




enum pageflags {
	PG_locked,		
	PG_writeback,		
	PG_referenced,
	PG_uptodate,
	PG_dirty,
	PG_lru,
	PG_head,		
	PG_waiters,		
	PG_active,
	PG_workingset,
	PG_error,
	PG_owner_priv_1,	
	PG_owner_2,		
	PG_arch_1,
	PG_reserved,
	PG_private,		
	PG_private_2,		
	PG_reclaim,		
	PG_swapbacked,		
	PG_unevictable,		
	PG_dropbehind,		
#ifdef CONFIG_MMU
	PG_mlocked,		
#endif
#ifdef CONFIG_MEMORY_FAILURE
	PG_hwpoison,		
#endif
#if defined(CONFIG_PAGE_IDLE_FLAG) && defined(CONFIG_64BIT)
	PG_young,
	PG_idle,
#endif
#ifdef CONFIG_ARCH_USES_PG_ARCH_2
	PG_arch_2,
#endif
#ifdef CONFIG_ARCH_USES_PG_ARCH_3
	PG_arch_3,
#endif
#ifdef CONFIG_64BIT
	PG_oem_reserved_1,
	PG_oem_reserved_2,
	PG_oem_reserved_3,
	PG_oem_reserved_4,
#endif
	__NR_PAGEFLAGS,

	PG_readahead = PG_reclaim,

	
	PG_swapcache = PG_owner_priv_1, 
	
	PG_checked = PG_owner_priv_1,

	
	PG_anon_exclusive = PG_owner_2,

	
	PG_mappedtodisk = PG_owner_2,

	
	PG_fscache = PG_private_2,	

	
	
	PG_pinned = PG_owner_priv_1,
	
	PG_savepinned = PG_dirty,
	
	PG_foreign = PG_owner_priv_1,
	
	PG_xen_remapped = PG_owner_priv_1,

	
	PG_isolated = PG_reclaim,

	
	PG_reported = PG_uptodate,

#ifdef CONFIG_MEMORY_HOTPLUG
	
	PG_vmemmap_self_hosted = PG_owner_priv_1,
#endif

	

	
	PG_has_hwpoisoned = PG_error,
	PG_large_rmappable = PG_workingset, 
	PG_partially_mapped = PG_reclaim, 
};

#define PAGEFLAGS_MASK		((1UL << NR_PAGEFLAGS) - 1)

#ifndef __GENERATING_BOUNDS_H

#ifdef CONFIG_HUGETLB_PAGE_OPTIMIZE_VMEMMAP
DECLARE_STATIC_KEY_FALSE(hugetlb_optimize_vmemmap_key);


static __always_inline const struct page *page_fixed_fake_head(const struct page *page)
{
	if (!static_branch_unlikely(&hugetlb_optimize_vmemmap_key))
		return page;

	
	if (IS_ALIGNED((unsigned long)page, PAGE_SIZE) &&
	    test_bit(PG_head, &page->flags)) {
		
		unsigned long head = READ_ONCE(page[1].compound_head);

		if (likely(head & 1))
			return (const struct page *)(head - 1);
	}
	return page;
}
#else
static inline const struct page *page_fixed_fake_head(const struct page *page)
{
	return page;
}
#endif

static __always_inline int page_is_fake_head(const struct page *page)
{
	return page_fixed_fake_head(page) != page;
}

static __always_inline unsigned long _compound_head(const struct page *page)
{
	unsigned long head = READ_ONCE(page->compound_head);

	if (unlikely(head & 1))
		return head - 1;
	return (unsigned long)page_fixed_fake_head(page);
}

#define compound_head(page)	((typeof(page))_compound_head(page))


#define page_folio(p)		(_Generic((p),				\
	const struct page *:	(const struct folio *)_compound_head(p), \
	struct page *:		(struct folio *)_compound_head(p)))


#define folio_page(folio, n)	nth_page(&(folio)->page, n)

static __always_inline int PageTail(const struct page *page)
{
	return READ_ONCE(page->compound_head) & 1 || page_is_fake_head(page);
}

static __always_inline int PageCompound(const struct page *page)
{
	return test_bit(PG_head, &page->flags) ||
	       READ_ONCE(page->compound_head) & 1;
}

#define	PAGE_POISON_PATTERN	-1l
static inline int PagePoisoned(const struct page *page)
{
	return READ_ONCE(page->flags) == PAGE_POISON_PATTERN;
}

#ifdef CONFIG_DEBUG_VM
void page_init_poison(struct page *page, size_t size);
#else
static inline void page_init_poison(struct page *page, size_t size)
{
}
#endif

static const unsigned long *const_folio_flags(const struct folio *folio,
		unsigned n)
{
	const struct page *page = &folio->page;

	VM_BUG_ON_PGFLAGS(page->compound_head & 1, page);
	VM_BUG_ON_PGFLAGS(n > 0 && !test_bit(PG_head, &page->flags), page);
	return &page[n].flags;
}

static unsigned long *folio_flags(struct folio *folio, unsigned n)
{
	struct page *page = &folio->page;

	VM_BUG_ON_PGFLAGS(page->compound_head & 1, page);
	VM_BUG_ON_PGFLAGS(n > 0 && !test_bit(PG_head, &page->flags), page);
	return &page[n].flags;
}


#define PF_POISONED_CHECK(page) ({					\
		VM_BUG_ON_PGFLAGS(PagePoisoned(page), page);		\
		page; })
#define PF_ANY(page, enforce)	PF_POISONED_CHECK(page)
#define PF_HEAD(page, enforce)	PF_POISONED_CHECK(compound_head(page))
#define PF_NO_TAIL(page, enforce) ({					\
		VM_BUG_ON_PGFLAGS(enforce && PageTail(page), page);	\
		PF_POISONED_CHECK(compound_head(page)); })
#define PF_NO_COMPOUND(page, enforce) ({				\
		VM_BUG_ON_PGFLAGS(enforce && PageCompound(page), page);	\
		PF_POISONED_CHECK(page); })
#define PF_SECOND(page, enforce) ({					\
		VM_BUG_ON_PGFLAGS(!PageHead(page), page);		\
		PF_POISONED_CHECK(&page[1]); })


#define FOLIO_PF_ANY		0
#define FOLIO_PF_HEAD		0
#define FOLIO_PF_NO_TAIL	0
#define FOLIO_PF_NO_COMPOUND	0
#define FOLIO_PF_SECOND		1

#define FOLIO_HEAD_PAGE		0
#define FOLIO_SECOND_PAGE	1


#define FOLIO_TEST_FLAG(name, page)					\
static __always_inline bool folio_test_##name(const struct folio *folio) \
{ return test_bit(PG_##name, const_folio_flags(folio, page)); }

#define FOLIO_SET_FLAG(name, page)					\
static __always_inline void folio_set_##name(struct folio *folio)	\
{ set_bit(PG_##name, folio_flags(folio, page)); }

#define FOLIO_CLEAR_FLAG(name, page)					\
static __always_inline void folio_clear_##name(struct folio *folio)	\
{ clear_bit(PG_##name, folio_flags(folio, page)); }

#define __FOLIO_SET_FLAG(name, page)					\
static __always_inline void __folio_set_##name(struct folio *folio)	\
{ __set_bit(PG_##name, folio_flags(folio, page)); }

#define __FOLIO_CLEAR_FLAG(name, page)					\
static __always_inline void __folio_clear_##name(struct folio *folio)	\
{ __clear_bit(PG_##name, folio_flags(folio, page)); }

#define FOLIO_TEST_SET_FLAG(name, page)					\
static __always_inline bool folio_test_set_##name(struct folio *folio)	\
{ return test_and_set_bit(PG_##name, folio_flags(folio, page)); }

#define FOLIO_TEST_CLEAR_FLAG(name, page)				\
static __always_inline bool folio_test_clear_##name(struct folio *folio) \
{ return test_and_clear_bit(PG_##name, folio_flags(folio, page)); }

#define FOLIO_FLAG(name, page)						\
FOLIO_TEST_FLAG(name, page)						\
FOLIO_SET_FLAG(name, page)						\
FOLIO_CLEAR_FLAG(name, page)

#define TESTPAGEFLAG(uname, lname, policy)				\
FOLIO_TEST_FLAG(lname, FOLIO_##policy)					\
static __always_inline int Page##uname(const struct page *page)		\
{ return test_bit(PG_##lname, &policy(page, 0)->flags); }

#define SETPAGEFLAG(uname, lname, policy)				\
FOLIO_SET_FLAG(lname, FOLIO_##policy)					\
static __always_inline void SetPage##uname(struct page *page)		\
{ set_bit(PG_##lname, &policy(page, 1)->flags); }

#define CLEARPAGEFLAG(uname, lname, policy)				\
FOLIO_CLEAR_FLAG(lname, FOLIO_##policy)					\
static __always_inline void ClearPage##uname(struct page *page)		\
{ clear_bit(PG_##lname, &policy(page, 1)->flags); }

#define __SETPAGEFLAG(uname, lname, policy)				\
__FOLIO_SET_FLAG(lname, FOLIO_##policy)					\
static __always_inline void __SetPage##uname(struct page *page)		\
{ __set_bit(PG_##lname, &policy(page, 1)->flags); }

#define __CLEARPAGEFLAG(uname, lname, policy)				\
__FOLIO_CLEAR_FLAG(lname, FOLIO_##policy)				\
static __always_inline void __ClearPage##uname(struct page *page)	\
{ __clear_bit(PG_##lname, &policy(page, 1)->flags); }

#define TESTSETFLAG(uname, lname, policy)				\
FOLIO_TEST_SET_FLAG(lname, FOLIO_##policy)				\
static __always_inline int TestSetPage##uname(struct page *page)	\
{ return test_and_set_bit(PG_##lname, &policy(page, 1)->flags); }

#define TESTCLEARFLAG(uname, lname, policy)				\
FOLIO_TEST_CLEAR_FLAG(lname, FOLIO_##policy)				\
static __always_inline int TestClearPage##uname(struct page *page)	\
{ return test_and_clear_bit(PG_##lname, &policy(page, 1)->flags); }

#define PAGEFLAG(uname, lname, policy)					\
	TESTPAGEFLAG(uname, lname, policy)				\
	SETPAGEFLAG(uname, lname, policy)				\
	CLEARPAGEFLAG(uname, lname, policy)

#define __PAGEFLAG(uname, lname, policy)				\
	TESTPAGEFLAG(uname, lname, policy)				\
	__SETPAGEFLAG(uname, lname, policy)				\
	__CLEARPAGEFLAG(uname, lname, policy)

#define TESTSCFLAG(uname, lname, policy)				\
	TESTSETFLAG(uname, lname, policy)				\
	TESTCLEARFLAG(uname, lname, policy)

#define FOLIO_TEST_FLAG_FALSE(name)					\
static inline bool folio_test_##name(const struct folio *folio)		\
{ return false; }
#define FOLIO_SET_FLAG_NOOP(name)					\
static inline void folio_set_##name(struct folio *folio) { }
#define FOLIO_CLEAR_FLAG_NOOP(name)					\
static inline void folio_clear_##name(struct folio *folio) { }
#define __FOLIO_SET_FLAG_NOOP(name)					\
static inline void __folio_set_##name(struct folio *folio) { }
#define __FOLIO_CLEAR_FLAG_NOOP(name)					\
static inline void __folio_clear_##name(struct folio *folio) { }
#define FOLIO_TEST_SET_FLAG_FALSE(name)					\
static inline bool folio_test_set_##name(struct folio *folio)		\
{ return false; }
#define FOLIO_TEST_CLEAR_FLAG_FALSE(name)				\
static inline bool folio_test_clear_##name(struct folio *folio)		\
{ return false; }

#define FOLIO_FLAG_FALSE(name)						\
FOLIO_TEST_FLAG_FALSE(name)						\
FOLIO_SET_FLAG_NOOP(name)						\
FOLIO_CLEAR_FLAG_NOOP(name)

#define TESTPAGEFLAG_FALSE(uname, lname)				\
FOLIO_TEST_FLAG_FALSE(lname)						\
static inline int Page##uname(const struct page *page) { return 0; }

#define SETPAGEFLAG_NOOP(uname, lname)					\
FOLIO_SET_FLAG_NOOP(lname)						\
static inline void SetPage##uname(struct page *page) {  }

#define CLEARPAGEFLAG_NOOP(uname, lname)				\
FOLIO_CLEAR_FLAG_NOOP(lname)						\
static inline void ClearPage##uname(struct page *page) {  }

#define __CLEARPAGEFLAG_NOOP(uname, lname)				\
__FOLIO_CLEAR_FLAG_NOOP(lname)						\
static inline void __ClearPage##uname(struct page *page) {  }

#define TESTSETFLAG_FALSE(uname, lname)					\
FOLIO_TEST_SET_FLAG_FALSE(lname)					\
static inline int TestSetPage##uname(struct page *page) { return 0; }

#define TESTCLEARFLAG_FALSE(uname, lname)				\
FOLIO_TEST_CLEAR_FLAG_FALSE(lname)					\
static inline int TestClearPage##uname(struct page *page) { return 0; }

#define PAGEFLAG_FALSE(uname, lname) TESTPAGEFLAG_FALSE(uname, lname)	\
	SETPAGEFLAG_NOOP(uname, lname) CLEARPAGEFLAG_NOOP(uname, lname)

#define TESTSCFLAG_FALSE(uname, lname)					\
	TESTSETFLAG_FALSE(uname, lname) TESTCLEARFLAG_FALSE(uname, lname)

__PAGEFLAG(Locked, locked, PF_NO_TAIL)
FOLIO_FLAG(waiters, FOLIO_HEAD_PAGE)
PAGEFLAG(Error, error, PF_NO_TAIL) TESTCLEARFLAG(Error, error, PF_NO_TAIL)
FOLIO_FLAG(referenced, FOLIO_HEAD_PAGE)
	FOLIO_TEST_CLEAR_FLAG(referenced, FOLIO_HEAD_PAGE)
	__FOLIO_SET_FLAG(referenced, FOLIO_HEAD_PAGE)
PAGEFLAG(Dirty, dirty, PF_HEAD) TESTSCFLAG(Dirty, dirty, PF_HEAD)
	__CLEARPAGEFLAG(Dirty, dirty, PF_HEAD)
PAGEFLAG(LRU, lru, PF_HEAD) __CLEARPAGEFLAG(LRU, lru, PF_HEAD)
	TESTCLEARFLAG(LRU, lru, PF_HEAD)
FOLIO_FLAG(active, FOLIO_HEAD_PAGE)
	__FOLIO_CLEAR_FLAG(active, FOLIO_HEAD_PAGE)
	FOLIO_TEST_CLEAR_FLAG(active, FOLIO_HEAD_PAGE)
PAGEFLAG(Workingset, workingset, PF_HEAD)
	TESTCLEARFLAG(Workingset, workingset, PF_HEAD)
PAGEFLAG(Checked, checked, PF_NO_COMPOUND)	   


PAGEFLAG(Pinned, pinned, PF_NO_COMPOUND)
	TESTSCFLAG(Pinned, pinned, PF_NO_COMPOUND)
PAGEFLAG(SavePinned, savepinned, PF_NO_COMPOUND);
PAGEFLAG(Foreign, foreign, PF_NO_COMPOUND);
PAGEFLAG(XenRemapped, xen_remapped, PF_NO_COMPOUND)
	TESTCLEARFLAG(XenRemapped, xen_remapped, PF_NO_COMPOUND)

PAGEFLAG(Reserved, reserved, PF_NO_COMPOUND)
	__CLEARPAGEFLAG(Reserved, reserved, PF_NO_COMPOUND)
	__SETPAGEFLAG(Reserved, reserved, PF_NO_COMPOUND)
FOLIO_FLAG(swapbacked, FOLIO_HEAD_PAGE)
	__FOLIO_CLEAR_FLAG(swapbacked, FOLIO_HEAD_PAGE)
	__FOLIO_SET_FLAG(swapbacked, FOLIO_HEAD_PAGE)


PAGEFLAG(Private, private, PF_ANY)
PAGEFLAG(Private2, private_2, PF_ANY) TESTSCFLAG(Private2, private_2, PF_ANY)


FOLIO_FLAG(owner_2, FOLIO_HEAD_PAGE)


TESTPAGEFLAG(Writeback, writeback, PF_NO_TAIL)
	TESTSCFLAG(Writeback, writeback, PF_NO_TAIL)
PAGEFLAG(MappedToDisk, mappedtodisk, PF_NO_TAIL)


PAGEFLAG(Reclaim, reclaim, PF_NO_TAIL)
	TESTCLEARFLAG(Reclaim, reclaim, PF_NO_TAIL)
FOLIO_FLAG(readahead, FOLIO_HEAD_PAGE)
	FOLIO_TEST_CLEAR_FLAG(readahead, FOLIO_HEAD_PAGE)

FOLIO_FLAG(dropbehind, FOLIO_HEAD_PAGE)
	FOLIO_TEST_CLEAR_FLAG(dropbehind, FOLIO_HEAD_PAGE)
	__FOLIO_SET_FLAG(dropbehind, FOLIO_HEAD_PAGE)

#ifdef CONFIG_HIGHMEM

#define PageHighMem(__p) is_highmem_idx(page_zonenum(__p))
#define folio_test_highmem(__f)	is_highmem_idx(folio_zonenum(__f))
#else
PAGEFLAG_FALSE(HighMem, highmem)
#endif


#ifdef CONFIG_DEBUG_KMAP_LOCAL_FORCE_MAP
#define folio_test_partial_kmap(f)	true
#else
#define folio_test_partial_kmap(f)	folio_test_highmem(f)
#endif

#ifdef CONFIG_SWAP
static __always_inline bool folio_test_swapcache(const struct folio *folio)
{
	return folio_test_swapbacked(folio) &&
			test_bit(PG_swapcache, const_folio_flags(folio, 0));
}

FOLIO_SET_FLAG(swapcache, FOLIO_HEAD_PAGE)
FOLIO_CLEAR_FLAG(swapcache, FOLIO_HEAD_PAGE)
#else
FOLIO_FLAG_FALSE(swapcache)
#endif

FOLIO_FLAG(unevictable, FOLIO_HEAD_PAGE)
	__FOLIO_CLEAR_FLAG(unevictable, FOLIO_HEAD_PAGE)
	FOLIO_TEST_CLEAR_FLAG(unevictable, FOLIO_HEAD_PAGE)

#ifdef CONFIG_MMU
FOLIO_FLAG(mlocked, FOLIO_HEAD_PAGE)
	__FOLIO_CLEAR_FLAG(mlocked, FOLIO_HEAD_PAGE)
	FOLIO_TEST_CLEAR_FLAG(mlocked, FOLIO_HEAD_PAGE)
	FOLIO_TEST_SET_FLAG(mlocked, FOLIO_HEAD_PAGE)
#else
FOLIO_FLAG_FALSE(mlocked)
	__FOLIO_CLEAR_FLAG_NOOP(mlocked)
	FOLIO_TEST_CLEAR_FLAG_FALSE(mlocked)
	FOLIO_TEST_SET_FLAG_FALSE(mlocked)
#endif

#ifdef CONFIG_MEMORY_FAILURE
PAGEFLAG(HWPoison, hwpoison, PF_ANY)
TESTSCFLAG(HWPoison, hwpoison, PF_ANY)
#define __PG_HWPOISON (1UL << PG_hwpoison)
#else
PAGEFLAG_FALSE(HWPoison, hwpoison)
#define __PG_HWPOISON 0
#endif

#ifdef CONFIG_PAGE_IDLE_FLAG
#ifdef CONFIG_64BIT
FOLIO_TEST_FLAG(young, FOLIO_HEAD_PAGE)
FOLIO_SET_FLAG(young, FOLIO_HEAD_PAGE)
FOLIO_TEST_CLEAR_FLAG(young, FOLIO_HEAD_PAGE)
FOLIO_FLAG(idle, FOLIO_HEAD_PAGE)
#endif

#else 
FOLIO_FLAG_FALSE(young)
FOLIO_TEST_CLEAR_FLAG_FALSE(young)
FOLIO_FLAG_FALSE(idle)
#endif


__PAGEFLAG(Reported, reported, PF_NO_COMPOUND)

#ifdef CONFIG_MEMORY_HOTPLUG
PAGEFLAG(VmemmapSelfHosted, vmemmap_self_hosted, PF_ANY)
#else
PAGEFLAG_FALSE(VmemmapSelfHosted, vmemmap_self_hosted)
#endif


#define PAGE_MAPPING_ANON	0x1
#define PAGE_MAPPING_MOVABLE	0x2
#define PAGE_MAPPING_KSM	(PAGE_MAPPING_ANON | PAGE_MAPPING_MOVABLE)
#define PAGE_MAPPING_FLAGS	(PAGE_MAPPING_ANON | PAGE_MAPPING_MOVABLE)


#define PAGE_MAPPING_DAX_SHARED	((void *)0x1)

static __always_inline bool folio_mapping_flags(const struct folio *folio)
{
	return ((unsigned long)folio->mapping & PAGE_MAPPING_FLAGS) != 0;
}

static __always_inline bool PageMappingFlags(const struct page *page)
{
	return ((unsigned long)page->mapping & PAGE_MAPPING_FLAGS) != 0;
}

static __always_inline bool folio_test_anon(const struct folio *folio)
{
	return ((unsigned long)folio->mapping & PAGE_MAPPING_ANON) != 0;
}

static __always_inline bool PageAnon(const struct page *page)
{
	return folio_test_anon(page_folio(page));
}

static __always_inline bool __folio_test_movable(const struct folio *folio)
{
	return ((unsigned long)folio->mapping & PAGE_MAPPING_FLAGS) ==
			PAGE_MAPPING_MOVABLE;
}

static __always_inline bool __PageMovable(const struct page *page)
{
	return ((unsigned long)page->mapping & PAGE_MAPPING_FLAGS) ==
				PAGE_MAPPING_MOVABLE;
}

#ifdef CONFIG_KSM

static __always_inline bool folio_test_ksm(const struct folio *folio)
{
	return ((unsigned long)folio->mapping & PAGE_MAPPING_FLAGS) ==
				PAGE_MAPPING_KSM;
}

static __always_inline bool PageKsm(const struct page *page)
{
	return folio_test_ksm(page_folio(page));
}
#else
TESTPAGEFLAG_FALSE(Ksm, ksm)
#endif

u64 stable_page_flags(const struct page *page);


static inline bool folio_xor_flags_has_waiters(struct folio *folio,
		unsigned long mask)
{
	return xor_unlock_is_negative_byte(mask, folio_flags(folio, 0));
}


static inline bool folio_test_uptodate(const struct folio *folio)
{
	bool ret = test_bit(PG_uptodate, const_folio_flags(folio, 0));
	
	if (ret)
		smp_rmb();

	return ret;
}

static inline bool PageUptodate(const struct page *page)
{
	return folio_test_uptodate(page_folio(page));
}

static __always_inline void __folio_mark_uptodate(struct folio *folio)
{
	smp_wmb();
	__set_bit(PG_uptodate, folio_flags(folio, 0));
}

static __always_inline void folio_mark_uptodate(struct folio *folio)
{
	
	smp_wmb();
	set_bit(PG_uptodate, folio_flags(folio, 0));
}

static __always_inline void __SetPageUptodate(struct page *page)
{
	__folio_mark_uptodate((struct folio *)page);
}

static __always_inline void SetPageUptodate(struct page *page)
{
	folio_mark_uptodate((struct folio *)page);
}

CLEARPAGEFLAG(Uptodate, uptodate, PF_NO_TAIL)

void __folio_start_writeback(struct folio *folio, bool keep_write);
void set_page_writeback(struct page *page);

#define folio_start_writeback(folio)			\
	__folio_start_writeback(folio, false)
#define folio_start_writeback_keepwrite(folio)	\
	__folio_start_writeback(folio, true)

static __always_inline bool folio_test_head(const struct folio *folio)
{
	return test_bit(PG_head, const_folio_flags(folio, FOLIO_PF_ANY));
}

static __always_inline int PageHead(const struct page *page)
{
	PF_POISONED_CHECK(page);
	return test_bit(PG_head, &page->flags) && !page_is_fake_head(page);
}

__SETPAGEFLAG(Head, head, PF_ANY)
__CLEARPAGEFLAG(Head, head, PF_ANY)
CLEARPAGEFLAG(Head, head, PF_ANY)


static inline bool folio_test_large(const struct folio *folio)
{
	return folio_test_head(folio);
}

static __always_inline void set_compound_head(struct page *page, struct page *head)
{
	WRITE_ONCE(page->compound_head, (unsigned long)head + 1);
}

static __always_inline void clear_compound_head(struct page *page)
{
	WRITE_ONCE(page->compound_head, 0);
}

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
static inline void ClearPageCompound(struct page *page)
{
	BUG_ON(!PageHead(page));
	ClearPageHead(page);
}
FOLIO_FLAG(large_rmappable, FOLIO_SECOND_PAGE)
FOLIO_FLAG(partially_mapped, FOLIO_SECOND_PAGE)
#else
FOLIO_FLAG_FALSE(large_rmappable)
FOLIO_FLAG_FALSE(partially_mapped)
#endif

#define PG_head_mask ((1UL << PG_head))

#ifdef CONFIG_TRANSPARENT_HUGEPAGE

static inline int PageTransHuge(const struct page *page)
{
	VM_BUG_ON_PAGE(PageTail(page), page);
	return PageHead(page);
}


static inline int PageTransCompound(const struct page *page)
{
	return PageCompound(page);
}


static inline int PageTransTail(const struct page *page)
{
	return PageTail(page);
}
#else
TESTPAGEFLAG_FALSE(TransHuge, transhuge)
TESTPAGEFLAG_FALSE(TransCompound, transcompound)
TESTPAGEFLAG_FALSE(TransCompoundMap, transcompoundmap)
TESTPAGEFLAG_FALSE(TransTail, transtail)
#endif

#if defined(CONFIG_MEMORY_FAILURE) && defined(CONFIG_TRANSPARENT_HUGEPAGE)

PAGEFLAG(HasHWPoisoned, has_hwpoisoned, PF_SECOND)
	TESTSCFLAG(HasHWPoisoned, has_hwpoisoned, PF_SECOND)
#else
PAGEFLAG_FALSE(HasHWPoisoned, has_hwpoisoned)
	TESTSCFLAG_FALSE(HasHWPoisoned, has_hwpoisoned)
#endif


enum pagetype {
	
	
	PGTY_buddy	= 0xf0,
	PGTY_offline	= 0xf1,
	PGTY_table	= 0xf2,
	PGTY_guard	= 0xf3,
	PGTY_hugetlb	= 0xf4,
	PGTY_slab	= 0xf5,
	PGTY_zsmalloc	= 0xf6,
	PGTY_unaccepted	= 0xf7,

	PGTY_mapcount_underflow = 0xff
};

static inline bool page_type_has_type(int page_type)
{
	return page_type < (PGTY_mapcount_underflow << 24);
}


static inline bool page_mapcount_is_type(unsigned int mapcount)
{
	return page_type_has_type(mapcount - 1);
}

static inline bool page_has_type(const struct page *page)
{
	return page_mapcount_is_type(data_race(page->page_type));
}

#define FOLIO_TYPE_OPS(lname, fname)					\
static __always_inline bool folio_test_##fname(const struct folio *folio) \
{									\
	return data_race(folio->page.page_type >> 24) == PGTY_##lname;	\
}									\
static __always_inline void __folio_set_##fname(struct folio *folio)	\
{									\
	if (folio_test_##fname(folio))					\
		return;							\
	VM_BUG_ON_FOLIO(data_race(folio->page.page_type) != UINT_MAX,	\
			folio);						\
	folio->page.page_type = (unsigned int)PGTY_##lname << 24;	\
}									\
static __always_inline void __folio_clear_##fname(struct folio *folio)	\
{									\
	if (folio->page.page_type == UINT_MAX)				\
		return;							\
	VM_BUG_ON_FOLIO(!folio_test_##fname(folio), folio);		\
	folio->page.page_type = UINT_MAX;				\
}

#define PAGE_TYPE_OPS(uname, lname, fname)				\
FOLIO_TYPE_OPS(lname, fname)						\
static __always_inline int Page##uname(const struct page *page)		\
{									\
	return data_race(page->page_type >> 24) == PGTY_##lname;	\
}									\
static __always_inline void __SetPage##uname(struct page *page)		\
{									\
	if (Page##uname(page))						\
		return;							\
	VM_BUG_ON_PAGE(data_race(page->page_type) != UINT_MAX, page);	\
	page->page_type = (unsigned int)PGTY_##lname << 24;		\
}									\
static __always_inline void __ClearPage##uname(struct page *page)	\
{									\
	if (page->page_type == UINT_MAX)				\
		return;							\
	VM_BUG_ON_PAGE(!Page##uname(page), page);			\
	page->page_type = UINT_MAX;					\
}


PAGE_TYPE_OPS(Buddy, buddy, buddy)


PAGE_TYPE_OPS(Offline, offline, offline)

extern void page_offline_freeze(void);
extern void page_offline_thaw(void);
extern void page_offline_begin(void);
extern void page_offline_end(void);


PAGE_TYPE_OPS(Table, table, pgtable)


PAGE_TYPE_OPS(Guard, guard, guard)

FOLIO_TYPE_OPS(slab, slab)


static inline bool PageSlab(const struct page *page)
{
	return folio_test_slab(page_folio(page));
}

#ifdef CONFIG_HUGETLB_PAGE
FOLIO_TYPE_OPS(hugetlb, hugetlb)
#else
FOLIO_TEST_FLAG_FALSE(hugetlb)
#endif

PAGE_TYPE_OPS(Zsmalloc, zsmalloc, zsmalloc)


PAGE_TYPE_OPS(Unaccepted, unaccepted, unaccepted)


static inline bool PageHuge(const struct page *page)
{
	return folio_test_hugetlb(page_folio(page));
}


static inline bool is_page_hwpoison(const struct page *page)
{
	const struct folio *folio;

	if (PageHWPoison(page))
		return true;
	folio = page_folio(page);
	return folio_test_hugetlb(folio) && PageHWPoison(&folio->page);
}

static inline bool folio_contain_hwpoisoned_page(struct folio *folio)
{
	return folio_test_hwpoison(folio) ||
	    (folio_test_large(folio) && folio_test_has_hwpoisoned(folio));
}

bool is_free_buddy_page(const struct page *page);

PAGEFLAG(Isolated, isolated, PF_ANY);

static __always_inline int PageAnonExclusive(const struct page *page)
{
	VM_BUG_ON_PGFLAGS(!PageAnon(page), page);
	
	if (PageHuge(page))
		page = compound_head(page);
	return test_bit(PG_anon_exclusive, &PF_ANY(page, 1)->flags);
}

static __always_inline void SetPageAnonExclusive(struct page *page)
{
	VM_BUG_ON_PGFLAGS(!PageAnon(page) || PageKsm(page), page);
	VM_BUG_ON_PGFLAGS(PageHuge(page) && !PageHead(page), page);
	set_bit(PG_anon_exclusive, &PF_ANY(page, 1)->flags);
}

static __always_inline void ClearPageAnonExclusive(struct page *page)
{
	VM_BUG_ON_PGFLAGS(!PageAnon(page) || PageKsm(page), page);
	VM_BUG_ON_PGFLAGS(PageHuge(page) && !PageHead(page), page);
	clear_bit(PG_anon_exclusive, &PF_ANY(page, 1)->flags);
}

static __always_inline void __ClearPageAnonExclusive(struct page *page)
{
	VM_BUG_ON_PGFLAGS(!PageAnon(page), page);
	VM_BUG_ON_PGFLAGS(PageHuge(page) && !PageHead(page), page);
	__clear_bit(PG_anon_exclusive, &PF_ANY(page, 1)->flags);
}

#ifdef CONFIG_MMU
#define __PG_MLOCKED		(1UL << PG_mlocked)
#else
#define __PG_MLOCKED		0
#endif


#define PAGE_FLAGS_CHECK_AT_FREE				\
	(1UL << PG_lru		| 1UL << PG_locked	|	\
	 1UL << PG_private	| 1UL << PG_private_2	|	\
	 1UL << PG_writeback	| 1UL << PG_reserved	|	\
	 1UL << PG_active 	|				\
	 1UL << PG_unevictable	| __PG_MLOCKED | LRU_GEN_MASK)


#define PAGE_FLAGS_CHECK_AT_PREP	\
	((PAGEFLAGS_MASK & ~__PG_HWPOISON) | LRU_GEN_MASK | LRU_REFS_MASK)


#define PAGE_FLAGS_SECOND						\
	(0xffUL 		| 1UL << PG_has_hwpoisoned |	\
	 1UL << PG_large_rmappable	| 1UL << PG_partially_mapped)

#define PAGE_FLAGS_PRIVATE				\
	(1UL << PG_private | 1UL << PG_private_2)

static inline int folio_has_private(const struct folio *folio)
{
	return !!(folio->flags & PAGE_FLAGS_PRIVATE);
}

#undef PF_ANY
#undef PF_HEAD
#undef PF_NO_TAIL
#undef PF_NO_COMPOUND
#undef PF_SECOND
#endif 

#endif	
