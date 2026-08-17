#ifndef IO_URING_TYPES_H
#define IO_URING_TYPES_H

#include <linux/blkdev.h>
#include <linux/hashtable.h>
#include <linux/task_work.h>
#include <linux/bitmap.h>
#include <linux/llist.h>
#include <linux/android_kabi.h>
#include <uapi/linux/io_uring.h>

enum {
	
	IOU_F_TWQ_LAZY_WAKE			= 1,
};

enum io_uring_cmd_flags {
	IO_URING_F_COMPLETE_DEFER	= 1,
	IO_URING_F_UNLOCKED		= 2,
	
	IO_URING_F_MULTISHOT		= 4,
	
	IO_URING_F_IOWQ			= 8,
	
	IO_URING_F_NONBLOCK		= INT_MIN,

	
	IO_URING_F_SQE128		= (1 << 8),
	IO_URING_F_CQE32		= (1 << 9),
	IO_URING_F_IOPOLL		= (1 << 10),

	
	IO_URING_F_CANCEL		= (1 << 11),
	IO_URING_F_COMPAT		= (1 << 12),
	IO_URING_F_TASK_DEAD		= (1 << 13),
};

struct io_wq_work_node {
	struct io_wq_work_node *next;
};

struct io_wq_work_list {
	struct io_wq_work_node *first;
	struct io_wq_work_node *last;
};

struct io_wq_work {
	struct io_wq_work_node list;
	atomic_t flags;
	
	int cancel_seq;
};

struct io_fixed_file {
	
	unsigned long file_ptr;
};

struct io_file_table {
	struct io_fixed_file *files;
	unsigned long *bitmap;
	unsigned int alloc_hint;
};

struct io_hash_bucket {
	spinlock_t		lock;
	struct hlist_head	list;
} ____cacheline_aligned_in_smp;

struct io_hash_table {
	struct io_hash_bucket	*hbs;
	unsigned		hash_bits;
};


#define IO_RINGFD_REG_MAX 16

struct io_uring_task {
	
	int				cached_refs;
	const struct io_ring_ctx 	*last;
	struct io_wq			*io_wq;
	struct file			*registered_rings[IO_RINGFD_REG_MAX];

	struct xarray			xa;
	struct wait_queue_head		wait;
	atomic_t			in_cancel;
	atomic_t			inflight_tracked;
	struct percpu_counter		inflight;

	struct { 
		struct llist_head	task_list;
		struct callback_head	task_work;
	} ____cacheline_aligned_in_smp;
};

struct io_uring {
	u32 head;
	u32 tail;
};


struct io_rings {
	
	struct io_uring		sq, cq;
	
	u32			sq_ring_mask, cq_ring_mask;
	
	u32			sq_ring_entries, cq_ring_entries;
	
	u32			sq_dropped;
	
	atomic_t		sq_flags;
	
	u32			cq_flags;
	
	u32			cq_overflow;
	
	struct io_uring_cqe	cqes[] ____cacheline_aligned_in_smp;
};

struct io_restriction {
	DECLARE_BITMAP(register_op, IORING_REGISTER_LAST);
	DECLARE_BITMAP(sqe_op, IORING_OP_LAST);
	u8 sqe_flags_allowed;
	u8 sqe_flags_required;
	bool registered;
};

struct io_submit_link {
	struct io_kiocb		*head;
	struct io_kiocb		*last;
};

struct io_submit_state {
	
	struct io_wq_work_node	free_list;
	
	struct io_wq_work_list	compl_reqs;
	struct io_submit_link	link;

	bool			plug_started;
	bool			need_plug;
	bool			cq_flush;
	unsigned short		submit_nr;
	struct blk_plug		plug;
};

struct io_alloc_cache {
	void			**entries;
	unsigned int		nr_cached;
	unsigned int		max_cached;
	size_t			elem_size;
};

struct io_ring_ctx {
	
	struct {
		unsigned int		flags;
		unsigned int		drain_next: 1;
		unsigned int		restricted: 1;
		unsigned int		off_timeout_used: 1;
		unsigned int		drain_active: 1;
		unsigned int		has_evfd: 1;
		
