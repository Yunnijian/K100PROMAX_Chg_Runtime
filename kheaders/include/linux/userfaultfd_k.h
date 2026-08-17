/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _LINUX_USERFAULTFD_K_H
#define _LINUX_USERFAULTFD_K_H

#ifdef CONFIG_USERFAULTFD

#include <linux/userfaultfd.h> 

#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <asm-generic/pgtable_uffd.h>
#include <linux/hugetlb_inline.h>


#define __VM_UFFD_FLAGS (VM_UFFD_MISSING | VM_UFFD_WP | VM_UFFD_MINOR)


#define UFFD_CLOEXEC O_CLOEXEC
#define UFFD_NONBLOCK O_NONBLOCK

#define UFFD_SHARED_FCNTL_FLAGS (O_CLOEXEC | O_NONBLOCK)
#define UFFD_FLAGS_SET (EFD_SHARED_FCNTL_FLAGS)


struct userfaultfd_ctx {
	
	wait_queue_head_t fault_pending_wqh;
	
	wait_queue_head_t fault_wqh;
	
	wait_queue_head_t fd_wqh;
	
	wait_queue_head_t event_wqh;
	
	seqcount_spinlock_t refile_seq;
	
	refcount_t refcount;
	
	unsigned int flags;
	
	unsigned int features;
	
	bool released;
	
	struct rw_semaphore map_changing_lock;
	
	atomic_t mmap_changing;
	
	struct mm_struct *mm;
};

extern vm_fault_t handle_userfault(struct vm_fault *vmf, unsigned long reason);


typedef unsigned int __bitwise uffd_flags_t;


enum mfill_atomic_mode {
	MFILL_ATOMIC_COPY,
	MFILL_ATOMIC_ZEROPAGE,
	MFILL_ATOMIC_CONTINUE,
	MFILL_ATOMIC_POISON,
	NR_MFILL_ATOMIC_MODES,
};

#define MFILL_ATOMIC_MODE_BITS (const_ilog2(NR_MFILL_ATOMIC_MODES - 1) + 1)
#define MFILL_ATOMIC_BIT(nr) BIT(MFILL_ATOMIC_MODE_BITS + (nr))
#define MFILL_ATOMIC_FLAG(nr) ((__force uffd_flags_t) MFILL_ATOMIC_BIT(nr))
#define MFILL_ATOMIC_MODE_MASK ((__force uffd_flags_t) (MFILL_ATOMIC_BIT(0) - 1))

static inline bool uffd_flags_mode_is(uffd_flags_t flags, enum mfill_atomic_mode expected)
{
	return (flags & MFILL_ATOMIC_MODE_MASK) == ((__force uffd_flags_t) expected);
}

static inline uffd_flags_t uffd_flags_set_mode(uffd_flags_t flags, enum mfill_atomic_mode mode)
{
	flags &= ~MFILL_ATOMIC_MODE_MASK;
	return flags | ((__force uffd_flags_t) mode);
}


#define MFILL_ATOMIC_WP MFILL_ATOMIC_FLAG(0)

extern int mfill_atomic_install_pte(pmd_t *dst_pmd,
				    struct vm_area_struct *dst_vma,
				    unsigned long dst_addr, struct page *page,
				    bool newly_allocated, uffd_flags_t flags);

extern ssize_t mfill_atomic_copy(struct userfaultfd_ctx *ctx, unsigned long dst_start,
				 unsigned long src_start, unsigned long len,
				 uffd_flags_t flags);
extern ssize_t mfill_atomic_zeropage(struct userfaultfd_ctx *ctx,
				     unsigned long dst_start,
				     unsigned long len);
extern ssize_t mfill_atomic_continue(struct userfaultfd_ctx *ctx, unsigned long dst_start,
				     unsigned long len, uffd_flags_t flags);
extern ssize_t mfill_atomic_poison(struct userfaultfd_ctx *ctx, unsigned long start,
				   unsigned long len, uffd_flags_t flags);
extern int mwriteprotect_range(struct userfaultfd_ctx *ctx, unsigned long start,
			       unsigned long len, bool enable_wp);
extern long uffd_wp_range(struct vm_area_struct *vma,
			  unsigned long start, unsigned long len, bool enable_wp);


void double_pt_lock(spinlock_t *ptl1, spinlock_t *ptl2);
void double_pt_unlock(spinlock_t *ptl1, spinlock_t *ptl2);
ssize_t move_pages(struct userfaultfd_ctx *ctx, unsigned long dst_start,
		   unsigned long src_start, unsigned long len, __u64 flags);
