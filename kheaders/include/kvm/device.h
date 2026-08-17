// SPDX-License-Identifier: GPL-2.0-only


#ifndef __KVM_DEVICE_H
#define __KVM_DEVICE_H

#include <asm/kvm_host.h>


struct pkvm_dev_resource {
	u64 base;
	u64 size;
};


struct pkvm_dev_iommu {
	u64 id;
	u64 endpoint;
};

#define PKVM_DEVICE_MAX_RESOURCE	32
#define PKVM_DEVICE_MAX_IOMMU		32

struct pkvm_device {
	struct pkvm_dev_resource resources[PKVM_DEVICE_MAX_RESOURCE];
	struct pkvm_dev_iommu iommus[PKVM_DEVICE_MAX_IOMMU];
	u32 nr_resources;
	u32 nr_iommus;
	u32 group_id;
	void *ctxt; 
	unsigned short refcount;
	int (*reset_handler)(void *cookie, bool host_to_guest);
	void *cookie; 
};

#endif 
