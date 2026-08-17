/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __QCOM_TZMEM_H
#define __QCOM_TZMEM_H

#include <linux/cleanup.h>
#include <linux/gfp.h>
#include <linux/types.h>

struct device;
struct qcom_tzmem_pool;


enum qcom_tzmem_policy {
	
	QCOM_TZMEM_POLICY_STATIC = 1,
	
	QCOM_TZMEM_POLICY_MULTIPLIER,
	
	QCOM_TZMEM_POLICY_ON_DEMAND,
};


struct qcom_tzmem_pool_config {
	size_t initial_size;
	enum qcom_tzmem_policy policy;
	size_t increment;
	size_t max_size;
};

struct qcom_tzmem_pool *
qcom_tzmem_pool_new(const struct qcom_tzmem_pool_config *config);
void qcom_tzmem_pool_free(struct qcom_tzmem_pool *pool);
struct qcom_tzmem_pool *
devm_qcom_tzmem_pool_new(struct device *dev,
			 const struct qcom_tzmem_pool_config *config);

void *qcom_tzmem_alloc(struct qcom_tzmem_pool *pool, size_t size, gfp_t gfp);
void qcom_tzmem_free(void *ptr);

DEFINE_FREE(qcom_tzmem, void *, if (_T) qcom_tzmem_free(_T))

phys_addr_t qcom_tzmem_to_phys(void *ptr);

#endif 
