/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_CLOSURE_H
#define _LINUX_CLOSURE_H

#include <linux/llist.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/workqueue.h>



struct closure;
struct closure_syncer;
typedef void (closure_fn) (struct work_struct *);
extern struct dentry *bcache_debug;

struct closure_waitlist {
	struct llist_head	list;
};

enum closure_state {
	

	CLOSURE_BITS_START	= (1U << 26),
	CLOSURE_DESTRUCTOR	= (1U << 26),
	CLOSURE_WAITING		= (1U << 28),
	CLOSURE_RUNNING		= (1U << 30),
};

#define CLOSURE_GUARD_MASK					\
	((CLOSURE_DESTRUCTOR|CLOSURE_WAITING|CLOSURE_RUNNING) << 1)

#define CLOSURE_REMAINING_MASK		(CLOSURE_BITS_START - 1)
#define CLOSURE_REMAINING_INITIALIZER	(1|CLOSURE_RUNNING)

struct closure {
	union {
		struct {
			struct workqueue_struct *wq;
			struct closure_syncer	*s;
			struct llist_node	list;
			closure_fn		*fn;
		};
		struct work_struct	work;
	};

	struct closure		*parent;

	atomic_t		remaining;
	bool			closure_get_happened;

#ifdef CONFIG_DEBUG_CLOSURES
#define CLOSURE_MAGIC_DEAD	0xc054dead
#define CLOSURE_MAGIC_ALIVE	0xc054a11e
#define CLOSURE_MAGIC_STACK	0xc05451cc

	unsigned int		magic;
	struct list_head	all;
	unsigned long		ip;
	unsigned long		waiting_on;
#endif
};

void closure_sub(struct closure *cl, int v);
void closure_put(struct closure *cl);
void __closure_wake_up(struct closure_waitlist *list);
bool closure_wait(struct closure_waitlist *list, struct closure *cl);
void __closure_sync(struct closure *cl);

static inline unsigned closure_nr_remaining(struct closure *cl)
{
	return atomic_read(&cl->remaining) & CLOSURE_REMAINING_MASK;
}


static inline void closure_sync(struct closure *cl)
{
#ifdef CONFIG_DEBUG_CLOSURES
	BUG_ON(closure_nr_remaining(cl) != 1 && !cl->closure_get_happened);
#endif

	if (cl->closure_get_happened)
		__closure_sync(cl);
}

int __closure_sync_timeout(struct closure *cl, unsigned long timeout);

static inline int closure_sync_timeout(struct closure *cl, unsigned long timeout)
{
#ifdef CONFIG_DEBUG_CLOSURES
	BUG_ON(closure_nr_remaining(cl) != 1 && !cl->closure_get_happened);
#endif
	return cl->closure_get_happened
		? __closure_sync_timeout(cl, timeout)
		: 0;
}

#ifdef CONFIG_DEBUG_CLOSURES

void closure_debug_create(struct closure *cl);
void closure_debug_destroy(struct closure *cl);

#else

static inline void closure_debug_create(struct closure *cl) {}
static inline void closure_debug_destroy(struct closure *cl) {}

#endif

static inline void closure_set_ip(struct closure *cl)
{
#ifdef CONFIG_DEBUG_CLOSURES
	cl->ip = _THIS_IP_;
#endif
}

static inline void closure_set_ret_ip(struct closure *cl)
{
#ifdef CONFIG_DEBUG_CLOSURES
	cl->ip = _RET_IP_;
#endif
}

static inline void closure_set_waiting(struct closure *cl, unsigned long f)
{
#ifdef CONFIG_DEBUG_CLOSURES
	cl->waiting_on = f;
#endif
}

static inline void closure_set_stopped(struct closure *cl)
{
	atomic_sub(CLOSURE_RUNNING, &cl->remaining);
}

static inline void set_closure_fn(struct closure *cl, closure_fn *fn,
				  struct workqueue_struct *wq)
{
	closure_set_ip(cl);
	cl->fn = fn;
	cl->wq = wq;
}

static inline void closure_queue(struct closure *cl)
{
	struct workqueue_struct *wq = cl->wq;
	
	BUILD_BUG_ON(offsetof(struct closure, fn)
		     != offsetof(struct work_struct, func));

	if (wq) {
		INIT_WORK(&cl->work, cl->work.func);
		BUG_ON(!queue_work(wq, &cl->work));
	} else
		cl->fn(&cl->work);
}


