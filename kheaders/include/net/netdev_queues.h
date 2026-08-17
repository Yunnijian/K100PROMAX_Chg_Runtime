/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_NET_QUEUES_H
#define _LINUX_NET_QUEUES_H

#include <linux/netdevice.h>


struct netdev_queue_stats_rx {
	u64 bytes;
	u64 packets;
	u64 alloc_fail;

	u64 hw_drops;
	u64 hw_drop_overruns;

	u64 csum_unnecessary;
	u64 csum_none;
	u64 csum_bad;

	u64 hw_gro_packets;
	u64 hw_gro_bytes;
	u64 hw_gro_wire_packets;
	u64 hw_gro_wire_bytes;

	u64 hw_drop_ratelimits;
};

struct netdev_queue_stats_tx {
	u64 bytes;
	u64 packets;

	u64 hw_drops;
	u64 hw_drop_errors;

	u64 csum_none;
	u64 needs_csum;

	u64 hw_gso_packets;
	u64 hw_gso_bytes;
	u64 hw_gso_wire_packets;
	u64 hw_gso_wire_bytes;

	u64 hw_drop_ratelimits;

	u64 stop;
	u64 wake;
};


struct netdev_stat_ops {
	void (*get_queue_stats_rx)(struct net_device *dev, int idx,
				   struct netdev_queue_stats_rx *stats);
	void (*get_queue_stats_tx)(struct net_device *dev, int idx,
				   struct netdev_queue_stats_tx *stats);
	void (*get_base_stats)(struct net_device *dev,
			       struct netdev_queue_stats_rx *rx,
			       struct netdev_queue_stats_tx *tx);
};

void netdev_stat_queue_sum(struct net_device *netdev,
			   int rx_start, int rx_end,
			   struct netdev_queue_stats_rx *rx_sum,
			   int tx_start, int tx_end,
			   struct netdev_queue_stats_tx *tx_sum);


struct netdev_queue_mgmt_ops {
	size_t			ndo_queue_mem_size;
	int			(*ndo_queue_mem_alloc)(struct net_device *dev,
						       void *per_queue_mem,
						       int idx);
	void			(*ndo_queue_mem_free)(struct net_device *dev,
						      void *per_queue_mem);
	int			(*ndo_queue_start)(struct net_device *dev,
						   void *per_queue_mem,
						   int idx);
	int			(*ndo_queue_stop)(struct net_device *dev,
						  void *per_queue_mem,
						  int idx);
};



#define netif_txq_try_stop(txq, get_desc, start_thrs)			\
	({								\
		int _res;						\
									\
		netif_tx_stop_queue(txq);				\
									\
		smp_mb__after_atomic();					\
									\
									\
		_res = 0;						\
		if (unlikely(get_desc >= start_thrs)) {			\
			netif_tx_start_queue(txq);			\
			_res = -1;					\
		}							\
		_res;							\
	})								\


#define netif_txq_maybe_stop(txq, get_desc, stop_thrs, start_thrs)	\
	({								\
		int _res;						\
									\
		_res = 1;						\
		if (unlikely(get_desc < stop_thrs))			\
			_res = netif_txq_try_stop(txq, get_desc, start_thrs); \
		_res;							\
	})								\


static inline void
netdev_txq_completed_mb(struct netdev_queue *dev_queue,
			unsigned int pkts, unsigned int bytes)
{
	if (IS_ENABLED(CONFIG_BQL))
		netdev_tx_completed_queue(dev_queue, pkts, bytes);
	else if (bytes)
		smp_mb();
}


#define __netif_txq_completed_wake(txq, pkts, bytes,			\
				   get_desc, start_thrs, down_cond)	\
	({								\
		int _res;						\
									\
									\
		netdev_txq_completed_mb(txq, pkts, bytes);		\
									\
		_res = -1;						\
		if (pkts && likely(get_desc >= start_thrs)) {		\
			_res = 1;					\
			if (unlikely(netif_tx_queue_stopped(txq)) &&	\
			    !(down_cond)) {				\
				netif_tx_wake_queue(txq);		\
				_res = 0;				\
			}						\
		}							\
		_res;							\
	})

#define netif_txq_completed_wake(txq, pkts, bytes, get_desc, start_thrs) \
	__netif_txq_completed_wake(txq, pkts, bytes, get_desc, start_thrs, false)



#define netif_subqueue_try_stop(dev, idx, get_desc, start_thrs)		\
	({								\
		struct netdev_queue *txq;				\
									\
		txq = netdev_get_tx_queue(dev, idx);			\
		netif_txq_try_stop(txq, get_desc, start_thrs);		\
	})

#define netif_subqueue_maybe_stop(dev, idx, get_desc, stop_thrs, start_thrs) \
	({								\
		struct netdev_queue *txq;				\
									\
		txq = netdev_get_tx_queue(dev, idx);			\
		netif_txq_maybe_stop(txq, get_desc, stop_thrs, start_thrs); \
	})

#define netif_subqueue_completed_wake(dev, idx, pkts, bytes,		\
				      get_desc, start_thrs)		\
	({								\
		struct netdev_queue *txq;				\
									\
		txq = netdev_get_tx_queue(dev, idx);			\
		netif_txq_completed_wake(txq, pkts, bytes,		\
					 get_desc, start_thrs);		\
	})

#endif
