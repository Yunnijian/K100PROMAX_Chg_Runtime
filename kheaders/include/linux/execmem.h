/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_EXECMEM_ALLOC_H
#define _LINUX_EXECMEM_ALLOC_H

#include <linux/types.h>
#include <linux/moduleloader.h>
#include <linux/cleanup.h>

#if (defined(CONFIG_KASAN_GENERIC) || defined(CONFIG_KASAN_SW_TAGS)) && \
		!defined(CONFIG_KASAN_VMALLOC)
#include <linux/kasan.h>
#define MODULE_ALIGN (PAGE_SIZE << KASAN_SHADOW_SCALE_SHIFT)
#else
#define MODULE_ALIGN PAGE_SIZE
#endif


enum execmem_type {
	EXECMEM_DEFAULT,
	EXECMEM_MODULE_TEXT = EXECMEM_DEFAULT,
	EXECMEM_KPROBES,
	EXECMEM_FTRACE,
	EXECMEM_BPF,
	EXECMEM_MODULE_DATA,
	EXECMEM_TYPE_MAX,
};


enum execmem_range_flags {
	EXECMEM_KASAN_SHADOW	= (1 << 0),
};


struct execmem_range {
	unsigned long   start;
	unsigned long   end;
	unsigned long   fallback_start;
	unsigned long   fallback_end;
	pgprot_t        pgprot;
	unsigned int	alignment;
	enum execmem_range_flags flags;
};


struct execmem_info {
	struct execmem_range	ranges[EXECMEM_TYPE_MAX];
};


struct execmem_info *execmem_arch_setup(void);


void *execmem_alloc(enum execmem_type type, size_t size);


void execmem_free(void *ptr);

DEFINE_FREE(execmem, void *, if (_T) execmem_free(_T));

#ifdef CONFIG_MMU

struct vm_struct *execmem_vmap(size_t size);
#endif

#if defined(CONFIG_EXECMEM) && !defined(CONFIG_ARCH_WANTS_EXECMEM_LATE)
void execmem_init(void);
#else
static inline void execmem_init(void) {}
#endif

#endif 
