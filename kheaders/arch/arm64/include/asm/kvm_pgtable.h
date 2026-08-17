// SPDX-License-Identifier: GPL-2.0-only


#ifndef __ARM64_KVM_PGTABLE_H__
#define __ARM64_KVM_PGTABLE_H__

#include <linux/bits.h>
#include <linux/kvm_host.h>
#include <linux/types.h>

#define KVM_PGTABLE_FIRST_LEVEL		-1
#define KVM_PGTABLE_LAST_LEVEL		3


#ifdef CONFIG_ARM64_4K_PAGES
#define KVM_PGTABLE_MIN_BLOCK_LEVEL	1
#else
#define KVM_PGTABLE_MIN_BLOCK_LEVEL	2
#endif

#define kvm_lpa2_is_enabled()		system_supports_lpa2()

static inline u64 kvm_get_parange_max(void)
{
	if (kvm_lpa2_is_enabled() ||
	   (IS_ENABLED(CONFIG_ARM64_PA_BITS_52) && PAGE_SHIFT == 16))
		return ID_AA64MMFR0_EL1_PARANGE_52;
	else
		return ID_AA64MMFR0_EL1_PARANGE_48;
}

static inline u64 kvm_get_parange(u64 mmfr0)
{
	u64 parange_max = kvm_get_parange_max();
	u64 parange = cpuid_feature_extract_unsigned_field(mmfr0,
				ID_AA64MMFR0_EL1_PARANGE_SHIFT);
	if (parange > parange_max)
		parange = parange_max;

	return parange;
}

typedef u64 kvm_pte_t;

#define KVM_PTE_VALID			BIT(0)

#define KVM_PTE_ADDR_MASK		GENMASK(47, PAGE_SHIFT)
#define KVM_PTE_ADDR_51_48		GENMASK(15, 12)
#define KVM_PTE_ADDR_MASK_LPA2		GENMASK(49, PAGE_SHIFT)
#define KVM_PTE_ADDR_51_50_LPA2		GENMASK(9, 8)

#define KVM_PHYS_INVALID		(-1ULL)

#define KVM_PTE_TYPE			BIT(1)
#define KVM_PTE_TYPE_BLOCK		0
#define KVM_PTE_TYPE_PAGE		1
#define KVM_PTE_TYPE_TABLE		1

#define KVM_PTE_LEAF_ATTR_LO				\
	(GENMASK(11, 2) & ~(kvm_lpa2_is_enabled() ?	\
			    KVM_PTE_LEAF_ATTR_LO_S2_SH : 0))

#define KVM_PTE_LEAF_ATTR_LO_S1_ATTRIDX	GENMASK(4, 2)
#define KVM_PTE_LEAF_ATTR_LO_S1_AP	GENMASK(7, 6)
#define KVM_PTE_LEAF_ATTR_LO_S1_AP_RO		\
	({ cpus_have_final_cap(ARM64_KVM_HVHE) ? 2 : 3; })
#define KVM_PTE_LEAF_ATTR_LO_S1_AP_RW		\
	({ cpus_have_final_cap(ARM64_KVM_HVHE) ? 0 : 1; })
#define KVM_PTE_LEAF_ATTR_LO_S1_SH	GENMASK(9, 8)
#define KVM_PTE_LEAF_ATTR_LO_S1_SH_IS	3
#define KVM_PTE_LEAF_ATTR_LO_S1_AF	BIT(10)

#define KVM_PTE_LEAF_ATTR_LO_S2_MEMATTR	GENMASK(5, 2)
#define KVM_PTE_LEAF_ATTR_LO_S2_S2AP_R	BIT(6)
#define KVM_PTE_LEAF_ATTR_LO_S2_S2AP_W	BIT(7)
#define KVM_PTE_LEAF_ATTR_LO_S2_SH	GENMASK(9, 8)
#define KVM_PTE_LEAF_ATTR_LO_S2_SH_IS	3
#define KVM_PTE_LEAF_ATTR_LO_S2_AF	BIT(10)

#define KVM_PTE_LEAF_ATTR_HI		GENMASK(63, 50)

#define KVM_PTE_LEAF_ATTR_HI_SW		GENMASK(58, 55)

