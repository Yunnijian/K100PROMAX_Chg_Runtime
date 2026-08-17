/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM block

#if !defined(_TRACE_BLOCK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_BLOCK_H

#include <linux/blktrace_api.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/tracepoint.h>
#include <uapi/linux/ioprio.h>

#define RWBS_LEN	10

#define IOPRIO_CLASS_STRINGS \
	{ IOPRIO_CLASS_NONE,	"none" }, \
	{ IOPRIO_CLASS_RT,	"rt" }, \
	{ IOPRIO_CLASS_BE,	"be" }, \
	{ IOPRIO_CLASS_IDLE,	"idle" }, \
	{ IOPRIO_CLASS_INVALID,	"invalid"}

#ifdef CONFIG_BUFFER_HEAD
DECLARE_EVENT_CLASS(block_buffer,

	TP_PROTO(struct buffer_head *bh),

	TP_ARGS(bh),

	TP_STRUCT__entry (
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  size_t,	size			)
	),

	TP_fast_assign(
		__entry->dev		= bh->b_bdev->bd_dev;
		__entry->sector		= bh->b_blocknr;
		__entry->size		= bh->b_size;
	),

	TP_printk("%d,%d sector=%llu size=%zu",
		MAJOR(__entry->dev), MINOR(__entry->dev),
		(unsigned long long)__entry->sector, __entry->size
	)
);


DEFINE_EVENT(block_buffer, block_touch_buffer,

	TP_PROTO(struct buffer_head *bh),

	TP_ARGS(bh)
);


DEFINE_EVENT(block_buffer, block_dirty_buffer,

	TP_PROTO(struct buffer_head *bh),

	TP_ARGS(bh)
);
#endif 


TRACE_EVENT(block_rq_requeue,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq),

	TP_STRUCT__entry(
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  unsigned int,	nr_sector		)
		__field(  unsigned short, ioprio		)
		__array(  char,		rwbs,	RWBS_LEN	)
		__dynamic_array( char,	cmd,	1		)
	),

	TP_fast_assign(
		__entry->dev	   = rq->q->disk ? disk_devt(rq->q->disk) : 0;
		__entry->sector    = blk_rq_trace_sector(rq);
		__entry->nr_sector = blk_rq_trace_nr_sectors(rq);
		__entry->ioprio    = req_get_ioprio(rq);

		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags);
		__get_str(cmd)[0] = '\0';
	),

	TP_printk("%d,%d %s (%s) %llu + %u %s,%u,%u [%d]",
		  MAJOR(__entry->dev), MINOR(__entry->dev),
		  __entry->rwbs, __get_str(cmd),
		  (unsigned long long)__entry->sector, __entry->nr_sector,
		  __print_symbolic(IOPRIO_PRIO_CLASS(__entry->ioprio),
				   IOPRIO_CLASS_STRINGS),
		  IOPRIO_PRIO_HINT(__entry->ioprio),
		  IOPRIO_PRIO_LEVEL(__entry->ioprio),  0)
);

DECLARE_EVENT_CLASS(block_rq_completion,

	TP_PROTO(struct request *rq, blk_status_t error, unsigned int nr_bytes),

	TP_ARGS(rq, error, nr_bytes),

	TP_STRUCT__entry(
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  unsigned int,	nr_sector		)
		__field(  int	,	error			)
		__field(  unsigned short, ioprio		)
		__array(  char,		rwbs,	RWBS_LEN	)
		__dynamic_array( char,	cmd,	1		)
	),

	TP_fast_assign(
		__entry->dev	   = rq->q->disk ? disk_devt(rq->q->disk) : 0;
		__entry->sector    = blk_rq_pos(rq);
		__entry->nr_sector = nr_bytes >> 9;
		__entry->error     = blk_status_to_errno(error);
		__entry->ioprio    = req_get_ioprio(rq);

		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags);
		__get_str(cmd)[0] = '\0';
	),

	TP_printk("%d,%d %s (%s) %llu + %u %s,%u,%u [%d]",
		  MAJOR(__entry->dev), MINOR(__entry->dev),
		  __entry->rwbs, __get_str(cmd),
		  (unsigned long long)__entry->sector, __entry->nr_sector,
		  __print_symbolic(IOPRIO_PRIO_CLASS(__entry->ioprio),
				   IOPRIO_CLASS_STRINGS),
		  IOPRIO_PRIO_HINT(__entry->ioprio),
		  IOPRIO_PRIO_LEVEL(__entry->ioprio), __entry->error)
);


DEFINE_EVENT(block_rq_completion, block_rq_complete,

	TP_PROTO(struct request *rq, blk_status_t error, unsigned int nr_bytes),

	TP_ARGS(rq, error, nr_bytes)
);


