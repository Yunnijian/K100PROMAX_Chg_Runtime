/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef _LINUX_NETFS_H
#define _LINUX_NETFS_H

#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/uio.h>

enum netfs_sreq_ref_trace;
typedef struct mempool_s mempool_t;


static inline void folio_start_private_2(struct folio *folio)
{
	VM_BUG_ON_FOLIO(folio_test_private_2(folio), folio);
	folio_get(folio);
	folio_set_private_2(folio);
}

enum netfs_io_source {
	NETFS_SOURCE_UNKNOWN,
	NETFS_FILL_WITH_ZEROES,
	NETFS_DOWNLOAD_FROM_SERVER,
	NETFS_READ_FROM_CACHE,
	NETFS_INVALID_READ,
	NETFS_UPLOAD_TO_SERVER,
	NETFS_WRITE_TO_CACHE,
	NETFS_INVALID_WRITE,
} __mode(byte);

typedef void (*netfs_io_terminated_t)(void *priv, ssize_t transferred_or_error,
				      bool was_async);


struct netfs_inode {
	struct inode		inode;		
	const struct netfs_request_ops *ops;
#if IS_ENABLED(CONFIG_FSCACHE)
	struct fscache_cookie	*cache;
#endif
	struct mutex		wb_lock;	
	loff_t			remote_i_size;	
	loff_t			zero_point;	
	atomic_t		io_count;	
	unsigned long		flags;
#define NETFS_ICTX_ODIRECT	0		
#define NETFS_ICTX_UNBUFFERED	1		
#define NETFS_ICTX_WRITETHROUGH	2		
#define NETFS_ICTX_MODIFIED_ATTR 3		
};


struct netfs_group {
	refcount_t		ref;
	void (*free)(struct netfs_group *netfs_group);
};


struct netfs_folio {
	struct netfs_group	*netfs_group;	
	unsigned int		dirty_offset;	
	unsigned int		dirty_len;	
};
#define NETFS_FOLIO_INFO	0x1UL	
#define NETFS_FOLIO_COPY_TO_CACHE ((struct netfs_group *)0x356UL) 

static inline bool netfs_is_folio_info(const void *priv)
{
	return (unsigned long)priv & NETFS_FOLIO_INFO;
}

static inline struct netfs_folio *__netfs_folio_info(const void *priv)
{
	if (netfs_is_folio_info(priv))
		return (struct netfs_folio *)((unsigned long)priv & ~NETFS_FOLIO_INFO);
	return NULL;
}

static inline struct netfs_folio *netfs_folio_info(struct folio *folio)
{
	return __netfs_folio_info(folio_get_private(folio));
}

static inline struct netfs_group *netfs_folio_group(struct folio *folio)
{
	struct netfs_folio *finfo;
	void *priv = folio_get_private(folio);

	finfo = netfs_folio_info(folio);
	if (finfo)
		return finfo->netfs_group;
	return priv;
}


struct netfs_io_stream {
	
	struct netfs_io_subrequest *construct;	
	size_t			sreq_max_len;	
	unsigned int		sreq_max_segs;	
	unsigned int		submit_off;	
	unsigned int		submit_len;	
	unsigned int		submit_extendable_to; 
	void (*prepare_write)(struct netfs_io_subrequest *subreq);
	void (*issue_write)(struct netfs_io_subrequest *subreq);
	
	struct list_head	subrequests;	
	struct netfs_io_subrequest *front;	
	unsigned long long	collected_to;	
	size_t			transferred;	
	enum netfs_io_source	source;		
	unsigned short		error;		
	unsigned char		stream_nr;	
	bool			avail;		
	bool			active;		
	bool			need_retry;	
	bool			failed;		
};


struct netfs_cache_resources {
	const struct netfs_cache_ops	*ops;
	void				*cache_priv;
	void				*cache_priv2;
	unsigned int			debug_id;	
	unsigned int			inval_counter;	
};