int move_pages_huge_pmd(struct mm_struct *mm, pmd_t *dst_pmd, pmd_t *src_pmd, pmd_t dst_pmdval,
			struct vm_area_struct *dst_vma,
			struct vm_area_struct *src_vma,
			unsigned long dst_addr, unsigned long src_addr);


static inline bool is_mergeable_vm_userfaultfd_ctx(struct vm_area_struct *vma,
					struct vm_userfaultfd_ctx vm_ctx)
{
	return vma->vm_userfaultfd_ctx.ctx == vm_ctx.ctx;
}


static inline bool uffd_disable_huge_pmd_share(struct vm_area_struct *vma)
{
	return vma->vm_flags & (VM_UFFD_WP | VM_UFFD_MINOR);
}


static inline bool uffd_disable_fault_around(struct vm_area_struct *vma)
{
	return vma->vm_flags & (VM_UFFD_WP | VM_UFFD_MINOR);
}

static inline bool userfaultfd_missing(struct vm_area_struct *vma)
{
	return vma->vm_flags & VM_UFFD_MISSING;
}

static inline bool userfaultfd_wp(struct vm_area_struct *vma)
{
	return vma->vm_flags & VM_UFFD_WP;
}

static inline bool userfaultfd_minor(struct vm_area_struct *vma)
{
	return vma->vm_flags & VM_UFFD_MINOR;
}

static inline bool userfaultfd_pte_wp(struct vm_area_struct *vma,
				      pte_t pte)
{
	return userfaultfd_wp(vma) && pte_uffd_wp(pte);
}

static inline bool userfaultfd_huge_pmd_wp(struct vm_area_struct *vma,
					   pmd_t pmd)
{
	return userfaultfd_wp(vma) && pmd_uffd_wp(pmd);
}

static inline bool userfaultfd_armed(struct vm_area_struct *vma)
{
	return vma->vm_flags & __VM_UFFD_FLAGS;
}

static inline bool vma_can_userfault(struct vm_area_struct *vma,
				     unsigned long vm_flags,
				     bool wp_async)
{
	vm_flags &= __VM_UFFD_FLAGS;

	if (vm_flags & VM_DROPPABLE)
		return false;

	if ((vm_flags & VM_UFFD_MINOR) &&
	    (!is_vm_hugetlb_page(vma) && !vma_is_shmem(vma)))
		return false;

	
	if (wp_async && (vm_flags == VM_UFFD_WP))
		return true;

#ifndef CONFIG_PTE_MARKER_UFFD_WP
	
	if ((vm_flags & VM_UFFD_WP) && !vma_is_anonymous(vma))
		return false;
#endif

	
	return vma_is_anonymous(vma) || is_vm_hugetlb_page(vma) ||
	    vma_is_shmem(vma);
}

static inline bool vma_has_uffd_without_event_remap(struct vm_area_struct *vma)
{
	struct userfaultfd_ctx *uffd_ctx = vma->vm_userfaultfd_ctx.ctx;

	return uffd_ctx && (uffd_ctx->features & UFFD_FEATURE_EVENT_REMAP) == 0;
}

extern int dup_userfaultfd(struct vm_area_struct *, struct list_head *);
extern void dup_userfaultfd_complete(struct list_head *);
void dup_userfaultfd_fail(struct list_head *);

extern void mremap_userfaultfd_prep(struct vm_area_struct *,
				    struct vm_userfaultfd_ctx *);
extern void mremap_userfaultfd_complete(struct vm_userfaultfd_ctx *,
					unsigned long from, unsigned long to,
					unsigned long len);

extern bool userfaultfd_remove(struct vm_area_struct *vma,
			       unsigned long start,
			       unsigned long end);

extern int userfaultfd_unmap_prep(struct vm_area_struct *vma,
		unsigned long start, unsigned long end, struct list_head *uf);
extern void userfaultfd_unmap_complete(struct mm_struct *mm,
				       struct list_head *uf);
extern bool userfaultfd_wp_unpopulated(struct vm_area_struct *vma);
extern bool userfaultfd_wp_async(struct vm_area_struct *vma);

void userfaultfd_reset_ctx(struct vm_area_struct *vma);

struct vm_area_struct *userfaultfd_clear_vma(struct vma_iterator *vmi,
					     struct vm_area_struct *prev,
					     struct vm_area_struct *vma,
					     unsigned long start,
					     unsigned long end);

