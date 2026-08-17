/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _NFS_FS_SB
#define _NFS_FS_SB

#include <linux/list.h>
#include <linux/backing-dev.h>
#include <linux/idr.h>
#include <linux/wait.h>
#include <linux/nfs_xdr.h>
#include <linux/sunrpc/xprt.h>
#include <linux/nfslocalio.h>

#include <linux/atomic.h>
#include <linux/refcount.h>

struct nfs4_session;
struct nfs_iostats;
struct nlm_host;
struct nfs4_sequence_args;
struct nfs4_sequence_res;
struct nfs_server;
struct nfs4_minor_version_ops;
struct nfs41_server_scope;
struct nfs41_impl_id;


struct nfs_client {
	refcount_t		cl_count;
	atomic_t		cl_mds_count;
	int			cl_cons_state;	
#define NFS_CS_READY		0		
#define NFS_CS_INITING		1		
#define NFS_CS_SESSION_INITING	2		
	unsigned long		cl_res_state;	
#define NFS_CS_CALLBACK		1		
#define NFS_CS_IDMAP		2		
#define NFS_CS_RENEWD		3		
#define NFS_CS_STOP_RENEW	4		
#define NFS_CS_CHECK_LEASE_TIME	5		
	unsigned long		cl_flags;	
#define NFS_CS_NORESVPORT	0		
#define NFS_CS_DISCRTRY		1		
#define NFS_CS_MIGRATION	2		
#define NFS_CS_INFINITE_SLOTS	3		
#define NFS_CS_NO_RETRANS_TIMEOUT	4	
#define NFS_CS_TSM_POSSIBLE	5		
#define NFS_CS_NOPING		6		
#define NFS_CS_DS		7		
#define NFS_CS_REUSEPORT	8		
#define NFS_CS_PNFS		9		
#define NFS_CS_LOCAL_IO		10		
	struct sockaddr_storage	cl_addr;	
	size_t			cl_addrlen;
	char *			cl_hostname;	
	char *			cl_acceptor;	
	struct list_head	cl_share_link;	
	struct list_head	cl_superblocks;	

	struct rpc_clnt *	cl_rpcclient;
	const struct nfs_rpc_ops *rpc_ops;	
	int			cl_proto;	
	struct nfs_subversion *	cl_nfs_mod;	

	u32			cl_minorversion;
	unsigned int		cl_nconnect;	
	unsigned int		cl_max_connect; 
	const char *		cl_principal;	
	struct xprtsec_parms	cl_xprtsec;	

#if IS_ENABLED(CONFIG_NFS_V4)
	struct list_head	cl_ds_clients; 
	u64			cl_clientid;	
	nfs4_verifier		cl_confirm;	
	unsigned long		cl_state;

	spinlock_t		cl_lock;

	unsigned long		cl_lease_time;
	unsigned long		cl_last_renewal;
	struct delayed_work	cl_renewd;

	struct rpc_wait_queue	cl_rpcwaitq;

	
	struct idmap *		cl_idmap;

	
	const char *		cl_owner_id;

	u32			cl_cb_ident;	
	const struct nfs4_minor_version_ops *cl_mvops;
	unsigned long		cl_mig_gen;

	
	struct nfs4_slot_table	*cl_slot_tbl;

	
	u32			cl_seqid;
	
	u32			cl_exchange_flags;
	struct nfs4_session	*cl_session;	
	bool			cl_preserve_clid;
	struct nfs41_server_owner *cl_serverowner;
	struct nfs41_server_scope *cl_serverscope;
	struct nfs41_impl_id	*cl_implid;
	
