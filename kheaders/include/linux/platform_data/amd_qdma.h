/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef _PLATDATA_AMD_QDMA_H
#define _PLATDATA_AMD_QDMA_H

#include <linux/dmaengine.h>


struct qdma_queue_info {
	enum dma_transfer_direction dir;
};

#define QDMA_FILTER_PARAM(qinfo)	((void *)(qinfo))

struct dma_slave_map;


struct qdma_platdata {
	u32			max_mm_channels;
	u32			irq_index;
	struct dma_slave_map	*device_map;
	struct device		*dma_dev;
};

#endif 