#define KVM_PTE_LEAF_ATTR_HI_S1_XN	BIT(54)

#define KVM_PTE_LEAF_ATTR_HI_S2_XN_PXN	1
#define KVM_PTE_LEAF_ATTR_HI_S2_XN_UXN	3
#define KVM_PTE_LEAF_ATTR_HI_S2_XN_XN	2
#define KVM_PTE_LEAF_ATTR_HI_S2_XN	GENMASK(54, 53)

#define KVM_PTE_LEAF_ATTR_HI_S1_GP	BIT(50)

#define KVM_PTE_LEAF_ATTR_S2_PERMS	(KVM_PTE_LEAF_ATTR_LO_S2_S2AP_R | \
					 KVM_PTE_LEAF_ATTR_LO_S2_S2AP_W | \
					 KVM_PTE_LEAF_ATTR_HI_S2_XN)

#define KVM_INVALID_PTE_OWNER_MASK	GENMASK(9, 2)
#define KVM_MAX_OWNER_ID              FIELD_MAX(KVM_INVALID_PTE_OWNER_MASK)


#define KVM_INVALID_PTE_LOCKED		BIT(10)

static inline bool kvm_pte_valid(kvm_pte_t pte)
{
	return pte & KVM_PTE_VALID;
}

static inline u64 kvm_pte_to_phys(kvm_pte_t pte)
{
	u64 pa;

	if (kvm_lpa2_is_enabled()) {
		pa = pte & KVM_PTE_ADDR_MASK_LPA2;
		pa |= FIELD_GET(KVM_PTE_ADDR_51_50_LPA2, pte) << 50;
	} else {
		pa = pte & KVM_PTE_ADDR_MASK;
		if (PAGE_SHIFT == 16)
			pa |= FIELD_GET(KVM_PTE_ADDR_51_48, pte) << 48;
	}

	return pa;
}

static inline kvm_pte_t kvm_phys_to_pte(u64 pa)
{
	kvm_pte_t pte;

	if (kvm_lpa2_is_enabled()) {
		pte = pa & KVM_PTE_ADDR_MASK_LPA2;
		pa &= GENMASK(51, 50);
		pte |= FIELD_PREP(KVM_PTE_ADDR_51_50_LPA2, pa >> 50);
	} else {
		pte = pa & KVM_PTE_ADDR_MASK;
		if (PAGE_SHIFT == 16) {
			pa &= GENMASK(51, 48);
			pte |= FIELD_PREP(KVM_PTE_ADDR_51_48, pa >> 48);
		}
	}

	return pte;
}

static inline kvm_pfn_t kvm_pte_to_pfn(kvm_pte_t pte)
{
	return __phys_to_pfn(kvm_pte_to_phys(pte));
}

static inline u64 kvm_granule_shift(s8 level)
{
	
	return ARM64_HW_PGTABLE_LEVEL_SHIFT(level);
}

static inline u64 kvm_granule_size(s8 level)
{
	return BIT(kvm_granule_shift(level));
}

static inline bool kvm_level_supports_block_mapping(s8 level)
{
	return level >= KVM_PGTABLE_MIN_BLOCK_LEVEL;
}

static inline u32 kvm_supported_block_sizes(void)
{
	s8 level = KVM_PGTABLE_MIN_BLOCK_LEVEL;
	u32 r = 0;

	for (; level <= KVM_PGTABLE_LAST_LEVEL; level++)
		r |= BIT(kvm_granule_shift(level));

	return r;
}

static inline bool kvm_is_block_size_supported(u64 size)
{
	bool is_power_of_two = IS_ALIGNED(size, size);

	return is_power_of_two && (size & kvm_supported_block_sizes());
}

static inline bool kvm_pte_table(kvm_pte_t pte, u32 level)
{
	if (level == KVM_PGTABLE_LAST_LEVEL)
		return false;

	if (!kvm_pte_valid(pte))
		return false;

	return FIELD_GET(KVM_PTE_TYPE, pte) == KVM_PTE_TYPE_TABLE;
}


