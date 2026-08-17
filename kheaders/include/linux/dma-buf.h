/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __DMA_BUF_H__
#define __DMA_BUF_H__

#include <linux/android_kabi.h>
#include <linux/iosys-map.h>
#include <linux/file.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/dma-fence.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/android_kabi.h>
#include <linux/refcount.h>
#include <linux/atomic.h>

struct device;
struct dma_buf;
struct dma_buf_attachment;


struct dma_buf_ops {
	
	bool cache_sgt_mapping;

	
	int (*attach)(struct dma_buf *, struct dma_buf_attachment *);

	
	void (*detach)(struct dma_buf *, struct dma_buf_attachment *);

	
	int (*pin)(struct dma_buf_attachment *attach);

	
	void (*unpin)(struct dma_buf_attachment *attach);

	
	struct sg_table * (*map_dma_buf)(struct dma_buf_attachment *,
					 enum dma_data_direction);
	
	void (*unmap_dma_buf)(struct dma_buf_attachment *,
			      struct sg_table *,
			      enum dma_data_direction);

	

	
	void (*release)(struct dma_buf *);

	
	int (*begin_cpu_access)(struct dma_buf *, enum dma_data_direction);

	
	int (*begin_cpu_access_partial)(struct dma_buf *dmabuf,
					enum dma_data_direction,
					unsigned int offset, unsigned int len);

	
	int (*end_cpu_access)(struct dma_buf *, enum dma_data_direction);

	
	int (*end_cpu_access_partial)(struct dma_buf *dmabuf,
				      enum dma_data_direction,
				      unsigned int offset, unsigned int len);

	
	int (*mmap)(struct dma_buf *, struct vm_area_struct *vma);

	int (*vmap)(struct dma_buf *dmabuf, struct iosys_map *map);
	void (*vunmap)(struct dma_buf *dmabuf, struct iosys_map *map);

	
	int (*get_flags)(struct dma_buf *dmabuf, unsigned long *flags);

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};


struct dma_buf {
	
	size_t size;

	
	struct file *file;

	
	struct list_head attachments;

	
	const struct dma_buf_ops *ops;

	
	unsigned vmapping_counter;

	
	struct iosys_map vmap_ptr;

	
	const char *exp_name;

	
	const char *name;

	
	spinlock_t name_lock;

	
	struct module *owner;

	
	struct list_head list_node;

	
	void *priv;

	
	struct dma_resv *resv;

	
	wait_queue_head_t poll;

	
	
	struct dma_buf_poll_cb_t {
		struct dma_fence_cb cb;
		wait_queue_head_t *poll;

		__poll_t active;
	} cb_in, cb_out;
#ifdef CONFIG_DMABUF_SYSFS_STATS
	
	struct dma_buf_sysfs_entry {
		union {
			struct kobject kobj;

			
			struct work_struct sysfs_add_work;
		};
		struct dma_buf *dmabuf;
	} *sysfs_entry;
#endif

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	
	ANDROID_BACKPORT_USE(1, atomic64_t nr_task_refs);
};


struct dma_buf_attach_ops {
	
	bool allow_peer2peer;

	
	void (*move_notify)(struct dma_buf_attachment *attach);

	ANDROID_KABI_RESERVE(1);
};


struct dma_buf_attachment {
	struct dma_buf *dmabuf;
	struct device *dev;
	struct list_head node;
	struct sg_table *sgt;
	enum dma_data_direction dir;
	bool peer2peer;
	const struct dma_buf_attach_ops *importer_ops;
	void *importer_priv;
	void *priv;
	unsigned long dma_map_attrs;

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};


struct dma_buf_export_info {
	const char *exp_name;
	struct module *owner;
	const struct dma_buf_ops *ops;
	size_t size;
	int flags;
	struct dma_resv *resv;
	void *priv;

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};

#if IS_ENABLED(CONFIG_DEBUG_FS)
int get_dmabuf_debugfs_data(int (*fn)(const struct dma_buf *, void *),
			void *private);
#endif


struct task_dma_buf_record {
	struct list_head node;
	struct dma_buf *dmabuf;
	unsigned long refcnt;
};


struct task_dma_buf_info {
	unsigned long rss;
	unsigned long rss_hwm;
	refcount_t refcnt;
	spinlock_t lock;
	struct list_head dmabufs;
	unsigned int dmabuf_count;
};


#define DEFINE_DMA_BUF_EXPORT_INFO(name)	\
	struct dma_buf_export_info name = { .exp_name = KBUILD_MODNAME, \
					 .owner = THIS_MODULE }


static inline void get_dma_buf(struct dma_buf *dmabuf)
{
	get_file(dmabuf->file);
}


