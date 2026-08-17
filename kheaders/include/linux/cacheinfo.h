/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_CACHEINFO_H
#define _LINUX_CACHEINFO_H

#include <linux/bitops.h>
#include <linux/cpuhplock.h>
#include <linux/cpumask_types.h>
#include <linux/smp.h>

struct device_node;
struct attribute;

enum cache_type {
	CACHE_TYPE_NOCACHE = 0,
	CACHE_TYPE_INST = BIT(0),
	CACHE_TYPE_DATA = BIT(1),
	CACHE_TYPE_SEPARATE = CACHE_TYPE_INST | CACHE_TYPE_DATA,
	CACHE_TYPE_UNIFIED = BIT(2),
};

extern unsigned int coherency_max_size;


struct cacheinfo {
	unsigned int id;
	enum cache_type type;
	unsigned int level;
	unsigned int coherency_line_size;
	unsigned int number_of_sets;
	unsigned int ways_of_associativity;
	unsigned int physical_line_partition;
	unsigned int size;
	cpumask_t shared_cpu_map;
	unsigned int attributes;
#define CACHE_WRITE_THROUGH	BIT(0)
#define CACHE_WRITE_BACK	BIT(1)
#define CACHE_WRITE_POLICY_MASK		\
	(CACHE_WRITE_THROUGH | CACHE_WRITE_BACK)
#define CACHE_READ_ALLOCATE	BIT(2)
#define CACHE_WRITE_ALLOCATE	BIT(3)
#define CACHE_ALLOCATE_POLICY_MASK	\
	(CACHE_READ_ALLOCATE | CACHE_WRITE_ALLOCATE)
#define CACHE_ID		BIT(4)
	void *fw_token;
	bool disable_sysfs;
	void *priv;
};

struct cpu_cacheinfo {
	struct cacheinfo *info_list;
	unsigned int per_cpu_data_slice_size;
	unsigned int num_levels;
	unsigned int num_leaves;
	bool cpu_map_populated;
	bool early_ci_levels;
};

struct cpu_cacheinfo *get_cpu_cacheinfo(unsigned int cpu);
int early_cache_level(unsigned int cpu);
int init_cache_level(unsigned int cpu);
int init_of_cache_level(unsigned int cpu);
int populate_cache_leaves(unsigned int cpu);
int cache_setup_acpi(unsigned int cpu);
bool last_level_cache_is_valid(unsigned int cpu);
bool last_level_cache_is_shared(unsigned int cpu_x, unsigned int cpu_y);
int fetch_cache_info(unsigned int cpu);
int detect_cache_attributes(unsigned int cpu);
#ifndef CONFIG_ACPI_PPTT

static inline
int acpi_get_cache_info(unsigned int cpu,
			unsigned int *levels, unsigned int *split_levels)
{
	return -ENOENT;
}
#else
int acpi_get_cache_info(unsigned int cpu,
			unsigned int *levels, unsigned int *split_levels);
#endif

const struct attribute_group *cache_get_priv_group(struct cacheinfo *this_leaf);


static inline struct cacheinfo *get_cpu_cacheinfo_level(int cpu, int level)
{
	struct cpu_cacheinfo *ci = get_cpu_cacheinfo(cpu);
	int i;

	lockdep_assert_cpus_held();

	for (i = 0; i < ci->num_leaves; i++) {
		if (ci->info_list[i].level == level) {
			if (ci->info_list[i].attributes & CACHE_ID)
				return &ci->info_list[i];
			return NULL;
		}
	}

	return NULL;
}


static inline int get_cpu_cacheinfo_id(int cpu, int level)
{
	struct cacheinfo *ci = get_cpu_cacheinfo_level(cpu, level);

	return ci ? ci->id : -1;
}

#ifdef CONFIG_ARM64
#define use_arch_cache_info()	(true)
#else
#define use_arch_cache_info()	(false)
#endif

#ifndef CONFIG_ARCH_HAS_CPU_CACHE_ALIASING
#define cpu_dcache_is_aliasing()	false
#else
#include <asm/cachetype.h>
#endif

#endif 
