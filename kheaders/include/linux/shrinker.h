/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SHRINKER_H
#define _LINUX_SHRINKER_H

#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/refcount.h>
#include <linux/completion.h>

#define SHRINKER_UNIT_BITS	BITS_PER_LONG


struct shrinker_info_unit {
	atomic_long_t nr_deferred[SHRINKER_UNIT_BITS];
	DECLARE_BITMAP(map, SHRINKER_UNIT_BITS);
};

struct shrinker_info {
	struct rcu_head rcu;
	int map_nr_max;
	struct shrinker_info_unit *unit[];
};


struct shrink_control {
	gfp_t gfp_mask;

	
	int nid;

	
	unsigned long nr_to_scan;

	
	unsigned long nr_scanned;

	
	struct mem_cgroup *memcg;
};

#define SHRINK_STOP (~0UL)
#define SHRINK_EMPTY (~0UL - 1)

struct shrinker {
	unsigned long (*count_objects)(struct shrinker *,
				       struct shrink_control *sc);
	unsigned long (*scan_objects)(struct shrinker *,
				      struct shrink_control *sc);

	long batch;	
	int seeks;	
	unsigned flags;

	
	refcount_t refcount;
	struct completion done;	
	struct rcu_head rcu;

	void *private_data;

	
	struct list_head list;
#ifdef CONFIG_MEMCG
	
	int id;
#endif
#ifdef CONFIG_SHRINKER_DEBUG
	int debugfs_id;
	const char *name;
	struct dentry *debugfs_entry;
#endif
	
	atomic_long_t *nr_deferred;
};
#define DEFAULT_SEEKS 2 


#define SHRINKER_REGISTERED	BIT(0)
#define SHRINKER_ALLOCATED	BIT(1)


#define SHRINKER_NUMA_AWARE	BIT(2)
#define SHRINKER_MEMCG_AWARE	BIT(3)

#define SHRINKER_NONSLAB	BIT(4)

__printf(2, 3)
struct shrinker *shrinker_alloc(unsigned int flags, const char *fmt, ...);
void shrinker_register(struct shrinker *shrinker);
void shrinker_free(struct shrinker *shrinker);

static inline bool shrinker_try_get(struct shrinker *shrinker)
{
	return refcount_inc_not_zero(&shrinker->refcount);
}

static inline void shrinker_put(struct shrinker *shrinker)
{
	if (refcount_dec_and_test(&shrinker->refcount))
		complete(&shrinker->done);
}

#ifdef CONFIG_SHRINKER_DEBUG
extern int __printf(2, 3) shrinker_debugfs_rename(struct shrinker *shrinker,
						  const char *fmt, ...);
#else 
static inline __printf(2, 3)
int shrinker_debugfs_rename(struct shrinker *shrinker, const char *fmt, ...)
{
	return 0;
}
#endif 
#endif 
