/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_MEMORY_H
#define __ASM_MEMORY_H

#include <linux/const.h>
#include <linux/sizes.h>
#include <asm/page-def.h>


#define PCI_IO_SIZE		SZ_16M


#define VMEMMAP_RANGE	(_PAGE_END(VA_BITS_MIN) - PAGE_OFFSET)
#define VMEMMAP_SIZE	((VMEMMAP_RANGE >> PAGE_SHIFT) * sizeof(struct page))


#define VA_BITS			(CONFIG_ARM64_VA_BITS)
#define _PAGE_OFFSET(va)	(-(UL(1) << (va)))
#define PAGE_OFFSET		(_PAGE_OFFSET(VA_BITS))
#define KIMAGE_VADDR		(MODULES_END)
#define MODULES_END		(MODULES_VADDR + MODULES_VSIZE)
#define MODULES_VADDR		(_PAGE_END(VA_BITS_MIN))
#define MODULES_VSIZE		(SZ_2G)
#define VMEMMAP_START		(VMEMMAP_END - VMEMMAP_SIZE)
#define VMEMMAP_END		(-UL(SZ_1G))
#define PCI_IO_START		(VMEMMAP_END + SZ_8M)
#define PCI_IO_END		(PCI_IO_START + PCI_IO_SIZE)
#define FIXADDR_TOP		(-UL(SZ_8M))

#if VA_BITS > 48
#ifdef CONFIG_ARM64_16K_PAGES
#define VA_BITS_MIN		(47)
#else
#define VA_BITS_MIN		(48)
#endif
#else
#define VA_BITS_MIN		(VA_BITS)
#endif

#define _PAGE_END(va)		(-(UL(1) << ((va) - 1)))

#define KERNEL_START		_text
#define KERNEL_END		_end


#if defined(CONFIG_KASAN_GENERIC) || defined(CONFIG_KASAN_SW_TAGS)
#define KASAN_SHADOW_OFFSET	_AC(CONFIG_KASAN_SHADOW_OFFSET, UL)
#define KASAN_SHADOW_END	((UL(1) << (64 - KASAN_SHADOW_SCALE_SHIFT)) + KASAN_SHADOW_OFFSET)
#define _KASAN_SHADOW_START(va)	(KASAN_SHADOW_END - (UL(1) << ((va) - KASAN_SHADOW_SCALE_SHIFT)))
#define KASAN_SHADOW_START	_KASAN_SHADOW_START(vabits_actual)
#define PAGE_END		KASAN_SHADOW_START
#define KASAN_THREAD_SHIFT	1
#else
#define KASAN_THREAD_SHIFT	0
#define PAGE_END		(_PAGE_END(VA_BITS_MIN))
#endif 

#define PHYSMEM_END		__pa(PAGE_END - 1)

#define MIN_THREAD_SHIFT	(14 + KASAN_THREAD_SHIFT)


#if defined(CONFIG_VMAP_STACK) && (MIN_THREAD_SHIFT < PAGE_SHIFT)
#define THREAD_SHIFT		PAGE_SHIFT
#else
#define THREAD_SHIFT		MIN_THREAD_SHIFT
#endif

#if THREAD_SHIFT >= PAGE_SHIFT
#define THREAD_SIZE_ORDER	(THREAD_SHIFT - PAGE_SHIFT)
#endif

#define THREAD_SIZE		(UL(1) << THREAD_SHIFT)


#ifdef CONFIG_VMAP_STACK
#define THREAD_ALIGN		(2 * THREAD_SIZE)
#else
#define THREAD_ALIGN		THREAD_SIZE
#endif

#define IRQ_STACK_SIZE		THREAD_SIZE

#define OVERFLOW_STACK_SIZE	SZ_4K

#if PAGE_SIZE == SZ_4K
#define NVHE_STACK_SHIFT       (PAGE_SHIFT + 1)
#else
#define NVHE_STACK_SHIFT       PAGE_SHIFT
#endif

#define NVHE_STACK_SIZE        (UL(1) << NVHE_STACK_SHIFT)


#define NVHE_STACKTRACE_SIZE	((OVERFLOW_STACK_SIZE + NVHE_STACK_SIZE) / 2 + sizeof(long))


#define SEGMENT_ALIGN		SZ_64K