DEFINE_EVENT(block_rq_completion, block_rq_error,

	TP_PROTO(struct request *rq, blk_status_t error, unsigned int nr_bytes),

	TP_ARGS(rq, error, nr_bytes)
);

DECLARE_EVENT_CLASS(block_rq,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq),

	TP_STRUCT__entry(
		__field(  dev_t,	dev			)
		__field(  sector_t,	sector			)
		__field(  unsigned int,	nr_sector		)
		__field(  unsigned int,	bytes			)
		__field(  unsigned short, ioprio		)
		__array(  char,		rwbs,	RWBS_LEN	)
		__array(  char,         comm,   TASK_COMM_LEN   )
		__dynamic_array( char,	cmd,	1		)
	),

	TP_fast_assign(
		__entry->dev	   = rq->q->disk ? disk_devt(rq->q->disk) : 0;
		__entry->sector    = blk_rq_trace_sector(rq);
		__entry->nr_sector = blk_rq_trace_nr_sectors(rq);
		__entry->bytes     = blk_rq_bytes(rq);
		__entry->ioprio	   = req_get_ioprio(rq);

		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags);
		__get_str(cmd)[0] = '\0';
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %u (%s) %llu + %u %s,%u,%u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev),
		  __entry->rwbs, __entry->bytes, __get_str(cmd),
		  (unsigned long long)__entry->sector, __entry->nr_sector,
		  __print_symbolic(IOPRIO_PRIO_CLASS(__entry->ioprio),
				   IOPRIO_CLASS_STRINGS),
		  IOPRIO_PRIO_HINT(__entry->ioprio),
		  IOPRIO_PRIO_LEVEL(__entry->ioprio), __entry->comm)
);


DEFINE_EVENT(block_rq, block_rq_insert,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq)
);


DEFINE_EVENT(block_rq, block_rq_issue,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq)
);


DEFINE_EVENT(block_rq, block_rq_merge,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq)
);


DEFINE_EVENT(block_rq, block_io_start,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq)
);


DEFINE_EVENT(block_rq, block_io_done,

	TP_PROTO(struct request *rq),

	TP_ARGS(rq)
);


