/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SECCOMP_TYPES_H
#define _LINUX_SECCOMP_TYPES_H

#include <linux/types.h>

#ifdef CONFIG_SECCOMP

struct seccomp_filter;

struct seccomp {
	int mode;
	atomic_t filter_count;
	struct seccomp_filter *filter;
};

#else

struct seccomp { };
struct seccomp_filter { };

#endif

#endif 
