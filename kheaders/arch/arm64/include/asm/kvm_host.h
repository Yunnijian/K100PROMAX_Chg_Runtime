/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __ARM64_KVM_HOST_H__
#define __ARM64_KVM_HOST_H__

#include <linux/android_kabi.h>
#include <linux/arm-smccc.h>
#include <linux/bitmap.h>
#include <linux/types.h>
#include <linux/jump_label.h>
#include <linux/kvm_types.h>
#include <linux/maple_tree.h>
#include <linux/percpu.h>
#include <linux/psci.h>
#include <asm/arch_gicv3.h>
#include <asm/barrier.h>
#include <asm/cpufeature.h>
#include <asm/cputype.h>
#include <asm/daifflags.h>
#include <asm/fpsimd.h>
#include <asm/kvm.h>
#include <asm/kvm_asm.h>
#include <asm/vncr_mapping.h>

#define __KVM_HAVE_ARCH_INTC_INITIALIZED

#define KVM_HALT_POLL_NS_DEFAULT 500000

#include <kvm/arm_vgic.h>
#include <kvm/arm_arch_timer.h>
#include <kvm/arm_pmu.h>

#define KVM_MAX_VCPUS VGIC_V3_MAX_CPUS

#define KVM_VCPU_MAX_FEATURES 7
#define KVM_VCPU_VALID_FEATURES	(BIT(KVM_VCPU_MAX_FEATURES) - 1)

#define KVM_REQ_SLEEP \
	KVM_ARCH_REQ_FLAGS(0, KVM_REQUEST_WAIT | KVM_REQUEST_NO_WAKEUP)
#define KVM_REQ_IRQ_PENDING	KVM_ARCH_REQ(1)
#define KVM_REQ_VCPU_RESET	KVM_ARCH_REQ(2)
#define KVM_REQ_RECORD_STEAL	KVM_ARCH_REQ(3)
#define KVM_REQ_RELOAD_GICv4	KVM_ARCH_REQ(4)
#define KVM_REQ_RELOAD_PMU	KVM_ARCH_REQ(5)
#define KVM_REQ_SUSPEND		KVM_ARCH_REQ(6)
#define KVM_REQ_RESYNC_PMU_EL0	KVM_ARCH_REQ(7)
#define KVM_REQ_NESTED_S2_UNMAP	KVM_ARCH_REQ(8)

#define KVM_DIRTY_LOG_MANUAL_CAPS   (KVM_DIRTY_LOG_MANUAL_PROTECT_ENABLE | \
				     KVM_DIRTY_LOG_INITIALLY_SET)

#define KVM_HAVE_MMU_RWLOCK


enum kvm_mode {
	KVM_MODE_DEFAULT,
	KVM_MODE_PROTECTED,
	KVM_MODE_NV,
	KVM_MODE_NONE,
};
#ifdef CONFIG_KVM
enum kvm_mode kvm_get_mode(void);
#else
static inline enum kvm_mode kvm_get_mode(void) { return KVM_MODE_NONE; };
#endif

extern unsigned int __ro_after_init kvm_sve_max_vl;
extern unsigned int __ro_after_init kvm_host_sve_max_vl;
int __init kvm_arm_init_sve(void);

u32 __attribute_const__ kvm_target_cpu(void);
void kvm_reset_vcpu(struct kvm_vcpu *vcpu);
void kvm_arm_vcpu_destroy(struct kvm_vcpu *vcpu);


struct kvm_hyp_memcache {
	phys_addr_t head;
	unsigned long nr_pages;
	unsigned long flags;
	void *mapping; 
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};

static inline void push_hyp_memcache(struct kvm_hyp_memcache *mc,
				     phys_addr_t *p,
				     phys_addr_t (*to_pa)(void *virt),
				     unsigned long order)
{
	*p = mc->head;
	mc->head = (to_pa(p) & PAGE_MASK) |
		   FIELD_PREP(~PAGE_MASK, order);
	mc->nr_pages++;
}

static inline void *pop_hyp_memcache(struct kvm_hyp_memcache *mc,
				     void *(*to_va)(phys_addr_t phys),
				     unsigned long *order)
{
	phys_addr_t *p = to_va(mc->head & PAGE_MASK);

	if (!mc->nr_pages)
		return NULL;

	*order = FIELD_GET(~PAGE_MASK, mc->head);

	mc->head = *p;
	mc->nr_pages--;

	return p;
}

static inline int __topup_hyp_memcache(struct kvm_hyp_memcache *mc,
				       unsigned long min_pages,
				       void *(*alloc_fn)(void *arg, unsigned long order),
				       phys_addr_t (*to_pa)(void *virt),
				       void *arg,
				       unsigned long order)
{
	while (mc->nr_pages < min_pages) {
		phys_addr_t *p = alloc_fn(arg, order);

		if (!p)
			return -ENOMEM;
		push_hyp_memcache(mc, p, to_pa, order);
	}

	return 0;
}

static inline void __free_hyp_memcache(struct kvm_hyp_memcache *mc,
				       void (*free_fn)(void *virt, void *arg, unsigned long order),
				       void *(*to_va)(phys_addr_t phys),
				       void *arg)
{
	unsigned long order;
	void *p;

	while (mc->nr_pages) {
		p = pop_hyp_memcache(mc, to_va, &order);
		free_fn(p, arg, order);
	}
}

#define HYP_MEMCACHE_ACCOUNT_KMEMCG BIT(1)
#define HYP_MEMCACHE_ACCOUNT_STAGE2 BIT(2)

void free_hyp_memcache(struct kvm_hyp_memcache *mc);
int topup_hyp_memcache(struct kvm_hyp_memcache *mc, unsigned long min_pages, unsigned long order);
int topup_hyp_memcache_gfp(struct kvm_hyp_memcache *mc, unsigned long min_pages,
			   unsigned long order, gfp_t gfp);

static inline void init_hyp_memcache(struct kvm_hyp_memcache *mc)
{
	memset(mc, 0, sizeof(*mc));
	mc->mapping = ZERO_SIZE_PTR; 
}

static inline void init_hyp_stage2_memcache(struct kvm_hyp_memcache *mc)
{
	memset(mc, 0, sizeof(*mc));
	mc->flags = HYP_MEMCACHE_ACCOUNT_KMEMCG | HYP_MEMCACHE_ACCOUNT_STAGE2;
}

struct kvm_vmid {
	atomic64_t id;
};

struct kvm_s2_mmu {
	struct kvm_vmid vmid;

	
	phys_addr_t	pgd_phys;
	struct kvm_pgtable *pgt;

	
	u64	vtcr;

	
	int __percpu *last_vcpu_ran;

#define KVM_ARM_EAGER_SPLIT_CHUNK_SIZE_DEFAULT 0
	
	struct kvm_mmu_memory_cache split_page_cache;
	uint64_t split_page_chunk_size;

	struct kvm_arch *arch;

	
	u64	tlb_vttbr;
	u64	tlb_vtcr;

	
	bool	nested_stage2_enabled;

	
	bool	pending_unmap;

	
	atomic_t refcnt;
};

struct kvm_arch_memory_slot {
};


struct kvm_smccc_features {
	unsigned long std_bmap;
	unsigned long std_hyp_bmap;
	unsigned long vendor_hyp_bmap;
};

struct kvm_pinned_page {
	union {
		struct rb_node	 	node;
		struct list_head 	list_node;
	};
	struct page		*page;
	u64			ipa;
	u64			__subtree_last;
	u8			order;
};

struct kvm_pinned_page
*kvm_pinned_pages_iter_first(struct rb_root_cached *root, u64 start, u64 end);
struct kvm_pinned_page
*kvm_pinned_pages_iter_next(struct kvm_pinned_page *ppage, u64 start, u64 end);
void kvm_pinned_pages_remove(struct kvm_pinned_page *ppage,
			     struct rb_root_cached *root);

