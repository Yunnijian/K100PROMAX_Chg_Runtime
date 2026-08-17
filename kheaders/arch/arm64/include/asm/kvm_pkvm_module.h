/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __ARM64_KVM_PKVM_MODULE_H__
#define __ARM64_KVM_PKVM_MODULE_H__

#include <asm/kvm_pgtable.h>
#include <linux/android_kabi.h>
#include <linux/export.h>

typedef void (*dyn_hcall_t)(struct user_pt_regs *);
struct kvm_hyp_iommu;
struct iommu_iotlb_gather;
struct kvm_hyp_iommu_domain;
struct pkvm_device;

#if defined(CONFIG_MODULES) && defined(CONFIG_KVM)
enum pkvm_psci_notification {
	PKVM_PSCI_CPU_SUSPEND,
	PKVM_PSCI_SYSTEM_SUSPEND,
	PKVM_PSCI_CPU_ENTRY,
};

struct pkvm_sglist_page {
	u64	pfn : 40;
	u8	order;
} __packed;


struct pkvm_module_trng_ops {
	const uuid_t *trng_uuid;
	int (*trng_rnd64)(u64 *entropy, int bits);

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};


struct pkvm_module_ops {
	int (*create_private_mapping)(phys_addr_t phys, size_t size,
				      enum kvm_pgtable_prot prot,
				      unsigned long *haddr);
	void *(*alloc_module_va)(u64 nr_pages);
	int (*map_module_page)(u64 pfn, void *va, enum kvm_pgtable_prot prot, bool is_protected);
	int (*register_serial_driver)(void (*hyp_putc_cb)(char));
	void (*putc)(char c);
	void (*puts)(const char *s);
	void (*putx64)(u64 x);
	void *(*fixmap_map)(phys_addr_t phys);
	void (*fixmap_unmap)(void);
	void *(*fixblock_map)(phys_addr_t phys);
	void (*fixblock_unmap)(void);
	void *(*linear_map_early)(phys_addr_t phys, size_t size, enum kvm_pgtable_prot prot);
	void (*linear_unmap_early)(void *addr, size_t size);
	void (*flush_dcache_to_poc)(void *addr, size_t size);
	void (*update_hcr_el2)(unsigned long set_mask, unsigned long clear_mask);
	void (*update_hfgwtr_el2)(unsigned long set_mask, unsigned long clear_mask);
	int (*register_host_perm_fault_handler)(int (*cb)(struct user_pt_regs *regs, u64 esr, u64 addr));
	int (*host_stage2_mod_prot)(u64 pfn, enum kvm_pgtable_prot prot, u64 nr_pages, bool update_iommu);
	int (*host_stage2_get_leaf)(phys_addr_t phys, kvm_pte_t *ptep, s8 *level);
	int (*host_stage2_enable_lazy_pte)(u64 addr, u64 nr_pages);
	int (*host_stage2_disable_lazy_pte)(u64 addr, u64 nr_pages);
	int (*register_host_smc_handler)(bool (*cb)(struct user_pt_regs *));
	int (*register_guest_smc_handler)(bool (*cb)(struct arm_smccc_1_2_regs *regs,
						     struct arm_smccc_1_2_regs *res,
						     pkvm_handle_t handle));
	int (*register_default_trap_handler)(bool (*cb)(struct user_pt_regs *));
	int (*register_illegal_abt_notifier)(void (*cb)(struct user_pt_regs *));
	int (*register_psci_notifier)(void (*cb)(enum pkvm_psci_notification, struct user_pt_regs *));
	int (*register_hyp_panic_notifier)(void (*cb)(struct user_pt_regs *));
	int (*register_unmask_serror)(bool (*unmask)(void), void (*mask)(void));
	int (*host_donate_hyp)(u64 pfn, u64 nr_pages, bool accept_mmio);
	int (*host_donate_hyp_prot)(u64 pfn, u64 nr_pages, bool accept_mmio, enum kvm_pgtable_prot prot);
	int (*host_donate_sglist_hyp)(struct pkvm_sglist_page *sglist, size_t nr_pages);
	int (*hyp_donate_host)(u64 pfn, u64 nr_pages);
	int (*host_share_hyp)(u64 pfn);
	int (*host_unshare_hyp)(u64 pfn);
	int (*pin_shared_mem)(void *from, void *to);
	void (*unpin_shared_mem)(void *from, void *to);
	void* (*memcpy)(void *to, const void *from, size_t count);
	void* (*memset)(void *dst, int c, size_t count);
	phys_addr_t (*hyp_pa)(void *x);
	void* (*hyp_va)(phys_addr_t phys);
	unsigned long (*kern_hyp_va)(unsigned long x);
	void* (*tracing_reserve_entry)(unsigned long length);
	void (*tracing_commit_entry)(void);
	void (*tracing_mod_hyp_printk)(u8 fmt_id, u64 a, u64 b, u64 c, u64 d);
	void * (*hyp_alloc)(size_t size);
	int (*hyp_alloc_errno)(void);
	void (*hyp_free)(void *addr);
	u8 (*hyp_alloc_missing_donations)(void);
	void * (*iommu_donate_pages)(u8 order, int flags);
	void (*iommu_reclaim_pages)(void *p, u8 order);
	int (*iommu_init_device)(struct kvm_hyp_iommu *iommu);
	void (*udelay)(unsigned long usecs);
	void (*iommu_iotlb_gather_add_page)(struct kvm_hyp_iommu_domain *domain,
					    struct iommu_iotlb_gather *gather,
					    unsigned long iova,
					    size_t size);
	int (*pkvm_unuse_dma)(phys_addr_t phys_addr, size_t size);
#ifdef CONFIG_LIST_HARDENED
	