struct kvm_pgtable_mm_ops {
	void*		(*zalloc_page)(void *arg);
	void*		(*zalloc_pages_exact)(size_t size);
	void		(*free_pages_exact)(void *addr, size_t size);
	void		(*free_unlinked_table)(void *addr, s8 level);
	void		(*get_page)(void *addr);
	void		(*put_page)(void *addr);
	int		(*page_count)(void *addr);
	void*		(*phys_to_virt)(phys_addr_t phys);
	phys_addr_t	(*virt_to_phys)(void *addr);
	void		(*dcache_clean_inval_poc)(void *addr, size_t size);
	void		(*icache_inval_pou)(void *addr, size_t size);
};

static inline kvm_pte_t *kvm_pte_follow(kvm_pte_t pte, struct kvm_pgtable_mm_ops *mm_ops)
{
	return mm_ops->phys_to_virt(kvm_pte_to_phys(pte));
}


enum kvm_pgtable_stage2_flags {
	KVM_PGTABLE_S2_NOFWB			= BIT(0),
	KVM_PGTABLE_S2_IDMAP			= BIT(1),
};


enum kvm_pgtable_prot {
	KVM_PGTABLE_PROT_X			= BIT(0),
	KVM_PGTABLE_PROT_W			= BIT(1),
	KVM_PGTABLE_PROT_R			= BIT(2),

	KVM_PGTABLE_PROT_DEVICE			= BIT(3),
	KVM_PGTABLE_PROT_NORMAL_NC		= BIT(4),
	KVM_PGTABLE_PROT_PXN			= BIT(5),
	KVM_PGTABLE_PROT_UXN			= BIT(6),

	KVM_PGTABLE_PROT_SW0			= BIT(55),
	KVM_PGTABLE_PROT_SW1			= BIT(56),
	KVM_PGTABLE_PROT_SW2			= BIT(57),
	KVM_PGTABLE_PROT_SW3			= BIT(58),
};




#define KVM_INVALID_PTE_OWNER_MASK	GENMASK(9, 2)


#define KVM_INVALID_PTE_LOCKED		BIT(10)


#define KVM_INVALID_PTE_MMIO_NOTE	BIT(11)

#define KVM_PGTABLE_PROT_RW	(KVM_PGTABLE_PROT_R | KVM_PGTABLE_PROT_W)
#define KVM_PGTABLE_PROT_RWX	(KVM_PGTABLE_PROT_RW | KVM_PGTABLE_PROT_X)

#define PKVM_HOST_MEM_PROT	KVM_PGTABLE_PROT_RWX
#define PKVM_HOST_MMIO_PROT	KVM_PGTABLE_PROT_RW

#define KVM_HOST_S2_DEFAULT_MASK   (KVM_PTE_LEAF_ATTR_HI |	\
				    KVM_PTE_LEAF_ATTR_LO)

#define KVM_HOST_S2_DEFAULT_MEM_PTE		\
	(PTE_S2_MEMATTR(MT_S2_NORMAL) |		\
	KVM_PTE_LEAF_ATTR_LO_S2_S2AP_R |	\
	KVM_PTE_LEAF_ATTR_LO_S2_S2AP_W |	\
	KVM_PTE_LEAF_ATTR_LO_S2_AF |		\
	(kvm_lpa2_is_enabled() ? 0 :		\
	FIELD_PREP(KVM_PTE_LEAF_ATTR_LO_S2_SH, KVM_PTE_LEAF_ATTR_LO_S2_SH_IS)))

#define KVM_HOST_S2_DEFAULT_MMIO_PTE		\
	(KVM_HOST_S2_DEFAULT_MEM_PTE |		\
	FIELD_PREP(KVM_PTE_LEAF_ATTR_HI_S2_XN, KVM_PTE_LEAF_ATTR_HI_S2_XN_XN))

#define PAGE_HYP		KVM_PGTABLE_PROT_RW
#define PAGE_HYP_EXEC		(KVM_PGTABLE_PROT_R | KVM_PGTABLE_PROT_X)
#define PAGE_HYP_RO		(KVM_PGTABLE_PROT_R)
#define PAGE_HYP_DEVICE		(PAGE_HYP | KVM_PGTABLE_PROT_DEVICE)

