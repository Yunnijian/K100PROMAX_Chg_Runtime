/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_MUTEX_TYPES_H
#define __LINUX_MUTEX_TYPES_H

#include <linux/atomic.h>
#include <linux/lockdep_types.h>
#include <linux/osq_lock.h>
#include <linux/spinlock_types.h>
#include <linux/types.h>
#include <linux/android_vendor.h>

#ifndef CONFIG_PREEMPT_RT


struct mutex {
	atomic_long_t		owner;
	raw_spinlock_t		wait_lock;
#ifdef CONFIG_MUTEX_SPIN_ON_OWNER
	struct optimistic_spin_queue osq; 
#endif
	struct list_head	wait_list;
#ifdef CONFIG_DEBUG_MUTEXES
	void			*magic;
#endif
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
	ANDROID_OEM_DATA_ARRAY(1, 2);
};

#else 

#include <linux/rtmutex.h>

struct mutex {
	struct rt_mutex_base	rtmutex;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
	ANDROID_OEM_DATA_ARRAY(1, 2);
};

#endif 

#endif 
