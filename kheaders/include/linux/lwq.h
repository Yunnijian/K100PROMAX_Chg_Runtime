/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef LWQ_H
#define LWQ_H

#include <linux/container_of.h>
#include <linux/spinlock.h>
#include <linux/llist.h>

struct lwq_node {
	struct llist_node node;
};

struct lwq {
	spinlock_t		lock;
	struct llist_node	*ready;		
	struct llist_head	new;		
};


static inline void lwq_init(struct lwq *q)
{
	spin_lock_init(&q->lock);
	q->ready = NULL;
	init_llist_head(&q->new);
}


static inline bool lwq_empty(struct lwq *q)
{
	
	return smp_load_acquire(&q->ready) == NULL && llist_empty(&q->new);
}

struct llist_node *__lwq_dequeue(struct lwq *q);

#define lwq_dequeue(q, type, member)					\
	({ struct llist_node *_n = __lwq_dequeue(q);			\
	  _n ? container_of(_n, type, member.node) : NULL; })

struct llist_node *lwq_dequeue_all(struct lwq *q);


#define lwq_for_each_safe(_n, _t1, _t2, _l, _member)			\
	for (_t1 = (_l);						\
	     *(_t1) ? (_n = container_of(*(_t1), typeof(*(_n)), _member.node),\
		       _t2 = ((*_t1)->next),				\
		       true)						\
	     : false;							\
	     (_n) ? (_t1 = &(_n)->_member.node.next, 0)			\
	     : ((*(_t1) = (_t2)),  0))


static inline bool lwq_enqueue(struct lwq_node *n, struct lwq *q)
{
	
	return llist_add(&n->node, &q->new) &&
		smp_load_acquire(&q->ready) == NULL;
}


static inline bool lwq_enqueue_batch(struct llist_node *n, struct lwq *q)
{
	struct llist_node *e = n;

	
	return llist_add_batch(llist_reverse_order(n), e, &q->new) &&
		smp_load_acquire(&q->ready) == NULL;
}
#endif 