typedef bool (*kvm_pgtable_force_pte_cb_t)(u64 addr, u64 end,
					   enum kvm_pgtable_prot prot);

typedef bool (*kvm_pgtable_pte_is_counted_cb_t)(kvm_pte_t pte, u32 level);


struct kvm_pgtable_pte_ops {
	kvm_pgtable_force_pte_cb_t		force_pte_cb;
	kvm_pgtable_pte_is_counted_cb_t		pte_is_counted_cb;
};


enum kvm_pgtable_walk_flags {
	KVM_PGTABLE_WALK_LEAF			= BIT(0),
	KVM_PGTABLE_WALK_TABLE_PRE		= BIT(1),
	KVM_PGTABLE_WALK_TABLE_POST		= BIT(2),
	KVM_PGTABLE_WALK_SHARED			= BIT(3),
	KVM_PGTABLE_WALK_IGNORE_EAGAIN		= BIT(4),
	KVM_PGTABLE_WALK_SKIP_BBM_TLBI		= BIT(5),
	KVM_PGTABLE_WALK_SKIP_CMO		= BIT(6),
};

struct kvm_pgtable_visit_ctx {
	kvm_pte_t				*ptep;
	kvm_pte_t				old;
	void					*arg;
	struct kvm_pgtable_mm_ops		*mm_ops;
	u64					start;
	struct kvm_pgtable_pte_ops		*pte_ops;
	u64					addr;
	u64					end;
	s8					level;
	enum kvm_pgtable_walk_flags		flags;
};

typedef int (*kvm_pgtable_visitor_fn_t)(const struct kvm_pgtable_visit_ctx *ctx,
					enum kvm_pgtable_walk_flags visit);

static inline bool kvm_pgtable_walk_shared(const struct kvm_pgtable_visit_ctx *ctx)
{
	return ctx->flags & KVM_PGTABLE_WALK_SHARED;
}


struct kvm_pgtable_walker {
	const kvm_pgtable_visitor_fn_t		cb;
	void * const				arg;
	const enum kvm_pgtable_walk_flags	flags;
};


#if defined(__KVM_NVHE_HYPERVISOR__) || defined(__KVM_VHE_HYPERVISOR__)

typedef kvm_pte_t *kvm_pteref_t;

static inline kvm_pte_t *kvm_dereference_pteref(struct kvm_pgtable_walker *walker,
						kvm_pteref_t pteref)
{
	return pteref;
}

static inline int kvm_pgtable_walk_begin(struct kvm_pgtable_walker *walker)
{
	
	if (walker->flags & KVM_PGTABLE_WALK_SHARED)
		return -EPERM;

	return 0;
}

static inline void kvm_pgtable_walk_end(struct kvm_pgtable_walker *walker) {}

static inline bool kvm_pgtable_walk_lock_held(void)
{
	return true;
}

#else

typedef kvm_pte_t __rcu *kvm_pteref_t;

static inline kvm_pte_t *kvm_dereference_pteref(struct kvm_pgtable_walker *walker,
						kvm_pteref_t pteref)
{
	return rcu_dereference_check(pteref, !(walker->flags & KVM_PGTABLE_WALK_SHARED));
}

static inline int kvm_pgtable_walk_begin(struct kvm_pgtable_walker *walker)
{
	if (walker->flags & KVM_PGTABLE_WALK_SHARED)
		rcu_read_lock();

	return 0;
}

static inline void kvm_pgtable_walk_end(struct kvm_pgtable_walker *walker)
{
	if (walker->flags & KVM_PGTABLE_WALK_SHARED)
		rcu_read_unlock();
}

static inline bool kvm_pgtable_walk_lock_held(void)
{
	return rcu_read_lock_held();
}

#endif


struct kvm_pgtable {
	union {
		struct rb_root_cached				pkvm_mappings;
		struct {
			u32					ia_bits;
			s8					start_level;
			kvm_pteref_t				pgd;
			struct kvm_pgtable_mm_ops		*mm_ops;

			
			enum kvm_pgtable_stage2_flags		flags;
			kvm_pgtable_force_pte_cb_t		force_pte_cb;
			struct kvm_pgtable_pte_ops		*pte_ops;
		};
	};
	struct kvm_s2_mmu					*mmu;
};


