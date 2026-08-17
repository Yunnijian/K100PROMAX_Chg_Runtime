/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_ENTRYCOMMON_H
#define __LINUX_ENTRYCOMMON_H

#include <linux/static_call_types.h>
#include <linux/ptrace.h>
#include <linux/syscalls.h>
#include <linux/seccomp.h>
#include <linux/sched.h>
#include <linux/context_tracking.h>
#include <linux/livepatch.h>
#include <linux/resume_user_mode.h>
#include <linux/tick.h>
#include <linux/kmsan.h>

#include <asm/entry-common.h>


#ifndef _TIF_PATCH_PENDING
# define _TIF_PATCH_PENDING		(0)
#endif

#ifndef _TIF_UPROBE
# define _TIF_UPROBE			(0)
#endif


#ifndef ARCH_SYSCALL_WORK_ENTER
# define ARCH_SYSCALL_WORK_ENTER	(0)
#endif


#ifndef ARCH_SYSCALL_WORK_EXIT
# define ARCH_SYSCALL_WORK_EXIT		(0)
#endif

#define SYSCALL_WORK_ENTER	(SYSCALL_WORK_SECCOMP |			\
				 SYSCALL_WORK_SYSCALL_TRACEPOINT |	\
				 SYSCALL_WORK_SYSCALL_TRACE |		\
				 SYSCALL_WORK_SYSCALL_EMU |		\
				 SYSCALL_WORK_SYSCALL_AUDIT |		\
				 SYSCALL_WORK_SYSCALL_USER_DISPATCH |	\
				 ARCH_SYSCALL_WORK_ENTER)
#define SYSCALL_WORK_EXIT	(SYSCALL_WORK_SYSCALL_TRACEPOINT |	\
				 SYSCALL_WORK_SYSCALL_TRACE |		\
				 SYSCALL_WORK_SYSCALL_AUDIT |		\
				 SYSCALL_WORK_SYSCALL_USER_DISPATCH |	\
				 SYSCALL_WORK_SYSCALL_EXIT_TRAP	|	\
				 ARCH_SYSCALL_WORK_EXIT)


#ifndef ARCH_EXIT_TO_USER_MODE_WORK
# define ARCH_EXIT_TO_USER_MODE_WORK		(0)
#endif

#define EXIT_TO_USER_MODE_WORK						\
	(_TIF_SIGPENDING | _TIF_NOTIFY_RESUME | _TIF_UPROBE |		\
	 _TIF_NEED_RESCHED | _TIF_PATCH_PENDING | _TIF_NOTIFY_SIGNAL |	\
	 ARCH_EXIT_TO_USER_MODE_WORK)


static __always_inline void arch_enter_from_user_mode(struct pt_regs *regs);

#ifndef arch_enter_from_user_mode
static __always_inline void arch_enter_from_user_mode(struct pt_regs *regs) {}
#endif


static __always_inline void enter_from_user_mode(struct pt_regs *regs)
{
	arch_enter_from_user_mode(regs);
	lockdep_hardirqs_off(CALLER_ADDR0);

	CT_WARN_ON(__ct_state() != CT_STATE_USER);
	user_exit_irqoff();

	instrumentation_begin();
	kmsan_unpoison_entry_regs(regs);
	trace_hardirqs_off_finish();
	instrumentation_end();
}


void syscall_enter_from_user_mode_prepare(struct pt_regs *regs);

long syscall_trace_enter(struct pt_regs *regs, long syscall,
			 unsigned long work);


static __always_inline long syscall_enter_from_user_mode_work(struct pt_regs *regs, long syscall)
{
	unsigned long work = READ_ONCE(current_thread_info()->syscall_work);

	if (work & SYSCALL_WORK_ENTER)
		syscall = syscall_trace_enter(regs, syscall, work);

	return syscall;
}


static __always_inline long syscall_enter_from_user_mode(struct pt_regs *regs, long syscall)
{
	long ret;

	enter_from_user_mode(regs);

	instrumentation_begin();
	local_irq_enable();
	ret = syscall_enter_from_user_mode_work(regs, syscall);
	instrumentation_end();

	return ret;
}


static inline void local_irq_enable_exit_to_user(unsigned long ti_work);

