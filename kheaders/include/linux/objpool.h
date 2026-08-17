/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_OBJPOOL_H
#define _LINUX_OBJPOOL_H

#include <linux/types.h>
#include <linux/refcount.h>
#include <linux/atomic.h>
#include <linux/cpumask.h>
#include <linux/irqflags.h>
#include <linux/smp.h>




struct objpool_slot {
	uint32_t            head;
	uint32_t            tail;
	uint32_t            last;
	uint32_t            mask;
	void               *entries[];
} __packed;

struct objpool_head;


typedef int (*objpool_init_obj_cb)(void *obj, void *context);


typedef int (*objpool_fini_cb)(struct objpool_head *head, void *context);


struct objpool_head {
	int                     obj_size;
	int                     nr_objs;
	int                     nr_possible_cpus;
	int                     capacity;
	gfp_t                   gfp;
	refcount_t              ref;
	unsigned long           flags;
	struct objpool_slot   **cpu_slots;
	objpool_fini_cb         release;
	void                   *context;
};

#define OBJPOOL_NR_OBJECT_MAX	(1UL << 24) 
#define OBJPOOL_OBJECT_SIZE_MAX	(1UL << 16) 


int objpool_init(struct objpool_head *pool, int nr_objs, int object_size,
		 gfp_t gfp, void *context, objpool_init_obj_cb objinit,
		 objpool_fini_cb release);


static inline void *__objpool_try_get_slot(struct objpool_head *pool, int cpu)
{
	struct objpool_slot *slot = pool->cpu_slots[cpu];
	
	uint32_t head = smp_load_acquire(&slot->head);

	while (head != READ_ONCE(slot->last)) {
		void *obj;

		
		if (READ_ONCE(slot->last) - head - 1 >= pool->nr_objs) {
			head = READ_ONCE(slot->head);
			continue;
		}

		
		obj = READ_ONCE(slot->entries[head & slot->mask]);

		
		if (try_cmpxchg_release(&slot->head, &head, head + 1))
			return obj;
	}

	return NULL;
}


static inline void *objpool_pop(struct objpool_head *pool)
{
	void *obj = NULL;
	unsigned long flags;
	int i, cpu;

	
	raw_local_irq_save(flags);

	cpu = raw_smp_processor_id();
	for (i = 0; i < pool->nr_possible_cpus; i++) {
		obj = __objpool_try_get_slot(pool, cpu);
		if (obj)
			break;
		cpu = cpumask_next_wrap(cpu, cpu_possible_mask, -1, 1);
	}
	raw_local_irq_restore(flags);

	return obj;
}


static inline int
__objpool_try_add_slot(void *obj, struct objpool_head *pool, int cpu)
{
	struct objpool_slot *slot = pool->cpu_slots[cpu];
	uint32_t head, tail;

	
	tail = READ_ONCE(slot->tail);

	do {
		head = READ_ONCE(slot->head);
		
		WARN_ON_ONCE(tail - head > pool->nr_objs);
	} while (!try_cmpxchg_acquire(&slot->tail, &tail, tail + 1));

	
	WRITE_ONCE(slot->entries[tail & slot->mask], obj);
	
	smp_store_release(&slot->last, tail + 1);

	return 0;
}


static inline int objpool_push(void *obj, struct objpool_head *pool)
{
	unsigned long flags;
	int rc;

	
	raw_local_irq_save(flags);
	rc = __objpool_try_add_slot(obj, pool, raw_smp_processor_id());
	raw_local_irq_restore(flags);

	return rc;
}



int objpool_drop(void *obj, struct objpool_head *pool);


void objpool_free(struct objpool_head *pool);


void objpool_fini(struct objpool_head *pool);

#endif 