typedef unsigned int pkvm_handle_t;

struct kvm_protected_vm {
	pkvm_handle_t handle;
	struct kvm_hyp_memcache stage2_teardown_mc;
	struct rb_root_cached pinned_pages;
	struct kvm_hyp_memcache teardown_iommu_mc;
	gpa_t pvmfw_load_addr;
	bool enabled;
	u32 ffa_support;
	bool smc_forwarded;
};

struct kvm_mpidr_data {
	u64			mpidr_mask;
	DECLARE_FLEX_ARRAY(u16, cmpidr_to_idx);
};

static inline u16 kvm_mpidr_index(struct kvm_mpidr_data *data, u64 mpidr)
{
	unsigned long index = 0, mask = data->mpidr_mask;
	unsigned long aff = mpidr & MPIDR_HWID_BITMASK;

	bitmap_gather(&index, &aff, &mask, fls(mask));

	return index;
}

struct kvm_sysreg_masks;

enum fgt_group_id {
	__NO_FGT_GROUP__,
	HFGxTR_GROUP,
	HDFGRTR_GROUP,
	HDFGWTR_GROUP = HDFGRTR_GROUP,
	HFGITR_GROUP,
	HAFGRTR_GROUP,

	
	__NR_FGT_GROUP_IDS__
};

struct kvm_arch {
	struct kvm_s2_mmu mmu;

	
	u64 fgu[__NR_FGT_GROUP_IDS__];

	
	struct kvm_s2_mmu *nested_mmus;
	size_t nested_mmus_size;
	int nested_mmus_next;

	
	struct vgic_dist	vgic;

	
	struct arch_timer_vm_data timer_data;

	
	u32 psci_version;

	
	struct mutex config_lock;

	
#define KVM_ARCH_FLAG_RETURN_NISV_IO_ABORT_TO_USER	0
	
#define KVM_ARCH_FLAG_MTE_ENABLED			1
	
#define KVM_ARCH_FLAG_HAS_RAN_ONCE			2
	
#define KVM_ARCH_FLAG_VCPU_FEATURES_CONFIGURED		3
	
#define KVM_ARCH_FLAG_SYSTEM_SUSPEND_ENABLED		4
	
#define KVM_ARCH_FLAG_VM_COUNTER_OFFSET			5
	
#define KVM_ARCH_FLAG_TIMER_PPIS_IMMUTABLE		6
	
#define KVM_ARCH_FLAG_ID_REGS_INITIALIZED		7
	
#define KVM_ARCH_FLAG_FGU_INITIALIZED			8
	
#define KVM_ARCH_FLAG_GUEST_HAS_SVE			9
	
#define KVM_ARCH_FLAG_MMIO_GUARD			10
	unsigned long flags;

	
	DECLARE_BITMAP(vcpu_features, KVM_VCPU_MAX_FEATURES);

	
	struct kvm_mpidr_data *mpidr_data;

	
	unsigned long *pmu_filter;
	struct arm_pmu *arm_pmu;

	cpumask_var_t supported_cpus;

	
	u8 pmcr_n;

	
	u8	idreg_debugfs_iter;

	
	struct kvm_smccc_features smccc_feat;
	struct maple_tree smccc_filter;

	
#define IDREG_IDX(id)		(((sys_reg_CRm(id) - 1) << 3) | sys_reg_Op2(id))
#define KVM_ARM_ID_REG_NUM	(IDREG_IDX(sys_reg(3, 0, 0, 7, 7)) + 1)
	u64 id_regs[KVM_ARM_ID_REG_NUM];

	u64 ctr_el0;

	
	struct kvm_sysreg_masks	*sysreg_masks;

	
	struct kvm_protected_vm pkvm;
};

struct kvm_vcpu_fault_info {
	u64 esr_el2;		
	u64 far_el2;		
	u64 hpfar_el2;		
	u64 disr_el1;		
};


