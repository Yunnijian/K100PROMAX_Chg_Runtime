/* SPDX-License-Identifier: GPL-2.0 */
#ifndef IOPRIO_H
#define IOPRIO_H

#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/iocontext.h>

#include <uapi/linux/ioprio.h>


#define IOPRIO_DEFAULT	IOPRIO_PRIO_VALUE(IOPRIO_CLASS_NONE, 0)


static inline bool ioprio_valid(unsigned short ioprio)
{
	unsigned short class = IOPRIO_PRIO_CLASS(ioprio);

	return class > IOPRIO_CLASS_NONE && class <= IOPRIO_CLASS_IDLE;
}


static inline int task_nice_ioprio(struct task_struct *task)
{
	return (task_nice(task) + 20) / 5;
}


static inline int task_nice_ioclass(struct task_struct *task)
{
	if (task->policy == SCHED_IDLE)
		return IOPRIO_CLASS_IDLE;
	else if (rt_or_dl_task_policy(task))
		return IOPRIO_CLASS_RT;
	else
		return IOPRIO_CLASS_BE;
}

#ifdef CONFIG_BLOCK

static inline int __get_task_ioprio(struct task_struct *p)
{
	struct io_context *ioc = p->io_context;
	int prio;

	if (!ioc)
		return IOPRIO_PRIO_VALUE(task_nice_ioclass(p),
					 task_nice_ioprio(p));

	if (p != current)
		lockdep_assert_held(&p->alloc_lock);

	prio = ioc->ioprio;
	if (IOPRIO_PRIO_CLASS(prio) == IOPRIO_CLASS_NONE)
		prio = IOPRIO_PRIO_VALUE(task_nice_ioclass(p),
					 task_nice_ioprio(p));
	return prio;
}
#else
static inline int __get_task_ioprio(struct task_struct *p)
{
	return IOPRIO_DEFAULT;
}
#endif 

static inline int get_current_ioprio(void)
{
	return __get_task_ioprio(current);
}

extern int set_task_ioprio(struct task_struct *task, int ioprio);

#ifdef CONFIG_BLOCK
extern int ioprio_check_cap(int ioprio);
#else
static inline int ioprio_check_cap(int ioprio)
{
	return -ENOTBLK;
}
#endif 

#endif
