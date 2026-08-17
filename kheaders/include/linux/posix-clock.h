/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _LINUX_POSIX_CLOCK_H_
#define _LINUX_POSIX_CLOCK_H_

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/posix-timers.h>
#include <linux/rwsem.h>

struct posix_clock;
struct posix_clock_context;


struct posix_clock_operations {
	struct module *owner;

	int  (*clock_adjtime)(struct posix_clock *pc, struct __kernel_timex *tx);

	int  (*clock_gettime)(struct posix_clock *pc, struct timespec64 *ts);

	int  (*clock_getres) (struct posix_clock *pc, struct timespec64 *ts);

	int  (*clock_settime)(struct posix_clock *pc,
			      const struct timespec64 *ts);

	
	long (*ioctl)(struct posix_clock_context *pccontext, unsigned int cmd,
		      unsigned long arg);

	int (*open)(struct posix_clock_context *pccontext, fmode_t f_mode);

	__poll_t (*poll)(struct posix_clock_context *pccontext, struct file *file,
			 poll_table *wait);

	int (*release)(struct posix_clock_context *pccontext);

	ssize_t (*read)(struct posix_clock_context *pccontext, uint flags,
			char __user *buf, size_t cnt);
};


struct posix_clock {
	struct posix_clock_operations ops;
	struct cdev cdev;
	struct device *dev;
	struct rw_semaphore rwsem;
	bool zombie;
};


struct posix_clock_context {
	struct posix_clock *clk;
	void *private_clkdata;
};


int posix_clock_register(struct posix_clock *clk, struct device *dev);


void posix_clock_unregister(struct posix_clock *clk);

#endif