#define __MAX__(x,y)	((x) ^ (((x) ^ (y)) & -((x) < (y))))
#define VNCR(r)						\
	__before_##r,					\
	r = __VNCR_START__ + ((VNCR_ ## r) / 8),	\
	__after_##r = __MAX__(__before_##r - 1, r)

enum vcpu_sysreg {
	__INVALID_SYSREG__,   
	MPIDR_EL1,	
	CLIDR_EL1,	
	CSSELR_EL1,	
	TPIDR_EL0,	
	TPIDRRO_EL0,	
	TPIDR_EL1,	
	CNTKCTL_EL1,	
	PAR_EL1,	
	MDCCINT_EL1,	
	OSLSR_EL1,	
	DISR_EL1,	

	
	PMCR_EL0,	
	PMSELR_EL0,	
	PMEVCNTR0_EL0,	
	PMEVCNTR30_EL0 = PMEVCNTR0_EL0 + 30,
	PMCCNTR_EL0,	
	PMEVTYPER0_EL0,	
	PMEVTYPER30_EL0 = PMEVTYPER0_EL0 + 30,
	PMCCFILTR_EL0,	
	PMCNTENSET_EL0,	
	PMINTENSET_EL1,	
	PMOVSSET_EL0,	
	PMUSERENR_EL0,	

	
	APIAKEYLO_EL1,
	APIAKEYHI_EL1,
	APIBKEYLO_EL1,
	APIBKEYHI_EL1,
	APDAKEYLO_EL1,
	APDAKEYHI_EL1,
	APDBKEYLO_EL1,
	APDBKEYHI_EL1,
	APGAKEYLO_EL1,
	APGAKEYHI_EL1,

	
	RGSR_EL1,	
	GCR_EL1,	
	TFSRE0_EL1,	

	POR_EL0,	

	
	SVCR,
	FPMR,

	
	DACR32_EL2,	
	IFSR32_EL2,	
	FPEXC32_EL2,	
	DBGVCR32_EL2,	

	
	SCTLR_EL2,	
	ACTLR_EL2,	
	MDCR_EL2,	
	CPTR_EL2,	
	HACR_EL2,	
	ZCR_EL2,	
	TTBR0_EL2,	
	TTBR1_EL2,	
	TCR_EL2,	
	SPSR_EL2,	
	ELR_EL2,	
	AFSR0_EL2,	
	AFSR1_EL2,	
	ESR_EL2,	
	FAR_EL2,	
	HPFAR_EL2,	
	MAIR_EL2,	
	AMAIR_EL2,	
	VBAR_EL2,	
	RVBAR_EL2,	
	CONTEXTIDR_EL2,	
	CNTHCTL_EL2,	
	SP_EL2,		
	CNTHP_CTL_EL2,
	CNTHP_CVAL_EL2,
	CNTHV_CTL_EL2,
	CNTHV_CVAL_EL2,

	__VNCR_START__,	

	VNCR(SCTLR_EL1),
	VNCR(ACTLR_EL1),
	VNCR(CPACR_EL1),
	VNCR(ZCR_EL1),	
	VNCR(TTBR0_EL1),
	VNCR(TTBR1_EL1),
	VNCR(TCR_EL1),	
	VNCR(TCR2_EL1),	
	VNCR(ESR_EL1),	
	VNCR(AFSR0_EL1),
	VNCR(AFSR1_EL1),
	VNCR(FAR_EL1),	
	VNCR(MAIR_EL1),	
	VNCR(VBAR_EL1),	
	VNCR(CONTEXTIDR_EL1),	
	VNCR(AMAIR_EL1),
	VNCR(MDSCR_EL1),
	VNCR(ELR_EL1),
	VNCR(SP_EL1),
	VNCR(SPSR_EL1),
	VNCR(TFSR_EL1),	
	VNCR(VPIDR_EL2),
	VNCR(VMPIDR_EL2),
	VNCR(HCR_EL2),	
	VNCR(HSTR_EL2),	
	VNCR(VTTBR_EL2),
	VNCR(VTCR_EL2),	
	VNCR(TPIDR_EL2),
	VNCR(HCRX_EL2),	

	
	VNCR(PIR_EL1),	 
	VNCR(PIRE0_EL1), 

	VNCR(POR_EL1),	

	VNCR(HFGRTR_EL2),
	VNCR(HFGWTR_EL2),
	VNCR(HFGITR_EL2),
	VNCR(HDFGRTR_EL2),
	VNCR(HDFGWTR_EL2),
	VNCR(HAFGRTR_EL2),

	VNCR(CNTVOFF_EL2),
	VNCR(CNTV_CVAL_EL0),
	VNCR(CNTV_CTL_EL0),
	VNCR(CNTP_CVAL_EL0),
	VNCR(CNTP_CTL_EL0),

	VNCR(ICH_HCR_EL2),

	NR_SYS_REGS	
};

struct kvm_sysreg_masks {
	struct {
		u64	res0;
		u64	res1;
	} mask[NR_SYS_REGS - __VNCR_START__];
};

struct kvm_cpu_context {
	struct user_pt_regs regs;	

	u64	spsr_abt;
	u64	spsr_und;
	u64	spsr_irq;
	u64	spsr_fiq;

	struct user_fpsimd_state fp_regs;

	u64 sys_regs[NR_SYS_REGS];

	struct kvm_vcpu *__hyp_running_vcpu;

	
	u64 *vncr_array;
};

struct cpu_sve_state {
	__u64 zcr_el1;

	
	__u32 fpsr;
	__u32 fpcr;

	
	__u8 sve_regs[];
};


struct kvm_host_data {
	struct kvm_cpu_context host_ctxt;

	
	struct cpu_sve_state *sve_state;

	
	u64	fpmr;

	
	enum {
		FP_STATE_FREE,
		FP_STATE_HOST_OWNED,
		FP_STATE_GUEST_OWNED,
	} fp_owner;

	
	 struct {
		
		struct kvm_guest_debug_arch regs;
		
		u64 pmscr_el1;
		u64 pmblimitr_el1;
		
		u64 trfcr_el1;
		u64 trblimitr_el1;
		
		u64 mdcr_el2;
	} host_debug_state;
};

struct kvm_host_psci_config {
	
	u32 version;
	u32 smccc_version;

	
	struct psci_0_1_function_ids function_ids_0_1;

	bool psci_0_1_cpu_suspend_implemented;
	bool psci_0_1_cpu_on_implemented;
	bool psci_0_1_cpu_off_implemented;
	bool psci_0_1_migrate_implemented;
};

extern struct kvm_host_psci_config kvm_nvhe_sym(kvm_host_psci_config);
#define kvm_host_psci_config CHOOSE_NVHE_SYM(kvm_host_psci_config)

extern s64 kvm_nvhe_sym(hyp_physvirt_offset);
#define hyp_physvirt_offset CHOOSE_NVHE_SYM(hyp_physvirt_offset)

extern u64 kvm_nvhe_sym(hyp_cpu_logical_map)[NR_CPUS];
#define hyp_cpu_logical_map CHOOSE_NVHE_SYM(hyp_cpu_logical_map)

struct vcpu_reset_state {
	unsigned long	pc;
	unsigned long	r0;
	bool		be;
	bool		reset;
};

struct kvm_hyp_req {
#define KVM_HYP_LAST_REQ	0
#define KVM_HYP_REQ_TYPE_MEM	1
#define KVM_HYP_REQ_TYPE_MAP	2
#define KVM_HYP_REQ_TYPE_SPLIT	3
	u8 type;
	union {
		struct {
#define REQ_MEM_DEST_HYP_ALLOC		1
#define REQ_MEM_DEST_VCPU_MEMCACHE	2
#define REQ_MEM_DEST_HYP_IOMMU		3
			u8	dest;
			int	nr_pages;
			int	sz_alloc; 
		} mem;
		struct {
			unsigned long	guest_ipa;
			size_t		size;
		} map;
		struct {
			unsigned long	guest_ipa;
			size_t		size;
		} split;
	};
};

#define KVM_HYP_REQ_MAX ((PAGE_SIZE >> 4) / sizeof(struct kvm_hyp_req))


struct kvm_hyp_pinned_page {
	u64	pfn : 40;
	u64	gfn : 40;
	u8	order;
} __packed;


static inline struct kvm_hyp_pinned_page *
next_kvm_hyp_pinned_page(struct kvm_hyp_req *page, struct kvm_hyp_pinned_page *ppage, bool valid)
{
	void *start = (void *)(page + KVM_HYP_REQ_MAX);
	void *end = (void *)page + PAGE_SIZE;

	if (WARN_ON(!PAGE_ALIGNED(page)))
		return NULL;

	if (!ppage)
		ppage = (struct kvm_hyp_pinned_page *)start;
	else
		ppage++;

	if (((void *)ppage + sizeof(*ppage)) >= end)
		return NULL;

	if (valid && (ppage->order == 0xFF))
		return NULL;

	return ppage;
}



#define	SMCCC_REQ_TYPE_MASK		GENMASK_ULL(7, 0)
#define SMCCC_REQ_DEST_MASK		GENMASK_ULL(15, 8)

#define SMCCC_REQ_NR_PAGES_MASK		GENMASK_ULL(31, 0)
#define SMCCC_REQ_SZ_ALLOC_MASK		GENMASK_ULL(63, 32)

static inline void hyp_reqs_smccc_decode(struct arm_smccc_res *res,
					 struct kvm_hyp_req *req)
{
	req->type = FIELD_GET(SMCCC_REQ_TYPE_MASK, res->a2);
	req->mem.dest = FIELD_GET(SMCCC_REQ_DEST_MASK, res->a2);
	req->mem.nr_pages = FIELD_GET(SMCCC_REQ_NR_PAGES_MASK, res->a3);
	req->mem.sz_alloc = FIELD_GET(SMCCC_REQ_SZ_ALLOC_MASK, res->a3);
}

struct kvm_vcpu_arch {
	struct kvm_cpu_context ctxt;

	
	void *sve_state;
	enum fp_type fp_type;
	unsigned int sve_max_vl;

	
	struct kvm_s2_mmu *hw_mmu;

	
	u64 hcr_el2;
	u64 hcrx_el2;
	u64 mdcr_el2;

	
	struct kvm_vcpu_fault_info fault;

	
	u8 cflags;

	
	u8 iflags;

	
	u8 sflags;

	
	bool pause;

	
	struct kvm_guest_debug_arch *debug_ptr;
	struct kvm_guest_debug_arch vcpu_debug_state;
	struct kvm_guest_debug_arch external_debug_state;

	
	struct vgic_cpu vgic_cpu;
	struct arch_timer_cpu timer_cpu;
	struct kvm_pmu pmu;

	
	struct {
		u32	mdscr_el1;
		bool	pstate_ss;
	} guest_debug_preserved;

	
	struct kvm_mp_state mp_state;
	spinlock_t mp_state_lock;

	union {
		
		struct kvm_mmu_memory_cache mmu_page_cache;
		
		struct kvm_hyp_memcache stage2_mc;
	};

	
	u64 vsesr_el2;

	
	struct vcpu_reset_state	reset_state;

	
	struct {
		u64 last_steal;
		gpa_t base;
	} steal;

	
	u32 *ccsidr;

	
	struct kvm_hyp_memcache iommu_mc;

	
	struct kvm_hyp_req *hyp_reqs;
};


#define __vcpu_single_flag(_set, _f)	_set, (_f), (_f)

#define __unpack_flag(_set, _f, _m)	_f
#define unpack_vcpu_flag(...)		__unpack_flag(__VA_ARGS__)

#define __build_check_flag(v, flagset, f, m)			\
	do {							\
		typeof(v->arch.flagset) *_fset;			\
								\
			\
		BUILD_BUG_ON(HWEIGHT(m) != HWEIGHT((f) | (m)));	\
			\
		BUILD_BUG_ON((sizeof(*_fset) * 8) <= __fls(m));	\
	} while (0)

#define __vcpu_get_flag(v, flagset, f, m)			\
	({							\
		__build_check_flag(v, flagset, f, m);		\
								\
		READ_ONCE(v->arch.flagset) & (m);		\
	})


#ifdef __KVM_NVHE_HYPERVISOR__

#define __vcpu_flags_preempt_disable()
#define __vcpu_flags_preempt_enable()
#else
#define __vcpu_flags_preempt_disable()	preempt_disable()
#define __vcpu_flags_preempt_enable()	preempt_enable()
#endif

#define __vcpu_set_flag(v, flagset, f, m)			\
	do {							\
		typeof(v->arch.flagset) *fset;			\
								\
		__build_check_flag(v, flagset, f, m);		\
								\
		fset = &v->arch.flagset;			\
		__vcpu_flags_preempt_disable();			\
		if (HWEIGHT(m) > 1)				\
			*fset &= ~(m);				\
		*fset |= (f);					\
		__vcpu_flags_preempt_enable();			\
	} while (0)

#define __vcpu_clear_flag(v, flagset, f, m)			\
	do {							\
		typeof(v->arch.flagset) *fset;			\
								\
		__build_check_flag(v, flagset, f, m);		\
								\
		fset = &v->arch.flagset;			\
		__vcpu_flags_preempt_disable();			\
		*fset &= ~(m);					\
		__vcpu_flags_preempt_enable();			\
	} while (0)

#define __vcpu_copy_flag(vt, vs, flagset, f, m)			\
	do {							\
		typeof(vs->arch.flagset) tmp, val;		\
								\
		__build_check_flag(vs, flagset, f, m);		\
								\
		val = READ_ONCE(vs->arch.flagset);		\
		val &= (m);					\
		tmp = READ_ONCE(vt->arch.flagset);		\
		tmp &= ~(m);					\
		tmp |= val;					\
		WRITE_ONCE(vt->arch.flagset, tmp);		\
	} while (0)


#define vcpu_get_flag(v, ...)	__vcpu_get_flag((v), __VA_ARGS__)
#define vcpu_set_flag(v, ...)	__vcpu_set_flag((v), __VA_ARGS__)
#define vcpu_clear_flag(v, ...)	__vcpu_clear_flag((v), __VA_ARGS__)
#define vcpu_copy_flag(vt, vs, ...) __vcpu_copy_flag((vt), (vs), __VA_ARGS__)


#define VCPU_INITIALIZED	__vcpu_single_flag(cflags, BIT(0))

#define VCPU_SVE_FINALIZED	__vcpu_single_flag(cflags, BIT(1))

#define VCPU_PKVM_FINALIZED	__vcpu_single_flag(cflags, BIT(2))


#define PENDING_EXCEPTION	__vcpu_single_flag(iflags, BIT(0))

#define INCREMENT_PC		__vcpu_single_flag(iflags, BIT(1))

#define EXCEPT_MASK		__vcpu_single_flag(iflags, GENMASK(3, 1))

#define PC_UPDATE_REQ		__vcpu_single_flag(iflags, GENMASK(3, 0))


#define __EXCEPT_MASK_VAL	unpack_vcpu_flag(EXCEPT_MASK)
#define __EXCEPT_SHIFT		__builtin_ctzl(__EXCEPT_MASK_VAL)
#define __vcpu_except_flags(_f)	iflags, (_f << __EXCEPT_SHIFT), __EXCEPT_MASK_VAL


#define EXCEPT_AA32_UND		__vcpu_except_flags(0)
#define EXCEPT_AA32_IABT	__vcpu_except_flags(1)
#define EXCEPT_AA32_DABT	__vcpu_except_flags(2)

#define EXCEPT_AA64_EL1_SYNC	__vcpu_except_flags(0)
#define EXCEPT_AA64_EL1_IRQ	__vcpu_except_flags(1)
#define EXCEPT_AA64_EL1_FIQ	__vcpu_except_flags(2)
#define EXCEPT_AA64_EL1_SERR	__vcpu_except_flags(3)

#define EXCEPT_AA64_EL2_SYNC	__vcpu_except_flags(4)
#define EXCEPT_AA64_EL2_IRQ	__vcpu_except_flags(5)
#define EXCEPT_AA64_EL2_FIQ	__vcpu_except_flags(6)
#define EXCEPT_AA64_EL2_SERR	__vcpu_except_flags(7)

#define DEBUG_DIRTY		__vcpu_single_flag(iflags, BIT(4))

#define DEBUG_STATE_SAVE_SPE	__vcpu_single_flag(iflags, BIT(5))

#define DEBUG_STATE_SAVE_TRBE	__vcpu_single_flag(iflags, BIT(6))

#define VCPU_HYP_CONTEXT	__vcpu_single_flag(iflags, BIT(7))

#define PKVM_HOST_STATE_DIRTY	__vcpu_single_flag(iflags, BIT(7))


#define ON_UNSUPPORTED_CPU	__vcpu_single_flag(sflags, BIT(2))

#define IN_WFIT			__vcpu_single_flag(sflags, BIT(3))

#define SYSREGS_ON_CPU		__vcpu_single_flag(sflags, BIT(4))

#define DBG_SS_ACTIVE_PENDING	__vcpu_single_flag(sflags, BIT(5))

#define PMUSERENR_ON_CPU	__vcpu_single_flag(sflags, BIT(6))

#define IN_WFI			__vcpu_single_flag(sflags, BIT(7))



#define vcpu_sve_pffr(vcpu) (kern_hyp_va((vcpu)->arch.sve_state) +	\
			     sve_ffr_offset((vcpu)->arch.sve_max_vl))

#define vcpu_sve_zcr_elx(vcpu)						\
	(unlikely(is_hyp_ctxt(vcpu)) ? ZCR_EL2 : ZCR_EL1)

#define sve_state_size(sve_max_vl) ({					\
	size_t __size_ret;						\
	unsigned int __vq;						\
									\
	if (WARN_ON(!sve_vl_valid(sve_max_vl))) {			\
		__size_ret = 0;						\
	} else {							\
		__vq = sve_vq_from_vl(sve_max_vl);			\
		__size_ret = SVE_SIG_REGS_SIZE(__vq);			\
	}								\
									\
	__size_ret;							\
})

