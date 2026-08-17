/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __KVM_ARM_PSCI_H__
#define __KVM_ARM_PSCI_H__

#include <linux/kvm_host.h>
#include <uapi/linux/psci.h>

#define KVM_ARM_PSCI_0_1	PSCI_VERSION(0, 1)
#define KVM_ARM_PSCI_0_2	PSCI_VERSION(0, 2)
#define KVM_ARM_PSCI_1_0	PSCI_VERSION(1, 0)
#define KVM_ARM_PSCI_1_1	PSCI_VERSION(1, 1)

#define KVM_ARM_PSCI_LATEST	KVM_ARM_PSCI_1_1

static inline int kvm_psci_version(struct kvm_vcpu *vcpu)
{
	
	if (vcpu_has_feature(vcpu, KVM_ARM_VCPU_PSCI_0_2)) {
		if (vcpu->kvm->arch.psci_version)
			return vcpu->kvm->arch.psci_version;

		return KVM_ARM_PSCI_LATEST;
	}

	return KVM_ARM_PSCI_0_1;
}


static inline void kvm_psci_narrow_to_32bit(struct kvm_vcpu *vcpu)
{
	int i;

	
	for (i = 1; i < 4; i++)
		vcpu_set_reg(vcpu, i, lower_32_bits(vcpu_get_reg(vcpu, i)));
}

static inline bool kvm_psci_valid_affinity(struct kvm_vcpu *vcpu,
					   unsigned long affinity)
{
	return !(affinity & ~MPIDR_HWID_BITMASK);
}


#define AFFINITY_MASK(level)	~((0x1UL << ((level) * MPIDR_LEVEL_BITS)) - 1)

static inline unsigned long psci_affinity_mask(unsigned long affinity_level)
{
	if (affinity_level <= 3)
		return MPIDR_HWID_BITMASK & AFFINITY_MASK(affinity_level);

	return 0;
}

int kvm_psci_call(struct kvm_vcpu *vcpu);

#endif 
