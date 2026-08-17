/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_REFCOUNT_TYPES_H
#define _LINUX_REFCOUNT_TYPES_H

#include <linux/types.h>


typedef struct refcount_struct {
	atomic_t refs;
} refcount_t;

#endif 