	unsigned long		cl_sp4_flags;
#define NFS_SP4_MACH_CRED_MINIMAL  1	
#define NFS_SP4_MACH_CRED_CLEANUP  2	
#define NFS_SP4_MACH_CRED_SECINFO  3	
#define NFS_SP4_MACH_CRED_STATEID  4	
#define NFS_SP4_MACH_CRED_WRITE    5	
#define NFS_SP4_MACH_CRED_COMMIT   6	
#define NFS_SP4_MACH_CRED_PNFS_CLEANUP  7 
#if IS_ENABLED(CONFIG_NFS_V4_1)
	wait_queue_head_t	cl_lock_waitq;
#endif 
#endif 

	
	char			cl_ipaddr[48];
	struct net		*cl_net;
	struct list_head	pending_cb_stateids;
	struct rcu_head		rcu;

#if IS_ENABLED(CONFIG_NFS_LOCALIO)
	struct timespec64	cl_nfssvc_boot;
	seqlock_t		cl_boot_lock;
	nfs_uuid_t		cl_uuid;
	spinlock_t		cl_localio_lock;
#endif 
};


struct nfs_server {
	struct nfs_client *	nfs_client;	
	struct list_head	client_link;	
	struct list_head	master_link;	
	struct rpc_clnt *	client;		
	struct rpc_clnt *	client_acl;	
	struct nlm_host		*nlm_host;	
	struct nfs_iostats __percpu *io_stats;	
	wait_queue_head_t	write_congestion_wait;	
	atomic_long_t		writeback;	
	unsigned int		write_congested;
	unsigned int		flags;		


#define NFS_MOUNT_LOOKUP_CACHE_NONEG	0x10000
#define NFS_MOUNT_LOOKUP_CACHE_NONE	0x20000
#define NFS_MOUNT_NORESVPORT		0x40000
#define NFS_MOUNT_LEGACY_INTERFACE	0x80000
#define NFS_MOUNT_LOCAL_FLOCK		0x100000
#define NFS_MOUNT_LOCAL_FCNTL		0x200000
#define NFS_MOUNT_SOFTERR		0x400000
#define NFS_MOUNT_SOFTREVAL		0x800000
#define NFS_MOUNT_WRITE_EAGER		0x01000000
#define NFS_MOUNT_WRITE_WAIT		0x02000000
#define NFS_MOUNT_TRUNK_DISCOVERY	0x04000000
#define NFS_MOUNT_SHUTDOWN			0x08000000
#define NFS_MOUNT_NO_ALIGNWRITE		0x10000000

	unsigned int		automount_inherit; 
#define NFS_AUTOMOUNT_INHERIT_BSIZE	0x0001
#define NFS_AUTOMOUNT_INHERIT_RSIZE	0x0002
#define NFS_AUTOMOUNT_INHERIT_WSIZE	0x0004

	unsigned int		caps;		
	__u64			fattr_valid;	
	unsigned int		rsize;		
	unsigned int		rpages;		
	unsigned int		wsize;		
	unsigned int		wpages;		
	unsigned int		wtmult;		
	unsigned int		dtsize;		
	unsigned short		port;		
	unsigned int		bsize;		
#ifdef CONFIG_NFS_V4_2
	unsigned int		gxasize;	
	unsigned int		sxasize;	
	unsigned int		lxasize;	
#endif
	unsigned int		acregmin;	
	unsigned int		acregmax;
	unsigned int		acdirmin;
	unsigned int		acdirmax;
	unsigned int		namelen;
	unsigned int		options;	
	unsigned int		clone_blksize;	
#define NFS_OPTION_FSCACHE	0x00000001	
#define NFS_OPTION_MIGRATION	0x00000002	

	enum nfs4_change_attr_type
				change_attr_type;

	struct nfs_fsid		fsid;
	int			s_sysfs_id;	
	__u64			maxfilesize;	
	struct timespec64	time_delta;	
	unsigned long		mount_time;	
	struct super_block	*super;		
	dev_t			s_dev;		
	struct nfs_auth_info	auth_info;	

#ifdef CONFIG_NFS_FSCACHE
	struct fscache_volume	*fscache;	
	char			*fscache_uniq;	
#endif

	
#define NFS_FH_NOEXPIRE_WITH_OPEN (0x1)
#define NFS_FH_VOLATILE_ANY (0x2)
#define NFS_FH_VOL_MIGRATION (0x4)
#define NFS_FH_VOL_RENAME (0x8)
#define NFS_FH_RENAME_UNSAFE (NFS_FH_VOLATILE_ANY | NFS_FH_VOL_RENAME)
	u32			fh_expire_type;	
	u32			pnfs_blksize;	
#if IS_ENABLED(CONFIG_NFS_V4)
	u32			attr_bitmask[3];
	u32			attr_bitmask_nl[3];
						
