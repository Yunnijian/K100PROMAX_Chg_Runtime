/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MINMAX_H
#define _LINUX_MINMAX_H

#include <linux/build_bug.h>
#include <linux/compiler.h>
#include <linux/const.h>
#include <linux/types.h>


#define __typecheck(x, y) \
	(!!(sizeof((typeof(x) *)1 == (typeof(y) *)1)))


#define __sign_use(ux) (is_signed_type(typeof(ux)) ? \
	(2 + __is_nonneg(ux)) : (1 + 2 * (sizeof(ux) < 4)))


#if __SIZEOF_POINTER__ == __SIZEOF_LONG_LONG__
#define __is_nonneg(ux) statically_true((long long)(ux) >= 0)
#else
#define __is_nonneg(ux) statically_true( \
	(typeof(__builtin_choose_expr(sizeof(ux) > 4, 1LL, 1L)))(ux) >= 0)
#endif

#define __types_ok(ux, uy) \
	(__sign_use(ux) & __sign_use(uy))

#define __types_ok3(ux, uy, uz) \
	(__sign_use(ux) & __sign_use(uy) & __sign_use(uz))

#define __cmp_op_min <
#define __cmp_op_max >

#define __cmp(op, x, y)	((x) __cmp_op_##op (y) ? (x) : (y))

#define __cmp_once_unique(op, type, x, y, ux, uy) \
	({ type ux = (x); type uy = (y); __cmp(op, ux, uy); })

#define __cmp_once(op, type, x, y) \
	__cmp_once_unique(op, type, x, y, __UNIQUE_ID(x_), __UNIQUE_ID(y_))

#define __careful_cmp_once(op, x, y, ux, uy) ({		\
	__auto_type ux = (x); __auto_type uy = (y);	\
	BUILD_BUG_ON_MSG(!__types_ok(ux, uy),		\
		#op"("#x", "#y") signedness error");	\
	__cmp(op, ux, uy); })

#define __careful_cmp(op, x, y) \
	__careful_cmp_once(op, x, y, __UNIQUE_ID(x_), __UNIQUE_ID(y_))


#define min(x, y)	__careful_cmp(min, x, y)


#define max(x, y)	__careful_cmp(max, x, y)


#define umin(x, y)	\
	__careful_cmp(min, (x) + 0u + 0ul + 0ull, (y) + 0u + 0ul + 0ull)


#define umax(x, y)	\
	__careful_cmp(max, (x) + 0u + 0ul + 0ull, (y) + 0u + 0ul + 0ull)

#define __careful_op3(op, x, y, z, ux, uy, uz) ({			\
	__auto_type ux = (x); __auto_type uy = (y);__auto_type uz = (z);\
	BUILD_BUG_ON_MSG(!__types_ok3(ux, uy, uz),			\
		#op"3("#x", "#y", "#z") signedness error");		\
	__cmp(op, ux, __cmp(op, uy, uz)); })


#define min3(x, y, z) \
	__careful_op3(min, x, y, z, __UNIQUE_ID(x_), __UNIQUE_ID(y_), __UNIQUE_ID(z_))


#define max3(x, y, z) \
	__careful_op3(max, x, y, z, __UNIQUE_ID(x_), __UNIQUE_ID(y_), __UNIQUE_ID(z_))


#define min_t(type, x, y) __cmp_once(min, type, x, y)


#define max_t(type, x, y) __cmp_once(max, type, x, y)


#define min_not_zero(x, y) ({			\
	typeof(x) __x = (x);			\
	typeof(y) __y = (y);			\
	__x == 0 ? __y : ((__y == 0) ? __x : min(__x, __y)); })

#define __clamp(val, lo, hi)	\
	((val) >= (hi) ? (hi) : ((val) <= (lo) ? (lo) : (val)))

#define __clamp_once(type, val, lo, hi, uval, ulo, uhi) ({			\
	type uval = (val);							\
	type ulo = (lo);							\
	type uhi = (hi);							\
	BUILD_BUG_ON_MSG(statically_true(ulo > uhi),				\
		"clamp() low limit " #lo " greater than high limit " #hi);	\
	BUILD_BUG_ON_MSG(!__types_ok3(uval, ulo, uhi),				\
		"clamp("#val", "#lo", "#hi") signedness error");		\
	__clamp(uval, ulo, uhi); })

#define __careful_clamp(type, val, lo, hi) \
	__clamp_once(type, val, lo, hi, __UNIQUE_ID(v_), __UNIQUE_ID(l_), __UNIQUE_ID(h_))


#define clamp(val, lo, hi) __careful_clamp(__auto_type, val, lo, hi)


#define clamp_t(type, val, lo, hi) __careful_clamp(type, val, lo, hi)


#define clamp_val(val, lo, hi) __careful_clamp(typeof(val), val, lo, hi)


#define __minmax_array(op, array, len) ({				\
	typeof(&(array)[0]) __array = (array);				\
	typeof(len) __len = (len);					\
	__unqual_scalar_typeof(__array[0]) __element = __array[--__len];\
	while (__len--)							\
		__element = op(__element, __array[__len]);		\
	__element; })


#define min_array(array, len) __minmax_array(min, array, len)


#define max_array(array, len) __minmax_array(max, array, len)

static inline bool in_range64(u64 val, u64 start, u64 len)
{
	return (val - start) < len;
}

static inline bool in_range32(u32 val, u32 start, u32 len)
{
	return (val - start) < len;
}


#define in_range(val, start, len)					\
	((sizeof(start) | sizeof(len) | sizeof(val)) <= sizeof(u32) ?	\
		in_range32(val, start, len) : in_range64(val, start, len))


#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)


#define MIN(a, b) __cmp(min, a, b)
#define MAX(a, b) __cmp(max, a, b)
#define MIN_T(type, a, b) __cmp(min, (type)(a), (type)(b))
#define MAX_T(type, a, b) __cmp(max, (type)(a), (type)(b))

#endif	
