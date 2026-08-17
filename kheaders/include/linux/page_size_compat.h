/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_PAGE_SIZE_COMPAT_H
#define __LINUX_PAGE_SIZE_COMPAT_H



#include <linux/page_size_compat_defs.h>

#ifndef __ASSEMBLY__

#include <linux/mman.h>
#include <linux/printk.h>

#define pgcompat_err(fmt, ...) \
	pr_err("pgcompat [%i (%s)]: " fmt, task_pid_nr(current), current->comm, ## __VA_ARGS__)

#define __offset_in_page_log(addr)							\
({											\
	if (static_branch_unlikely(&page_shift_compat_enabled) &&			\
			__offset_in_page(addr))						\
		pgcompat_err("%s: addr (0x%08lx) not page aligned", __func__, addr);	\
	(__offset_in_page(addr));							\
})

#define __PAGE_ALIGNED(addr)    (!__offset_in_page_log(addr))


#define __PAGE_SIZE_ROUND_UP_ADJ(size) \
	((size) + (((1 << (__PAGE_SHIFT - PAGE_SHIFT)) - 1) << PAGE_SHIFT))


#define __VM_NO_COMPAT      _BITULL(58)
#define __MAP_NO_COMPAT     _BITUL(31)


#define __COMPAT_PAGE_ALIGN(size, flags) \
	(flags & __MAP_NO_COMPAT) ? PAGE_ALIGN(size) : __PAGE_ALIGN(size)


static __always_inline unsigned long calc_vm_flag_bits(struct file *file, unsigned long flags)
{
	unsigned long flag_bits = __calc_vm_flag_bits(file, flags);

	if (static_branch_unlikely(&page_shift_compat_enabled))
		flag_bits |= _calc_vm_trans(flags, __MAP_NO_COMPAT,  __VM_NO_COMPAT );

	return flag_bits;
}

extern unsigned long ___filemap_len(struct inode *inode, unsigned long pgoff,
				    unsigned long len, unsigned long flags);

extern void ___filemap_fixup(unsigned long addr, unsigned long prot, unsigned long file_backed_len,
			     unsigned long len);

static __always_inline unsigned long __filemap_len(struct inode *inode, unsigned long pgoff,
						   unsigned long len, unsigned long flags)
{
	if (static_branch_unlikely(&page_shift_compat_enabled))
		return ___filemap_len(inode, pgoff, len, flags);
	else
		return len;
}

static __always_inline void __filemap_fixup(unsigned long addr, unsigned long prot,
					    unsigned long file_backed_len, unsigned long len)
{

	if (static_branch_unlikely(&page_shift_compat_enabled))
		___filemap_fixup(addr, prot, file_backed_len, len);
}

extern void __fold_filemap_fixup_entry(struct vma_iterator *iter, unsigned long *end);

extern int __fixup_swap_header(struct file *swap_file, struct address_space *mapping);

#ifdef CONFIG_PROC_PAGE_MONITOR
extern bool __is_emulated_pagemap_file(struct file *file);
#else
static inline bool __is_emulated_pagemap_file(struct file *file)
{
	return false;
}
#endif

static __always_inline void __adjust_cachestat_counters(struct cachestat *cs)
{
	unsigned int nr_sub_pages = __PAGE_SIZE / PAGE_SIZE;

	if (nr_sub_pages <= 1)
		return;

	cs->nr_cache /= nr_sub_pages;
	cs->nr_dirty /= nr_sub_pages;
	cs->nr_writeback /= nr_sub_pages;
	cs->nr_evicted /= nr_sub_pages;
	cs->nr_recently_evicted /= nr_sub_pages;
}

#endif 

#endif 
