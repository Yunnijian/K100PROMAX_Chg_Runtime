/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _LINUX_DQL_H
#define _LINUX_DQL_H

#ifdef __KERNEL__

#include <linux/bitops.h>
#include <asm/bug.h>

#define DQL_HIST_LEN		4
#define DQL_HIST_ENT(dql, idx)	((dql)->history[(idx) % DQL_HIST_LEN])

struct dql {
	
	unsigned int	num_queued;		
	unsigned int	adj_limit;		
	unsigned int	last_obj_cnt;		

	
	unsigned short	stall_thrs;

	unsigned long	history_head;		
	
	unsigned long	history[DQL_HIST_LEN];

	

	unsigned int	limit ____cacheline_aligned_in_smp; 
	unsigned int	num_completed;		

	unsigned int	prev_ovlimit;		
	unsigned int	prev_num_queued;	
	unsigned int	prev_last_obj_cnt;	

	unsigned int	lowest_slack;		
	unsigned long	slack_start_time;	

	
	unsigned int	max_limit;		
	unsigned int	min_limit;		
	unsigned int	slack_hold_time;	

	
	unsigned short	stall_max;
	unsigned long	last_reap;		
	unsigned long	stall_cnt;		
};


#define DQL_MAX_OBJECT (UINT_MAX / 16)
#define DQL_MAX_LIMIT ((UINT_MAX / 2) - DQL_MAX_OBJECT)


static inline void dql_queue_stall(struct dql *dql)
{
	unsigned long map, now, now_hi, i;

	now = jiffies;
	now_hi = now / BITS_PER_LONG;

	
	if (unlikely(now_hi != dql->history_head)) {
		
		for (i = 0; i < DQL_HIST_LEN; i++) {
			
			if (now_hi * BITS_PER_LONG ==
			    (dql->history_head + i) * BITS_PER_LONG)
				break;
			DQL_HIST_ENT(dql, dql->history_head + i + 1) = 0;
		}
		
		smp_wmb();
		WRITE_ONCE(dql->history_head, now_hi);
	}

	
	map = DQL_HIST_ENT(dql, now_hi);

	
	if (!(map & BIT_MASK(now)))
		WRITE_ONCE(DQL_HIST_ENT(dql, now_hi), map | BIT_MASK(now));
}


static inline void dql_queued(struct dql *dql, unsigned int count)
{
	if (WARN_ON_ONCE(count > DQL_MAX_OBJECT))
		return;

	dql->last_obj_cnt = count;

	
	barrier();

	dql->num_queued += count;

	
	if (READ_ONCE(dql->stall_thrs))
		dql_queue_stall(dql);
}


static inline int dql_avail(const struct dql *dql)
{
	return READ_ONCE(dql->adj_limit) - READ_ONCE(dql->num_queued);
}


void dql_completed(struct dql *dql, unsigned int count);


void dql_reset(struct dql *dql);


void dql_init(struct dql *dql, unsigned int hold_time);

#endif 

#endif 
