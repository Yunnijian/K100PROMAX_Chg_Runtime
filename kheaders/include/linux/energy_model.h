/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_ENERGY_MODEL_H
#define _LINUX_ENERGY_MODEL_H
#include <linux/cpumask.h>
#include <linux/device.h>
#include <linux/jump_label.h>
#include <linux/kobject.h>
#include <linux/kref.h>
#include <linux/rcupdate.h>
#include <linux/sched/cpufreq.h>
#include <linux/sched/topology.h>
#include <linux/types.h>


struct em_perf_state {
	unsigned long performance;
	unsigned long frequency;
	unsigned long power;
	unsigned long cost;
	unsigned long flags;
};


#define EM_PERF_STATE_INEFFICIENT BIT(0)


struct em_perf_table {
	struct rcu_head rcu;
	struct kref kref;
	struct em_perf_state state[];
};


struct em_perf_domain {
	struct em_perf_table __rcu *em_table;
	int nr_perf_states;
	int min_perf_state;
	int max_perf_state;
	unsigned long flags;
	unsigned long cpus[];
};


#define EM_PERF_DOMAIN_MICROWATTS BIT(0)
#define EM_PERF_DOMAIN_SKIP_INEFFICIENCIES BIT(1)
#define EM_PERF_DOMAIN_ARTIFICIAL BIT(2)

#define em_span_cpus(em) (to_cpumask((em)->cpus))
#define em_is_artificial(em) ((em)->flags & EM_PERF_DOMAIN_ARTIFICIAL)

#ifdef CONFIG_ENERGY_MODEL

#define EM_MAX_POWER (64000000) 


#ifdef CONFIG_64BIT
#define EM_MAX_NUM_CPUS 4096
#else
#define EM_MAX_NUM_CPUS 16
#endif

struct em_data_callback {
	
	int (*active_power)(struct device *dev, unsigned long *power,
			    unsigned long *freq);

	
	int (*get_cost)(struct device *dev, unsigned long freq,
			unsigned long *cost);
};
#define EM_SET_ACTIVE_POWER_CB(em_cb, cb) ((em_cb).active_power = cb)
#define EM_ADV_DATA_CB(_active_power_cb, _cost_cb)	\
	{ .active_power = _active_power_cb,		\
	  .get_cost = _cost_cb }
#define EM_DATA_CB(_active_power_cb)			\
		EM_ADV_DATA_CB(_active_power_cb, NULL)

struct em_perf_domain *em_cpu_get(int cpu);
struct em_perf_domain *em_pd_get(struct device *dev);
int em_dev_update_perf_domain(struct device *dev,
			      struct em_perf_table *new_table);
int em_dev_register_perf_domain(struct device *dev, unsigned int nr_states,
				struct em_data_callback *cb, cpumask_t *span,
				bool microwatts);
void em_dev_unregister_perf_domain(struct device *dev);
struct em_perf_table *em_table_alloc(struct em_perf_domain *pd);
void em_table_free(struct em_perf_table *table);
int em_dev_compute_costs(struct device *dev, struct em_perf_state *table,
			 int nr_states);
int em_dev_update_chip_binning(struct device *dev);
int em_update_performance_limits(struct em_perf_domain *pd,
		unsigned long freq_min_khz, unsigned long freq_max_khz);


static inline int
em_pd_get_efficient_state(struct em_perf_state *table,
			  struct em_perf_domain *pd, unsigned long max_util)
{
	unsigned long pd_flags = pd->flags;
	int min_ps = pd->min_perf_state;
	int max_ps = pd->max_perf_state;
	struct em_perf_state *ps;
	int i;

	for (i = min_ps; i <= max_ps; i++) {
		ps = &table[i];
		if (ps->performance >= max_util) {
			if (pd_flags & EM_PERF_DOMAIN_SKIP_INEFFICIENCIES &&
			    ps->flags & EM_PERF_STATE_INEFFICIENT)
				continue;
			return i;
		}
	}

	return max_ps;
}


static inline unsigned long em_cpu_energy(struct em_perf_domain *pd,
				unsigned long max_util, unsigned long sum_util,
				unsigned long allowed_cpu_cap)
{
	struct em_perf_table *em_table;
	struct em_perf_state *ps;
	int i;

#ifdef CONFIG_SCHED_DEBUG
	WARN_ONCE(!rcu_read_lock_held(), "EM: rcu read lock needed\n");
#endif

	if (!sum_util)
		return 0;

	
	max_util = min(max_util, allowed_cpu_cap);

	
	em_table = rcu_dereference(pd->em_table);
	i = em_pd_get_efficient_state(em_table->state, pd, max_util);
	ps = &em_table->state[i];

	
	return ps->cost * sum_util;
}


static inline int em_pd_nr_perf_states(struct em_perf_domain *pd)
{
	return pd->nr_perf_states;
}


static inline
struct em_perf_state *em_perf_state_from_pd(struct em_perf_domain *pd)
{
	return rcu_dereference(pd->em_table)->state;
}

#else
struct em_data_callback {};
#define EM_ADV_DATA_CB(_active_power_cb, _cost_cb) { }
#define EM_DATA_CB(_active_power_cb) { }
#define EM_SET_ACTIVE_POWER_CB(em_cb, cb) do { } while (0)

static inline
int em_dev_register_perf_domain(struct device *dev, unsigned int nr_states,
				struct em_data_callback *cb, cpumask_t *span,
				bool microwatts)
{
	return -EINVAL;
}
static inline void em_dev_unregister_perf_domain(struct device *dev)
{
}
static inline struct em_perf_domain *em_cpu_get(int cpu)
{
	return NULL;
}
static inline struct em_perf_domain *em_pd_get(struct device *dev)
{
	return NULL;
}
static inline unsigned long em_cpu_energy(struct em_perf_domain *pd,
			unsigned long max_util, unsigned long sum_util,
			unsigned long allowed_cpu_cap)
{
	return 0;
}
static inline int em_pd_nr_perf_states(struct em_perf_domain *pd)
{
	return 0;
}
static inline
struct em_perf_table *em_table_alloc(struct em_perf_domain *pd)
{
	return NULL;
}
static inline void em_table_free(struct em_perf_table *table) {}
static inline
int em_dev_update_perf_domain(struct device *dev,
			      struct em_perf_table *new_table)
{
	return -EINVAL;
}
static inline
struct em_perf_state *em_perf_state_from_pd(struct em_perf_domain *pd)
{
	return NULL;
}
static inline
int em_dev_compute_costs(struct device *dev, struct em_perf_state *table,
			 int nr_states)
{
	return -EINVAL;
}
static inline int em_dev_update_chip_binning(struct device *dev)
{
	return -EINVAL;
}
static inline
int em_update_performance_limits(struct em_perf_domain *pd,
		unsigned long freq_min_khz, unsigned long freq_max_khz)
{
	return -EINVAL;
}
#endif

#endif
