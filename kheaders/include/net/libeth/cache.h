/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LIBETH_CACHE_H
#define __LIBETH_CACHE_H

#include <linux/cache.h>


#if defined(CONFIG_64BIT) && SMP_CACHE_BYTES == 64
#define libeth_cacheline_group_assert(type, grp, sz)			      \
	static_assert(offsetof(type, __cacheline_group_end__##grp) -	      \
		      offsetofend(type, __cacheline_group_begin__##grp) ==    \
		      (sz))
#define __libeth_cacheline_struct_assert(type, sz)			      \
	static_assert(sizeof(type) == (sz))
#else 
#define libeth_cacheline_group_assert(type, grp, sz)			      \
	static_assert(offsetof(type, __cacheline_group_end__##grp) -	      \
		      offsetofend(type, __cacheline_group_begin__##grp) <=    \
		      (sz))
#define __libeth_cacheline_struct_assert(type, sz)			      \
	static_assert(sizeof(type) <= (sz))
#endif 

#define __libeth_cls1(sz1)	SMP_CACHE_ALIGN(sz1)
#define __libeth_cls2(sz1, sz2)	(SMP_CACHE_ALIGN(sz1) + SMP_CACHE_ALIGN(sz2))
#define __libeth_cls3(sz1, sz2, sz3)					      \
	(SMP_CACHE_ALIGN(sz1) + SMP_CACHE_ALIGN(sz2) + SMP_CACHE_ALIGN(sz3))
#define __libeth_cls(...)						      \
	CONCATENATE(__libeth_cls, COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)


#define libeth_cacheline_struct_assert(type, ...)			      \
	__libeth_cacheline_struct_assert(type, __libeth_cls(__VA_ARGS__));    \
	static_assert(__alignof(type) >= SMP_CACHE_BYTES)


#define libeth_cacheline_set_assert(type, ro, rw, c)			      \
	libeth_cacheline_group_assert(type, read_mostly, ro);		      \
	libeth_cacheline_group_assert(type, read_write, rw);		      \
	libeth_cacheline_group_assert(type, cold, c);			      \
	libeth_cacheline_struct_assert(type, ro, rw, c)

#endif 