#define MT_NORMAL		0
#define MT_NORMAL_TAGGED	1
#define MT_NORMAL_NC		2
#define MT_DEVICE_nGnRnE	3
#define MT_DEVICE_nGnRE		4


#define MT_S2_NORMAL		0xf
#define MT_S2_NORMAL_NC		0x5
#define MT_S2_DEVICE_nGnRE	0x1


#define MT_S2_FWB_NORMAL	6
#define MT_S2_FWB_NORMAL_NC	5
#define MT_S2_FWB_DEVICE_nGnRE	1

#ifdef CONFIG_ARM64_4K_PAGES
#define IOREMAP_MAX_ORDER	(PUD_SHIFT)
#else
#define IOREMAP_MAX_ORDER	(PMD_SHIFT)
#endif


#define RESERVED_SWAPPER_OFFSET	(PAGE_SIZE)


#define TRAMP_SWAPPER_OFFSET	(2 * PAGE_SIZE)

#ifndef __ASSEMBLY__

#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/mmdebug.h>
#include <linux/types.h>
#include <asm/boot.h>
#include <asm/bug.h>
#include <asm/sections.h>
#include <asm/sysreg.h>

static inline u64 __pure read_tcr(void)
{
	u64  tcr;

	// read_sysreg() uses asm volatile, so avoid it here
	asm("mrs %0, tcr_el1" : "=r"(tcr));
	return tcr;
}

#if VA_BITS > 48
// For reasons of #include hell, we can't use TCR_T1SZ_OFFSET/TCR_T1SZ_MASK here
#define vabits_actual		(64 - ((read_tcr() >> 16) & 63))
#else
#define vabits_actual		((u64)VA_BITS)
#endif

extern s64			memstart_addr;

#define PHYS_OFFSET		({ VM_BUG_ON(memstart_addr & 1); memstart_addr; })

#ifdef CONFIG_EXECMEM
extern u64 module_direct_base;
extern u64 module_plt_base;
#endif


extern u64			kimage_voffset;

static inline unsigned long kaslr_offset(void)
{
	return (u64)&_text - KIMAGE_VADDR;
}

#ifdef CONFIG_RANDOMIZE_BASE
void kaslr_init(void);
static inline bool kaslr_enabled(void)
{
	extern bool __kaslr_is_enabled;
	return __kaslr_is_enabled;
}
#else
static inline void kaslr_init(void) { }
static inline bool kaslr_enabled(void) { return false; }
#endif


#define MIN_MEMBLOCK_ADDR	0
#define MAX_MEMBLOCK_ADDR	U64_MAX


#define PHYS_PFN_OFFSET	(PHYS_OFFSET >> PAGE_SHIFT)


#define __untagged_addr(addr)	\
	((__force __typeof__(addr))sign_extend64((__force u64)(addr), 55))

#define untagged_addr(addr)	({					\
	u64 __addr = (__force u64)(addr);					\
	__addr &= __untagged_addr(__addr);				\
	(__force __typeof__(addr))__addr;				\
})

#if defined(CONFIG_KASAN_SW_TAGS) || defined(CONFIG_KASAN_HW_TAGS)
#define __tag_shifted(tag)	((u64)(tag) << 56)
#define __tag_reset(addr)	__untagged_addr(addr)
#define __tag_get(addr)		(__u8)((u64)(addr) >> 56)
#else
#define __tag_shifted(tag)	0UL
#define __tag_reset(addr)	(addr)
#define __tag_get(addr)		0
#endif 

static inline const void *__tag_set(const void *addr, u8 tag)
{
	u64 __addr = (u64)addr & ~__tag_shifted(0xff);
	return (const void *)(__addr | __tag_shifted(tag));
}

#ifdef CONFIG_KASAN_HW_TAGS
#define arch_enable_tag_checks_sync()		mte_enable_kernel_sync()
#define arch_enable_tag_checks_async()		mte_enable_kernel_async()
#define arch_enable_tag_checks_asymm()		mte_enable_kernel_asymm()
#define arch_suppress_tag_checks_start()	mte_enable_tco()
#define arch_suppress_tag_checks_stop()		mte_disable_tco()
#define arch_force_async_tag_fault()		mte_check_tfsr_exit()
#define arch_get_random_tag()			mte_get_random_tag()
#define arch_get_mem_tag(addr)			mte_get_mem_tag(addr)
#define arch_set_mem_tag_range(addr, size, tag, init)	\
			mte_set_mem_tag_range((addr), (size), (tag), (init))
