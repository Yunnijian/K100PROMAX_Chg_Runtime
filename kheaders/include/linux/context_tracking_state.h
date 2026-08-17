/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_CONTEXT_TRACKING_STATE_H
#define _LINUX_CONTEXT_TRACKING_STATE_H

#include <linux/percpu.h>
#include <linux/static_key.h>
#include <linux/context_tracking_irq.h>


#define CT_NESTING_IRQ_NONIDLE	((LONG_MAX / 2) + 1)

enum ctx_state {
	CT_STATE_DISABLED	= -1,	
	CT_STATE_KERNEL		= 0,
	CT_STATE_IDLE		= 1,
	CT_STATE_USER		= 2,
	CT_STATE_GUEST		= 3,
	CT_STATE_MAX		= 4,
};


#define CT_RCU_WATCHING CT_STATE_MAX

#define CT_STATE_MASK (CT_STATE_MAX - 1)
#define CT_RCU_WATCHING_MASK (~CT_STATE_MASK)

struct context_tracking {
#ifdef CONFIG_CONTEXT_TRACKING_USER
	
	bool active;
	int recursion;
#endif
#ifdef CONFIG_CONTEXT_TRACKING
	atomic_t state;
#endif
#ifdef CONFIG_CONTEXT_TRACKING_IDLE
	long nesting;		
	long nmi_nesting;	
#endif
};

#ifdef CONFIG_CONTEXT_TRACKING
DECLARE_PER_CPU(struct context_tracking, context_tracking);
#endif

#ifdef CONFIG_CONTEXT_TRACKING_USER
static __always_inline int __ct_state(void)
{
	return raw_atomic_read(this_cpu_ptr(&context_tracking.state)) & CT_STATE_MASK;
}
#endif

#ifdef CONFIG_CONTEXT_TRACKING_IDLE
static __always_inline int ct_rcu_watching(void)
{
	return atomic_read(this_cpu_ptr(&context_tracking.state)) & CT_RCU_WATCHING_MASK;
}

static __always_inline int ct_rcu_watching_cpu(int cpu)
{
	struct context_tracking *ct = per_cpu_ptr(&context_tracking, cpu);

	return atomic_read(&ct->state) & CT_RCU_WATCHING_MASK;
}

static __always_inline int ct_rcu_watching_cpu_acquire(int cpu)
{
	struct context_tracking *ct = per_cpu_ptr(&context_tracking, cpu);

	return atomic_read_acquire(&ct->state) & CT_RCU_WATCHING_MASK;
}

static __always_inline long ct_nesting(void)
{
	return __this_cpu_read(context_tracking.nesting);
}

static __always_inline long ct_nesting_cpu(int cpu)
{
	struct context_tracking *ct = per_cpu_ptr(&context_tracking, cpu);

	return ct->nesting;
}

static __always_inline long ct_nmi_nesting(void)
{
	return __this_cpu_read(context_tracking.nmi_nesting);
}

static __always_inline long ct_nmi_nesting_cpu(int cpu)
{
	struct context_tracking *ct = per_cpu_ptr(&context_tracking, cpu);

	return ct->nmi_nesting;
}
#endif 

#ifdef CONFIG_CONTEXT_TRACKING_USER
extern struct static_key_false context_tracking_key;

static __always_inline bool context_tracking_enabled(void)
{
	return static_branch_unlikely(&context_tracking_key);
}

static __always_inline bool context_tracking_enabled_cpu(int cpu)
{
	return context_tracking_enabled() && per_cpu(context_tracking.active, cpu);
}

static __always_inline bool context_tracking_enabled_this_cpu(void)
{
	return context_tracking_enabled() && __this_cpu_read(context_tracking.active);
}


static __always_inline int ct_state(void)
{
	int ret;

	if (!context_tracking_enabled())
		return CT_STATE_DISABLED;

	preempt_disable();
	ret = __ct_state();
	preempt_enable();

	return ret;
}

#else
static __always_inline bool context_tracking_enabled(void) { return false; }
static __always_inline bool context_tracking_enabled_cpu(int cpu) { return false; }
static __always_inline bool context_tracking_enabled_this_cpu(void) { return false; }
#endif 

#endif
