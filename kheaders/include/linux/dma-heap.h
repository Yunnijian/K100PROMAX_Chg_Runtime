/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _DMA_HEAPS_H
#define _DMA_HEAPS_H

#include <linux/types.h>

struct dma_heap;


struct dma_heap_ops {
	struct dma_buf *(*allocate)(struct dma_heap *heap,
				    unsigned long len,
				    u32 fd_flags,
				    u64 heap_flags);
	long (*get_pool_size)(struct dma_heap *heap);
};


struct dma_heap_export_info {
	const char *name;
	const struct dma_heap_ops *ops;
	void *priv;
};

void *dma_heap_get_drvdata(struct dma_heap *heap);


struct device *dma_heap_get_dev(struct dma_heap *heap);


const char *dma_heap_get_name(struct dma_heap *heap);

struct dma_heap *dma_heap_add(const struct dma_heap_export_info *exp_info);


void dma_heap_put(struct dma_heap *heap);


struct dma_heap *dma_heap_find(const char *name);


struct dma_buf *dma_heap_buffer_alloc(struct dma_heap *heap, size_t len,
				      u32 fd_flags,
				      u64 heap_flags);


void dma_heap_buffer_free(struct dma_buf *);


int dma_heap_bufferfd_alloc(struct dma_heap *heap, size_t len,
			    u32 fd_flags,
			    u64 heap_flags);


long dma_heap_try_get_pool_size_kb(void);
#endif 
