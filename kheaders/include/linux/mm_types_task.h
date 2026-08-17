/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_TYPES_TASK_H
#define _LINUX_MM_TYPES_TASK_H



#include <linux/types.h>

#include <asm/page.h>

#ifdef CONFIG_ARCH_WANT_BATCHED_UNMAP_TLB_FLUSH
#include <asm/tlbbatch.h>
#endif

#define ALLOC_SPLIT_PTLOCKS	(SPINLOCK_SIZE > BITS_PER_LONG/8)


enum {
	MM_FILEPAGES,	
	MM_ANONPAGES,	
	MM_SWAPENTS,	
	MM_SHMEMPAGES,	
	NR_MM_COUNTERS
};

struct page;

struct page_frag {
	struct page *page;
#if (BITS_PER_LONG > 32) || (PAGE_SIZE >= 65536)
	__u32 offset;
	__u32 size;
#else
	__u16 offset;
	__u16 size;
#endif
};


struct tlbflush_unmap_batch {
#ifdef CONFIG_ARCH_WANT_BATCHED_UNMAP_TLB_FLUSH
	
	struct arch_tlbflush_unmap_batch arch;

	
	bool flush_required;

	
	bool writable;
#endif
};

#endif 
