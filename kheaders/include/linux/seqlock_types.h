/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_SEQLOCK_TYPES_H
#define __LINUX_SEQLOCK_TYPES_H

#include <linux/lockdep_types.h>
#include <linux/mutex_types.h>
#include <linux/spinlock_types.h>


typedef struct seqcount {
	unsigned sequence;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
} seqcount_t;


#if defined(CONFIG_LOCKDEP) || defined(CONFIG_PREEMPT_RT)
#define __SEQ_LOCK(expr)	expr
#else
#define __SEQ_LOCK(expr)
#endif

#define SEQCOUNT_LOCKNAME(lockname, locktype, preemptible, lockbase)	\
typedef struct seqcount_##lockname {					\
	seqcount_t		seqcount;				\
	__SEQ_LOCK(locktype	*lock);					\
} seqcount_##lockname##_t;

SEQCOUNT_LOCKNAME(raw_spinlock, raw_spinlock_t,  false,    raw_spin)
SEQCOUNT_LOCKNAME(spinlock,     spinlock_t,      __SEQ_RT, spin)
SEQCOUNT_LOCKNAME(rwlock,       rwlock_t,        __SEQ_RT, read)
SEQCOUNT_LOCKNAME(mutex,        struct mutex,    true,     mutex)
#undef SEQCOUNT_LOCKNAME


typedef struct {
	
	seqcount_spinlock_t seqcount;
	spinlock_t lock;
} seqlock_t;

#endif 