int userfaultfd_register_range(struct userfaultfd_ctx *ctx,
			       struct vm_area_struct *vma,
			       unsigned long vm_flags,
			       unsigned long start, unsigned long end,
			       bool wp_async);

void userfaultfd_release_new(struct userfaultfd_ctx *ctx);

void userfaultfd_release_all(struct mm_struct *mm,
			     struct userfaultfd_ctx *ctx);

#else 


static inline vm_fault_t handle_userfault(struct vm_fault *vmf,
				unsigned long reason)
{
	return VM_FAULT_SIGBUS;
}

static inline long uffd_wp_range(struct vm_area_struct *vma,
				 unsigned long start, unsigned long len,
				 bool enable_wp)
{
	return false;
}

static inline bool is_mergeable_vm_userfaultfd_ctx(struct vm_area_struct *vma,
					struct vm_userfaultfd_ctx vm_ctx)
{
	return true;
}

static inline bool userfaultfd_missing(struct vm_area_struct *vma)
{
	return false;
}

static inline bool userfaultfd_wp(struct vm_area_struct *vma)
{
	return false;
}

static inline bool userfaultfd_minor(struct vm_area_struct *vma)
{
	return false;
}

static inline bool userfaultfd_pte_wp(struct vm_area_struct *vma,
				      pte_t pte)
{
	return false;
}

static inline bool userfaultfd_huge_pmd_wp(struct vm_area_struct *vma,
					   pmd_t pmd)
{
	return false;
}


static inline bool userfaultfd_armed(struct vm_area_struct *vma)
{
	return false;
}

static inline int dup_userfaultfd(struct vm_area_struct *vma,
				  struct list_head *l)
{
	return 0;
}

static inline void dup_userfaultfd_complete(struct list_head *l)
{
}

static inline void dup_userfaultfd_fail(struct list_head *l)
{
}

static inline void mremap_userfaultfd_prep(struct vm_area_struct *vma,
					   struct vm_userfaultfd_ctx *ctx)
{
}

static inline void mremap_userfaultfd_complete(struct vm_userfaultfd_ctx *ctx,
					       unsigned long from,
					       unsigned long to,
					       unsigned long len)
{
}

static inline bool userfaultfd_remove(struct vm_area_struct *vma,
				      unsigned long start,
				      unsigned long end)
{
	return true;
}

static inline int userfaultfd_unmap_prep(struct vm_area_struct *vma,
					 unsigned long start, unsigned long end,
					 struct list_head *uf)
{
	return 0;
}

static inline void userfaultfd_unmap_complete(struct mm_struct *mm,
					      struct list_head *uf)
{
}

static inline bool uffd_disable_fault_around(struct vm_area_struct *vma)
{
	return false;
}

static inline bool userfaultfd_wp_unpopulated(struct vm_area_struct *vma)
{
	return false;
}

static inline bool userfaultfd_wp_async(struct vm_area_struct *vma)
{
	return false;
}

static inline bool vma_has_uffd_without_event_remap(struct vm_area_struct *vma)
{
	return false;
}

#endif 

static inline bool userfaultfd_wp_use_markers(struct vm_area_struct *vma)
{
	
	if (!userfaultfd_wp(vma))
		return false;

	
	if (!vma_is_anonymous(vma))
		return true;

	
	return userfaultfd_wp_unpopulated(vma);
}

static inline bool pte_marker_entry_uffd_wp(swp_entry_t entry)
{
#ifdef CONFIG_PTE_MARKER_UFFD_WP
	return is_pte_marker_entry(entry) &&
	    (pte_marker_get(entry) & PTE_MARKER_UFFD_WP);
#else
	return false;
#endif
}

static inline bool pte_marker_uffd_wp(pte_t pte)
{
#ifdef CONFIG_PTE_MARKER_UFFD_WP
	swp_entry_t entry;

	if (!is_swap_pte(pte))
		return false;

	entry = pte_to_swp_entry(pte);

	return pte_marker_entry_uffd_wp(entry);
#else
	return false;
#endif
}


static inline bool pte_swp_uffd_wp_any(pte_t pte)
{
#ifdef CONFIG_PTE_MARKER_UFFD_WP
	if (!is_swap_pte(pte))
		return false;

	if (pte_swp_uffd_wp(pte))
		return true;

	if (pte_marker_uffd_wp(pte))
		return true;
#endif
	return false;
}

#endif 
