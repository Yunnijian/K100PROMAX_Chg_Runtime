/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BITMAP_H
#define __LINUX_BITMAP_H

#ifndef __ASSEMBLY__

#include <linux/align.h>
#include <linux/bitops.h>
#include <linux/cleanup.h>
#include <linux/errno.h>
#include <linux/find.h>
#include <linux/limits.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/bitmap-str.h>

struct device;










unsigned long *bitmap_alloc(unsigned int nbits, gfp_t flags);
unsigned long *bitmap_zalloc(unsigned int nbits, gfp_t flags);
unsigned long *bitmap_alloc_node(unsigned int nbits, gfp_t flags, int node);
unsigned long *bitmap_zalloc_node(unsigned int nbits, gfp_t flags, int node);
void bitmap_free(const unsigned long *bitmap);

DEFINE_FREE(bitmap, unsigned long *, if (_T) bitmap_free(_T))


unsigned long *devm_bitmap_alloc(struct device *dev,
				 unsigned int nbits, gfp_t flags);
unsigned long *devm_bitmap_zalloc(struct device *dev,
				  unsigned int nbits, gfp_t flags);



bool __bitmap_equal(const unsigned long *bitmap1,
		    const unsigned long *bitmap2, unsigned int nbits);
bool __pure __bitmap_or_equal(const unsigned long *src1,
			      const unsigned long *src2,
			      const unsigned long *src3,
			      unsigned int nbits);
void __bitmap_complement(unsigned long *dst, const unsigned long *src,
			 unsigned int nbits);
void __bitmap_shift_right(unsigned long *dst, const unsigned long *src,
			  unsigned int shift, unsigned int nbits);
void __bitmap_shift_left(unsigned long *dst, const unsigned long *src,
			 unsigned int shift, unsigned int nbits);
void bitmap_cut(unsigned long *dst, const unsigned long *src,
		unsigned int first, unsigned int cut, unsigned int nbits);
bool __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
		 const unsigned long *bitmap2, unsigned int nbits);
void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
		 const unsigned long *bitmap2, unsigned int nbits);
void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
		  const unsigned long *bitmap2, unsigned int nbits);
bool __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
		    const unsigned long *bitmap2, unsigned int nbits);
void __bitmap_replace(unsigned long *dst,
		      const unsigned long *old, const unsigned long *new,
		      const unsigned long *mask, unsigned int nbits);
bool __bitmap_intersects(const unsigned long *bitmap1,
			 const unsigned long *bitmap2, unsigned int nbits);
bool __bitmap_subset(const unsigned long *bitmap1,
		     const unsigned long *bitmap2, unsigned int nbits);
unsigned int __bitmap_weight(const unsigned long *bitmap, unsigned int nbits);
unsigned int __bitmap_weight_and(const unsigned long *bitmap1,
				 const unsigned long *bitmap2, unsigned int nbits);
unsigned int __bitmap_weight_andnot(const unsigned long *bitmap1,
				    const unsigned long *bitmap2, unsigned int nbits);
void __bitmap_set(unsigned long *map, unsigned int start, int len);
void __bitmap_clear(unsigned long *map, unsigned int start, int len);

unsigned long bitmap_find_next_zero_area_off(unsigned long *map,
					     unsigned long size,
					     unsigned long start,
					     unsigned int nr,
					     unsigned long align_mask,
					     unsigned long align_offset);


static __always_inline
unsigned long bitmap_find_next_zero_area(unsigned long *map,
					 unsigned long size,
					 unsigned long start,
					 unsigned int nr,
					 unsigned long align_mask)
{
	return bitmap_find_next_zero_area_off(map, size, start, nr,
					      align_mask, 0);
}

void bitmap_remap(unsigned long *dst, const unsigned long *src,
		const unsigned long *old, const unsigned long *new, unsigned int nbits);
int bitmap_bitremap(int oldbit,
		const unsigned long *old, const unsigned long *new, int bits);
void bitmap_onto(unsigned long *dst, const unsigned long *orig,
		const unsigned long *relmap, unsigned int bits);
void bitmap_fold(unsigned long *dst, const unsigned long *orig,
		unsigned int sz, unsigned int nbits);

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define bitmap_size(nbits)	(ALIGN(nbits, BITS_PER_LONG) / BITS_PER_BYTE)