		unsigned int		task_complete: 1;
		unsigned int		lockless_cq: 1;
		unsigned int		syscall_iopoll: 1;
		unsigned int		poll_activated: 1;
		unsigned int		drain_disabled: 1;
		unsigned int		compat: 1;
		unsigned int		iowq_limits_set : 1;

		struct task_struct	*submitter_task;
		struct io_rings		*rings;
		struct percpu_ref	refs;

		clockid_t		clockid;
		enum tk_offsets		clock_offset;

		enum task_work_notify_mode	notify_method;
		unsigned			sq_thread_idle;
	} ____cacheline_aligned_in_smp;

	
	struct {
		struct mutex		uring_lock;

		
		u32			*sq_array;
		struct io_uring_sqe	*sq_sqes;
		unsigned		cached_sq_head;
		unsigned		sq_entries;

		
		struct io_rsrc_node	*rsrc_node;
		atomic_t		cancel_seq;

		
		bool			poll_multi_queue;
		struct io_wq_work_list	iopoll_list;

		struct io_file_table	file_table;
		struct io_mapped_ubuf	**user_bufs;
		unsigned		nr_user_files;
		unsigned		nr_user_bufs;

		struct io_submit_state	submit_state;

		struct xarray		io_bl_xa;

		struct io_hash_table	cancel_table_locked;
		struct io_alloc_cache	apoll_cache;
		struct io_alloc_cache	netmsg_cache;
		struct io_alloc_cache	rw_cache;
		struct io_alloc_cache	uring_cache;

		
		struct hlist_head	cancelable_uring_cmd;
	} ____cacheline_aligned_in_smp;

	struct {
		
		struct io_uring_cqe	*cqe_cached;
		struct io_uring_cqe	*cqe_sentinel;

		unsigned		cached_cq_tail;
		unsigned		cq_entries;
		struct io_ev_fd	__rcu	*io_ev_fd;
		unsigned		cq_extra;
	} ____cacheline_aligned_in_smp;

	
	struct {
		struct llist_head	work_llist;
		unsigned long		check_cq;
		atomic_t		cq_wait_nr;
		atomic_t		cq_timeouts;
		struct wait_queue_head	cq_wait;
		ANDROID_KABI_IGNORE(8, struct llist_head	retry_llist);
	} ____cacheline_aligned_in_smp;

	
	struct {
		spinlock_t		timeout_lock;
		struct list_head	timeout_list;
		struct list_head	ltimeout_list;
		unsigned		cq_last_tm_flush;
	} ____cacheline_aligned_in_smp;

	spinlock_t		completion_lock;

	struct list_head	io_buffers_comp;
	struct list_head	cq_overflow_list;
	struct io_hash_table	cancel_table;

	struct hlist_head	waitid_list;

#ifdef CONFIG_FUTEX
	struct hlist_head	futex_list;
	struct io_alloc_cache	futex_cache;
#endif

	const struct cred	*sq_creds;	
	struct io_sq_data	*sq_data;	

	struct wait_queue_head	sqo_sq_wait;
	struct list_head	sqd_list;

	unsigned int		file_alloc_start;
	unsigned int		file_alloc_end;

	struct list_head	io_buffers_cache;

	
	struct wait_queue_head		poll_wq;
	struct io_restriction		restrictions;

	
	struct io_rsrc_data		*file_data;
	struct io_rsrc_data		*buf_data;

	
	struct list_head		rsrc_ref_list;
	struct io_alloc_cache		rsrc_node_cache;
	struct wait_queue_head		rsrc_quiesce_wq;
	unsigned			rsrc_quiesce;

	u32			pers_next;
	struct xarray		personalities;

	
	struct io_wq_hash		*hash_map;

	
	struct user_struct		*user;
	struct mm_struct		*mm_account;

	
	struct llist_head		fallback_llist;
	struct delayed_work		fallback_work;
	struct work_struct		exit_work;
	struct list_head		tctx_list;
	struct completion		ref_comp;

	
	u32				iowq_limits[2];