#define vcpu_sve_max_vq(vcpu) sve_vq_from_vl((vcpu)->arch.sve_max_vl)

#define vcpu_sve_state_size(vcpu) sve_state_size((vcpu)->arch.sve_max_vl)

#define KVM_GUESTDBG_VALID_MASK (KVM_GUESTDBG_ENABLE | \
				 KVM_GUESTDBG_USE_SW_BP | \
				 KVM_GUESTDBG_USE_HW | \
				 KVM_GUESTDBG_SINGLESTEP)

#define kvm_has_sve(kvm)	(system_supports_sve() &&		\
				 test_bit(KVM_ARCH_FLAG_GUEST_HAS_SVE, &(kvm)->arch.flags))

#ifdef __KVM_NVHE_HYPERVISOR__
#define vcpu_has_sve(vcpu)	kvm_has_sve(kern_hyp_va((vcpu)->kvm))
#else
#define vcpu_has_sve(vcpu)	kvm_has_sve((vcpu)->kvm)
#endif

#ifdef CONFIG_ARM64_PTR_AUTH
#define vcpu_has_ptrauth(vcpu)						\
	((cpus_have_final_cap(ARM64_HAS_ADDRESS_AUTH) ||		\
	  cpus_have_final_cap(ARM64_HAS_GENERIC_AUTH)) &&		\
	 (vcpu_has_feature(vcpu, KVM_ARM_VCPU_PTRAUTH_ADDRESS) ||       \
	  vcpu_has_feature(vcpu, KVM_ARM_VCPU_PTRAUTH_GENERIC)))
