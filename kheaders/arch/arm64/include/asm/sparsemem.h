/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_SPARSEMEM_H
#define __ASM_SPARSEMEM_H

#include <asm/pgtable-prot.h>

#define MAX_PHYSMEM_BITS		PHYS_MASK_SHIFT
#define MAX_POSSIBLE_PHYSMEM_BITS	(52)


#ifdef CONFIG_ARM64_64K_PAGES
#define SECTION_SIZE_BITS 29

#else


#define SECTION_SIZE_BITS 27
#endif 

#endif