struct netfs_io_subrequest {
	struct netfs_io_request *rreq;		
	struct work_struct	work;
	struct list_head	rreq_link;	
	struct iov_iter		io_iter;	
	unsigned long long	start;		
	size_t			len;		
	size_t			transferred;	
	size_t			consumed;	
	size_t			prev_donated;	
	size_t			next_donated;	
	refcount_t		ref;
	short			error;		
	unsigned short		debug_index;	
	unsigned int		nr_segs;	
	enum netfs_io_source	source;		
	unsigned char		stream_nr;	
	unsigned char		curr_folioq_slot; 
	unsigned char		curr_folio_order; 
	struct folio_queue	*curr_folioq;	
	unsigned long		flags;
#define NETFS_SREQ_COPY_TO_CACHE	0	
#define NETFS_SREQ_CLEAR_TAIL		1	
#define NETFS_SREQ_SEEK_DATA_READ	3	
#define NETFS_SREQ_NO_PROGRESS		4	
#define NETFS_SREQ_ONDEMAND		5	
#define NETFS_SREQ_BOUNDARY		6	
#define NETFS_SREQ_HIT_EOF		7	
#define NETFS_SREQ_IN_PROGRESS		8	
#define NETFS_SREQ_NEED_RETRY		9	
#define NETFS_SREQ_RETRYING		10	
#define NETFS_SREQ_FAILED		11	
};

enum netfs_io_origin {
	NETFS_READAHEAD,		
	NETFS_READPAGE,			
	NETFS_READ_GAPS,		
	NETFS_READ_FOR_WRITE,		
	NETFS_DIO_READ,			
	NETFS_WRITEBACK,		
	NETFS_WRITETHROUGH,		
	NETFS_UNBUFFERED_WRITE,		
	NETFS_DIO_WRITE,		
	NETFS_PGPRIV2_COPY_TO_CACHE,	
	nr__netfs_io_origin
} __mode(byte);


struct netfs_io_request {
	union {
		struct work_struct work;
		struct rcu_head rcu;
	};
	struct inode		*inode;		
	struct address_space	*mapping;	
	struct kiocb		*iocb;		
	struct netfs_cache_resources cache_resources;
	struct readahead_control *ractl;	
	struct list_head	proc_link;	
	struct list_head	subrequests;	
	struct netfs_io_stream	io_streams[2];	
#define NR_IO_STREAMS 2 //wreq->nr_io_streams
	struct netfs_group	*group;		
	struct folio_queue	*buffer;	
	struct folio_queue	*buffer_tail;	
	struct iov_iter		iter;		
	struct iov_iter		io_iter;	
	void			*netfs_priv;	
	void			*netfs_priv2;	
	struct bio_vec		*direct_bv;	
	unsigned int		direct_bv_count; 
	unsigned int		debug_id;
	unsigned int		rsize;		
	unsigned int		wsize;		
	atomic_t		subreq_counter;	
	unsigned int		nr_group_rel;	
	spinlock_t		lock;		
	atomic_t		nr_outstanding;	
	unsigned long long	submitted;	
	unsigned long long	len;		
	size_t			transferred;	
	long			error;		
	enum netfs_io_origin	origin;		
	bool			direct_bv_unpin; 
	u8			buffer_head_slot; 
	u8			buffer_tail_slot; 
	unsigned long long	i_size;		
	unsigned long long	start;		
	atomic64_t		issued_to;	
	unsigned long long	collected_to;	
	unsigned long long	cleaned_to;	
	pgoff_t			no_unlock_folio; 
	size_t			prev_donated;	
	refcount_t		ref;
	unsigned long		flags;
#define NETFS_RREQ_NO_UNLOCK_FOLIO	2	
#define NETFS_RREQ_DONT_UNLOCK_FOLIOS	3	
#define NETFS_RREQ_FAILED		4	
#define NETFS_RREQ_IN_PROGRESS		5	
#define NETFS_RREQ_UPLOAD_TO_SERVER	8	
#define NETFS_RREQ_NONBLOCK		9	
#define NETFS_RREQ_BLOCKED		10	
#define NETFS_RREQ_PAUSE		11	
#define NETFS_RREQ_USE_IO_ITER		12	
#define NETFS_RREQ_ALL_QUEUED		13	
#define NETFS_RREQ_NEED_RETRY		14	
#define NETFS_RREQ_USE_PGPRIV2		31	
	const struct netfs_request_ops *netfs_ops;
	void (*cleanup)(struct netfs_io_request *req);
};