	struct callback_head		poll_wq_task_work;
	struct list_head		defer_list;

	struct io_alloc_cache		msg_cache;
	spinlock_t			msg_lock;

#ifdef CONFIG_NET_RX_BUSY_POLL
	struct list_head	napi_list;	
	spinlock_t		napi_lock;	

	
	ktime_t			napi_busy_poll_dt;
	bool			napi_prefer_busy_poll;
	bool			napi_enabled;

	DECLARE_HASHTABLE(napi_ht, 4);
#endif

	
	unsigned			evfd_last_cq_tail;

	
	unsigned short			n_ring_pages;
	unsigned short			n_sqe_pages;
	struct page			**ring_pages;
	struct page			**sqe_pages;
};

struct io_tw_state {
};

enum {
	REQ_F_FIXED_FILE_BIT	= IOSQE_FIXED_FILE_BIT,
	REQ_F_IO_DRAIN_BIT	= IOSQE_IO_DRAIN_BIT,
	REQ_F_LINK_BIT		= IOSQE_IO_LINK_BIT,
	REQ_F_HARDLINK_BIT	= IOSQE_IO_HARDLINK_BIT,
	REQ_F_FORCE_ASYNC_BIT	= IOSQE_ASYNC_BIT,
	REQ_F_BUFFER_SELECT_BIT	= IOSQE_BUFFER_SELECT_BIT,
	REQ_F_CQE_SKIP_BIT	= IOSQE_CQE_SKIP_SUCCESS_BIT,

	
	REQ_F_FAIL_BIT		= 8,
	REQ_F_INFLIGHT_BIT,
	REQ_F_CUR_POS_BIT,
	REQ_F_NOWAIT_BIT,
	REQ_F_LINK_TIMEOUT_BIT,
	REQ_F_NEED_CLEANUP_BIT,
	REQ_F_POLLED_BIT,
	REQ_F_BUFFER_SELECTED_BIT,
	REQ_F_BUFFER_RING_BIT,
	REQ_F_REISSUE_BIT,
	REQ_F_CREDS_BIT,
	REQ_F_REFCOUNT_BIT,
	REQ_F_ARM_LTIMEOUT_BIT,
	REQ_F_ASYNC_DATA_BIT,
	REQ_F_SKIP_LINK_CQES_BIT,
	REQ_F_SINGLE_POLL_BIT,
	REQ_F_DOUBLE_POLL_BIT,
	REQ_F_MULTISHOT_BIT,
	REQ_F_APOLL_MULTISHOT_BIT,
	REQ_F_CLEAR_POLLIN_BIT,
	REQ_F_HASH_LOCKED_BIT,
	
	REQ_F_SUPPORT_NOWAIT_BIT,
	REQ_F_ISREG_BIT,
	REQ_F_POLL_NO_LAZY_BIT,
	REQ_F_CAN_POLL_BIT,
	REQ_F_BL_EMPTY_BIT,
	REQ_F_BL_NO_RECYCLE_BIT,
	REQ_F_BUFFERS_COMMIT_BIT,

	
	__REQ_F_LAST_BIT,
};

typedef u64 __bitwise io_req_flags_t;
#define IO_REQ_FLAG(bitno)	((__force io_req_flags_t) BIT_ULL((bitno)))

enum {
	
	REQ_F_FIXED_FILE	= IO_REQ_FLAG(REQ_F_FIXED_FILE_BIT),
	
	REQ_F_IO_DRAIN		= IO_REQ_FLAG(REQ_F_IO_DRAIN_BIT),
	
	REQ_F_LINK		= IO_REQ_FLAG(REQ_F_LINK_BIT),
	
	REQ_F_HARDLINK		= IO_REQ_FLAG(REQ_F_HARDLINK_BIT),
	
	REQ_F_FORCE_ASYNC	= IO_REQ_FLAG(REQ_F_FORCE_ASYNC_BIT),
	
	REQ_F_BUFFER_SELECT	= IO_REQ_FLAG(REQ_F_BUFFER_SELECT_BIT),
	
