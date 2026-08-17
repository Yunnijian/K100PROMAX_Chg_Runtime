/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_HRTIMER_TYPES_H
#define _LINUX_HRTIMER_TYPES_H

#include <linux/types.h>
#include <linux/timerqueue_types.h>

struct hrtimer_clock_base;


enum hrtimer_restart {
	HRTIMER_NORESTART,	
	HRTIMER_RESTART,	
};


struct hrtimer {
	struct timerqueue_node		node;
	ktime_t				_softexpires;
	enum hrtimer_restart		(*function)(struct hrtimer *);
	struct hrtimer_clock_base	*base;
	u8				state;
	u8				is_rel;
	u8				is_soft;
	u8				is_hard;
};

#endif 
