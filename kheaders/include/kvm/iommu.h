/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KVM_IOMMU_H
#define __KVM_IOMMU_H

#include <asm/kvm_host.h>
#include <kvm/power_domain.h>
#include <linux/io-pgtable.h>


#define KVM_IOMMU_DOMAIN_IDMAP_ID		0


#define KVM_IOMMU_DOMAIN_IDMAP_TYPE		0

#define KVM_IOMMU_DOMAIN_ANY_TYPE		1

#define KVM_IOMMU_DOMAIN_NR_START		(KVM_IOMMU_DOMAIN_IDMAP_ID + 1)

struct kvm_hyp_iommu_domain {
	atomic_t		refs;
	pkvm_handle_t		domain_id;
	void			*priv;
	void			*vm;
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};

extern void **kvm_nvhe_sym(kvm_hyp_iommu_domains);
#define kvm_hyp_iommu_domains kvm_nvhe_sym(kvm_hyp_iommu_domains)


#define KVM_IOMMU_MAX_DOMAINS	(1 << 16)


#define KVM_IOMMU_DOMAINS_PER_PAGE \
	(PAGE_SIZE / sizeof(struct kvm_hyp_iommu_domain))


#define KVM_IOMMU_DOMAINS_ROOT_ENTRIES \
	(KVM_IOMMU_MAX_DOMAINS / KVM_IOMMU_DOMAINS_PER_PAGE)

#define KVM_IOMMU_DOMAINS_ROOT_SIZE \
	(KVM_IOMMU_DOMAINS_ROOT_ENTRIES * sizeof(void *))

#define KVM_IOMMU_DOMAINS_ROOT_ORDER_NR	\
	(1 << get_order(KVM_IOMMU_DOMAINS_ROOT_SIZE))

struct kvm_hyp_iommu {
	u32				lock;   
	struct kvm_power_domain		power_domain;
	bool				power_is_off;
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	ANDROID_KABI_RESERVE(3);
	ANDROID_KABI_RESERVE(4);
};

#endif 
