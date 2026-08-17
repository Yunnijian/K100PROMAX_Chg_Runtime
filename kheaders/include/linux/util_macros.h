/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_HELPER_MACROS_H_
#define _LINUX_HELPER_MACROS_H_

#include <linux/math.h>


#define find_closest(x, a, as)						\
({									\
	typeof(as) __fc_i, __fc_as = (as) - 1;				\
	long __fc_mid_x, __fc_x = (x);					\
	long __fc_left, __fc_right;					\
	typeof(*a) const *__fc_a = (a);					\
	for (__fc_i = 0; __fc_i < __fc_as; __fc_i++) {			\
		__fc_mid_x = (__fc_a[__fc_i] + __fc_a[__fc_i + 1]) / 2;	\
		if (__fc_x <= __fc_mid_x) {				\
			__fc_left = __fc_x - __fc_a[__fc_i];		\
			__fc_right = __fc_a[__fc_i + 1] - __fc_x;	\
			if (__fc_right < __fc_left)			\
				__fc_i++;				\
			break;						\
		}							\
	}								\
	(__fc_i);							\
})


#define find_closest_descending(x, a, as)				\
({									\
	typeof(as) __fc_i, __fc_as = (as) - 1;				\
	long __fc_mid_x, __fc_x = (x);					\
	long __fc_left, __fc_right;					\
	typeof(*a) const *__fc_a = (a);					\
	for (__fc_i = __fc_as; __fc_i >= 1; __fc_i--) {			\
		__fc_mid_x = (__fc_a[__fc_i] + __fc_a[__fc_i - 1]) / 2;	\
		if (__fc_x <= __fc_mid_x) {				\
			__fc_left = __fc_x - __fc_a[__fc_i];		\
			__fc_right = __fc_a[__fc_i - 1] - __fc_x;	\
			if (__fc_right < __fc_left)			\
				__fc_i--;				\
			break;						\
		}							\
	}								\
	(__fc_i);							\
})


#define is_insidevar(ptr, var)						\
	((uintptr_t)(ptr) >= (uintptr_t)(var) &&			\
	 (uintptr_t)(ptr) <  (uintptr_t)(var) + sizeof(var))

#endif
