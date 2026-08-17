/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __LINUX_IRQCHIP_ARM_GIC_V3_PRIO_H
#define __LINUX_IRQCHIP_ARM_GIC_V3_PRIO_H


#define GICV3_PRIO_UNMASKED	0xe0
#define GICV3_PRIO_IRQ		0xc0
#define GICV3_PRIO_NMI		0x80

#define GICV3_PRIO_PSR_I_SET	(1 << 4)

#ifndef __ASSEMBLER__

#define __gicv3_prio_to_ns(p)	(0xff & ((p) << 1))
#define __gicv3_ns_to_prio(ns)	(0x80 | ((ns) >> 1))

#define __gicv3_prio_valid_ns(p) \
	(__gicv3_ns_to_prio(__gicv3_prio_to_ns(p)) == (p))

static_assert(__gicv3_prio_valid_ns(GICV3_PRIO_NMI));
static_assert(__gicv3_prio_valid_ns(GICV3_PRIO_IRQ));

static_assert(GICV3_PRIO_NMI < GICV3_PRIO_IRQ);
static_assert(GICV3_PRIO_IRQ < GICV3_PRIO_UNMASKED);

static_assert(GICV3_PRIO_IRQ < (GICV3_PRIO_IRQ | GICV3_PRIO_PSR_I_SET));

#endif 

#endif 
