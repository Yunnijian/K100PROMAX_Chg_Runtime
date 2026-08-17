/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BITS_H
#define __LINUX_BITS_H

#include <linux/const.h>
#include <vdso/bits.h>
#include <uapi/linux/bits.h>
#include <asm/bitsperlong.h>

#define BIT_MASK(nr)		(UL(1) << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)	(ULL(1) << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)	((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE		8
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)


#if !defined(__ASSEMBLY__)



#include <linux/build_bug.h>
#include <linux/overflow.h>
#define GENMASK_INPUT_CHECK(h, l) \
	(BUILD_BUG_ON_ZERO(__builtin_choose_expr( \
		__is_constexpr((l) > (h)), (l) > (h), 0)))


#define GENMASK_TYPE(t, h, l)					\
	((t)(GENMASK_INPUT_CHECK(h, l) +			\
	     (type_max(t) << (l) &				\
	      type_max(t) >> (BITS_PER_TYPE(t) - 1 - (h)))))

#define GENMASK_U8(h, l)	GENMASK_TYPE(u8, h, l)
#define GENMASK_U16(h, l)	GENMASK_TYPE(u16, h, l)
#define GENMASK_U32(h, l)	GENMASK_TYPE(u32, h, l)
#define GENMASK_U64(h, l)	GENMASK_TYPE(u64, h, l)

#else 


#define GENMASK_INPUT_CHECK(h, l) 0

#endif 

#define GENMASK(h, l) \
	(GENMASK_INPUT_CHECK(h, l) + __GENMASK(h, l))
#define GENMASK_ULL(h, l) \
	(GENMASK_INPUT_CHECK(h, l) + __GENMASK_ULL(h, l))

#if !defined(__ASSEMBLY__)

#define GENMASK_U128(h, l) \
	(GENMASK_INPUT_CHECK(h, l) + __GENMASK_U128(h, l))
#endif

#endif	
