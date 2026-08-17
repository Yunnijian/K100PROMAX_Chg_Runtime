/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_SCHED_EXT_H
#define _LINUX_SCHED_EXT_H

#ifdef CONFIG_SCHED_CLASS_EXT

#include <linux/llist.h>
#include <linux/rhashtable-types.h>

enum scx_public_consts {
	SCX_OPS_NAME_LEN	= 128,

	SCX_SLICE_DFL		= 20 * 1000000,	
	SCX_SLICE_INF		= U64_MAX,	
};


enum scx_dsq_id_flags {
	SCX_DSQ_FLAG_BUILTIN	= 1LLU << 63,
	SCX_DSQ_FLAG_LOCAL_ON	= 1LLU << 62,

	SCX_DSQ_INVALID		= SCX_DSQ_FLAG_BUILTIN | 0,
	SCX_DSQ_GLOBAL		= SCX_DSQ_FLAG_BUILTIN | 1,
	SCX_DSQ_LOCAL		= SCX_DSQ_FLAG_BUILTIN | 2,
	SCX_DSQ_LOCAL_ON	= SCX_DSQ_FLAG_BUILTIN | SCX_DSQ_FLAG_LOCAL_ON,
	SCX_DSQ_LOCAL_CPU_MASK	= 0xffffffffLLU,
};


struct scx_dispatch_q {
	raw_spinlock_t		lock;
	struct list_head	list;	
	struct rb_root		priq;	
	u32			nr;
	u32			seq;	
	u64			id;
	struct rhash_head	hash_node;
	struct llist_node	free_node;
	struct rcu_head		rcu;
};


enum scx_ent_flags {
	SCX_TASK_QUEUED		= 1 << 0, 
	SCX_TASK_RESET_RUNNABLE_AT = 1 << 2, 
	SCX_TASK_DEQD_FOR_SLEEP	= 1 << 3, 

	SCX_TASK_STATE_SHIFT	= 8,	  
	SCX_TASK_STATE_BITS	= 2,
	SCX_TASK_STATE_MASK	= ((1 << SCX_TASK_STATE_BITS) - 1) << SCX_TASK_STATE_SHIFT,

	SCX_TASK_CURSOR		= 1 << 31, 
};


enum scx_task_state {
	SCX_TASK_NONE,		
	SCX_TASK_INIT,		
	SCX_TASK_READY,		
	SCX_TASK_ENABLED,	

	SCX_TASK_NR_STATES,
};


enum scx_ent_dsq_flags {
	SCX_TASK_DSQ_ON_PRIQ	= 1 << 0, 
};


enum scx_kf_mask {
	SCX_KF_UNLOCKED		= 0,	  
	
	SCX_KF_CPU_RELEASE	= 1 << 0, 
	
	SCX_KF_DISPATCH		= 1 << 1, 
	SCX_KF_ENQUEUE		= 1 << 2, 
	SCX_KF_SELECT_CPU	= 1 << 3, 
	SCX_KF_REST		= 1 << 4, 

	__SCX_KF_RQ_LOCKED	= SCX_KF_CPU_RELEASE | SCX_KF_DISPATCH |
				  SCX_KF_ENQUEUE | SCX_KF_SELECT_CPU | SCX_KF_REST,
	__SCX_KF_TERMINAL	= SCX_KF_ENQUEUE | SCX_KF_SELECT_CPU | SCX_KF_REST,
};

enum scx_dsq_lnode_flags {
	SCX_DSQ_LNODE_ITER_CURSOR = 1 << 0,

	
	__SCX_DSQ_LNODE_PRIV_SHIFT = 16,
};

struct scx_dsq_list_node {
	struct list_head	node;
	u32			flags;
	u32			priv;		
};


struct sched_ext_entity {
	struct scx_dispatch_q	*dsq;
	struct scx_dsq_list_node dsq_list;	
	struct rb_node		dsq_priq;	
	u32			dsq_seq;
	u32			dsq_flags;	
	u32			flags;		
	u32			weight;
	s32			sticky_cpu;
	s32			holding_cpu;
	u32			kf_mask;	
	struct task_struct	*kf_tasks[2];	
	atomic_long_t		ops_state;

	struct list_head	runnable_node;	
	unsigned long		runnable_at;

#ifdef CONFIG_SCHED_CORE
	u64			core_sched_at;	
#endif
	u64			ddsp_dsq_id;
	u64			ddsp_enq_flags;

	

	
	u64			slice;

	
	u64			dsq_vtime;

	
	bool			disallow;	

	
#ifdef CONFIG_EXT_GROUP_SCHED
	struct cgroup		*cgrp_moving_from;
#endif
	struct list_head	tasks_node;
};

void sched_ext_free(struct task_struct *p);
void print_scx_info(const char *log_lvl, struct task_struct *p);
struct scx_dispatch_q *find_user_dsq(u64 dsq_id);

#else	

static inline void sched_ext_free(struct task_struct *p) {}
static inline void print_scx_info(const char *log_lvl, struct task_struct *p) {}

#endif	
#endif	
