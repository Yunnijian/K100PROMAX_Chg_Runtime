/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __LINUX_IRQCHIP_ARM_GIC_COMMON_H
#define __LINUX_IRQCHIP_ARM_GIC_COMMON_H

#include <linux/irqchip/arm-vgic-info.h>

#define GICD_INT_DEF_PRI		0xa0

struct irq_domain;
struct fwnode_handle;
int gicv2m_init(struct fwnode_handle *parent_handle,
		struct irq_domain *parent);

#endif 
