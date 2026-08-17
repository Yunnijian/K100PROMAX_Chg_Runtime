/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __LINUX_NFSLOCALIO_H
#define __LINUX_NFSLOCALIO_H


struct nfsd_file;

#if IS_ENABLED(CONFIG_NFS_LOCALIO)

#include <linux/module.h>
#include <linux/list.h>
#include <linux/uuid.h>
#include <linux/sunrpc/clnt.h>
#include <linux/sunrpc/svcauth.h>
#include <linux/nfs.h>
#include <net/net_namespace.h>


typedef struct {
	uuid_t uuid;
	struct list_head list;
	struct net __rcu *net; 
	struct auth_domain *dom; 
} nfs_uuid_t;

void nfs_uuid_init(nfs_uuid_t *);
bool nfs_uuid_begin(nfs_uuid_t *);
void nfs_uuid_end(nfs_uuid_t *);
void nfs_uuid_is_local(const uuid_t *, struct list_head *,
		       struct net *, struct auth_domain *, struct module *);
void nfs_uuid_invalidate_clients(struct list_head *list);
void nfs_uuid_invalidate_one_client(nfs_uuid_t *nfs_uuid);


extern struct nfsd_file *
nfsd_open_local_fh(struct net *, struct auth_domain *, struct rpc_clnt *,
		   const struct cred *, const struct nfs_fh *,
		   const fmode_t) __must_hold(rcu);

struct nfsd_localio_operations {
	bool (*nfsd_net_try_get)(struct net *);
	void (*nfsd_net_put)(struct net *);
	struct nfsd_file *(*nfsd_open_local_fh)(struct net *,
						struct auth_domain *,
						struct rpc_clnt *,
						const struct cred *,
						const struct nfs_fh *,
						const fmode_t);
	struct net *(*nfsd_file_put_local)(struct nfsd_file *);
	struct file *(*nfsd_file_file)(struct nfsd_file *);
} ____cacheline_aligned;

extern void nfsd_localio_ops_init(void);
extern const struct nfsd_localio_operations *nfs_to;

struct nfsd_file *nfs_open_local_fh(nfs_uuid_t *,
		   struct rpc_clnt *, const struct cred *,
		   const struct nfs_fh *, const fmode_t);

static inline void nfs_to_nfsd_net_put(struct net *net)
{
	
	rcu_read_lock();
	nfs_to->nfsd_net_put(net);
	rcu_read_unlock();
}

static inline void nfs_to_nfsd_file_put_local(struct nfsd_file *localio)
{
	
	struct net *net = nfs_to->nfsd_file_put_local(localio);

	nfs_to_nfsd_net_put(net);
}

#else   
static inline void nfsd_localio_ops_init(void)
{
}
static inline void nfs_to_nfsd_file_put_local(struct nfsd_file *localio)
{
}
#endif  

#endif  