#ifndef local_irq_enable_exit_to_user
static inline void local_irq_enable_exit_to_user(unsigned long ti_work)
{
	local_irq_enable();
}
#endif


static inline void local_irq_disable_exit_to_user(void);

#ifndef local_irq_disable_exit_to_user
static inline void local_irq_disable_exit_to_user(void)
{
	local_irq_disable();
}
#endif


static inline void arch_exit_to_user_mode_work(struct pt_regs *regs,
					       unsigned long ti_work);

#ifndef arch_exit_to_user_mode_work
static inline void arch_exit_to_user_mode_work(struct pt_regs *regs,
					       unsigned long ti_work)
{
}
#endif


static inline void arch_exit_to_user_mode_prepare(struct pt_regs *regs,
						  unsigned long ti_work);

#ifndef arch_exit_to_user_mode_prepare
static inline void arch_exit_to_user_mode_prepare(struct pt_regs *regs,
						  unsigned long ti_work)
{
}
#endif


static __always_inline void arch_exit_to_user_mode(void);

#ifndef arch_exit_to_user_mode
static __always_inline void arch_exit_to_user_mode(void) { }
#endif


void arch_do_signal_or_restart(struct pt_regs *regs);


unsigned long exit_to_user_mode_loop(struct pt_regs *regs,
				     unsigned long ti_work);


static __always_inline void exit_to_user_mode_prepare(struct pt_regs *regs)
{
	unsigned long ti_work;

	lockdep_assert_irqs_disabled();

	
	tick_nohz_user_enter_prepare();

	ti_work = read_thread_flags();
	if (unlikely(ti_work & EXIT_TO_USER_MODE_WORK))
		ti_work = exit_to_user_mode_loop(regs, ti_work);

	arch_exit_to_user_mode_prepare(regs, ti_work);

	
	kmap_assert_nomap();
	lockdep_assert_irqs_disabled();
	lockdep_sys_exit();
}


static __always_inline void exit_to_user_mode(void)
{
	instrumentation_begin();
	trace_hardirqs_on_prepare();
	lockdep_hardirqs_on_prepare();
	instrumentation_end();

	user_enter_irqoff();
	arch_exit_to_user_mode();
	lockdep_hardirqs_on(CALLER_ADDR0);
}


void syscall_exit_to_user_mode_work(struct pt_regs *regs);


void syscall_exit_to_user_mode(struct pt_regs *regs);


void irqentry_enter_from_user_mode(struct pt_regs *regs);


void irqentry_exit_to_user_mode(struct pt_regs *regs);

#ifndef irqentry_state

typedef struct irqentry_state {
	union {
		bool	exit_rcu;
		bool	lockdep;
	};
} irqentry_state_t;
#endif


irqentry_state_t noinstr irqentry_enter(struct pt_regs *regs);


void raw_irqentry_exit_cond_resched(void);
#ifdef CONFIG_PREEMPT_DYNAMIC
#if defined(CONFIG_HAVE_PREEMPT_DYNAMIC_CALL)
#define irqentry_exit_cond_resched_dynamic_enabled	raw_irqentry_exit_cond_resched
#define irqentry_exit_cond_resched_dynamic_disabled	NULL
DECLARE_STATIC_CALL(irqentry_exit_cond_resched, raw_irqentry_exit_cond_resched);
#define irqentry_exit_cond_resched()	static_call(irqentry_exit_cond_resched)()
#elif defined(CONFIG_HAVE_PREEMPT_DYNAMIC_KEY)
DECLARE_STATIC_KEY_TRUE(sk_dynamic_irqentry_exit_cond_resched);
void dynamic_irqentry_exit_cond_resched(void);
#define irqentry_exit_cond_resched()	dynamic_irqentry_exit_cond_resched()
#endif
#else 
#define irqentry_exit_cond_resched()	raw_irqentry_exit_cond_resched()
#endif 


void noinstr irqentry_exit(struct pt_regs *regs, irqentry_state_t state);


irqentry_state_t noinstr irqentry_nmi_enter(struct pt_regs *regs);


void noinstr irqentry_nmi_exit(struct pt_regs *regs, irqentry_state_t irq_state);

#endif