	u32			exclcreat_bitmask[3];
						
	u32			cache_consistency_bitmask[3];
						
	u32			acl_bitmask;	
	struct pnfs_layoutdriver_type  *pnfs_curr_ld; 
	struct rpc_wait_queue	roc_rpcwaitq;
	void			*pnfs_ld_data;	

	
	struct rb_root		state_owners;
#endif
	atomic64_t		owner_ctr;
	struct list_head	state_owners_lru;
	struct list_head	layouts;
	struct list_head	delegations;
	struct list_head	ss_copies;
	struct list_head	ss_src_copies;

	unsigned long		delegation_flags;
#define NFS4SERV_DELEGRETURN		(1)
#define NFS4SERV_DELEGATION_EXPIRED	(2)
#define NFS4SERV_DELEGRETURN_DELAYED	(3)
	unsigned long		delegation_gen;
	unsigned long		mig_gen;
	unsigned long		mig_status;
#define NFS_MIG_IN_TRANSITION		(1)
#define NFS_MIG_FAILED			(2)
#define NFS_MIG_TSM_POSSIBLE		(3)

	void (*destroy)(struct nfs_server *);

	atomic_t active; 

	
	struct sockaddr_storage	mountd_address;
	size_t			mountd_addrlen;
	u32			mountd_version;
	unsigned short		mountd_port;
	unsigned short		mountd_protocol;
	struct rpc_wait_queue	uoc_rpcwaitq;

	
	unsigned int		read_hdrsize;

	
	const struct cred	*cred;
	bool			has_sec_mnt_opts;
	struct kobject		kobj;
	struct rcu_head		rcu;
};


#define NFS_CAP_READDIRPLUS	(1U << 0)
#define NFS_CAP_HARDLINKS	(1U << 1)
#define NFS_CAP_SYMLINKS	(1U << 2)
#define NFS_CAP_ACLS		(1U << 3)
#define NFS_CAP_ATOMIC_OPEN	(1U << 4)
#define NFS_CAP_LGOPEN		(1U << 5)
#define NFS_CAP_CASE_INSENSITIVE	(1U << 6)
#define NFS_CAP_CASE_PRESERVING	(1U << 7)
#define NFS_CAP_REBOOT_LAYOUTRETURN	(1U << 8)
#define NFS_CAP_OPEN_XOR	(1U << 12)
#define NFS_CAP_DELEGTIME	(1U << 13)
#define NFS_CAP_POSIX_LOCK	(1U << 14)
#define NFS_CAP_UIDGID_NOMAP	(1U << 15)
#define NFS_CAP_STATEID_NFSV41	(1U << 16)
#define NFS_CAP_ATOMIC_OPEN_V1	(1U << 17)
#define NFS_CAP_SECURITY_LABEL	(1U << 18)
#define NFS_CAP_SEEK		(1U << 19)
#define NFS_CAP_ALLOCATE	(1U << 20)
#define NFS_CAP_DEALLOCATE	(1U << 21)
#define NFS_CAP_LAYOUTSTATS	(1U << 22)
#define NFS_CAP_CLONE		(1U << 23)
#define NFS_CAP_COPY		(1U << 24)
#define NFS_CAP_OFFLOAD_CANCEL	(1U << 25)
#define NFS_CAP_LAYOUTERROR	(1U << 26)
#define NFS_CAP_COPY_NOTIFY	(1U << 27)
#define NFS_CAP_XATTR		(1U << 28)
#define NFS_CAP_READ_PLUS	(1U << 29)
#define NFS_CAP_FS_LOCATIONS	(1U << 30)
#define NFS_CAP_MOVEABLE	(1U << 31)
#endif