#endif 





#define __is_lm_address(addr)	(((u64)(addr) - PAGE_OFFSET) < (PAGE_END - PAGE_OFFSET))

#define __lm_to_phys(addr)	(((addr) - PAGE_OFFSET) + PHYS_OFFSET)
#define __kimg_to_phys(addr)	((addr) - kimage_voffset)

#define __virt_to_phys_nodebug(x) ({					\
	phys_addr_t __x = (phys_addr_t)(__tag_reset(x));		\
	__is_lm_address(__x) ? __lm_to_phys(__x) : __kimg_to_phys(__x);	\
})

#define __pa_symbol_nodebug(x)	__kimg_to_phys((phys_addr_t)(x))

#ifdef CONFIG_DEBUG_VIRTUAL
extern phys_addr_t __virt_to_phys(unsigned long x);
extern phys_addr_t __phys_addr_symbol(unsigned long x);
#else
#define __virt_to_phys(x)	__virt_to_phys_nodebug(x)
#define __phys_addr_symbol(x)	__pa_symbol_nodebug(x)
#endif 

#define __phys_to_virt(x)	((unsigned long)((x) - PHYS_OFFSET) | PAGE_OFFSET)
#define __phys_to_kimg(x)	((unsigned long)((x) + kimage_voffset))


#define virt_to_phys virt_to_phys
static inline phys_addr_t virt_to_phys(const volatile void *x)
{
	return __virt_to_phys((unsigned long)(x));
}

#define phys_to_virt phys_to_virt
static inline void *phys_to_virt(phys_addr_t x)
{
	return (void *)(__phys_to_virt(x));
}


#include <asm-generic/memory_model.h>

static inline unsigned long virt_to_pfn(const void *kaddr)
{
	return __phys_to_pfn(virt_to_phys(kaddr));
}


#define __pa(x)			__virt_to_phys((unsigned long)(x))
#define __pa_symbol(x)		__phys_addr_symbol(RELOC_HIDE((unsigned long)(x), 0))
#define __pa_nodebug(x)		__virt_to_phys_nodebug((unsigned long)(x))
#define __va(x)			((void *)__phys_to_virt((phys_addr_t)(x)))
#define pfn_to_kaddr(pfn)	__va((pfn) << PAGE_SHIFT)
#define sym_to_pfn(x)		__phys_to_pfn(__pa_symbol(x))


#define ARCH_PFN_OFFSET		((unsigned long)PHYS_PFN_OFFSET)

#if defined(CONFIG_DEBUG_VIRTUAL)
#define page_to_virt(x)	({						\
	__typeof__(x) __page = x;					\
	void *__addr = __va(page_to_phys(__page));			\
	(void *)__tag_set((const void *)__addr, page_kasan_tag(__page));\
})
#define virt_to_page(x)		pfn_to_page(virt_to_pfn(x))
#else
#define page_to_virt(x)	({						\
	__typeof__(x) __page = x;					\
	u64 __idx = ((u64)__page - VMEMMAP_START) / sizeof(struct page);\
	u64 __addr = PAGE_OFFSET + (__idx * PAGE_SIZE);			\
	(void *)__tag_set((const void *)__addr, page_kasan_tag(__page));\
})

#define virt_to_page(x)	({						\
	u64 __idx = (__tag_reset((u64)x) - PAGE_OFFSET) / PAGE_SIZE;	\
	u64 __addr = VMEMMAP_START + (__idx * sizeof(struct page));	\
	(struct page *)__addr;						\
})
#endif 

#define virt_addr_valid(addr)	({					\
	__typeof__(addr) __addr = __tag_reset(addr);			\
	__is_lm_address(__addr) && pfn_is_map_memory(virt_to_pfn(__addr));	\
})

void dump_mem_limit(void);
#endif 


#if defined(CONFIG_EFI) && defined(CONFIG_ARM_GIC_V3_ITS)
# define INIT_MEMBLOCK_RESERVED_REGIONS	(INIT_MEMBLOCK_REGIONS + NR_CPUS + 1)
#endif


#ifdef CONFIG_EFI
#define INIT_MEMBLOCK_MEMORY_REGIONS	(INIT_MEMBLOCK_REGIONS * 8)
#endif


#endif 