static __always_inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
	unsigned int len = bitmap_size(nbits);

	if (small_const_nbits(nbits))
		*dst = 0;
	else
		memset(dst, 0, len);
}

static __always_inline void bitmap_fill(unsigned long *dst, unsigned int nbits)
{
	unsigned int len = bitmap_size(nbits);

	if (small_const_nbits(nbits))
		*dst = ~0UL;
	else
		memset(dst, 0xff, len);
}

static __always_inline
void bitmap_copy(unsigned long *dst, const unsigned long *src, unsigned int nbits)
{
	unsigned int len = bitmap_size(nbits);

	if (small_const_nbits(nbits))
		*dst = *src;
	else
		memcpy(dst, src, len);
}


static __always_inline
void bitmap_copy_clear_tail(unsigned long *dst, const unsigned long *src, unsigned int nbits)
{
	bitmap_copy(dst, src, nbits);
	if (nbits % BITS_PER_LONG)
		dst[nbits / BITS_PER_LONG] &= BITMAP_LAST_WORD_MASK(nbits);
}

static inline void bitmap_copy_and_extend(unsigned long *to,
					  const unsigned long *from,
					  unsigned int count, unsigned int size)
{
	unsigned int copy = BITS_TO_LONGS(count);

	memcpy(to, from, copy * sizeof(long));
	if (count % BITS_PER_LONG)
		to[copy - 1] &= BITMAP_LAST_WORD_MASK(count);
	memset(to + copy, 0, bitmap_size(size) - copy * sizeof(long));
}


#if BITS_PER_LONG == 64
void bitmap_from_arr32(unsigned long *bitmap, const u32 *buf,
							unsigned int nbits);
void bitmap_to_arr32(u32 *buf, const unsigned long *bitmap,
							unsigned int nbits);
#else
#define bitmap_from_arr32(bitmap, buf, nbits)			\
	bitmap_copy_clear_tail((unsigned long *) (bitmap),	\
			(const unsigned long *) (buf), (nbits))
#define bitmap_to_arr32(buf, bitmap, nbits)			\
	bitmap_copy_clear_tail((unsigned long *) (buf),		\
			(const unsigned long *) (bitmap), (nbits))
#endif


#if BITS_PER_LONG == 32
void bitmap_from_arr64(unsigned long *bitmap, const u64 *buf, unsigned int nbits);
void bitmap_to_arr64(u64 *buf, const unsigned long *bitmap, unsigned int nbits);
#else
#define bitmap_from_arr64(bitmap, buf, nbits)			\
	bitmap_copy_clear_tail((unsigned long *)(bitmap), (const unsigned long *)(buf), (nbits))
#define bitmap_to_arr64(buf, bitmap, nbits)			\
	bitmap_copy_clear_tail((unsigned long *)(buf), (const unsigned long *)(bitmap), (nbits))
#endif