	REQ_F_CQE_SKIP		= IO_REQ_FLAG(REQ_F_CQE_SKIP_BIT),

	
	REQ_F_FAIL		= IO_REQ_FLAG(REQ_F_FAIL_BIT),
	
	REQ_F_INFLIGHT		= IO_REQ_FLAG(REQ_F_INFLIGHT_BIT),
	
	REQ_F_CUR_POS		= IO_REQ_FLAG(REQ_F_CUR_POS_BIT),
	
	REQ_F_NOWAIT		= IO_REQ_FLAG(REQ_F_NOWAIT_BIT),
	
	REQ_F_LINK_TIMEOUT	= IO_REQ_FLAG(REQ_F_LINK_TIMEOUT_BIT),
	
	REQ_F_NEED_CLEANUP	= IO_REQ_FLAG(REQ_F_NEED_CLEANUP_BIT),
	
	REQ_F_POLLED		= IO_REQ_FLAG(REQ_F_POLLED_BIT),
	
	REQ_F_BUFFER_SELECTED	= IO_REQ_FLAG(REQ_F_BUFFER_SELECTED_BIT),
	
	REQ_F_BUFFER_RING	= IO_REQ_FLAG(REQ_F_BUFFER_RING_BIT),
	
	REQ_F_REISSUE		= IO_REQ_FLAG(REQ_F_REISSUE_BIT),
	
	REQ_F_SUPPORT_NOWAIT	= IO_REQ_FLAG(REQ_F_SUPPORT_NOWAIT_BIT),
	
	REQ_F_ISREG		= IO_REQ_FLAG(REQ_F_ISREG_BIT),
	
	REQ_F_CREDS		= IO_REQ_FLAG(REQ_F_CREDS_BIT),
	
	REQ_F_REFCOUNT		= IO_REQ_FLAG(REQ_F_REFCOUNT_BIT),
	
	REQ_F_ARM_LTIMEOUT	= IO_REQ_FLAG(REQ_F_ARM_LTIMEOUT_BIT),
	
	REQ_F_ASYNC_DATA	= IO_REQ_FLAG(REQ_F_ASYNC_DATA_BIT),
	
	REQ_F_SKIP_LINK_CQES	= IO_REQ_FLAG(REQ_F_SKIP_LINK_CQES_BIT),
	
	REQ_F_SINGLE_POLL	= IO_REQ_FLAG(REQ_F_SINGLE_POLL_BIT),
	
	REQ_F_DOUBLE_POLL	= IO_REQ_FLAG(REQ_F_DOUBLE_POLL_BIT),
	
	REQ_F_MULTISHOT		= IO_REQ_FLAG(REQ_F_MULTISHOT_BIT),
	
	REQ_F_APOLL_MULTISHOT	= IO_REQ_FLAG(REQ_F_APOLL_MULTISHOT_BIT),
	
	REQ_F_CLEAR_POLLIN	= IO_REQ_FLAG(REQ_F_CLEAR_POLLIN_BIT),
	
	REQ_F_HASH_LOCKED	= IO_REQ_FLAG(REQ_F_HASH_LOCKED_BIT),
	
	REQ_F_POLL_NO_LAZY	= IO_REQ_FLAG(REQ_F_POLL_NO_LAZY_BIT),
	
	REQ_F_CAN_POLL		= IO_REQ_FLAG(REQ_F_CAN_POLL_BIT),
	
	REQ_F_BL_EMPTY		= IO_REQ_FLAG(REQ_F_BL_EMPTY_BIT),
	
	REQ_F_BL_NO_RECYCLE	= IO_REQ_FLAG(REQ_F_BL_NO_RECYCLE_BIT),
	
	REQ_F_BUFFERS_COMMIT	= IO_REQ_FLAG(REQ_F_BUFFERS_COMMIT_BIT),
};

typedef void (*io_req_tw_func_t)(struct io_kiocb *req, struct io_tw_state *ts);

struct io_task_work {
	struct llist_node		node;
	io_req_tw_func_t		func;
};

