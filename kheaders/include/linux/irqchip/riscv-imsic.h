/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __LINUX_IRQCHIP_RISCV_IMSIC_H
#define __LINUX_IRQCHIP_RISCV_IMSIC_H

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/fwnode.h>
#include <asm/csr.h>

#define IMSIC_MMIO_PAGE_SHIFT		12
#define IMSIC_MMIO_PAGE_SZ		BIT(IMSIC_MMIO_PAGE_SHIFT)
#define IMSIC_MMIO_PAGE_LE		0x00
#define IMSIC_MMIO_PAGE_BE		0x04

#define IMSIC_MIN_ID			63
#define IMSIC_MAX_ID			2048

#define IMSIC_EIDELIVERY		0x70

#define IMSIC_EITHRESHOLD		0x72

#define IMSIC_EIP0			0x80
#define IMSIC_EIP63			0xbf
#define IMSIC_EIPx_BITS			32

#define IMSIC_EIE0			0xc0
#define IMSIC_EIE63			0xff
#define IMSIC_EIEx_BITS			32

#define IMSIC_FIRST			IMSIC_EIDELIVERY
#define IMSIC_LAST			IMSIC_EIE63

#define IMSIC_MMIO_SETIPNUM_LE		0x00
#define IMSIC_MMIO_SETIPNUM_BE		0x04

struct imsic_local_config {
	phys_addr_t				msi_pa;
	void __iomem				*msi_va;
};

struct imsic_global_config {
	

	
	u32					guest_index_bits;
	u32					hart_index_bits;
	u32					group_index_bits;
	u32					group_index_shift;

	
	phys_addr_t				base_addr;

	
	u32					nr_ids;

	
	u32					nr_guest_ids;

	
	struct imsic_local_config __percpu	*local;
};

#ifdef CONFIG_RISCV_IMSIC

const struct imsic_global_config *imsic_get_global_config(void);

#else

static inline const struct imsic_global_config *imsic_get_global_config(void)
{
	return NULL;
}

#endif

#ifdef CONFIG_ACPI
int imsic_platform_acpi_probe(struct fwnode_handle *fwnode);
struct fwnode_handle *imsic_acpi_get_fwnode(struct device *dev);
#else
static inline struct fwnode_handle *imsic_acpi_get_fwnode(struct device *dev) { return NULL; }
#endif

#endif
