/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_GUNYAH_H
#define _ASM_GUNYAH_H

#include <linux/irq.h>
#include <linux/irqdomain.h>

static inline int arch_gunyah_fill_irq_fwspec_params(u32 virq,
						 struct irq_fwspec *fwspec)
{
	
	if (WARN(virq < 32, "Unexpected virq: %d\n", virq)) {
		return -EINVAL;
	} else if (virq <= 1019) {
		fwspec->param_count = 3;
		fwspec->param[0] = 0; 
		fwspec->param[1] = virq - 32; 
		fwspec->param[2] = IRQ_TYPE_EDGE_RISING;
	} else if (WARN(virq < 4096, "Unexpected virq: %d\n", virq)) {
		return -EINVAL;
	} else if (virq < 5120) {
		fwspec->param_count = 3;
		fwspec->param[0] = 2; 
		fwspec->param[1] = virq - 4096; 
		fwspec->param[2] = IRQ_TYPE_EDGE_RISING;
	} else {
		WARN(1, "Unexpected virq: %d\n", virq);
		return -EINVAL;
	}
	return 0;
}

enum arch_gunyah_memtype {
	
	GUNYAH_MEMTYPE_DEVICE_nGnRnE	= 0,
	GUNYAH_DEVICE_nGnRE		= 1,
	GUNYAH_DEVICE_nGRE		= 2,
	GUNYAH_DEVICE_GRE		= 3,

	GUNYAH_NORMAL_NC	= 0b0101,
	GUNYAH_NORMAL_ONC_IWT	= 0b0110,
	GUNYAH_NORMAL_ONC_IWB	= 0b0111,
	GUNYAH_NORMAL_OWT_INC	= 0b1001,
	GUNYAH_NORMAL_WT	= 0b1010,
	GUNYAH_NORMAL_OWT_IWB	= 0b1011,
	GUNYAH_NORMAL_OWB_INC	= 0b1101,
	GUNYAH_NORMAL_OWB_IWT	= 0b1110,
	GUNYAH_NORMAL_WB	= 0b1111,
	
};

#define ARCH_GUNYAH_DEFAULT_MEMTYPE	GUNYAH_NORMAL_WB

#endif