#else
#define vcpu_has_ptrauth(vcpu)		false
#endif

#define vcpu_on_unsupported_cpu(vcpu)					\
	vcpu_get_flag(vcpu, ON_UNSUPPORTED_CPU)

#define vcpu_set_on_unsupported_cpu(vcpu)				\
	vcpu_set_flag(vcpu, ON_UNSUPPORTED_CPU)

#define vcpu_clear_on_unsupported_cpu(vcpu)				\
	vcpu_clear_flag(vcpu, ON_UNSUPPORTED_CPU)

#define vcpu_gp_regs(v)		(&(v)->arch.ctxt.regs)


static inline u64 *___ctxt_sys_reg(const struct kvm_cpu_context *ctxt, int r)
{
#if !defined (__KVM_NVHE_HYPERVISOR__)
	if (unlikely(cpus_have_final_cap(ARM64_HAS_NESTED_VIRT) &&
		     r >= __VNCR_START__ && ctxt->vncr_array))
		return &ctxt->vncr_array[r - __VNCR_START__];
#endif
	return (u64 *)&ctxt->sys_regs[r];
}

#define __ctxt_sys_reg(c,r)						\
	({								\
		BUILD_BUG_ON(__builtin_constant_p(r) &&			\
			     (r) >= NR_SYS_REGS);			\
		___ctxt_sys_reg(c, r);					\
	})

#define ctxt_sys_reg(c,r)	(*__ctxt_sys_reg(c,r))

u64 kvm_vcpu_sanitise_vncr_reg(const struct kvm_vcpu *, enum vcpu_sysreg);
#define __vcpu_sys_reg(v,r)						\
	(*({								\
		const struct kvm_cpu_context *ctxt = &(v)->arch.ctxt;	\
		u64 *__r = __ctxt_sys_reg(ctxt, (r));			\
		if (vcpu_has_nv((v)) && (r) >= __VNCR_START__)		\
			*__r = kvm_vcpu_sanitise_vncr_reg((v), (r));	\
		__r;							\
	}))

static inline bool __vcpu_read_sys_reg_from_cpu(int reg, u64 *val)
{
	
	if (!has_vhe())
		return false;

	switch (reg) {
	case SCTLR_EL1:		*val = read_sysreg_s(SYS_SCTLR_EL12);	break;
	case CPACR_EL1:		*val = read_sysreg_s(SYS_CPACR_EL12);	break;
	case TTBR0_EL1:		*val = read_sysreg_s(SYS_TTBR0_EL12);	break;
	case TTBR1_EL1:		*val = read_sysreg_s(SYS_TTBR1_EL12);	break;
	case TCR_EL1:		*val = read_sysreg_s(SYS_TCR_EL12);	break;
	case ESR_EL1:		*val = read_sysreg_s(SYS_ESR_EL12);	break;
	case AFSR0_EL1:		*val = read_sysreg_s(SYS_AFSR0_EL12);	break;
	case AFSR1_EL1:		*val = read_sysreg_s(SYS_AFSR1_EL12);	break;
	case FAR_EL1:		*val = read_sysreg_s(SYS_FAR_EL12);	break;
	case MAIR_EL1:		*val = read_sysreg_s(SYS_MAIR_EL12);	break;
	case VBAR_EL1:		*val = read_sysreg_s(SYS_VBAR_EL12);	break;
	case CONTEXTIDR_EL1:	*val = read_sysreg_s(SYS_CONTEXTIDR_EL12);break;
	case TPIDR_EL0:		*val = read_sysreg_s(SYS_TPIDR_EL0);	break;
	case TPIDRRO_EL0:	*val = read_sysreg_s(SYS_TPIDRRO_EL0);	break;
	case TPIDR_EL1:		*val = read_sysreg_s(SYS_TPIDR_EL1);	break;
	case AMAIR_EL1:		*val = read_sysreg_s(SYS_AMAIR_EL12);	break;
	case CNTKCTL_EL1:	*val = read_sysreg_s(SYS_CNTKCTL_EL12);	break;
	case ELR_EL1:		*val = read_sysreg_s(SYS_ELR_EL12);	break;
	case SPSR_EL1:		*val = read_sysreg_s(SYS_SPSR_EL12);	break;
	case PAR_EL1:		*val = read_sysreg_par();		break;
	case DACR32_EL2:	*val = read_sysreg_s(SYS_DACR32_EL2);	break;
	case IFSR32_EL2:	*val = read_sysreg_s(SYS_IFSR32_EL2);	break;
	case DBGVCR32_EL2:	*val = read_sysreg_s(SYS_DBGVCR32_EL2);	break;
	case ZCR_EL1:		*val = read_sysreg_s(SYS_ZCR_EL12);	break;
	default:		return false;
	}

	return true;
}

static inline bool __vcpu_write_sys_reg_to_cpu(u64 val, int reg)
{
	
	if (!has_vhe())
		return false;

	switch (reg) {
	case SCTLR_EL1:		write_sysreg_s(val, SYS_SCTLR_EL12);	break;
	case CPACR_EL1:		write_sysreg_s(val, SYS_CPACR_EL12);	break;
	case TTBR0_EL1:		write_sysreg_s(val, SYS_TTBR0_EL12);	break;
	case TTBR1_EL1:		write_sysreg_s(val, SYS_TTBR1_EL12);	break;
	case TCR_EL1:		write_sysreg_s(val, SYS_TCR_EL12);	break;
	case ESR_EL1:		write_sysreg_s(val, SYS_ESR_EL12);	break;
	case AFSR0_EL1:		write_sysreg_s(val, SYS_AFSR0_EL12);	break;
	case AFSR1_EL1:		write_sysreg_s(val, SYS_AFSR1_EL12);	break;
	case FAR_EL1:		write_sysreg_s(val, SYS_FAR_EL12);	break;
	case MAIR_EL1:		write_sysreg_s(val, SYS_MAIR_EL12);	break;
	case VBAR_EL1:		write_sysreg_s(val, SYS_VBAR_EL12);	break;
	case CONTEXTIDR_EL1:	write_sysreg_s(val, SYS_CONTEXTIDR_EL12);break;
	case TPIDR_EL0:		write_sysreg_s(val, SYS_TPIDR_EL0);	break;
	case TPIDRRO_EL0:	write_sysreg_s(val, SYS_TPIDRRO_EL0);	break;
	case TPIDR_EL1:		write_sysreg_s(val, SYS_TPIDR_EL1);	break;
	case AMAIR_EL1:		write_sysreg_s(val, SYS_AMAIR_EL12);	break;
	case CNTKCTL_EL1:	write_sysreg_s(val, SYS_CNTKCTL_EL12);	break;
	case ELR_EL1:		write_sysreg_s(val, SYS_ELR_EL12);	break;
	case SPSR_EL1:		write_sysreg_s(val, SYS_SPSR_EL12);	break;
	case PAR_EL1:		write_sysreg_s(val, SYS_PAR_EL1);	break;
	case DACR32_EL2:	write_sysreg_s(val, SYS_DACR32_EL2);	break;
	case IFSR32_EL2:	write_sysreg_s(val, SYS_IFSR32_EL2);	break;
	case DBGVCR32_EL2:	write_sysreg_s(val, SYS_DBGVCR32_EL2);	break;
	case ZCR_EL1:		write_sysreg_s(val, SYS_ZCR_EL12);	break;
	default:		return false;
	}

	return true;
}

struct kvm_vm_stat {
	struct kvm_vm_stat_generic generic;
	atomic64_t protected_hyp_mem;
	atomic64_t protected_shared_mem;
};

struct kvm_vcpu_stat {
	struct kvm_vcpu_stat_generic generic;
	u64 hvc_exit_stat;
	u64 wfe_exit_stat;
	u64 wfi_exit_stat;
	u64 mmio_exit_user;
	u64 mmio_exit_kernel;
	u64 signal_exits;
	u64 exits;
};

