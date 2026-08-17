/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_RCUPDATE_WAIT_H
#define _LINUX_SCHED_RCUPDATE_WAIT_H



#include <linux/rcupdate.h>
#include <linux/completion.h>
#include <linux/sched.h>


struct rcu_synchronize {
	struct rcu_head head;
	struct completion completion;
};
void wakeme_after_rcu(struct rcu_head *head);

void __wait_rcu_gp(bool checktiny, unsigned int state, int n, call_rcu_func_t *crcu_array,
		   struct rcu_synchronize *rs_array);

#define _wait_rcu_gp(checktiny, state, ...) \
do {												\
	call_rcu_func_t __crcu_array[] = { __VA_ARGS__ };					\
	struct rcu_synchronize __rs_array[ARRAY_SIZE(__crcu_array)];				\
	__wait_rcu_gp(checktiny, state, ARRAY_SIZE(__crcu_array), __crcu_array, __rs_array);	\
} while (0)

#define wait_rcu_gp(...) _wait_rcu_gp(false, TASK_UNINTERRUPTIBLE, __VA_ARGS__)
#define wait_rcu_gp_state(state, ...) _wait_rcu_gp(false, state, __VA_ARGS__)


#define synchronize_rcu_mult(...) \
	_wait_rcu_gp(IS_ENABLED(CONFIG_TINY_RCU), TASK_UNINTERRUPTIBLE, __VA_ARGS__)

static inline void cond_resched_rcu(void)
{
#if defined(CONFIG_DEBUG_ATOMIC_SLEEP) || !defined(CONFIG_PREEMPT_RCU)
	rcu_read_unlock();
	cond_resched();
	rcu_read_lock();
#endif
}

#endif 
