/* SPDX-License-Identifier: GPL-2.0 */



#ifndef __LINUX_PAGE_SIZE_COMPAT_DEFS_H
#define __LINUX_PAGE_SIZE_COMPAT_DEFS_H

#include <asm/page.h>

#define __MAX_PAGE_SHIFT		14
#define __MAX_PAGE_SIZE		(_AC(1, UL) << __MAX_PAGE_SHIFT)
#define __MAX_PAGE_MASK		(~(__MAX_PAGE_SIZE - 1))

#ifndef __ASSEMBLY__

#include <linux/align.h>
#include <linux/jump_label.h>
#include <linux/sched.h>
#include <linux/math.h>

DECLARE_STATIC_KEY_FALSE(page_shift_compat_enabled);
extern int page_shift_compat __ro_after_init;

#ifdef CONFIG_X86_64
static __always_inline unsigned int __page_shift(void)
{
	if (static_branch_unlikely(&page_shift_compat_enabled))
		return page_shift_compat;
	else
		return PAGE_SHIFT;
}
#else	
#define __page_shift()		PAGE_SHIFT
#endif	

#define __PAGE_SHIFT			__page_shift()
#define __PAGE_SIZE			(_AC(1, UL) << __PAGE_SHIFT)
#define __PAGE_MASK			(~(__PAGE_SIZE - 1))
#define __PAGE_ALIGN(addr)		ALIGN(addr, __PAGE_SIZE)
#define __PAGE_ALIGN_DOWN(addr)	ALIGN_DOWN(addr, __PAGE_SIZE)

#define __offset_in_page(p)		((unsigned long)(p) & ~__PAGE_MASK)


static inline unsigned long __page_size_count(unsigned long val)
{
	unsigned int nr_subpages = __PAGE_SIZE / PAGE_SIZE;

	return DIV_ROUND_UP(val, nr_subpages);
}

#endif 

#endif 
