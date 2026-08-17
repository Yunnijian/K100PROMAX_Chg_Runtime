/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_VDSO_PROCESSOR_H
#define __ASM_VDSO_PROCESSOR_H

#ifndef __ASSEMBLY__

static inline void cpu_relax(void)
{
	asm volatile("yield" ::: "memory");
}

#endif 

#endif 
