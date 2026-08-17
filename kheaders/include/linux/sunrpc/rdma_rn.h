/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _LINUX_SUNRPC_RDMA_RN_H
#define _LINUX_SUNRPC_RDMA_RN_H

#include <rdma/ib_verbs.h>


struct rpcrdma_notification {
	void			(*rn_done)(struct rpcrdma_notification *rn);
	u32			rn_index;
};

int rpcrdma_rn_register(struct ib_device *device,
			struct rpcrdma_notification *rn,
			void (*done)(struct rpcrdma_notification *rn));
void rpcrdma_rn_unregister(struct ib_device *device,
			   struct rpcrdma_notification *rn);
int rpcrdma_ib_client_register(void);
void rpcrdma_ib_client_unregister(void);

#endif 
