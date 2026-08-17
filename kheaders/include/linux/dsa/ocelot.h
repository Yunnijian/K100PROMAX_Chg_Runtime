/* SPDX-License-Identifier: GPL-2.0
 * Copyright 2019-2021 NXP
 */

#ifndef _NET_DSA_TAG_OCELOT_H
#define _NET_DSA_TAG_OCELOT_H

#include <linux/if_bridge.h>
#include <linux/if_vlan.h>
#include <linux/kthread.h>
#include <linux/packing.h>
#include <linux/skbuff.h>
#include <net/dsa.h>

struct ocelot_skb_cb {
	struct sk_buff *clone;
	unsigned int ptp_class; 
	unsigned long ptp_tx_time; 
	u32 tstamp_lo;
	u8 ptp_cmd;
	u8 ts_id;
};

#define OCELOT_SKB_CB(skb) \
	((struct ocelot_skb_cb *)((skb)->cb))

#define IFH_TAG_TYPE_C			0
#define IFH_TAG_TYPE_S			1

#define IFH_REW_OP_NOOP			0x0
#define IFH_REW_OP_DSCP			0x1
#define IFH_REW_OP_ONE_STEP_PTP		0x2
#define IFH_REW_OP_TWO_STEP_PTP		0x3
#define IFH_REW_OP_ORIGIN_PTP		0x5

#define OCELOT_TAG_LEN			16
#define OCELOT_SHORT_PREFIX_LEN		4
#define OCELOT_LONG_PREFIX_LEN		16
#define OCELOT_TOTAL_TAG_LEN	(OCELOT_SHORT_PREFIX_LEN + OCELOT_TAG_LEN)



struct felix_deferred_xmit_work {
	struct dsa_port *dp;
	struct sk_buff *skb;
	struct kthread_work work;
};

struct ocelot_8021q_tagger_data {
	void (*xmit_work_fn)(struct kthread_work *work);
};

static inline struct ocelot_8021q_tagger_data *
ocelot_8021q_tagger_data(struct dsa_switch *ds)
{
	BUG_ON(ds->dst->tag_ops->proto != DSA_TAG_PROTO_OCELOT_8021Q);

	return ds->tagger_data;
}

static inline void ocelot_xfh_get_rew_val(void *extraction, u64 *rew_val)
{
	packing(extraction, rew_val, 116, 85, OCELOT_TAG_LEN, UNPACK, 0);
}

static inline void ocelot_xfh_get_len(void *extraction, u64 *len)
{
	u64 llen, wlen;

	packing(extraction, &llen, 84, 79, OCELOT_TAG_LEN, UNPACK, 0);
	packing(extraction, &wlen, 78, 71, OCELOT_TAG_LEN, UNPACK, 0);

	*len = 60 * wlen + llen - 80;
}

static inline void ocelot_xfh_get_src_port(void *extraction, u64 *src_port)
{
	packing(extraction, src_port, 46, 43, OCELOT_TAG_LEN, UNPACK, 0);
}

static inline void ocelot_xfh_get_qos_class(void *extraction, u64 *qos_class)
{
	packing(extraction, qos_class, 19, 17, OCELOT_TAG_LEN, UNPACK, 0);
}

static inline void ocelot_xfh_get_tag_type(void *extraction, u64 *tag_type)
{
	packing(extraction, tag_type, 16, 16, OCELOT_TAG_LEN, UNPACK, 0);
}

static inline void ocelot_xfh_get_vlan_tci(void *extraction, u64 *vlan_tci)
{
	packing(extraction, vlan_tci, 15, 0, OCELOT_TAG_LEN, UNPACK, 0);
}

static inline void ocelot_ifh_set_bypass(void *injection, u64 bypass)
{
	packing(injection, &bypass, 127, 127, OCELOT_TAG_LEN, PACK, 0);
}

static inline void ocelot_ifh_set_rew_op(void *injection, u64 rew_op)
{
	packing(injection, &rew_op, 125, 117, OCELOT_TAG_LEN, PACK, 0);
}

static inline void ocelot_ifh_set_dest(void *injection, u64 dest)
{
	packing(injection, &dest, 67, 56, OCELOT_TAG_LEN, PACK, 0);
}

static inline void ocelot_ifh_set_qos_class(void *injection, u64 qos_class)
{
	packing(injection, &qos_class, 19, 17, OCELOT_TAG_LEN, PACK, 0);
}

static inline void seville_ifh_set_dest(void *injection, u64 dest)
{
	packing(injection, &dest, 67, 57, OCELOT_TAG_LEN, PACK, 0);
}

static inline void ocelot_ifh_set_src(void *injection, u64 src)
{
	packing(injection, &src, 46, 43, OCELOT_TAG_LEN, PACK, 0);
}

static inline void ocelot_ifh_set_tag_type(void *injection, u64 tag_type)
{
	packing(injection, &tag_type, 16, 16, OCELOT_TAG_LEN, PACK, 0);
}

static inline void ocelot_ifh_set_vlan_tci(void *injection, u64 vlan_tci)
{
	packing(injection, &vlan_tci, 15, 0, OCELOT_TAG_LEN, PACK, 0);
}


static inline u32 ocelot_ptp_rew_op(struct sk_buff *skb)
{
	struct sk_buff *clone = OCELOT_SKB_CB(skb)->clone;
	u8 ptp_cmd = OCELOT_SKB_CB(skb)->ptp_cmd;
	u32 rew_op = 0;

	if (ptp_cmd == IFH_REW_OP_TWO_STEP_PTP && clone) {
		rew_op = ptp_cmd;
		rew_op |= OCELOT_SKB_CB(clone)->ts_id << 3;
	} else if (ptp_cmd == IFH_REW_OP_ORIGIN_PTP) {
		rew_op = ptp_cmd;
	}

	return rew_op;
}


static inline void ocelot_xmit_get_vlan_info(struct sk_buff *skb,
					     struct net_device *br,
					     u64 *vlan_tci, u64 *tag_type)
{
	struct vlan_ethhdr *hdr;
	u16 proto, tci;

	if (!br || !br_vlan_enabled(br)) {
		*vlan_tci = 0;
		*tag_type = IFH_TAG_TYPE_C;
		return;
	}

	hdr = (struct vlan_ethhdr *)skb_mac_header(skb);
	br_vlan_get_proto(br, &proto);

	if (ntohs(hdr->h_vlan_proto) == proto) {
		vlan_remove_tag(skb, &tci);
		*vlan_tci = tci;
	} else {
		rcu_read_lock();
		br_vlan_get_pvid_rcu(br, &tci);
		rcu_read_unlock();
		*vlan_tci = tci;
	}

	*tag_type = (proto != ETH_P_8021Q) ? IFH_TAG_TYPE_S : IFH_TAG_TYPE_C;
}

#endif