TRACE_EVENT(block_bio_complete,

	TP_PROTO(struct request_queue *q, struct bio *bio),

	TP_ARGS(q, bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( sector_t,	sector		)
		__field( unsigned,	nr_sector	)
		__field( int,		error		)
		__array( char,		rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
		__entry->error		= blk_status_to_errno(bio->bi_status);
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf);
	),

	TP_printk("%d,%d %s %llu + %u [%d]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->error)
);

DECLARE_EVENT_CLASS(block_bio,

	TP_PROTO(struct bio *bio),

	TP_ARGS(bio),

	TP_STRUCT__entry(
		__field( dev_t,		dev			)
		__field( sector_t,	sector			)
		__field( unsigned int,	nr_sector		)
		__array( char,		rwbs,	RWBS_LEN	)
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf);
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %llu + %u [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector, __entry->comm)
);


DEFINE_EVENT(block_bio, block_bio_bounce,
	TP_PROTO(struct bio *bio),
	TP_ARGS(bio)
);


DEFINE_EVENT(block_bio, block_bio_backmerge,
	TP_PROTO(struct bio *bio),
	TP_ARGS(bio)
);


DEFINE_EVENT(block_bio, block_bio_frontmerge,
	TP_PROTO(struct bio *bio),
	TP_ARGS(bio)
);


DEFINE_EVENT(block_bio, block_bio_queue,
	TP_PROTO(struct bio *bio),
	TP_ARGS(bio)
);


DEFINE_EVENT(block_bio, block_getrq,
	TP_PROTO(struct bio *bio),
	TP_ARGS(bio)
);


DEFINE_EVENT(block_rq, blk_zone_append_update_request_bio,
	     TP_PROTO(struct request *rq),
	     TP_ARGS(rq)
);


TRACE_EVENT(block_plug,

	TP_PROTO(struct request_queue *q),

	TP_ARGS(q),

	TP_STRUCT__entry(
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("[%s]", __entry->comm)
);

DECLARE_EVENT_CLASS(block_unplug,

	TP_PROTO(struct request_queue *q, unsigned int depth, bool explicit),

	TP_ARGS(q, depth, explicit),

	TP_STRUCT__entry(
		__field( int,		nr_rq			)
		__array( char,		comm,	TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->nr_rq = depth;
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("[%s] %d", __entry->comm, __entry->nr_rq)
);


DEFINE_EVENT(block_unplug, block_unplug,

	TP_PROTO(struct request_queue *q, unsigned int depth, bool explicit),

	TP_ARGS(q, depth, explicit)
);


TRACE_EVENT(block_split,

	TP_PROTO(struct bio *bio, unsigned int new_sector),

	TP_ARGS(bio, new_sector),

	TP_STRUCT__entry(
		__field( dev_t,		dev				)
		__field( sector_t,	sector				)
		__field( sector_t,	new_sector			)
		__array( char,		rwbs,		RWBS_LEN	)
		__array( char,		comm,		TASK_COMM_LEN	)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->new_sector	= new_sector;
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf);
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
	),

	TP_printk("%d,%d %s %llu / %llu [%s]",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  (unsigned long long)__entry->new_sector,
		  __entry->comm)
);


TRACE_EVENT(block_bio_remap,

	TP_PROTO(struct bio *bio, dev_t dev, sector_t from),

	TP_ARGS(bio, dev, from),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( sector_t,	sector		)
		__field( unsigned int,	nr_sector	)
		__field( dev_t,		old_dev		)
		__field( sector_t,	old_sector	)
		__array( char,		rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
		__entry->dev		= bio_dev(bio);
		__entry->sector		= bio->bi_iter.bi_sector;
		__entry->nr_sector	= bio_sectors(bio);
		__entry->old_dev	= dev;
		__entry->old_sector	= from;
		blk_fill_rwbs(__entry->rwbs, bio->bi_opf);
	),

	TP_printk("%d,%d %s %llu + %u <- (%d,%d) %llu",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector,
		  MAJOR(__entry->old_dev), MINOR(__entry->old_dev),
		  (unsigned long long)__entry->old_sector)
);


TRACE_EVENT(block_rq_remap,

	TP_PROTO(struct request *rq, dev_t dev, sector_t from),

	TP_ARGS(rq, dev, from),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( sector_t,	sector		)
		__field( unsigned int,	nr_sector	)
		__field( dev_t,		old_dev		)
		__field( sector_t,	old_sector	)
		__field( unsigned int,	nr_bios		)
		__array( char,		rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
		__entry->dev		= disk_devt(rq->q->disk);
		__entry->sector		= blk_rq_pos(rq);
		__entry->nr_sector	= blk_rq_sectors(rq);
		__entry->old_dev	= dev;
		__entry->old_sector	= from;
		__entry->nr_bios	= blk_rq_count_bios(rq);
		blk_fill_rwbs(__entry->rwbs, rq->cmd_flags);
	),

	TP_printk("%d,%d %s %llu + %u <- (%d,%d) %llu %u",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sector,
		  MAJOR(__entry->old_dev), MINOR(__entry->old_dev),
		  (unsigned long long)__entry->old_sector, __entry->nr_bios)
);


TRACE_EVENT(blkdev_zone_mgmt,

	TP_PROTO(struct bio *bio, sector_t nr_sectors),

	TP_ARGS(bio, nr_sectors),

	TP_STRUCT__entry(
	    __field(  dev_t,	dev		)
	    __field(  sector_t,	sector		)
	    __field(  sector_t, nr_sectors	)
	    __array(  char,	rwbs,	RWBS_LEN)
	),

	TP_fast_assign(
	    __entry->dev	= bio_dev(bio);
	    __entry->sector	= bio->bi_iter.bi_sector;
	    __entry->nr_sectors	= bio_sectors(bio);
	    blk_fill_rwbs(__entry->rwbs, bio->bi_opf);
        ),

	TP_printk("%d,%d %s %llu + %llu",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->rwbs,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sectors)
);

DECLARE_EVENT_CLASS(block_zwplug,

	TP_PROTO(struct request_queue *q, unsigned int zno, sector_t sector,
		 unsigned int nr_sectors),

	TP_ARGS(q, zno, sector, nr_sectors),

	TP_STRUCT__entry(
		__field( dev_t,		dev		)
		__field( unsigned int,	zno		)
		__field( sector_t,	sector		)
		__field( unsigned int,	nr_sectors	)
	),

	TP_fast_assign(
		__entry->dev		= disk_devt(q->disk);
		__entry->zno		= zno;
		__entry->sector		= sector;
		__entry->nr_sectors	= nr_sectors;
	),

	TP_printk("%d,%d zone %u, BIO %llu + %u",
		  MAJOR(__entry->dev), MINOR(__entry->dev), __entry->zno,
		  (unsigned long long)__entry->sector,
		  __entry->nr_sectors)
);

DEFINE_EVENT(block_zwplug, disk_zone_wplug_add_bio,

	TP_PROTO(struct request_queue *q, unsigned int zno, sector_t sector,
		 unsigned int nr_sectors),

	TP_ARGS(q, zno, sector, nr_sectors)
);

DEFINE_EVENT(block_zwplug, blk_zone_wplug_bio,

	TP_PROTO(struct request_queue *q, unsigned int zno, sector_t sector,
		 unsigned int nr_sectors),

	TP_ARGS(q, zno, sector, nr_sectors)
);

#endif 


#include <trace/define_trace.h>

