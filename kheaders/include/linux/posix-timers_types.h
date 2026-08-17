/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _linux_POSIX_TIMERS_TYPES_H
#define _linux_POSIX_TIMERS_TYPES_H

#include <linux/mutex_types.h>
#include <linux/timerqueue_types.h>
#include <linux/types.h>


#define CPUCLOCK_PID(clock)		((pid_t) ~((clock) >> 3))
#define CPUCLOCK_PERTHREAD(clock) \
	(((clock) & (clockid_t) CPUCLOCK_PERTHREAD_MASK) != 0)

#define CPUCLOCK_PERTHREAD_MASK	4
#define CPUCLOCK_WHICH(clock)	((clock) & (clockid_t) CPUCLOCK_CLOCK_MASK)
#define CPUCLOCK_CLOCK_MASK	3
#define CPUCLOCK_PROF		0
#define CPUCLOCK_VIRT		1
#define CPUCLOCK_SCHED		2
#define CPUCLOCK_MAX		3
#define CLOCKFD			CPUCLOCK_MAX
#define CLOCKFD_MASK		(CPUCLOCK_PERTHREAD_MASK|CPUCLOCK_CLOCK_MASK)

#ifdef CONFIG_POSIX_TIMERS


struct posix_cputimer_base {
	u64			nextevt;
	struct timerqueue_head	tqhead;
};


struct posix_cputimers {
	struct posix_cputimer_base	bases[CPUCLOCK_MAX];
	unsigned int			timers_active;
	unsigned int			expiry_active;
};


struct posix_cputimers_work {
	struct callback_head	work;
	struct mutex		mutex;
	unsigned int		scheduled;
};

#else 

struct posix_cputimers { };

#endif 

#endif 