unsigned long kvm_arm_num_regs(struct kvm_vcpu *vcpu);
int kvm_arm_copy_reg_indices(struct kvm_vcpu *vcpu, u64 __user *indices);
int kvm_arm_get_reg(struct kvm_vcpu *vcpu, const struct kvm_one_reg *reg);
int kvm_arm_set_reg(struct kvm_vcpu *vcpu, const struct kvm_one_reg *reg);

unsigned long kvm_arm_num_sys_reg_descs(struct kvm_vcpu *vcpu);
int kvm_arm_copy_sys_reg_indices(struct kvm_vcpu *vcpu, u64 __user *uindices);

int __kvm_arm_vcpu_get_events(struct kvm_vcpu *vcpu,
			      struct kvm_vcpu_events *events);

int __kvm_arm_vcpu_set_events(struct kvm_vcpu *vcpu,
			      struct kvm_vcpu_events *events);

void kvm_arm_halt_guest(struct kvm *kvm);
void kvm_arm_resume_guest(struct kvm *kvm);

#define vcpu_has_run_once(vcpu)	!!rcu_access_pointer((vcpu)->pid)

#ifndef __KVM_NVHE_HYPERVISOR__
#define kvm_call_hyp_nvhe_smccc(f, ...)					\
	({								\
		struct arm_smccc_res res;				\
									\
		arm_smccc_1_1_hvc(KVM_HOST_SMCCC_FUNC(f),		\
				  ##__VA_ARGS__, &res);			\
		WARN_ON(res.a0 != SMCCC_RET_SUCCESS);			\
									\
		res;							\
	})

#define kvm_call_hyp_nvhe(f, ...)					\
	({								\
		struct arm_smccc_res res;				\
									\
		arm_smccc_1_1_hvc(KVM_HOST_SMCCC_FUNC(f),		\
				  ##__VA_ARGS__, &res);			\
		WARN_ON(res.a0 != SMCCC_RET_SUCCESS);			\
									\
		res.a1;							\
	})


#define kvm_call_hyp(f, ...)						\
	do {								\
		if (has_vhe()) {					\
			f(__VA_ARGS__);					\
			isb();						\
		} else {						\
			kvm_call_hyp_nvhe(f, ##__VA_ARGS__);		\
		}							\
	} while(0)

#define kvm_call_hyp_ret(f, ...)					\
	({								\
		typeof(f(__VA_ARGS__)) ret;				\
									\
		if (has_vhe()) {					\
			ret = f(__VA_ARGS__);				\
			isb();						\
		} else {						\
			ret = kvm_call_hyp_nvhe(f, ##__VA_ARGS__);	\
		}							\
									\
		ret;							\
	})
#else 
#define kvm_call_hyp(f, ...) f(__VA_ARGS__)
#define kvm_call_hyp_ret(f, ...) f(__VA_ARGS__)
#define kvm_call_hyp_nvhe(f, ...) f(__VA_ARGS__)
#endif 

int handle_exit(struct kvm_vcpu *vcpu, int exception_index);
void handle_exit_early(struct kvm_vcpu *vcpu, int exception_index);

int kvm_handle_cp14_load_store(struct kvm_vcpu *vcpu);
int kvm_handle_cp14_32(struct kvm_vcpu *vcpu);
int kvm_handle_cp14_64(struct kvm_vcpu *vcpu);
int kvm_handle_cp15_32(struct kvm_vcpu *vcpu);
int kvm_handle_cp15_64(struct kvm_vcpu *vcpu);
int kvm_handle_sys_reg(struct kvm_vcpu *vcpu);
int kvm_handle_cp10_id(struct kvm_vcpu *vcpu);

void kvm_sys_regs_create_debugfs(struct kvm *kvm);
void kvm_reset_sys_regs(struct kvm_vcpu *vcpu);

int __init kvm_sys_reg_table_init(void);
struct sys_reg_desc;
int __init populate_sysreg_config(const struct sys_reg_desc *sr,
				  unsigned int idx);
int __init populate_nv_trap_config(void);

bool lock_all_vcpus(struct kvm *kvm);
void unlock_all_vcpus(struct kvm *kvm);

void kvm_calculate_traps(struct kvm_vcpu *vcpu);


void kvm_mmio_write_buf(void *buf, unsigned int len, unsigned long data);
unsigned long kvm_mmio_read_buf(const void *buf, unsigned int len);

int kvm_handle_mmio_return(struct kvm_vcpu *vcpu);
int io_mem_abort(struct kvm_vcpu *vcpu, phys_addr_t fault_ipa);


static inline bool kvm_arch_pmi_in_guest(struct kvm_vcpu *vcpu)
{
	return IS_ENABLED(CONFIG_GUEST_PERF_EVENTS) && !!vcpu;
}

long kvm_hypercall_pv_features(struct kvm_vcpu *vcpu);
gpa_t kvm_init_stolen_time(struct kvm_vcpu *vcpu);
void kvm_update_stolen_time(struct kvm_vcpu *vcpu);

bool kvm_arm_pvtime_supported(void);
int kvm_arm_pvtime_set_attr(struct kvm_vcpu *vcpu,
			    struct kvm_device_attr *attr);
int kvm_arm_pvtime_get_attr(struct kvm_vcpu *vcpu,
			    struct kvm_device_attr *attr);
int kvm_arm_pvtime_has_attr(struct kvm_vcpu *vcpu,
			    struct kvm_device_attr *attr);

extern unsigned int __ro_after_init kvm_arm_vmid_bits;
int __init kvm_arm_vmid_alloc_init(void);
void __init kvm_arm_vmid_alloc_free(void);
void kvm_arm_vmid_update(struct kvm_vmid *kvm_vmid);
void kvm_arm_vmid_clear_active(void);

static inline void kvm_arm_pvtime_vcpu_init(struct kvm_vcpu_arch *vcpu_arch)
{
	vcpu_arch->steal.base = INVALID_GPA;
}

static inline bool kvm_arm_is_pvtime_enabled(struct kvm_vcpu_arch *vcpu_arch)
{
	return (vcpu_arch->steal.base != INVALID_GPA);
}

void kvm_set_sei_esr(struct kvm_vcpu *vcpu, u64 syndrome);

struct kvm_vcpu *kvm_mpidr_to_vcpu(struct kvm *kvm, unsigned long mpidr);

DECLARE_KVM_HYP_PER_CPU(struct kvm_host_data, kvm_host_data);


#if defined(__KVM_NVHE_HYPERVISOR__) || defined(__KVM_VHE_HYPERVISOR__)
#define host_data_ptr(f)	(&this_cpu_ptr(&kvm_host_data)->f)
#else
#define host_data_ptr(f)						\
	(static_branch_unlikely(&kvm_protected_mode_initialized) ?	\
	 &this_cpu_ptr(&kvm_host_data)->f :				\
	 &this_cpu_ptr_hyp_sym(kvm_host_data)->f)
#endif


static inline bool guest_owns_fp_regs(void)
{
	return *host_data_ptr(fp_owner) == FP_STATE_GUEST_OWNED;
}


static inline bool host_owns_fp_regs(void)
{
	return *host_data_ptr(fp_owner) == FP_STATE_HOST_OWNED;
}

static inline void kvm_init_host_cpu_context(struct kvm_cpu_context *cpu_ctxt)
{
	
	ctxt_sys_reg(cpu_ctxt, MPIDR_EL1) = read_cpuid_mpidr();
}

static inline bool kvm_system_needs_idmapped_vectors(void)
{
	return cpus_have_final_cap(ARM64_SPECTRE_V3A);
}

static inline void kvm_arch_sync_events(struct kvm *kvm) {}

void kvm_arm_init_debug(void);
void kvm_arm_vcpu_init_debug(struct kvm_vcpu *vcpu);
void kvm_arm_setup_debug(struct kvm_vcpu *vcpu);
void kvm_arm_clear_debug(struct kvm_vcpu *vcpu);
void kvm_arm_reset_debug_ptr(struct kvm_vcpu *vcpu);

#define __vcpu_save_guest_debug_regs(vcpu)				\
	do {								\
		u64 val = vcpu_read_sys_reg(vcpu, MDSCR_EL1);		\
									\
		(vcpu)->arch.guest_debug_preserved.mdscr_el1 = val;	\
	} while(0)

#define __vcpu_restore_guest_debug_regs(vcpu)				\
	do {								\
		u64 val = (vcpu)->arch.guest_debug_preserved.mdscr_el1;	\
									\
		vcpu_write_sys_reg(vcpu, val, MDSCR_EL1);		\
	} while (0)

#define kvm_vcpu_os_lock_enabled(vcpu)		\
	(!!(__vcpu_sys_reg(vcpu, OSLSR_EL1) & OSLSR_EL1_OSLK))

#define kvm_vcpu_needs_debug_regs(vcpu)		\
	((vcpu)->guest_debug || kvm_vcpu_os_lock_enabled(vcpu))

int kvm_arm_vcpu_arch_set_attr(struct kvm_vcpu *vcpu,
			       struct kvm_device_attr *attr);
int kvm_arm_vcpu_arch_get_attr(struct kvm_vcpu *vcpu,
			       struct kvm_device_attr *attr);
int kvm_arm_vcpu_arch_has_attr(struct kvm_vcpu *vcpu,
			       struct kvm_device_attr *attr);

int kvm_vm_ioctl_mte_copy_tags(struct kvm *kvm,
			       struct kvm_arm_copy_mte_tags *copy_tags);
int kvm_vm_ioctl_set_counter_offset(struct kvm *kvm,
				    struct kvm_arm_counter_offset *offset);
int kvm_vm_ioctl_get_reg_writable_masks(struct kvm *kvm,
					struct reg_mask_range *range);


int kvm_arch_vcpu_run_map_fp(struct kvm_vcpu *vcpu);
void kvm_arch_vcpu_load_fp(struct kvm_vcpu *vcpu);
void kvm_arch_vcpu_ctxflush_fp(struct kvm_vcpu *vcpu);
void kvm_arch_vcpu_ctxsync_fp(struct kvm_vcpu *vcpu);
void kvm_arch_vcpu_put_fp(struct kvm_vcpu *vcpu);

static inline bool kvm_pmu_counter_deferred(struct perf_event_attr *attr)
{
	return (!has_vhe() && attr->exclude_host);
}


void kvm_arch_vcpu_load_debug_state_flags(struct kvm_vcpu *vcpu);
void kvm_arch_vcpu_put_debug_state_flags(struct kvm_vcpu *vcpu);

#ifdef CONFIG_KVM
void kvm_set_pmu_events(u64 set, struct perf_event_attr *attr);
void kvm_clr_pmu_events(u64 clr);
bool kvm_set_pmuserenr(u64 val);
#else
static inline void kvm_set_pmu_events(u64 set, struct perf_event_attr *attr) {}
static inline void kvm_clr_pmu_events(u64 clr) {}
static inline bool kvm_set_pmuserenr(u64 val)
{
	return false;
}
#endif

void kvm_vcpu_load_vhe(struct kvm_vcpu *vcpu);
void kvm_vcpu_put_vhe(struct kvm_vcpu *vcpu);

int __init kvm_set_ipa_limit(void);
u32 kvm_get_pa_bits(struct kvm *kvm);

#define __KVM_HAVE_ARCH_VM_ALLOC
struct kvm *kvm_arch_alloc_vm(void);

#define __KVM_HAVE_ARCH_FLUSH_REMOTE_TLBS

#define __KVM_HAVE_ARCH_FLUSH_REMOTE_TLBS_RANGE

#define kvm_vm_is_protected(kvm)	(is_protected_kvm_enabled() && (kvm)->arch.pkvm.enabled)

#define vcpu_is_protected(vcpu)		kvm_vm_is_protected((vcpu)->kvm)

int kvm_arm_vcpu_finalize(struct kvm_vcpu *vcpu, int feature);
bool kvm_arm_vcpu_is_finalized(struct kvm_vcpu *vcpu);

#define kvm_arm_vcpu_sve_finalized(vcpu) vcpu_get_flag(vcpu, VCPU_SVE_FINALIZED)

#define kvm_has_mte(kvm)					\
	(system_supports_mte() &&				\
	 test_bit(KVM_ARCH_FLAG_MTE_ENABLED, &(kvm)->arch.flags))

#define kvm_supports_32bit_el0()				\
	(system_supports_32bit_el0() &&				\
	 !static_branch_unlikely(&arm64_mismatched_32bit_el0))

#define kvm_vm_has_ran_once(kvm)					\
	(test_bit(KVM_ARCH_FLAG_HAS_RAN_ONCE, &(kvm)->arch.flags))

static inline bool __vcpu_has_feature(const struct kvm_arch *ka, int feature)
{
	return test_bit(feature, ka->vcpu_features);
}

#define kvm_vcpu_has_feature(k, f)	__vcpu_has_feature(&(k)->arch, (f))
#define vcpu_has_feature(v, f)	__vcpu_has_feature(&(v)->kvm->arch, (f))

#define kvm_vcpu_initialized(v) vcpu_get_flag(vcpu, VCPU_INITIALIZED)

int kvm_trng_call(struct kvm_vcpu *vcpu);
#ifdef CONFIG_KVM
extern phys_addr_t hyp_mem_base;
extern phys_addr_t hyp_mem_size;
void __init kvm_hyp_reserve(void);
#else
static inline void kvm_hyp_reserve(void) { }
#endif

void kvm_arm_vcpu_power_off(struct kvm_vcpu *vcpu);
bool kvm_arm_vcpu_stopped(struct kvm_vcpu *vcpu);

static inline u64 *__vm_id_reg(struct kvm_arch *ka, u32 reg)
{
	switch (reg) {
	case sys_reg(3, 0, 0, 1, 0) ... sys_reg(3, 0, 0, 7, 7):
		return &ka->id_regs[IDREG_IDX(reg)];
	case SYS_CTR_EL0:
		return &ka->ctr_el0;
	default:
		WARN_ON_ONCE(1);
		return NULL;
	}
}

#define kvm_read_vm_id_reg(kvm, reg)					\
	({ u64 __val = *__vm_id_reg(&(kvm)->arch, reg); __val; })

void kvm_set_vm_id_reg(struct kvm *kvm, u32 reg, u64 val);

#define __expand_field_sign_unsigned(id, fld, val)			\
	((u64)SYS_FIELD_VALUE(id, fld, val))

#define __expand_field_sign_signed(id, fld, val)			\
	({								\
		u64 __val = SYS_FIELD_VALUE(id, fld, val);		\
		sign_extend64(__val, id##_##fld##_WIDTH - 1);		\
	})

#define get_idreg_field_unsigned(kvm, id, fld)				\
	({								\
		u64 __val = kvm_read_vm_id_reg((kvm), SYS_##id);	\
		FIELD_GET(id##_##fld##_MASK, __val);			\
	})

#define get_idreg_field_signed(kvm, id, fld)				\
	({								\
		u64 __val = get_idreg_field_unsigned(kvm, id, fld);	\
		sign_extend64(__val, id##_##fld##_WIDTH - 1);		\
	})

#define get_idreg_field_enum(kvm, id, fld)				\
	get_idreg_field_unsigned(kvm, id, fld)

#define kvm_cmp_feat_signed(kvm, id, fld, op, limit)			\
	(get_idreg_field_signed((kvm), id, fld) op __expand_field_sign_signed(id, fld, limit))

#define kvm_cmp_feat_unsigned(kvm, id, fld, op, limit)			\
	(get_idreg_field_unsigned((kvm), id, fld) op __expand_field_sign_unsigned(id, fld, limit))

#define kvm_cmp_feat(kvm, id, fld, op, limit)				\
	(id##_##fld##_SIGNED ?						\
	 kvm_cmp_feat_signed(kvm, id, fld, op, limit) :			\
	 kvm_cmp_feat_unsigned(kvm, id, fld, op, limit))

#define kvm_has_feat(kvm, id, fld, limit)				\
	kvm_cmp_feat(kvm, id, fld, >=, limit)

#define kvm_has_feat_enum(kvm, id, fld, val)				\
	kvm_cmp_feat_unsigned(kvm, id, fld, ==, val)

#define kvm_has_feat_range(kvm, id, fld, min, max)			\
	(kvm_cmp_feat(kvm, id, fld, >=, min) &&				\
	kvm_cmp_feat(kvm, id, fld, <=, max))


#define kvm_has_pauth(k, l)						\
	({								\
		bool pa, pi, pa3;					\
									\
		pa  = kvm_has_feat((k), ID_AA64ISAR1_EL1, APA, l);	\
		pa &= kvm_has_feat((k), ID_AA64ISAR1_EL1, GPA, IMP);	\
		pi  = kvm_has_feat((k), ID_AA64ISAR1_EL1, API, l);	\
		pi &= kvm_has_feat((k), ID_AA64ISAR1_EL1, GPI, IMP);	\
		pa3  = kvm_has_feat((k), ID_AA64ISAR2_EL1, APA3, l);	\
		pa3 &= kvm_has_feat((k), ID_AA64ISAR2_EL1, GPA3, IMP);	\
									\
		(pa + pi + pa3) == 1;					\
	})

#define kvm_has_fpmr(k)					\
	(system_supports_fpmr() &&			\
	 kvm_has_feat((k), ID_AA64PFR2_EL1, FPMR, IMP))


#define HYP_ALLOC_MGT_HEAP_ID		0
#define HYP_ALLOC_MGT_IOMMU_ID		1

unsigned long __pkvm_reclaim_hyp_alloc_mgt(unsigned long nr_pages);
int __pkvm_topup_hyp_alloc_mgt_mc(unsigned long id, struct kvm_hyp_memcache *mc);
int __pkvm_topup_hyp_alloc_mgt_gfp(unsigned long id, unsigned long nr_pages,
				   unsigned long sz_alloc, gfp_t gfp);

struct kvm_iommu_driver {
	int (*init_driver)(void);
	void (*remove_driver)(void);
	pkvm_handle_t (*get_iommu_id_by_of)(struct device_node *np);
	int (*get_device_iommu_num_ids)(struct device *dev);
	int (*get_device_iommu_id)(struct device *dev, u32 id,
				   pkvm_handle_t *out_iommu, u32 *out_sid);
	void *(*guest_alloc)(void *flags, unsigned long order);
	void (*guest_free)(void *addr, void *flags, unsigned long order);
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	ANDROID_KABI_RESERVE(3);
	ANDROID_KABI_RESERVE(4);
	ANDROID_KABI_RESERVE(5);
	ANDROID_KABI_RESERVE(6);
	ANDROID_KABI_RESERVE(7);
	ANDROID_KABI_RESERVE(8);
};

struct kvm_iommu_ops;
int kvm_iommu_register_driver(struct kvm_iommu_driver *kern_ops);
int kvm_iommu_init_hyp(struct kvm_iommu_ops *hyp_ops,
		       struct kvm_hyp_memcache *atomic_mc);
int kvm_iommu_init_driver(void);
void kvm_iommu_remove_driver(void);
pkvm_handle_t kvm_get_iommu_id_by_of(struct device_node *np);

struct page *kvm_iommu_cma_alloc(void);
bool kvm_iommu_cma_release(struct page *p);

int pkvm_iommu_suspend(struct device *dev);
int pkvm_iommu_resume(struct device *dev);

int kvm_iommu_guest_alloc_mc(struct kvm_hyp_memcache *mc, u32 pgsize, u32 nr_pages);
void kvm_iommu_guest_free_mc(struct kvm_hyp_memcache *mc);

struct kvm_iommu_sg {
	phys_addr_t phys;
	size_t pgsize;
	unsigned int pgcount;
};


#define kvm_iommu_sg_nents_size(n) (PAGE_ALIGN((n) * sizeof(struct kvm_iommu_sg)))

static inline unsigned int kvm_iommu_sg_nents_round(unsigned int nents)
{
	return kvm_iommu_sg_nents_size(nents) / sizeof(struct kvm_iommu_sg);
}

static inline struct kvm_iommu_sg *kvm_iommu_sg_alloc(unsigned int nents, gfp_t gfp)
{
	return alloc_pages_exact(kvm_iommu_sg_nents_size(nents), gfp);
}

static inline void kvm_iommu_sg_free(struct kvm_iommu_sg *sg, unsigned int nents)
{
	free_pages_exact(sg, kvm_iommu_sg_nents_size(nents));
}


#ifndef __KVM_NVHE_HYPERVISOR__
int kvm_iommu_attach_dev(pkvm_handle_t iommu_id, pkvm_handle_t domain_id,
			 unsigned int endpoint, unsigned int pasid,
			 unsigned int ssid_bits, unsigned long flags);
int kvm_iommu_attach_dev_nested(pkvm_handle_t iommu_id, pkvm_handle_t domain_id,
				unsigned int endpoint, unsigned int pasid,
				unsigned long flags, void *s1_desc_hva,
				size_t s1_desc_size);
int kvm_iommu_detach_dev(pkvm_handle_t iommu_id, pkvm_handle_t domain_id,
			 unsigned int endpoint, unsigned int pasid);
int kvm_iommu_detach_dev_nested(pkvm_handle_t iommu_id, pkvm_handle_t domain_id,
				unsigned int endpoint, unsigned int pasid);
int kvm_iommu_alloc_domain(pkvm_handle_t domain_id, int type);
int kvm_iommu_free_domain(pkvm_handle_t domain_id);
int kvm_iommu_map_pages(pkvm_handle_t domain_id, unsigned long iova,
			phys_addr_t paddr, size_t pgsize, size_t pgcount,
			int prot, gfp_t gfp, size_t *total_mapped);
size_t kvm_iommu_unmap_pages(pkvm_handle_t domain_id, unsigned long iova,
			     size_t pgsize, size_t pgcount);
phys_addr_t kvm_iommu_iova_to_phys(pkvm_handle_t domain_id, unsigned long iova);
int kvm_iommu_iotlb_sync_map(pkvm_handle_t domain_id, unsigned long iova, size_t size);
size_t kvm_iommu_map_sg(pkvm_handle_t domain_id, struct kvm_iommu_sg *sg,
			unsigned long iova, unsigned int nent,
			unsigned int prot, gfp_t gfp);
int kvm_iommu_iotlb_inv_nested_domain(pkvm_handle_t domain_id, unsigned long iova, size_t size,
				      size_t granule, bool leaf);
int kvm_iommu_nested_cfg_sync(pkvm_handle_t iommu_id, void *cmd_desc_hva, size_t cmd_desc_size);
#endif

int kvm_iommu_share_hyp_sg(struct kvm_iommu_sg *sg, unsigned int nents);
int kvm_iommu_unshare_hyp_sg(struct kvm_iommu_sg *sg, unsigned int nents);
int kvm_iommu_device_num_ids(struct device *dev);
int kvm_iommu_device_id(struct device *dev, u32 idx,
			pkvm_handle_t *out_iommu, u32 *out_sid);
#define __KVM_HAVE_ARCH_ASSIGNED_DEVICE_GROUP

static inline phys_addr_t kvm_host_pa(void *addr)
{
	return __pa(addr);
}

static inline void *kvm_host_va(phys_addr_t phys)
{
	return __va(phys);
}
#endif 
