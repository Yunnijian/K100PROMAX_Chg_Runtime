/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_IF_HSR_H_
#define _LINUX_IF_HSR_H_

#include <linux/types.h>

struct net_device;


enum hsr_version {
	HSR_V0 = 0,
	HSR_V1,
	PRP_V1,
};

enum hsr_port_type {
	HSR_PT_NONE = 0,	
	HSR_PT_SLAVE_A,
	HSR_PT_SLAVE_B,
	HSR_PT_INTERLINK,
	HSR_PT_MASTER,
	HSR_PT_PORTS,	
};


struct hsr_tag {
	__be16		path_and_LSDU_size;
	__be16		sequence_nr;
	__be16		encap_proto;
} __packed;

#define HSR_HLEN	6

#if IS_ENABLED(CONFIG_HSR)
extern bool is_hsr_master(struct net_device *dev);
extern int hsr_get_version(struct net_device *dev, enum hsr_version *ver);
struct net_device *hsr_get_port_ndev(struct net_device *ndev,
				     enum hsr_port_type pt);
int hsr_get_port_type(struct net_device *hsr_dev, struct net_device *dev,
		      enum hsr_port_type *type);
#else
static inline bool is_hsr_master(struct net_device *dev)
{
	return false;
}
static inline int hsr_get_version(struct net_device *dev,
				  enum hsr_version *ver)
{
	return -EINVAL;
}

static inline struct net_device *hsr_get_port_ndev(struct net_device *ndev,
						   enum hsr_port_type pt)
{
	return ERR_PTR(-EINVAL);
}

static inline int hsr_get_port_type(struct net_device *hsr_dev,
				    struct net_device *dev,
				    enum hsr_port_type *type)
{
	return -EINVAL;
}
#endif 

#endif 