static inline bool dma_buf_is_dynamic(struct dma_buf *dmabuf)
{
	return !!dmabuf->ops->pin;
}


static inline bool
dma_buf_attachment_is_dynamic(struct dma_buf_attachment *attach)
{
	return !!attach->importer_ops;
}

struct dma_buf_attachment *dma_buf_attach(struct dma_buf *dmabuf,
					  struct device *dev);
struct dma_buf_attachment *
dma_buf_dynamic_attach(struct dma_buf *dmabuf, struct device *dev,
		       const struct dma_buf_attach_ops *importer_ops,
		       void *importer_priv);
void dma_buf_detach(struct dma_buf *dmabuf,
		    struct dma_buf_attachment *attach);
int dma_buf_pin(struct dma_buf_attachment *attach);
void dma_buf_unpin(struct dma_buf_attachment *attach);

struct dma_buf *dma_buf_export(const struct dma_buf_export_info *exp_info);

int dma_buf_fd(struct dma_buf *dmabuf, int flags);
struct dma_buf *dma_buf_get(int fd);
void dma_buf_put(struct dma_buf *dmabuf);

void dma_buf_mangle_sg_table(struct sg_table *sg_table);
struct sg_table *dma_buf_map_attachment(struct dma_buf_attachment *,
					enum dma_data_direction);
void dma_buf_unmap_attachment(struct dma_buf_attachment *, struct sg_table *,
				enum dma_data_direction);
void dma_buf_move_notify(struct dma_buf *dma_buf);
int dma_buf_begin_cpu_access(struct dma_buf *dma_buf,
			     enum dma_data_direction dir);
int dma_buf_begin_cpu_access_partial(struct dma_buf *dma_buf,
				     enum dma_data_direction dir,
				     unsigned int offset, unsigned int len);
int dma_buf_end_cpu_access(struct dma_buf *dma_buf,
			   enum dma_data_direction dir);
int dma_buf_end_cpu_access_partial(struct dma_buf *dma_buf,
				     enum dma_data_direction dir,
				     unsigned int offset, unsigned int len);
struct sg_table *
dma_buf_map_attachment_unlocked(struct dma_buf_attachment *attach,
				enum dma_data_direction direction);
void dma_buf_unmap_attachment_unlocked(struct dma_buf_attachment *attach,
				       struct sg_table *sg_table,
				       enum dma_data_direction direction);

int dma_buf_mmap(struct dma_buf *, struct vm_area_struct *,
		 unsigned long);
int dma_buf_vmap(struct dma_buf *dmabuf, struct iosys_map *map);
void dma_buf_vunmap(struct dma_buf *dmabuf, struct iosys_map *map);
int dma_buf_vmap_unlocked(struct dma_buf *dmabuf, struct iosys_map *map);
void dma_buf_vunmap_unlocked(struct dma_buf *dmabuf, struct iosys_map *map);
long dma_buf_set_name(struct dma_buf *dmabuf, const char *name);
int dma_buf_get_flags(struct dma_buf *dmabuf, unsigned long *flags);
struct dma_buf *dma_buf_iter_begin(void);
struct dma_buf *dma_buf_iter_next(struct dma_buf *dmbuf);

#ifdef CONFIG_DMA_SHARED_BUFFER

int is_dma_buf_file(struct file *file);
int dma_buf_account_task(struct dma_buf *dmabuf, struct task_dma_buf_info *dmabuf_info);
void dma_buf_unaccount_task(struct dma_buf *dmabuf, struct task_dma_buf_info *dmabuf_info);
int copy_dmabuf_info(u64 clone_flags, struct task_struct *task);
void get_dmabuf_info(struct task_dma_buf_info *dmabuf_info);
void put_dmabuf_info(struct task_dma_buf_info *dmabuf_info);
int dma_buf_begin_new_exec(struct files_struct *old_files);

#else 

static inline int is_dma_buf_file(struct file *file) { return 0; }
static inline int dma_buf_account_task(struct dma_buf *dmabuf,
				       struct task_dma_buf_info *dmabuf_info) { return 0; }
static inline void dma_buf_unaccount_task(struct dma_buf *dmabuf,
					  struct task_dma_buf_info *dmabuf_info) {}
static inline int copy_dmabuf_info(u64 clone_flags,
				   struct task_struct *task) { return 0; }
static inline void get_dmabuf_info(struct task_dma_buf_info *dmabuf_info) {}
static inline void put_dmabuf_info(struct task_dma_buf_info *dmabuf_info) {}
static inline int dma_buf_begin_new_exec(struct files_struct *old_files) { return 0; }

#endif 

#endif 
