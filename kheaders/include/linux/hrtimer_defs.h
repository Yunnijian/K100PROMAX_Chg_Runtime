/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_HRTIMER_DEFS_H
#define _LINUX_HRTIMER_DEFS_H

#include <linux/ktime.h>
#include <linux/timerqueue.h>
#include <linux/seqlock.h>

#ifdef CONFIG_HIGH_RES_TIMERS


# define HIGH_RES_NSEC		1
# define KTIME_HIGH_RES		(HIGH_RES_NSEC)
# define MONOTONIC_RES_NSEC	HIGH_RES_NSEC
# define KTIME_MONOTONIC_RES	KTIME_HIGH_RES

#else

# define MONOTONIC_RES_NSEC	LOW_RES_NSEC
# define KTIME_MONOTONIC_RES	KTIME_LOW_RES

#endif

#ifdef CONFIG_64BIT
# define __hrtimer_clock_base_align	____cacheline_aligned
#else
# define __hrtimer_clock_base_align
#endif


struct hrtimer_clock_base {
	struct hrtimer_cpu_base	*cpu_base;
	unsigned int		index;
	clockid_t		clockid;
	seqcount_raw_spinlock_t	seq;
	struct hrtimer		*running;
	struct timerqueue_head	active;
	ktime_t			(*get_time)(void);
	ktime_t			offset;
} __hrtimer_clock_base_align;

enum  hrtimer_base_type {
	HRTIMER_BASE_MONOTONIC,
	HRTIMER_BASE_REALTIME,
	HRTIMER_BASE_BOOTTIME,
	HRTIMER_BASE_TAI,
	HRTIMER_BASE_MONOTONIC_SOFT,
	HRTIMER_BASE_REALTIME_SOFT,
	HRTIMER_BASE_BOOTTIME_SOFT,
	HRTIMER_BASE_TAI_SOFT,
	HRTIMER_MAX_CLOCK_BASES,
};


struct hrtimer_cpu_base {
	raw_spinlock_t			lock;
	unsigned int			cpu;
	unsigned int			active_bases;
	unsigned int			clock_was_set_seq;
	unsigned int			hres_active		: 1,
					in_hrtirq		: 1,
					hang_detected		: 1,
					softirq_activated       : 1,
					online			: 1;
#ifdef CONFIG_HIGH_RES_TIMERS
	unsigned int			nr_events;
	unsigned short			nr_retries;
	unsigned short			nr_hangs;
	unsigned int			max_hang_time;
#endif
#ifdef CONFIG_PREEMPT_RT
	spinlock_t			softirq_expiry_lock;
	atomic_t			timer_waiters;
#endif
	ktime_t				expires_next;
	struct hrtimer			*next_timer;
	ktime_t				softirq_expires_next;
	struct hrtimer			*softirq_next_timer;
	struct hrtimer_clock_base	clock_base[HRTIMER_MAX_CLOCK_BASES];
	call_single_data_t		csd;
} ____cacheline_aligned;


#endif