struct io_cqe {
	__u64	user_data;
	__s32	res;
	
	union {
		__u32	flags;
		int	fd;
	};
};


struct io_cmd_data {
	struct file		*file;
	
	__u8			data[56];
};

static inline void io_kiocb_cmd_sz_check(size_t cmd_sz)
{
	BUILD_BUG_ON(cmd_sz > sizeof(struct io_cmd_data));
}
#define io_kiocb_to_cmd(req, cmd_type) ( \
	io_kiocb_cmd_sz_check(sizeof(cmd_type)) , \
	((cmd_type *)&(req)->cmd) \
)
#define cmd_to_io_kiocb(ptr)	((struct io_kiocb *) ptr)

struct io_kiocb {
	union {
		
		struct file		*file;
		struct io_cmd_data	cmd;
	};

	u8				opcode;
	
	u8				iopoll_completed;
	
	u16				buf_index;

	unsigned			nr_tw;

	
	io_req_flags_t			flags;

	struct io_cqe			cqe;

	struct io_ring_ctx		*ctx;
	struct task_struct		*task;

	union {
		
		struct io_mapped_ubuf	*imu;

		
		struct io_buffer	*kbuf;

		
		struct io_buffer_list	*buf_list;
	};

	union {
		
		struct io_wq_work_node	comp_list;
		
		__poll_t apoll_events;
	};

	struct io_rsrc_node		*rsrc_node;

	atomic_t			refs;
	bool				cancel_seq_set;
	struct io_task_work		io_task_work;
	union {
		
		struct hlist_node	hash_node;

		
		struct rcu_head		rcu_head;
	};
	
	struct async_poll		*apoll;
	
	void				*async_data;
	
	atomic_t			poll_refs;
	struct io_kiocb			*link;
	
	const struct cred		*creds;
	struct io_wq_work		work;

	struct {
		u64			extra1;
		u64			extra2;
	} big_cqe;
};

ANDROID_KABI_TYPE_STRING("s#io_kiocb", "structure_type io_kiocb { member union_type { member pointer_type { s#file } file data_member_location(0) , member s#io_cmd_data cmd data_member_location(0) } byte_size(64) data_member_location(0) , member t#u8 opcode data_member_location(64) , member t#u8 iopoll_completed data_member_location(65) , member t#u16 buf_index data_member_location(66) , member base_type unsigned int byte_size(4) encoding(7) nr_tw data_member_location(68) , member t#io_req_flags_t flags data_member_location(72) , member s#io_cqe cqe data_member_location(80) , member pointer_type { s#io_ring_ctx } ctx data_member_location(96) , member pointer_type { s#task_struct } task data_member_location(104) , member union_type { member pointer_type { s#io_mapped_ubuf } imu data_member_location(0) , member pointer_type { s#io_buffer } kbuf data_member_location(0) , member pointer_type { s#io_buffer_list } buf_list data_member_location(0) } byte_size(8) data_member_location(112) , member union_type { member s#io_wq_work_node comp_list data_member_location(0) , member t#__poll_t apoll_events data_member_location(0) } byte_size(8) data_member_location(120) , member pointer_type { s#io_rsrc_node } rsrc_node data_member_location(128) , member t#atomic_t refs data_member_location(136) , member t#bool cancel_seq_set data_member_location(140) , member s#io_task_work io_task_work data_member_location(144) , member s#hlist_node hash_node data_member_location(160) , member pointer_type { s#async_poll } apoll data_member_location(176) , member pointer_type { base_type void } async_data data_member_location(184) , member t#atomic_t poll_refs data_member_location(192) , member pointer_type { s#io_kiocb } link data_member_location(200) , member pointer_type { const_type { s#cred } } creds data_member_location(208) , member s#io_wq_work work data_member_location(216) , member structure_type { member t#u64 extra1 data_member_location(0) , member t#u64 extra2 data_member_location(8) } byte_size(16) big_cqe data_member_location(232) } byte_size(248)");

struct io_overflow_cqe {
	struct list_head list;
	struct io_uring_cqe cqe;
};

#endif