static inline void closure_get(struct closure *cl)
{
	cl->closure_get_happened = true;

#ifdef CONFIG_DEBUG_CLOSURES
	BUG_ON((atomic_inc_return(&cl->remaining) &
		CLOSURE_REMAINING_MASK) <= 1);
#else
	atomic_inc(&cl->remaining);
#endif
}


static inline bool closure_get_not_zero(struct closure *cl)
{
	unsigned old = atomic_read(&cl->remaining);
	do {
		if (!(old & CLOSURE_REMAINING_MASK))
			return false;

	} while (!atomic_try_cmpxchg_acquire(&cl->remaining, &old, old + 1));

	return true;
}


static inline void closure_init(struct closure *cl, struct closure *parent)
{
	cl->fn = NULL;
	cl->parent = parent;
	if (parent)
		closure_get(parent);

	atomic_set(&cl->remaining, CLOSURE_REMAINING_INITIALIZER);
	cl->closure_get_happened = false;

	closure_debug_create(cl);
	closure_set_ip(cl);
}

static inline void closure_init_stack(struct closure *cl)
{
	memset(cl, 0, sizeof(struct closure));
	atomic_set(&cl->remaining, CLOSURE_REMAINING_INITIALIZER);
#ifdef CONFIG_DEBUG_CLOSURES
	cl->magic = CLOSURE_MAGIC_STACK;
#endif
}

static inline void closure_init_stack_release(struct closure *cl)
{
	memset(cl, 0, sizeof(struct closure));
	atomic_set_release(&cl->remaining, CLOSURE_REMAINING_INITIALIZER);
#ifdef CONFIG_DEBUG_CLOSURES
	cl->magic = CLOSURE_MAGIC_STACK;
#endif
}


static inline void closure_wake_up(struct closure_waitlist *list)
{
	
	smp_mb();
	__closure_wake_up(list);
}

#define CLOSURE_CALLBACK(name)	void name(struct work_struct *ws)
#define closure_type(name, type, member)				\
	struct closure *cl = container_of(ws, struct closure, work);	\
	type *name = container_of(cl, type, member)


#define continue_at(_cl, _fn, _wq)					\
do {									\
	set_closure_fn(_cl, _fn, _wq);					\
	closure_sub(_cl, CLOSURE_RUNNING + 1);				\
} while (0)


#define closure_return(_cl)	continue_at((_cl), NULL, NULL)

void closure_return_sync(struct closure *cl);


#define continue_at_nobarrier(_cl, _fn, _wq)				\
do {									\
	set_closure_fn(_cl, _fn, _wq);					\
	closure_queue(_cl);						\
} while (0)


#define closure_return_with_destructor(_cl, _destructor)		\
do {									\
	set_closure_fn(_cl, _destructor, NULL);				\
	closure_sub(_cl, CLOSURE_RUNNING - CLOSURE_DESTRUCTOR + 1);	\
} while (0)


static inline void closure_call(struct closure *cl, closure_fn fn,
				struct workqueue_struct *wq,
				struct closure *parent)
{
	closure_init(cl, parent);
	continue_at_nobarrier(cl, fn, wq);
}

#define __closure_wait_event(waitlist, _cond)				\
do {									\
	struct closure cl;						\
									\
	closure_init_stack(&cl);					\
									\
	while (1) {							\
		closure_wait(waitlist, &cl);				\
		if (_cond)						\
			break;						\
		closure_sync(&cl);					\
	}								\
	closure_wake_up(waitlist);					\
	closure_sync(&cl);						\
} while (0)

#define closure_wait_event(waitlist, _cond)				\
do {									\
	if (!(_cond))							\
		__closure_wait_event(waitlist, _cond);			\
} while (0)

#define __closure_wait_event_timeout(waitlist, _cond, _until)		\
({									\
	struct closure cl;						\
	long _t;							\
									\
	closure_init_stack(&cl);					\
									\
	while (1) {							\
		closure_wait(waitlist, &cl);				\
		if (_cond) {						\
			_t = max_t(long, 1L, _until - jiffies);		\
			break;						\
		}							\
		_t = max_t(long, 0L, _until - jiffies);			\
		if (!_t)						\
			break;						\
		closure_sync_timeout(&cl, _t);				\
	}								\
	closure_wake_up(waitlist);					\
	closure_sync(&cl);						\
	_t;								\
})


#define closure_wait_event_timeout(waitlist, _cond, _timeout)		\
({									\
	unsigned long _until = jiffies + _timeout;			\
	(_cond)								\
		? max_t(long, 1L, _until - jiffies)			\
		: __closure_wait_event_timeout(waitlist, _cond, _until);\
})

#endif 
