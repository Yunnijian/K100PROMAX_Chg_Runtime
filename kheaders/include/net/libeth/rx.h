/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LIBETH_RX_H
#define __LIBETH_RX_H

#include <linux/if_vlan.h>

#include <net/page_pool/helpers.h>
#include <net/xdp.h>




#define LIBETH_SKB_HEADROOM	(NET_SKB_PAD + NET_IP_ALIGN)

#define LIBETH_MAX_HEADROOM	LIBETH_SKB_HEADROOM

#define LIBETH_RX_LL_LEN	(ETH_HLEN + 2 * VLAN_HLEN + ETH_FCS_LEN)

#define LIBETH_MAX_HEAD		roundup_pow_of_two(max(MAX_HEADER, 256))


#define LIBETH_RX_PAGE_ORDER	0

#define LIBETH_RX_BUF_STRIDE	SKB_DATA_ALIGN(128)

#define LIBETH_RX_PAGE_LEN(hr)						  \
	ALIGN_DOWN(SKB_MAX_ORDER(hr, LIBETH_RX_PAGE_ORDER),		  \
		   LIBETH_RX_BUF_STRIDE)


struct libeth_fqe {
	struct page		*page;
	u32			offset;
	u32			truesize;
} __aligned_largest;


enum libeth_fqe_type {
	LIBETH_FQE_MTU		= 0U,
	LIBETH_FQE_SHORT,
	LIBETH_FQE_HDR,
};


struct libeth_fq {
	struct_group_tagged(libeth_fq_fp, fp,
		struct page_pool	*pp;
		struct libeth_fqe	*fqes;

		u32			truesize;
		u32			count;
	);

	
	enum libeth_fqe_type	type:2;
	bool			hsplit:1;

	u32			buf_len;
	int			nid;
};

int libeth_rx_fq_create(struct libeth_fq *fq, struct napi_struct *napi);
void libeth_rx_fq_destroy(struct libeth_fq *fq);


static inline dma_addr_t libeth_rx_alloc(const struct libeth_fq_fp *fq, u32 i)
{
	struct libeth_fqe *buf = &fq->fqes[i];

	buf->truesize = fq->truesize;
	buf->page = page_pool_dev_alloc(fq->pp, &buf->offset, &buf->truesize);
	if (unlikely(!buf->page))
		return DMA_MAPPING_ERROR;

	return page_pool_get_dma_addr(buf->page) + buf->offset +
	       fq->pp->p.offset;
}

void libeth_rx_recycle_slow(struct page *page);


static inline bool libeth_rx_sync_for_cpu(const struct libeth_fqe *fqe,
					  u32 len)
{
	struct page *page = fqe->page;

	
	if (unlikely(!len)) {
		libeth_rx_recycle_slow(page);
		return false;
	}

	page_pool_dma_sync_for_cpu(page->pp, page, fqe->offset, len);

	return true;
}



enum {
	LIBETH_RX_PT_OUTER_L2			= 0U,
	LIBETH_RX_PT_OUTER_IPV4,
	LIBETH_RX_PT_OUTER_IPV6,
};

enum {
	LIBETH_RX_PT_NOT_FRAG			= 0U,
	LIBETH_RX_PT_FRAG,
};

enum {
	LIBETH_RX_PT_TUNNEL_IP_NONE		= 0U,
	LIBETH_RX_PT_TUNNEL_IP_IP,
	LIBETH_RX_PT_TUNNEL_IP_GRENAT,
	LIBETH_RX_PT_TUNNEL_IP_GRENAT_MAC,
	LIBETH_RX_PT_TUNNEL_IP_GRENAT_MAC_VLAN,
};

enum {
	LIBETH_RX_PT_TUNNEL_END_NONE		= 0U,
	LIBETH_RX_PT_TUNNEL_END_IPV4,
	LIBETH_RX_PT_TUNNEL_END_IPV6,
};

enum {
	LIBETH_RX_PT_INNER_NONE			= 0U,
	LIBETH_RX_PT_INNER_UDP,
	LIBETH_RX_PT_INNER_TCP,
	LIBETH_RX_PT_INNER_SCTP,
	LIBETH_RX_PT_INNER_ICMP,
	LIBETH_RX_PT_INNER_TIMESYNC,
};

#define LIBETH_RX_PT_PAYLOAD_NONE		PKT_HASH_TYPE_NONE
#define LIBETH_RX_PT_PAYLOAD_L2			PKT_HASH_TYPE_L2
#define LIBETH_RX_PT_PAYLOAD_L3			PKT_HASH_TYPE_L3
#define LIBETH_RX_PT_PAYLOAD_L4			PKT_HASH_TYPE_L4

struct libeth_rx_pt {
	u32					outer_ip:2;
	u32					outer_frag:1;
	u32					tunnel_type:3;
	u32					tunnel_end_prot:2;
	u32					tunnel_end_frag:1;
	u32					inner_prot:3;
	enum pkt_hash_types			payload_layer:2;

	u32					pad:2;
	enum xdp_rss_hash_type			hash_type:16;
};

void libeth_rx_pt_gen_hash_type(struct libeth_rx_pt *pt);


static inline u32 libeth_rx_pt_get_ip_ver(struct libeth_rx_pt pt)
{
#if !IS_ENABLED(CONFIG_IPV6)
	switch (pt.outer_ip) {
	case LIBETH_RX_PT_OUTER_IPV4:
		return LIBETH_RX_PT_OUTER_IPV4;
	default:
		return LIBETH_RX_PT_OUTER_L2;
	}
#else
	return pt.outer_ip;
#endif
}



static inline bool libeth_rx_pt_has_checksum(const struct net_device *dev,
					     struct libeth_rx_pt pt)
{
	
	return likely(pt.inner_prot > LIBETH_RX_PT_INNER_NONE &&
		      (dev->features & NETIF_F_RXCSUM));
}

static inline bool libeth_rx_pt_has_hash(const struct net_device *dev,
					 struct libeth_rx_pt pt)
{
	return likely(pt.payload_layer > LIBETH_RX_PT_PAYLOAD_NONE &&
		      (dev->features & NETIF_F_RXHASH));
}


static inline void libeth_rx_pt_set_hash(struct sk_buff *skb, u32 hash,
					 struct libeth_rx_pt pt)
{
	skb_set_hash(skb, hash, pt.payload_layer);
}

#endif 
