/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_CPUMASK_TYPES_H
#define __LINUX_CPUMASK_TYPES_H

#include <linux/bitops.h>
#include <linux/threads.h>


typedef struct cpumask { DECLARE_BITMAP(bits, NR_CPUS); } cpumask_t;


#define cpumask_bits(maskp) ((maskp)->bits)


#ifdef CONFIG_CPUMASK_OFFSTACK
typedef struct cpumask *cpumask_var_t;
#else
typedef struct cpumask cpumask_var_t[1];
#endif 

#endif 
