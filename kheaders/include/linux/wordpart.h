/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_WORDPART_H
#define _LINUX_WORDPART_H


#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))


#define lower_32_bits(n) ((u32)((n) & 0xffffffff))


#define upper_16_bits(n) ((u16)((n) >> 16))


#define lower_16_bits(n) ((u16)((n) & 0xffff))


#define REPEAT_BYTE(x)	((~0ul / 0xff) * (x))


#define REPEAT_BYTE_U32(x)	lower_32_bits(REPEAT_BYTE(x))


#ifdef __LITTLE_ENDIAN
#  define aligned_byte_mask(n) ((1UL << 8*(n))-1)
#else
#  define aligned_byte_mask(n) (~0xffUL << (BITS_PER_LONG - 8 - 8*(n)))
#endif

#endif // _LINUX_WORDPART_H
