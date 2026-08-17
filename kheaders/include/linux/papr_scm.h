/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LINUX_PAPR_SCM_H
#define __LINUX_PAPR_SCM_H



#define PAPR_PMEM_UNARMED                   (1ULL << (63 - 0))

#define PAPR_PMEM_SHUTDOWN_DIRTY            (1ULL << (63 - 1))

#define PAPR_PMEM_SHUTDOWN_CLEAN            (1ULL << (63 - 2))

#define PAPR_PMEM_EMPTY                     (1ULL << (63 - 3))

#define PAPR_PMEM_HEALTH_CRITICAL           (1ULL << (63 - 4))

#define PAPR_PMEM_HEALTH_FATAL              (1ULL << (63 - 5))

#define PAPR_PMEM_HEALTH_UNHEALTHY          (1ULL << (63 - 6))

#define PAPR_PMEM_HEALTH_NON_CRITICAL       (1ULL << (63 - 7))

#define PAPR_PMEM_ENCRYPTED                 (1ULL << (63 - 8))

#define PAPR_PMEM_SCRUBBED_AND_LOCKED       (1ULL << (63 - 9))

#define PAPR_PMEM_SAVE_FAILED               (1ULL << (63 - 10))


#define PAPR_PMEM_UNARMED_MASK (PAPR_PMEM_UNARMED |            \
				PAPR_PMEM_HEALTH_UNHEALTHY)


#define PAPR_PMEM_BAD_SHUTDOWN_MASK (PAPR_PMEM_SHUTDOWN_DIRTY)


#define PAPR_PMEM_BAD_RESTORE_MASK  (PAPR_PMEM_EMPTY)


#define PAPR_PMEM_SMART_EVENT_MASK (PAPR_PMEM_HEALTH_CRITICAL | \
					PAPR_PMEM_HEALTH_FATAL | \
					PAPR_PMEM_HEALTH_UNHEALTHY)

#define PAPR_PMEM_SAVE_MASK                (PAPR_PMEM_SAVE_FAILED)

#define PAPR_SCM_PERF_STATS_EYECATCHER __stringify(SCMSTATS)
#define PAPR_SCM_PERF_STATS_VERSION 0x1

#endif 