int kvm_pgtable_hyp_init(struct kvm_pgtable *pgt, u32 va_bits,
			 struct kvm_pgtable_mm_ops *mm_ops);


void kvm_pgtable_hyp_destroy(struct kvm_pgtable *pgt);


int kvm_pgtable_hyp_map(struct kvm_pgtable *pgt, u64 addr, u64 size, u64 phys,
			enum kvm_pgtable_prot prot);


u64 kvm_pgtable_hyp_unmap(struct kvm_pgtable *pgt, u64 addr, u64 size);


u64 kvm_get_vtcr(u64 mmfr0, u64 mmfr1, u32 phys_shift);


size_t kvm_pgtable_stage2_pgd_size(u64 vtcr);


int __kvm_pgtable_stage2_init(struct kvm_pgtable *pgt, struct kvm_s2_mmu *mmu,
			      struct kvm_pgtable_mm_ops *mm_ops,
			      enum kvm_pgtable_stage2_flags flags,
			      struct kvm_pgtable_pte_ops *pte_ops);

static inline int kvm_pgtable_stage2_init(struct kvm_pgtable *pgt, struct kvm_s2_mmu *mmu,
					  struct kvm_pgtable_mm_ops *mm_ops,
					  struct kvm_pgtable_pte_ops *pte_ops)
{
	return __kvm_pgtable_stage2_init(pgt, mmu, mm_ops, 0, pte_ops);
}


void kvm_pgtable_stage2_destroy(struct kvm_pgtable *pgt);


void kvm_pgtable_stage2_free_unlinked(struct kvm_pgtable_mm_ops *mm_ops,
				      struct kvm_pgtable_pte_ops *pte_ops,
				      void *pgtable, s8 level);


kvm_pte_t *kvm_pgtable_stage2_create_unlinked(struct kvm_pgtable *pgt,
					      u64 phys, s8 level,
					      enum kvm_pgtable_prot prot,
					      void *mc, bool force_pte);


int kvm_pgtable_stage2_map(struct kvm_pgtable *pgt, u64 addr, u64 size,
			   u64 phys, enum kvm_pgtable_prot prot,
			   void *mc, enum kvm_pgtable_walk_flags flags);


int kvm_pgtable_stage2_annotate(struct kvm_pgtable *pgt, u64 addr, u64 size,
				void *mc, kvm_pte_t annotation);


int kvm_pgtable_stage2_unmap(struct kvm_pgtable *pgt, u64 addr, u64 size);


int kvm_pgtable_stage2_reclaim_leaves(struct kvm_pgtable *pgt, u64 addr, u64 size);


int kvm_pgtable_stage2_wrprotect(struct kvm_pgtable *pgt, u64 addr, u64 size);


kvm_pte_t kvm_pgtable_stage2_mkyoung(struct kvm_pgtable *pgt, u64 addr,
				     enum kvm_pgtable_walk_flags flags);


bool kvm_pgtable_stage2_test_clear_young(struct kvm_pgtable *pgt, u64 addr,
					 u64 size, bool mkold);


int kvm_pgtable_stage2_relax_perms(struct kvm_pgtable *pgt, u64 addr,
				   enum kvm_pgtable_prot prot,
				   enum kvm_pgtable_walk_flags flags);


int kvm_pgtable_stage2_flush(struct kvm_pgtable *pgt, u64 addr, u64 size);


int kvm_pgtable_stage2_split(struct kvm_pgtable *pgt, u64 addr, u64 size, void *mc);


int kvm_pgtable_walk(struct kvm_pgtable *pgt, u64 addr, u64 size,
		     struct kvm_pgtable_walker *walker);


int kvm_pgtable_get_leaf(struct kvm_pgtable *pgt, u64 addr,
			 kvm_pte_t *ptep, s8 *level);


enum kvm_pgtable_prot kvm_pgtable_stage2_pte_prot(kvm_pte_t pte);


enum kvm_pgtable_prot kvm_pgtable_hyp_pte_prot(kvm_pte_t pte);


void kvm_tlb_flush_vmid_range(struct kvm_s2_mmu *mmu,
				phys_addr_t addr, size_t size);
#endif	
