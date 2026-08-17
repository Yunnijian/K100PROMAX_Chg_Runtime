/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LIBETH_TX_H
#define __LIBETH_TX_H

#include <linux/skbuff.h>

#include <net/libeth/types.h>




enum libeth_sqe_type {
	LIBETH_SQE_EMPTY		= 0U,
	LIBETH_SQE_CTX,
	LIBETH_SQE_SLAB,
	LIBETH_SQE_FRAG,
	LIBETH_SQE_SKB,
};


struct libeth_sqe {
	enum libeth_sqe_type		type:32;
	u32				rs_idx;

	union {
		void				*raw;
		struct sk_buff			*skb;
	};

	DEFINE_DMA_UNMAP_ADDR(dma);
	DEFINE_DMA_UNMAP_LEN(len);

	u32				nr_frags;
	u32				packets;
	u32				bytes;

	unsigned long			priv;
} __aligned_largest;


#define LIBETH_SQE_CHECK_PRIV(p)					  \
	static_assert(sizeof(p) <= sizeof_field(struct libeth_sqe, priv))


struct libeth_cq_pp {
	struct device			*dev;
	struct libeth_sq_napi_stats	*ss;

	bool				napi;
};


static inline void libeth_tx_complete(struct libeth_sqe *sqe,
				      const struct libeth_cq_pp *cp)
{
	switch (sqe->type) {
	case LIBETH_SQE_EMPTY:
		return;
	case LIBETH_SQE_SKB:
	case LIBETH_SQE_FRAG:
	case LIBETH_SQE_SLAB:
		dma_unmap_page(cp->dev, dma_unmap_addr(sqe, dma),
			       dma_unmap_len(sqe, len), DMA_TO_DEVICE);
		break;
	default:
		break;
	}

	switch (sqe->type) {
	case LIBETH_SQE_SKB:
		cp->ss->packets += sqe->packets;
		cp->ss->bytes += sqe->bytes;

		napi_consume_skb(sqe->skb, cp->napi);
		break;
	case LIBETH_SQE_SLAB:
		kfree(sqe->raw);
		break;
	default:
		break;
	}

	sqe->type = LIBETH_SQE_EMPTY;
}

#endif 
