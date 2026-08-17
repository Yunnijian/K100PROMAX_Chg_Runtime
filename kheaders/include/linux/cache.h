/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_CACHE_H
#define __LINUX_CACHE_H

#include <uapi/linux/kernel.h>
#include <asm/cache.h>

#ifndef L1_CACHE_ALIGN
#define L1_CACHE_ALIGN(x) __ALIGN_KERNEL(x, L1_CACHE_BYTES)
#endif

#ifndef SMP_CACHE_BYTES
#define SMP_CACHE_BYTES L1_CACHE_BYTES
#endif


#ifndef SMP_CACHE_ALIGN
#define SMP_CACHE_ALIGN(x)	ALIGN(x, SMP_CACHE_BYTES)
#endif


#ifndef __LARGEST_ALIGN
#define __LARGEST_ALIGN		sizeof(struct { long x; } __aligned_largest)
#endif

#ifndef LARGEST_ALIGN
#define LARGEST_ALIGN(x)	ALIGN(x, __LARGEST_ALIGN)
#endif


#ifndef __read_mostly
#define __read_mostly
#endif


#ifndef __ro_after_init
#define __ro_after_init __section(".data..ro_after_init")
#endif

#ifndef ____cacheline_aligned
#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))
#endif

#ifndef ____cacheline_aligned_in_smp
#ifdef CONFIG_SMP
#define ____cacheline_aligned_in_smp ____cacheline_aligned
#else
#define ____cacheline_aligned_in_smp
#endif 
#endif

#ifndef __cacheline_aligned
#define __cacheline_aligned					\
  __attribute__((__aligned__(SMP_CACHE_BYTES),			\
		 __section__(".data..cacheline_aligned")))
#endif 

#ifndef __cacheline_aligned_in_smp
#ifdef CONFIG_SMP
#define __cacheline_aligned_in_smp __cacheline_aligned
#else
#define __cacheline_aligned_in_smp
#endif 
#endif


#ifndef INTERNODE_CACHE_SHIFT
#define INTERNODE_CACHE_SHIFT L1_CACHE_SHIFT
#endif

#if !defined(____cacheline_internodealigned_in_smp)
#if defined(CONFIG_SMP)
#define ____cacheline_internodealigned_in_smp \
	__attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))
#else
#define ____cacheline_internodealigned_in_smp
#endif
#endif

#ifndef CONFIG_ARCH_HAS_CACHE_LINE_SIZE
#define cache_line_size()	L1_CACHE_BYTES
#endif

#ifndef __cacheline_group_begin
#define __cacheline_group_begin(GROUP) \
	__u8 __cacheline_group_begin__##GROUP[0]
#endif

#ifndef __cacheline_group_end
#define __cacheline_group_end(GROUP) \
	__u8 __cacheline_group_end__##GROUP[0]
#endif


#define __cacheline_group_begin_aligned(GROUP, ...)		\
	__cacheline_group_begin(GROUP)				\
	__aligned((__VA_ARGS__ + 0) ? : SMP_CACHE_BYTES)


#define __cacheline_group_end_aligned(GROUP, ...)		\
	__cacheline_group_end(GROUP) __aligned(sizeof(long));	\
	struct { } __cacheline_group_pad__##GROUP		\
	__aligned((__VA_ARGS__ + 0) ? : SMP_CACHE_BYTES)

#ifndef CACHELINE_ASSERT_GROUP_MEMBER
#define CACHELINE_ASSERT_GROUP_MEMBER(TYPE, GROUP, MEMBER) \
	BUILD_BUG_ON(!(offsetof(TYPE, MEMBER) >= \
		       offsetofend(TYPE, __cacheline_group_begin__##GROUP) && \
		       offsetofend(TYPE, MEMBER) <= \
		       offsetof(TYPE, __cacheline_group_end__##GROUP)))
#endif

#ifndef CACHELINE_ASSERT_GROUP_SIZE
#define CACHELINE_ASSERT_GROUP_SIZE(TYPE, GROUP, SIZE) \
	BUILD_BUG_ON(offsetof(TYPE, __cacheline_group_end__##GROUP) - \
		     offsetofend(TYPE, __cacheline_group_begin__##GROUP) > \
		     SIZE)
#endif


#if defined(CONFIG_SMP)
struct cacheline_padding {
	char x[0];
} ____cacheline_internodealigned_in_smp;
#define CACHELINE_PADDING(name)		struct cacheline_padding name
#else
#define CACHELINE_PADDING(name)
#endif

#ifdef ARCH_DMA_MINALIGN
#define ARCH_HAS_DMA_MINALIGN
#else
#define ARCH_DMA_MINALIGN __alignof__(unsigned long long)
#endif

#endif 