static __always_inline
bool bitmap_and(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return (*dst = *src1 & *src2 & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	return __bitmap_and(dst, src1, src2, nbits);
}

static __always_inline
void bitmap_or(unsigned long *dst, const unsigned long *src1,
	       const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src1 | *src2;
	else
		__bitmap_or(dst, src1, src2, nbits);
}

static __always_inline
void bitmap_xor(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src1 ^ *src2;
	else
		__bitmap_xor(dst, src1, src2, nbits);
}

static __always_inline
bool bitmap_andnot(unsigned long *dst, const unsigned long *src1,
		   const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return (*dst = *src1 & ~(*src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	return __bitmap_andnot(dst, src1, src2, nbits);
}

static __always_inline
void bitmap_complement(unsigned long *dst, const unsigned long *src, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = ~(*src);
	else
		__bitmap_complement(dst, src, nbits);
}

#ifdef __LITTLE_ENDIAN
#define BITMAP_MEM_ALIGNMENT 8
#else
#define BITMAP_MEM_ALIGNMENT (8 * sizeof(unsigned long))
#endif
#define BITMAP_MEM_MASK (BITMAP_MEM_ALIGNMENT - 1)

static __always_inline
bool bitmap_equal(const unsigned long *src1, const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return !((*src1 ^ *src2) & BITMAP_LAST_WORD_MASK(nbits));
	if (__builtin_constant_p(nbits & BITMAP_MEM_MASK) &&
	    IS_ALIGNED(nbits, BITMAP_MEM_ALIGNMENT))
		return !memcmp(src1, src2, nbits / 8);
	return __bitmap_equal(src1, src2, nbits);
}


static __always_inline
bool bitmap_or_equal(const unsigned long *src1, const unsigned long *src2,
		     const unsigned long *src3, unsigned int nbits)
{
	if (!small_const_nbits(nbits))
		return __bitmap_or_equal(src1, src2, src3, nbits);

	return !(((*src1 | *src2) ^ *src3) & BITMAP_LAST_WORD_MASK(nbits));
}

static __always_inline
bool bitmap_intersects(const unsigned long *src1, const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ((*src1 & *src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	else
		return __bitmap_intersects(src1, src2, nbits);
}

static __always_inline
bool bitmap_subset(const unsigned long *src1, const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ! ((*src1 & ~(*src2)) & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_subset(src1, src2, nbits);
}

static __always_inline
bool bitmap_empty(const unsigned long *src, unsigned nbits)
{
	if (small_const_nbits(nbits))
		return ! (*src & BITMAP_LAST_WORD_MASK(nbits));

	return find_first_bit(src, nbits) == nbits;
}

static __always_inline
bool bitmap_full(const unsigned long *src, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ! (~(*src) & BITMAP_LAST_WORD_MASK(nbits));

	return find_first_zero_bit(src, nbits) == nbits;
}

static __always_inline
unsigned int bitmap_weight(const unsigned long *src, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return hweight_long(*src & BITMAP_LAST_WORD_MASK(nbits));
	return __bitmap_weight(src, nbits);
}

static __always_inline
unsigned long bitmap_weight_and(const unsigned long *src1,
				const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return hweight_long(*src1 & *src2 & BITMAP_LAST_WORD_MASK(nbits));
	return __bitmap_weight_and(src1, src2, nbits);
}

static __always_inline
unsigned long bitmap_weight_andnot(const unsigned long *src1,
				   const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return hweight_long(*src1 & ~(*src2) & BITMAP_LAST_WORD_MASK(nbits));
	return __bitmap_weight_andnot(src1, src2, nbits);
}

static __always_inline
void bitmap_set(unsigned long *map, unsigned int start, unsigned int nbits)
{
	if (__builtin_constant_p(nbits) && nbits == 1)
		__set_bit(start, map);
	else if (small_const_nbits(start + nbits))
		*map |= GENMASK(start + nbits - 1, start);
	else if (__builtin_constant_p(start & BITMAP_MEM_MASK) &&
		 IS_ALIGNED(start, BITMAP_MEM_ALIGNMENT) &&
		 __builtin_constant_p(nbits & BITMAP_MEM_MASK) &&
		 IS_ALIGNED(nbits, BITMAP_MEM_ALIGNMENT))
		memset((char *)map + start / 8, 0xff, nbits / 8);
	else
		__bitmap_set(map, start, nbits);
}

static __always_inline
void bitmap_clear(unsigned long *map, unsigned int start, unsigned int nbits)
{
	if (__builtin_constant_p(nbits) && nbits == 1)
		__clear_bit(start, map);
	else if (small_const_nbits(start + nbits))
		*map &= ~GENMASK(start + nbits - 1, start);
	else if (__builtin_constant_p(start & BITMAP_MEM_MASK) &&
		 IS_ALIGNED(start, BITMAP_MEM_ALIGNMENT) &&
		 __builtin_constant_p(nbits & BITMAP_MEM_MASK) &&
		 IS_ALIGNED(nbits, BITMAP_MEM_ALIGNMENT))
		memset((char *)map + start / 8, 0, nbits / 8);
	else
		__bitmap_clear(map, start, nbits);
}

static __always_inline
void bitmap_shift_right(unsigned long *dst, const unsigned long *src,
			unsigned int shift, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = (*src & BITMAP_LAST_WORD_MASK(nbits)) >> shift;
	else
		__bitmap_shift_right(dst, src, shift, nbits);
}

static __always_inline
void bitmap_shift_left(unsigned long *dst, const unsigned long *src,
		       unsigned int shift, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = (*src << shift) & BITMAP_LAST_WORD_MASK(nbits);
	else
		__bitmap_shift_left(dst, src, shift, nbits);
}

static __always_inline
void bitmap_replace(unsigned long *dst,
		    const unsigned long *old,
		    const unsigned long *new,
		    const unsigned long *mask,
		    unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = (*old & ~(*mask)) | (*new & *mask);
	else
		__bitmap_replace(dst, old, new, mask, nbits);
}


static __always_inline
void bitmap_scatter(unsigned long *dst, const unsigned long *src,
		    const unsigned long *mask, unsigned int nbits)
{
	unsigned int n = 0;
	unsigned int bit;

	bitmap_zero(dst, nbits);

	for_each_set_bit(bit, mask, nbits)
		__assign_bit(bit, dst, test_bit(n++, src));
}


static __always_inline
void bitmap_gather(unsigned long *dst, const unsigned long *src,
		   const unsigned long *mask, unsigned int nbits)
{
	unsigned int n = 0;
	unsigned int bit;

	bitmap_zero(dst, nbits);

	for_each_set_bit(bit, mask, nbits)
		__assign_bit(n++, dst, test_bit(bit, src));
}

static __always_inline
void bitmap_next_set_region(unsigned long *bitmap, unsigned int *rs,
			    unsigned int *re, unsigned int end)
{
	*rs = find_next_bit(bitmap, end, *rs);
	*re = find_next_zero_bit(bitmap, end, *rs + 1);
}


static __always_inline
void bitmap_release_region(unsigned long *bitmap, unsigned int pos, int order)
{
	bitmap_clear(bitmap, pos, BIT(order));
}


static __always_inline
int bitmap_allocate_region(unsigned long *bitmap, unsigned int pos, int order)
{
	unsigned int len = BIT(order);

	if (find_next_bit(bitmap, pos + len, pos) < pos + len)
		return -EBUSY;
	bitmap_set(bitmap, pos, len);
	return 0;
}


static __always_inline
int bitmap_find_free_region(unsigned long *bitmap, unsigned int bits, int order)
{
	unsigned int pos, end;		

	for (pos = 0; (end = pos + BIT(order)) <= bits; pos = end) {
		if (!bitmap_allocate_region(bitmap, pos, order))
			return pos;
	}
	return -ENOMEM;
}


#if __BITS_PER_LONG == 64
#define BITMAP_FROM_U64(n) (n)
#else
#define BITMAP_FROM_U64(n) ((unsigned long) ((u64)(n) & ULONG_MAX)), \
				((unsigned long) ((u64)(n) >> 32))
#endif


static __always_inline void bitmap_from_u64(unsigned long *dst, u64 mask)
{
	bitmap_from_arr64(dst, &mask, 64);
}


static __always_inline
unsigned long bitmap_read(const unsigned long *map, unsigned long start, unsigned long nbits)
{
	size_t index = BIT_WORD(start);
	unsigned long offset = start % BITS_PER_LONG;
	unsigned long space = BITS_PER_LONG - offset;
	unsigned long value_low, value_high;

	if (unlikely(!nbits || nbits > BITS_PER_LONG))
		return 0;

	if (space >= nbits)
		return (map[index] >> offset) & BITMAP_LAST_WORD_MASK(nbits);

	value_low = map[index] & BITMAP_FIRST_WORD_MASK(start);
	value_high = map[index + 1] & BITMAP_LAST_WORD_MASK(start + nbits);
	return (value_low >> offset) | (value_high << space);
}


static __always_inline
void bitmap_write(unsigned long *map, unsigned long value,
		  unsigned long start, unsigned long nbits)
{
	size_t index;
	unsigned long offset;
	unsigned long space;
	unsigned long mask;
	bool fit;

	if (unlikely(!nbits || nbits > BITS_PER_LONG))
		return;

	mask = BITMAP_LAST_WORD_MASK(nbits);
	value &= mask;
	offset = start % BITS_PER_LONG;
	space = BITS_PER_LONG - offset;
	fit = space >= nbits;
	index = BIT_WORD(start);

	map[index] &= (fit ? (~(mask << offset)) : ~BITMAP_FIRST_WORD_MASK(start));
	map[index] |= value << offset;
	if (fit)
		return;

	map[index + 1] &= BITMAP_FIRST_WORD_MASK(start + nbits);
	map[index + 1] |= (value >> space);
}

#define bitmap_get_value8(map, start)			\
	bitmap_read(map, start, BITS_PER_BYTE)
#define bitmap_set_value8(map, value, start)		\
	bitmap_write(map, value, start, BITS_PER_BYTE)

#endif 

#endif 