struct netfs_request_ops {
	mempool_t *request_pool;
	mempool_t *subrequest_pool;
	int (*init_request)(struct netfs_io_request *rreq, struct file *file);
	void (*free_request)(struct netfs_io_request *rreq);
	void (*free_subrequest)(struct netfs_io_subrequest *rreq);

	
	void (*expand_readahead)(struct netfs_io_request *rreq);
	int (*prepare_read)(struct netfs_io_subrequest *subreq);
	void (*issue_read)(struct netfs_io_subrequest *subreq);
	bool (*is_still_valid)(struct netfs_io_request *rreq);
	int (*check_write_begin)(struct file *file, loff_t pos, unsigned len,
				 struct folio **foliop, void **_fsdata);
	void (*done)(struct netfs_io_request *rreq);

	
	void (*update_i_size)(struct inode *inode, loff_t i_size);
	void (*post_modify)(struct inode *inode);

	
	void (*begin_writeback)(struct netfs_io_request *wreq);
	void (*prepare_write)(struct netfs_io_subrequest *subreq);
	void (*issue_write)(struct netfs_io_subrequest *subreq);
	void (*retry_request)(struct netfs_io_request *wreq, struct netfs_io_stream *stream);
	void (*invalidate_cache)(struct netfs_io_request *wreq);
};


enum netfs_read_from_hole {
	NETFS_READ_HOLE_IGNORE,
	NETFS_READ_HOLE_CLEAR,
	NETFS_READ_HOLE_FAIL,
};


struct netfs_cache_ops {
	
	void (*end_operation)(struct netfs_cache_resources *cres);

	
	int (*read)(struct netfs_cache_resources *cres,
		    loff_t start_pos,
		    struct iov_iter *iter,
		    enum netfs_read_from_hole read_hole,
		    netfs_io_terminated_t term_func,
		    void *term_func_priv);

	
	int (*write)(struct netfs_cache_resources *cres,
		     loff_t start_pos,
		     struct iov_iter *iter,
		     netfs_io_terminated_t term_func,
		     void *term_func_priv);

	
	void (*issue_write)(struct netfs_io_subrequest *subreq);

	
	void (*expand_readahead)(struct netfs_cache_resources *cres,
				 unsigned long long *_start,
				 unsigned long long *_len,
				 unsigned long long i_size);

	
	enum netfs_io_source (*prepare_read)(struct netfs_io_subrequest *subreq,
					     unsigned long long i_size);

	
	void (*prepare_write_subreq)(struct netfs_io_subrequest *subreq);

	
	int (*prepare_write)(struct netfs_cache_resources *cres,
			     loff_t *_start, size_t *_len, size_t upper_len,
			     loff_t i_size, bool no_space_allocated_yet);

	
	enum netfs_io_source (*prepare_ondemand_read)(struct netfs_cache_resources *cres,
						      loff_t start, size_t *_len,
						      loff_t i_size,
						      unsigned long *_flags, ino_t ino);

	
	int (*query_occupancy)(struct netfs_cache_resources *cres,
			       loff_t start, size_t len, size_t granularity,
			       loff_t *_data_start, size_t *_data_len);
};


ssize_t netfs_unbuffered_read_iter_locked(struct kiocb *iocb, struct iov_iter *iter);
ssize_t netfs_unbuffered_read_iter(struct kiocb *iocb, struct iov_iter *iter);
ssize_t netfs_buffered_read_iter(struct kiocb *iocb, struct iov_iter *iter);
ssize_t netfs_file_read_iter(struct kiocb *iocb, struct iov_iter *iter);


ssize_t netfs_perform_write(struct kiocb *iocb, struct iov_iter *iter,
			    struct netfs_group *netfs_group);
ssize_t netfs_buffered_write_iter_locked(struct kiocb *iocb, struct iov_iter *from,
					 struct netfs_group *netfs_group);
