/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PAGE_SIZE_MIGRATION_INLINE_H
#define _LINUX_PAGE_SIZE_MIGRATION_INLINE_H



#include <linux/mm_types.h>
#include <linux/sizes.h>

#include <asm/page.h>



#define VM_PAD_WIDTH		4
#define VM_PAD_SHIFT		(BITS_PER_LONG - VM_PAD_WIDTH - 1)
#define VM_TOTAL_PAD_PAGES	((1ULL << VM_PAD_WIDTH) - 1)
#define VM_PAD_MASK		(VM_TOTAL_PAD_PAGES << VM_PAD_SHIFT)
#define VMA_PAD_START(vma)	(vma->vm_end - (vma_pad_pages(vma) << PAGE_SHIFT))

#if PAGE_SIZE == SZ_4K && defined(CONFIG_64BIT)

static inline unsigned long vma_pad_fixup_flags(struct vm_area_struct *vma,
						unsigned long newflags)
{
	if (newflags & VM_PAD_MASK)
		return (newflags & ~VM_PAD_MASK) | (vma->vm_flags & VM_PAD_MASK);
	else
		return newflags;
}
#else 
static inline unsigned long vma_pad_fixup_flags(struct vm_area_struct *vma,
						unsigned long newflags)
{
	return newflags;
}
#endif 

#endif 