	typeof(__list_add_valid_or_report) *list_add_valid_or_report;
	typeof(__list_del_entry_valid_or_report) *list_del_entry_valid_or_report;
#endif
	int (*iommu_snapshot_host_stage2)(struct kvm_hyp_iommu_domain *domain);
	void * (*iommu_donate_pages_atomic)(u8 order);
	void (*iommu_reclaim_pages_atomic)(void *p, u8 order);
	int (*hyp_smp_processor_id)(void);
	int (*device_register_reset)(u64 phys, void *cookie,
				     int (*cb)(void *cookie, bool host_to_guest));
	ANDROID_KABI_USE(1, int (*register_guest_trng_ops)(
				    const struct pkvm_module_trng_ops *ops));
	ANDROID_KABI_USE(2, int (*map_module_pages)(u64 pfn, void *va, u64 nr_pages,
				    enum kvm_pgtable_prot prot, bool is_protected));
	ANDROID_KABI_USE(3, int (*unmap_module_pages)(u64 pfn, void *va, u64 nr_pages));
	ANDROID_KABI_USE(4, int (*request_hyp_alloc)(void));
	ANDROID_KABI_RESERVE(5);
	ANDROID_KABI_RESERVE(6);
	ANDROID_KABI_RESERVE(7);
	ANDROID_KABI_RESERVE(8);
	ANDROID_KABI_RESERVE(9);
	ANDROID_KABI_RESERVE(10);
	ANDROID_KABI_RESERVE(11);
	ANDROID_KABI_RESERVE(12);
	ANDROID_KABI_RESERVE(13);
	ANDROID_KABI_RESERVE(14);
	ANDROID_KABI_RESERVE(15);
	ANDROID_KABI_RESERVE(16);
	ANDROID_KABI_RESERVE(17);
	ANDROID_KABI_RESERVE(18);
	ANDROID_KABI_RESERVE(19);
	ANDROID_KABI_RESERVE(20);
	ANDROID_KABI_RESERVE(21);
	ANDROID_KABI_RESERVE(22);
	ANDROID_KABI_RESERVE(23);
	ANDROID_KABI_RESERVE(24);
	ANDROID_KABI_RESERVE(25);
	ANDROID_KABI_RESERVE(26);
	ANDROID_KABI_RESERVE(27);
	ANDROID_KABI_RESERVE(28);
	ANDROID_KABI_RESERVE(29);
	ANDROID_KABI_RESERVE(30);
	ANDROID_KABI_RESERVE(31);
	ANDROID_KABI_RESERVE(32);
};

int __pkvm_load_el2_module(struct module *this, unsigned long *token);

int __pkvm_register_el2_call(unsigned long hfn_hyp_va);

unsigned long pkvm_el2_mod_kern_va(unsigned long addr);

static inline unsigned long __pkvm_el2_mod_va(struct pkvm_el2_module *mod, void *kern_va)
{
	unsigned long offset = (unsigned long)(kern_va - mod->sections.start);

	WARN_ON(kern_va < mod->sections.start || kern_va >= mod->sections.end);

	return (unsigned long)mod->hyp_va + offset;
}

void pkvm_el2_mod_frob_sections(Elf_Ehdr *ehdr, Elf_Shdr *sechdrs, char *secstrings);
#else
static inline int __pkvm_load_el2_module(struct module *this,
					 unsigned long *token)
{
	return -ENOSYS;
}

static inline int __pkvm_register_el2_call(unsigned long hfn_hyp_va)
{
	return -ENOSYS;
}

static inline unsigned long pkvm_el2_mod_kern_va(unsigned long addr)
{
	return 0;
}

static inline unsigned long __pkvm_el2_mod_va(void *mod, void *kern_va)
{
	WARN_ON(1);
	return 0;
}
#endif 

int pkvm_load_early_modules(void);

#ifdef MODULE




#define pkvm_el2_mod_va(kern_va, token) __pkvm_el2_mod_va(&THIS_MODULE->arch.hyp, kern_va)

#define pkvm_load_el2_module(init_fn, token)				\
({									\
	THIS_MODULE->arch.hyp.init = init_fn;				\
	__pkvm_load_el2_module(THIS_MODULE, token);			\
})

static inline int pkvm_register_el2_mod_call(dyn_hcall_t hfn,
					     unsigned long token)
{
	return __pkvm_register_el2_call(pkvm_el2_mod_va(hfn, token));
}

#define pkvm_el2_mod_call(id, ...)					\
	({								\
		struct arm_smccc_res res;				\
									\
		arm_smccc_1_1_hvc(KVM_HOST_SMCCC_ID(id),		\
				  ##__VA_ARGS__, &res);			\
		WARN_ON(res.a0 != SMCCC_RET_SUCCESS);			\
									\
		res.a1;							\
	})

#define pkvm_el2_mod_call_smccc(id, ...)				\
	({								\
		struct arm_smccc_res res;				\
									\
		arm_smccc_1_1_hvc(KVM_HOST_SMCCC_ID(id),		\
				  ##__VA_ARGS__, &res);			\
		WARN_ON(res.a0 != SMCCC_RET_SUCCESS);			\
									\
		res;							\
	})
#endif
#endif