ssize_t netfs_unbuffered_write_iter(struct kiocb *iocb, struct iov_iter *from);
ssize_t netfs_unbuffered_write_iter_locked(struct kiocb *iocb, struct iov_iter *iter,
					   struct netfs_group *netfs_group);
ssize_t netfs_file_write_iter(struct kiocb *iocb, struct iov_iter *from);


struct readahead_control;
void netfs_readahead(struct readahead_control *);
int netfs_read_folio(struct file *, struct folio *);
int netfs_write_begin(struct netfs_inode *, struct file *,
		      struct address_space *, loff_t pos, unsigned int len,
		      struct folio **, void **fsdata);
int netfs_writepages(struct address_space *mapping,
		     struct writeback_control *wbc);
bool netfs_dirty_folio(struct address_space *mapping, struct folio *folio);
int netfs_unpin_writeback(struct inode *inode, struct writeback_control *wbc);
void netfs_clear_inode_writeback(struct inode *inode, const void *aux);
void netfs_invalidate_folio(struct folio *folio, size_t offset, size_t length);
bool netfs_release_folio(struct folio *folio, gfp_t gfp);


vm_fault_t netfs_page_mkwrite(struct vm_fault *vmf, struct netfs_group *netfs_group);


void netfs_read_subreq_progress(struct netfs_io_subrequest *subreq,
				bool was_async);
void netfs_read_subreq_terminated(struct netfs_io_subrequest *subreq,
				  int error, bool was_async);
void netfs_get_subrequest(struct netfs_io_subrequest *subreq,
			  enum netfs_sreq_ref_trace what);
void netfs_put_subrequest(struct netfs_io_subrequest *subreq,
			  bool was_async, enum netfs_sreq_ref_trace what);
ssize_t netfs_extract_user_iter(struct iov_iter *orig, size_t orig_len,
				struct iov_iter *new,
				iov_iter_extraction_t extraction_flags);
size_t netfs_limit_iter(const struct iov_iter *iter, size_t start_offset,
			size_t max_size, size_t max_segs);
void netfs_prepare_write_failed(struct netfs_io_subrequest *subreq);
void netfs_write_subrequest_terminated(void *_op, ssize_t transferred_or_error,
				       bool was_async);
void netfs_queue_write_request(struct netfs_io_subrequest *subreq);

int netfs_start_io_read(struct inode *inode);
void netfs_end_io_read(struct inode *inode);
int netfs_start_io_write(struct inode *inode);
void netfs_end_io_write(struct inode *inode);
int netfs_start_io_direct(struct inode *inode);
void netfs_end_io_direct(struct inode *inode);


static inline struct netfs_inode *netfs_inode(struct inode *inode)
{
	return container_of(inode, struct netfs_inode, inode);
}


static inline void netfs_inode_init(struct netfs_inode *ctx,
				    const struct netfs_request_ops *ops,
				    bool use_zero_point)
{
	ctx->ops = ops;
	ctx->remote_i_size = i_size_read(&ctx->inode);
	ctx->zero_point = LLONG_MAX;
	ctx->flags = 0;
	atomic_set(&ctx->io_count, 0);
#if IS_ENABLED(CONFIG_FSCACHE)
	ctx->cache = NULL;
#endif
	mutex_init(&ctx->wb_lock);
	
	if (use_zero_point) {
		ctx->zero_point = ctx->remote_i_size;
		mapping_set_release_always(ctx->inode.i_mapping);
	}
}


static inline void netfs_resize_file(struct netfs_inode *ctx, loff_t new_i_size,
				     bool changed_on_server)
{
	if (changed_on_server)
		ctx->remote_i_size = new_i_size;
	if (new_i_size < ctx->zero_point)
		ctx->zero_point = new_i_size;
}


static inline struct fscache_cookie *netfs_i_cookie(struct netfs_inode *ctx)
{
#if IS_ENABLED(CONFIG_FSCACHE)
	return ctx->cache;
#else
	return NULL;
#endif
}


static inline void netfs_wait_for_outstanding_io(struct inode *inode)
{
	struct netfs_inode *ictx = netfs_inode(inode);

	wait_var_event(&ictx->io_count, atomic_read(&ictx->io_count) == 0);
}

#endif 
