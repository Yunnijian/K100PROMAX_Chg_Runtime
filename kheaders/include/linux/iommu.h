/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LINUX_IOMMU_H
#define __LINUX_IOMMU_H

#include <linux/android_kabi.h>
#include <linux/scatterlist.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/iova_bitmap.h>

#define IOMMU_READ	(1 << 0)
#define IOMMU_WRITE	(1 << 1)
#define IOMMU_CACHE	(1 << 2) 
#define IOMMU_NOEXEC	(1 << 3)
#define IOMMU_MMIO	(1 << 4) 

#define IOMMU_PRIV	(1 << 5)

struct iommu_ops;
struct iommu_group;
struct bus_type;
struct device;
struct iommu_domain;
struct iommu_domain_ops;
struct iommu_dirty_ops;
struct notifier_block;
struct iommu_sva;
struct iommu_dma_cookie;
struct iommu_fault_param;

#define IOMMU_FAULT_PERM_READ	(1 << 0) 
#define IOMMU_FAULT_PERM_WRITE	(1 << 1) 
#define IOMMU_FAULT_PERM_EXEC	(1 << 2) 
#define IOMMU_FAULT_PERM_PRIV	(1 << 3) 


enum iommu_fault_type {
	IOMMU_FAULT_PAGE_REQ = 1,	
};


struct iommu_fault_page_request {
#define IOMMU_FAULT_PAGE_REQUEST_PASID_VALID	(1 << 0)
#define IOMMU_FAULT_PAGE_REQUEST_LAST_PAGE	(1 << 1)
#define IOMMU_FAULT_PAGE_RESPONSE_NEEDS_PASID	(1 << 2)
	u32	flags;
	u32	pasid;
	u32	grpid;
	u32	perm;
	u64	addr;
	u64	private_data[2];
};


struct iommu_fault {
	u32 type;
	struct iommu_fault_page_request prm;
};


enum iommu_page_response_code {
	IOMMU_PAGE_RESP_SUCCESS = 0,
	IOMMU_PAGE_RESP_INVALID,
	IOMMU_PAGE_RESP_FAILURE,
};


struct iommu_page_response {
	u32	pasid;
	u32	grpid;
	u32	code;
};

struct iopf_fault {
	struct iommu_fault fault;
	
	struct list_head list;
};

struct iopf_group {
	struct iopf_fault last_fault;
	struct list_head faults;
	size_t fault_count;
	
	struct list_head pending_node;
	struct work_struct work;
	struct iommu_attach_handle *attach_handle;
	
	struct iommu_fault_param *fault_param;
	
	struct list_head node;
	u32 cookie;
};


struct iopf_queue {
	struct workqueue_struct *wq;
	struct list_head devices;
	struct mutex lock;
};


#define IOMMU_FAULT_READ	0x0
#define IOMMU_FAULT_WRITE	0x1

typedef int (*iommu_fault_handler_t)(struct iommu_domain *,
			struct device *, unsigned long, int, void *);

struct iommu_domain_geometry {
	dma_addr_t aperture_start; 
	dma_addr_t aperture_end;   
	bool force_aperture;       
};


#define __IOMMU_DOMAIN_PAGING	(1U << 0)  
#define __IOMMU_DOMAIN_DMA_API	(1U << 1)  
#define __IOMMU_DOMAIN_PT	(1U << 2)  
#define __IOMMU_DOMAIN_DMA_FQ	(1U << 3)  

#define __IOMMU_DOMAIN_SVA	(1U << 4)  
#define __IOMMU_DOMAIN_PLATFORM	(1U << 5)

#define __IOMMU_DOMAIN_NESTED	(1U << 6)  

#define IOMMU_DOMAIN_ALLOC_FLAGS ~__IOMMU_DOMAIN_DMA_FQ

#define IOMMU_DOMAIN_BLOCKED	(0U)
#define IOMMU_DOMAIN_IDENTITY	(__IOMMU_DOMAIN_PT)
#define IOMMU_DOMAIN_UNMANAGED	(__IOMMU_DOMAIN_PAGING)
#define IOMMU_DOMAIN_DMA	(__IOMMU_DOMAIN_PAGING |	\
				 __IOMMU_DOMAIN_DMA_API)
#define IOMMU_DOMAIN_DMA_FQ	(__IOMMU_DOMAIN_PAGING |	\
				 __IOMMU_DOMAIN_DMA_API |	\
				 __IOMMU_DOMAIN_DMA_FQ)
#define IOMMU_DOMAIN_SVA	(__IOMMU_DOMAIN_SVA)
#define IOMMU_DOMAIN_PLATFORM	(__IOMMU_DOMAIN_PLATFORM)
#define IOMMU_DOMAIN_NESTED	(__IOMMU_DOMAIN_NESTED)

struct iommu_domain {
	unsigned type;
	const struct iommu_domain_ops *ops;
	const struct iommu_dirty_ops *dirty_ops;
	const struct iommu_ops *owner; 
	unsigned long pgsize_bitmap;	
	struct iommu_domain_geometry geometry;
	struct iommu_dma_cookie *iova_cookie;
	int (*iopf_handler)(struct iopf_group *group);
	void *fault_data;
	union {
		struct {
			iommu_fault_handler_t handler;
			void *handler_token;
		};
		struct {	
			struct mm_struct *mm;
			int users;
			
			struct list_head next;
		};
	};
};

static inline bool iommu_is_dma_domain(struct iommu_domain *domain)
{
	return domain->type & __IOMMU_DOMAIN_DMA_API;
}

enum iommu_cap {
	IOMMU_CAP_CACHE_COHERENCY,	
	IOMMU_CAP_NOEXEC,		
	IOMMU_CAP_PRE_BOOT_PROTECTION,	
	
	IOMMU_CAP_ENFORCE_CACHE_COHERENCY,
	
	IOMMU_CAP_DEFERRED_FLUSH,
	IOMMU_CAP_DIRTY_TRACKING,	
};


enum iommu_resv_type {
	
	IOMMU_RESV_DIRECT,
	
	IOMMU_RESV_DIRECT_RELAXABLE,
	
	IOMMU_RESV_RESERVED,
	
	IOMMU_RESV_MSI,
	
	IOMMU_RESV_SW_MSI,
};


struct iommu_resv_region {
	struct list_head	list;
	phys_addr_t		start;
	size_t			length;
	int			prot;
	enum iommu_resv_type	type;
	void (*free)(struct device *dev, struct iommu_resv_region *region);
};

struct iommu_iort_rmr_data {
	struct iommu_resv_region rr;

	
	const u32 *sids;
	u32 num_sids;
};


enum iommu_dev_features {
	IOMMU_DEV_FEAT_SVA,
	IOMMU_DEV_FEAT_IOPF,
};

#define IOMMU_NO_PASID	(0U) 
#define IOMMU_FIRST_GLOBAL_PASID	(1U) 
#define IOMMU_PASID_INVALID	(-1U)
typedef unsigned int ioasid_t;


#define IOMMU_DIRTY_NO_CLEAR (1 << 0)

#ifdef CONFIG_IOMMU_API


struct iommu_iotlb_gather {
	unsigned long		start;
	unsigned long		end;
	size_t			pgsize;
	struct list_head	freelist;
	bool			queued;
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};


struct iommu_dirty_bitmap {
	struct iova_bitmap *bitmap;
	struct iommu_iotlb_gather *gather;
};


struct iommu_dirty_ops {
	int (*set_dirty_tracking)(struct iommu_domain *domain, bool enabled);
	int (*read_and_clear_dirty)(struct iommu_domain *domain,
				    unsigned long iova, size_t size,
				    unsigned long flags,
				    struct iommu_dirty_bitmap *dirty);
};


struct iommu_user_data {
	unsigned int type;
	void __user *uptr;
	size_t len;
};


struct iommu_user_data_array {
	unsigned int type;
	void __user *uptr;
	size_t entry_len;
	u32 entry_num;
};


static inline int __iommu_copy_struct_from_user(
	void *dst_data, const struct iommu_user_data *src_data,
	unsigned int data_type, size_t data_len, size_t min_len)
{
	if (WARN_ON(!dst_data || !src_data))
		return -EINVAL;
	if (src_data->type != data_type)
		return -EINVAL;
	if (src_data->len < min_len || data_len < src_data->len)
		return -EINVAL;
	return copy_struct_from_user(dst_data, data_len, src_data->uptr,
				     src_data->len);
}


#define iommu_copy_struct_from_user(kdst, user_data, data_type, min_last) \
	__iommu_copy_struct_from_user(kdst, user_data, data_type,         \
				      sizeof(*kdst),                      \
				      offsetofend(typeof(*kdst), min_last))


static inline int __iommu_copy_struct_from_user_array(
	void *dst_data, const struct iommu_user_data_array *src_array,
	unsigned int data_type, unsigned int index, size_t data_len,
	size_t min_len)
{
	struct iommu_user_data src_data;

	if (WARN_ON(!src_array || index >= src_array->entry_num))
		return -EINVAL;
	if (!src_array->entry_num)
		return -EINVAL;
	src_data.uptr = src_array->uptr + src_array->entry_len * index;
	src_data.len = src_array->entry_len;
	src_data.type = src_array->type;

	return __iommu_copy_struct_from_user(dst_data, &src_data, data_type,
					     data_len, min_len);
}


#define iommu_copy_struct_from_user_array(kdst, user_array, data_type, index, \
					  min_last)                           \
	__iommu_copy_struct_from_user_array(                                  \
		kdst, user_array, data_type, index, sizeof(*(kdst)),          \
		offsetofend(typeof(*(kdst)), min_last))


struct iommu_ops {
	bool (*capable)(struct device *dev, enum iommu_cap);
	void *(*hw_info)(struct device *dev, u32 *length, u32 *type);

	
	struct iommu_domain *(*domain_alloc)(unsigned iommu_domain_type);
	struct iommu_domain *(*domain_alloc_user)(
		struct device *dev, u32 flags, struct iommu_domain *parent,
		const struct iommu_user_data *user_data);
	struct iommu_domain *(*domain_alloc_paging)(struct device *dev);
	struct iommu_domain *(*domain_alloc_sva)(struct device *dev,
						 struct mm_struct *mm);

	struct iommu_device *(*probe_device)(struct device *dev);
	void (*release_device)(struct device *dev);
	void (*probe_finalize)(struct device *dev);
	struct iommu_group *(*device_group)(struct device *dev);

	
	void (*get_resv_regions)(struct device *dev, struct list_head *list);

	int (*of_xlate)(struct device *dev, const struct of_phandle_args *args);
	bool (*is_attach_deferred)(struct device *dev);

	
	int (*dev_enable_feat)(struct device *dev, enum iommu_dev_features f);
	int (*dev_disable_feat)(struct device *dev, enum iommu_dev_features f);

	void (*page_response)(struct device *dev, struct iopf_fault *evt,
			      struct iommu_page_response *msg);

	int (*def_domain_type)(struct device *dev);
	void (*remove_dev_pasid)(struct device *dev, ioasid_t pasid,
				 struct iommu_domain *domain);

	const struct iommu_domain_ops *default_domain_ops;
	unsigned long pgsize_bitmap;
	struct module *owner;
	struct iommu_domain *identity_domain;
	struct iommu_domain *blocked_domain;
	struct iommu_domain *release_domain;
	struct iommu_domain *default_domain;
	u8 user_pasid_table:1;
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	ANDROID_KABI_RESERVE(3);
	ANDROID_KABI_RESERVE(4);
};


struct iommu_map_cookie_sg {
	struct iommu_domain *domain;
};


struct iommu_domain_ops {
	int (*attach_dev)(struct iommu_domain *domain, struct device *dev);
	int (*set_dev_pasid)(struct iommu_domain *domain, struct device *dev,
			     ioasid_t pasid);

	int (*map_pages)(struct iommu_domain *domain, unsigned long iova,
			 phys_addr_t paddr, size_t pgsize, size_t pgcount,
			 int prot, gfp_t gfp, size_t *mapped);
	size_t (*unmap_pages)(struct iommu_domain *domain, unsigned long iova,
			      size_t pgsize, size_t pgcount,
			      struct iommu_iotlb_gather *iotlb_gather);

	void (*flush_iotlb_all)(struct iommu_domain *domain);
	int (*iotlb_sync_map)(struct iommu_domain *domain, unsigned long iova,
			      size_t size);
	void (*iotlb_sync)(struct iommu_domain *domain,
			   struct iommu_iotlb_gather *iotlb_gather);
	int (*cache_invalidate_user)(struct iommu_domain *domain,
				     struct iommu_user_data_array *array);

	phys_addr_t (*iova_to_phys)(struct iommu_domain *domain,
				    dma_addr_t iova);

	bool (*enforce_cache_coherency)(struct iommu_domain *domain);
	int (*enable_nesting)(struct iommu_domain *domain);
	int (*set_pgtable_quirks)(struct iommu_domain *domain,
				  unsigned long quirks);

	void (*free)(struct iommu_domain *domain);

	struct iommu_map_cookie_sg *(*alloc_cookie_sg)(unsigned long iova, int prot,
						       unsigned int nents, gfp_t gfp);
	int (*add_deferred_map_sg)(struct iommu_map_cookie_sg *cookie,
				   phys_addr_t paddr, size_t pgsize, size_t pgcount);
	size_t (*consume_deferred_map_sg)(struct iommu_map_cookie_sg *cookie);
	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	ANDROID_KABI_RESERVE(3);
	ANDROID_KABI_RESERVE(4);
};


struct iommu_device {
	struct list_head list;
	const struct iommu_ops *ops;
	struct fwnode_handle *fwnode;
	struct device *dev;
	struct iommu_group *singleton_group;
	u32 max_pasids;
};


struct iommu_fault_param {
	struct mutex lock;
	refcount_t users;
	struct rcu_head rcu;

	struct device *dev;
	struct iopf_queue *queue;
	struct list_head queue_list;

	struct list_head partial;
	struct list_head faults;
};


struct dev_iommu {
	struct mutex lock;
	struct iommu_fault_param __rcu	*fault_param;
	struct iommu_fwspec		*fwspec;
	struct iommu_device		*iommu_dev;
	void				*priv;
	u32				max_pasids;
	u32				attach_deferred:1;
	u32				pci_32bit_workaround:1;
	u32				require_direct:1;
	u32				shadow_on_flush:1;
};

int iommu_device_register(struct iommu_device *iommu,
			  const struct iommu_ops *ops,
			  struct device *hwdev);
void iommu_device_unregister(struct iommu_device *iommu);
int  iommu_device_sysfs_add(struct iommu_device *iommu,
			    struct device *parent,
			    const struct attribute_group **groups,
			    const char *fmt, ...) __printf(4, 5);
void iommu_device_sysfs_remove(struct iommu_device *iommu);
int  iommu_device_link(struct iommu_device   *iommu, struct device *link);
void iommu_device_unlink(struct iommu_device *iommu, struct device *link);
int iommu_deferred_attach(struct device *dev, struct iommu_domain *domain);

static inline struct iommu_device *dev_to_iommu_device(struct device *dev)
{
	return (struct iommu_device *)dev_get_drvdata(dev);
}


static inline struct iommu_device *__iommu_get_iommu_dev(struct device *dev)
{
	return dev->iommu->iommu_dev;
}

#define iommu_get_iommu_dev(dev, type, member) \
	container_of(__iommu_get_iommu_dev(dev), type, member)

static inline void iommu_iotlb_gather_init(struct iommu_iotlb_gather *gather)
{
	*gather = (struct iommu_iotlb_gather) {
		.start	= ULONG_MAX,
		.freelist = LIST_HEAD_INIT(gather->freelist),
	};
}

extern int bus_iommu_probe(const struct bus_type *bus);
extern bool iommu_present(const struct bus_type *bus);
extern bool device_iommu_capable(struct device *dev, enum iommu_cap cap);
extern bool iommu_group_has_isolated_msi(struct iommu_group *group);
extern struct iommu_domain *iommu_domain_alloc(const struct bus_type *bus);
struct iommu_domain *iommu_paging_domain_alloc(struct device *dev);
extern void iommu_domain_free(struct iommu_domain *domain);
extern int iommu_attach_device(struct iommu_domain *domain,
			       struct device *dev);
extern void iommu_detach_device(struct iommu_domain *domain,
				struct device *dev);
extern struct iommu_domain *iommu_get_domain_for_dev(struct device *dev);
extern struct iommu_domain *iommu_get_dma_domain(struct device *dev);
extern int iommu_map(struct iommu_domain *domain, unsigned long iova,
		     phys_addr_t paddr, size_t size, int prot, gfp_t gfp);
int iommu_map_nosync(struct iommu_domain *domain, unsigned long iova,
		phys_addr_t paddr, size_t size, int prot, gfp_t gfp);
int iommu_sync_map(struct iommu_domain *domain, unsigned long iova,
		size_t size);
extern size_t iommu_unmap(struct iommu_domain *domain, unsigned long iova,
			  size_t size);
extern size_t iommu_unmap_fast(struct iommu_domain *domain,
			       unsigned long iova, size_t size,
			       struct iommu_iotlb_gather *iotlb_gather);
extern ssize_t iommu_map_sg(struct iommu_domain *domain, unsigned long iova,
			    struct scatterlist *sg, unsigned int nents,
			    int prot, gfp_t gfp);
extern phys_addr_t iommu_iova_to_phys(struct iommu_domain *domain, dma_addr_t iova);
extern void iommu_set_fault_handler(struct iommu_domain *domain,
			iommu_fault_handler_t handler, void *token);

extern void iommu_get_resv_regions(struct device *dev, struct list_head *list);
extern void iommu_put_resv_regions(struct device *dev, struct list_head *list);
extern void iommu_set_default_passthrough(bool cmd_line);
extern void iommu_set_default_translated(bool cmd_line);
extern bool iommu_default_passthrough(void);
extern struct iommu_resv_region *
iommu_alloc_resv_region(phys_addr_t start, size_t length, int prot,
			enum iommu_resv_type type, gfp_t gfp);
extern int iommu_get_group_resv_regions(struct iommu_group *group,
					struct list_head *head);

extern int iommu_attach_group(struct iommu_domain *domain,
			      struct iommu_group *group);
extern void iommu_detach_group(struct iommu_domain *domain,
			       struct iommu_group *group);
extern struct iommu_group *iommu_group_alloc(void);
extern void *iommu_group_get_iommudata(struct iommu_group *group);
extern void iommu_group_set_iommudata(struct iommu_group *group,
				      void *iommu_data,
				      void (*release)(void *iommu_data));
extern int iommu_group_set_name(struct iommu_group *group, const char *name);
extern int iommu_group_add_device(struct iommu_group *group,
				  struct device *dev);
extern void iommu_group_remove_device(struct device *dev);
extern int iommu_group_for_each_dev(struct iommu_group *group, void *data,
				    int (*fn)(struct device *, void *));
extern struct iommu_group *iommu_group_get(struct device *dev);
extern struct iommu_group *iommu_group_ref_get(struct iommu_group *group);
extern void iommu_group_put(struct iommu_group *group);

extern int iommu_group_id(struct iommu_group *group);
extern struct iommu_domain *iommu_group_default_domain(struct iommu_group *);

int iommu_enable_nesting(struct iommu_domain *domain);
int iommu_set_pgtable_quirks(struct iommu_domain *domain,
		unsigned long quirks);

void iommu_set_dma_strict(void);

extern int report_iommu_fault(struct iommu_domain *domain, struct device *dev,
			      unsigned long iova, int flags);

static inline void iommu_flush_iotlb_all(struct iommu_domain *domain)
{
	if (domain->ops->flush_iotlb_all)
		domain->ops->flush_iotlb_all(domain);
}

static inline void iommu_iotlb_sync(struct iommu_domain *domain,
				  struct iommu_iotlb_gather *iotlb_gather)
{
	if (domain->ops->iotlb_sync)
		domain->ops->iotlb_sync(domain, iotlb_gather);

	iommu_iotlb_gather_init(iotlb_gather);
}


static inline
bool iommu_iotlb_gather_is_disjoint(struct iommu_iotlb_gather *gather,
				    unsigned long iova, size_t size)
{
	unsigned long start = iova, end = start + size - 1;

	return gather->end != 0 &&
		(end + 1 < gather->start || start > gather->end + 1);
}



static inline void iommu_iotlb_gather_add_range(struct iommu_iotlb_gather *gather,
						unsigned long iova, size_t size)
{
	unsigned long end = iova + size - 1;

	if (gather->start > iova)
		gather->start = iova;
	if (gather->end < end)
		gather->end = end;
}


#define _iommu_iotlb_add_page(domain, gather, iova, size, sync)		\
	if (((gather)->pgsize && (gather)->pgsize != (size)) ||		\
	    iommu_iotlb_gather_is_disjoint((gather), (iova), (size)))	\
		sync((domain), (gather));				\
	(gather)->pgsize = (size);					\
	iommu_iotlb_gather_add_range((gather), (iova), (size))


static inline void iommu_iotlb_gather_add_page(struct iommu_domain *domain,
					       struct iommu_iotlb_gather *gather,
					       unsigned long iova, size_t size)
{
	_iommu_iotlb_add_page(domain, gather, iova, size, iommu_iotlb_sync);
}

static inline bool iommu_iotlb_gather_queued(struct iommu_iotlb_gather *gather)
{
	return gather && gather->queued;
}

static inline void iommu_dirty_bitmap_init(struct iommu_dirty_bitmap *dirty,
					   struct iova_bitmap *bitmap,
					   struct iommu_iotlb_gather *gather)
{
	if (gather)
		iommu_iotlb_gather_init(gather);

	dirty->bitmap = bitmap;
	dirty->gather = gather;
}

static inline void iommu_dirty_bitmap_record(struct iommu_dirty_bitmap *dirty,
					     unsigned long iova,
					     unsigned long length)
{
	if (dirty->bitmap)
		iova_bitmap_set(dirty->bitmap, iova, length);

	if (dirty->gather)
		iommu_iotlb_gather_add_range(dirty->gather, iova, length);
}


extern struct iommu_group *pci_device_group(struct device *dev);

extern struct iommu_group *generic_device_group(struct device *dev);

struct iommu_group *fsl_mc_device_group(struct device *dev);
extern struct iommu_group *generic_single_device_group(struct device *dev);


struct iommu_fwspec {
	struct fwnode_handle	*iommu_fwnode;
	u32			flags;
	unsigned int		num_ids;
	u32			ids[];
};


#define IOMMU_FWSPEC_PCI_RC_ATS			(1 << 0)


struct iommu_attach_handle {
	struct iommu_domain		*domain;
};


struct iommu_sva {
	struct iommu_attach_handle	handle;
	struct device			*dev;
	refcount_t			users;
};

struct iommu_mm_data {
	u32			pasid;
	struct list_head	sva_domains;
};

int iommu_fwspec_init(struct device *dev, struct fwnode_handle *iommu_fwnode);
void iommu_fwspec_free(struct device *dev);
int iommu_fwspec_add_ids(struct device *dev, const u32 *ids, int num_ids);

static inline struct iommu_fwspec *dev_iommu_fwspec_get(struct device *dev)
{
	if (dev->iommu)
		return dev->iommu->fwspec;
	else
		return NULL;
}

static inline void dev_iommu_fwspec_set(struct device *dev,
					struct iommu_fwspec *fwspec)
{
	dev->iommu->fwspec = fwspec;
}

static inline void *dev_iommu_priv_get(struct device *dev)
{
	if (dev->iommu)
		return dev->iommu->priv;
	else
		return NULL;
}

void dev_iommu_priv_set(struct device *dev, void *priv);

extern struct mutex iommu_probe_device_lock;
int iommu_probe_device(struct device *dev);

int iommu_dev_enable_feature(struct device *dev, enum iommu_dev_features f);
int iommu_dev_disable_feature(struct device *dev, enum iommu_dev_features f);

int iommu_device_use_default_domain(struct device *dev);
void iommu_device_unuse_default_domain(struct device *dev);

int iommu_group_claim_dma_owner(struct iommu_group *group, void *owner);
void iommu_group_release_dma_owner(struct iommu_group *group);
bool iommu_group_dma_owner_claimed(struct iommu_group *group);

int iommu_device_claim_dma_owner(struct device *dev, void *owner);
void iommu_device_release_dma_owner(struct device *dev);

int iommu_attach_device_pasid(struct iommu_domain *domain,
			      struct device *dev, ioasid_t pasid,
			      struct iommu_attach_handle *handle);
void iommu_detach_device_pasid(struct iommu_domain *domain,
			       struct device *dev, ioasid_t pasid);
ioasid_t iommu_alloc_global_pasid(struct device *dev);
void iommu_free_global_pasid(ioasid_t pasid);
#else 

struct iommu_ops {};
struct iommu_group {};
struct iommu_fwspec {};
struct iommu_device {};
struct iommu_fault_param {};
struct iommu_iotlb_gather {};
struct iommu_dirty_bitmap {};
struct iommu_dirty_ops {};

static inline bool iommu_present(const struct bus_type *bus)
{
	return false;
}

static inline bool device_iommu_capable(struct device *dev, enum iommu_cap cap)
{
	return false;
}

static inline struct iommu_domain *iommu_domain_alloc(const struct bus_type *bus)
{
	return NULL;
}

static inline struct iommu_domain *iommu_paging_domain_alloc(struct device *dev)
{
	return ERR_PTR(-ENODEV);
}

static inline void iommu_domain_free(struct iommu_domain *domain)
{
}

static inline int iommu_attach_device(struct iommu_domain *domain,
				      struct device *dev)
{
	return -ENODEV;
}

static inline void iommu_detach_device(struct iommu_domain *domain,
				       struct device *dev)
{
}

static inline struct iommu_domain *iommu_get_domain_for_dev(struct device *dev)
{
	return NULL;
}

static inline int iommu_map(struct iommu_domain *domain, unsigned long iova,
			    phys_addr_t paddr, size_t size, int prot, gfp_t gfp)
{
	return -ENODEV;
}

static inline size_t iommu_unmap(struct iommu_domain *domain,
				 unsigned long iova, size_t size)
{
	return 0;
}

static inline size_t iommu_unmap_fast(struct iommu_domain *domain,
				      unsigned long iova, int gfp_order,
				      struct iommu_iotlb_gather *iotlb_gather)
{
	return 0;
}

static inline ssize_t iommu_map_sg(struct iommu_domain *domain,
				   unsigned long iova, struct scatterlist *sg,
				   unsigned int nents, int prot, gfp_t gfp)
{
	return -ENODEV;
}

static inline void iommu_flush_iotlb_all(struct iommu_domain *domain)
{
}

static inline void iommu_iotlb_sync(struct iommu_domain *domain,
				  struct iommu_iotlb_gather *iotlb_gather)
{
}

static inline phys_addr_t iommu_iova_to_phys(struct iommu_domain *domain, dma_addr_t iova)
{
	return 0;
}

static inline void iommu_set_fault_handler(struct iommu_domain *domain,
				iommu_fault_handler_t handler, void *token)
{
}

static inline void iommu_get_resv_regions(struct device *dev,
					struct list_head *list)
{
}

static inline void iommu_put_resv_regions(struct device *dev,
					struct list_head *list)
{
}

static inline int iommu_get_group_resv_regions(struct iommu_group *group,
					       struct list_head *head)
{
	return -ENODEV;
}

static inline void iommu_set_default_passthrough(bool cmd_line)
{
}

static inline void iommu_set_default_translated(bool cmd_line)
{
}

static inline bool iommu_default_passthrough(void)
{
	return true;
}

static inline int iommu_attach_group(struct iommu_domain *domain,
				     struct iommu_group *group)
{
	return -ENODEV;
}

static inline void iommu_detach_group(struct iommu_domain *domain,
				      struct iommu_group *group)
{
}

static inline struct iommu_group *iommu_group_alloc(void)
{
	return ERR_PTR(-ENODEV);
}

static inline void *iommu_group_get_iommudata(struct iommu_group *group)
{
	return NULL;
}

static inline void iommu_group_set_iommudata(struct iommu_group *group,
					     void *iommu_data,
					     void (*release)(void *iommu_data))
{
}

static inline int iommu_group_set_name(struct iommu_group *group,
				       const char *name)
{
	return -ENODEV;
}

static inline int iommu_group_add_device(struct iommu_group *group,
					 struct device *dev)
{
	return -ENODEV;
}

static inline void iommu_group_remove_device(struct device *dev)
{
}

static inline int iommu_group_for_each_dev(struct iommu_group *group,
					   void *data,
					   int (*fn)(struct device *, void *))
{
	return -ENODEV;
}

static inline struct iommu_group *iommu_group_get(struct device *dev)
{
	return NULL;
}

static inline void iommu_group_put(struct iommu_group *group)
{
}

static inline int iommu_group_id(struct iommu_group *group)
{
	return -ENODEV;
}

static inline int iommu_set_pgtable_quirks(struct iommu_domain *domain,
		unsigned long quirks)
{
	return 0;
}

static inline int iommu_device_register(struct iommu_device *iommu,
					const struct iommu_ops *ops,
					struct device *hwdev)
{
	return -ENODEV;
}

static inline struct iommu_device *dev_to_iommu_device(struct device *dev)
{
	return NULL;
}

static inline void iommu_iotlb_gather_init(struct iommu_iotlb_gather *gather)
{
}

static inline void iommu_iotlb_gather_add_page(struct iommu_domain *domain,
					       struct iommu_iotlb_gather *gather,
					       unsigned long iova, size_t size)
{
}

static inline bool iommu_iotlb_gather_queued(struct iommu_iotlb_gather *gather)
{
	return false;
}

static inline void iommu_dirty_bitmap_init(struct iommu_dirty_bitmap *dirty,
					   struct iova_bitmap *bitmap,
					   struct iommu_iotlb_gather *gather)
{
}

static inline void iommu_dirty_bitmap_record(struct iommu_dirty_bitmap *dirty,
					     unsigned long iova,
					     unsigned long length)
{
}

static inline void iommu_device_unregister(struct iommu_device *iommu)
{
}

static inline int  iommu_device_sysfs_add(struct iommu_device *iommu,
					  struct device *parent,
					  const struct attribute_group **groups,
					  const char *fmt, ...)
{
	return -ENODEV;
}

static inline void iommu_device_sysfs_remove(struct iommu_device *iommu)
{
}

static inline int iommu_device_link(struct device *dev, struct device *link)
{
	return -EINVAL;
}

static inline void iommu_device_unlink(struct device *dev, struct device *link)
{
}

static inline int iommu_fwspec_init(struct device *dev,
				    struct fwnode_handle *iommu_fwnode)
{
	return -ENODEV;
}

static inline void iommu_fwspec_free(struct device *dev)
{
}

static inline int iommu_fwspec_add_ids(struct device *dev, u32 *ids,
				       int num_ids)
{
	return -ENODEV;
}

static inline int
iommu_dev_enable_feature(struct device *dev, enum iommu_dev_features feat)
{
	return -ENODEV;
}

static inline int
iommu_dev_disable_feature(struct device *dev, enum iommu_dev_features feat)
{
	return -ENODEV;
}

static inline struct iommu_fwspec *dev_iommu_fwspec_get(struct device *dev)
{
	return NULL;
}

static inline int iommu_device_use_default_domain(struct device *dev)
{
	return 0;
}

static inline void iommu_device_unuse_default_domain(struct device *dev)
{
}

static inline int
iommu_group_claim_dma_owner(struct iommu_group *group, void *owner)
{
	return -ENODEV;
}

static inline void iommu_group_release_dma_owner(struct iommu_group *group)
{
}

static inline bool iommu_group_dma_owner_claimed(struct iommu_group *group)
{
	return false;
}

static inline void iommu_device_release_dma_owner(struct device *dev)
{
}

static inline int iommu_device_claim_dma_owner(struct device *dev, void *owner)
{
	return -ENODEV;
}

static inline int iommu_attach_device_pasid(struct iommu_domain *domain,
					    struct device *dev, ioasid_t pasid,
					    struct iommu_attach_handle *handle)
{
	return -ENODEV;
}

static inline void iommu_detach_device_pasid(struct iommu_domain *domain,
					     struct device *dev, ioasid_t pasid)
{
}

static inline ioasid_t iommu_alloc_global_pasid(struct device *dev)
{
	return IOMMU_PASID_INVALID;
}

static inline void iommu_free_global_pasid(ioasid_t pasid) {}
#endif 

#if IS_ENABLED(CONFIG_LOCKDEP) && IS_ENABLED(CONFIG_IOMMU_API)
void iommu_group_mutex_assert(struct device *dev);
#else
static inline void iommu_group_mutex_assert(struct device *dev)
{
}
#endif


static inline ssize_t iommu_map_sgtable(struct iommu_domain *domain,
			unsigned long iova, struct sg_table *sgt, int prot)
{
	return iommu_map_sg(domain, iova, sgt->sgl, sgt->orig_nents, prot,
			    GFP_KERNEL);
}

#ifdef CONFIG_IOMMU_DEBUGFS
extern	struct dentry *iommu_debugfs_dir;
void iommu_debugfs_setup(void);
#else
static inline void iommu_debugfs_setup(void) {}
#endif

#ifdef CONFIG_IOMMU_DMA
#include <linux/msi.h>

int iommu_get_msi_cookie(struct iommu_domain *domain, dma_addr_t base);

int iommu_dma_prepare_msi(struct msi_desc *desc, phys_addr_t msi_addr);
void iommu_dma_compose_msi_msg(struct msi_desc *desc, struct msi_msg *msg);

#else 

struct msi_desc;
struct msi_msg;

static inline int iommu_get_msi_cookie(struct iommu_domain *domain, dma_addr_t base)
{
	return -ENODEV;
}

static inline int iommu_dma_prepare_msi(struct msi_desc *desc, phys_addr_t msi_addr)
{
	return 0;
}

static inline void iommu_dma_compose_msi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
}

#endif	


#define TEGRA_STREAM_ID_BYPASS 0x7f

static inline bool tegra_dev_iommu_get_stream_id(struct device *dev, u32 *stream_id)
{
#ifdef CONFIG_IOMMU_API
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);

	if (fwspec && fwspec->num_ids == 1) {
		*stream_id = fwspec->ids[0] & 0xffff;
		return true;
	}
#endif

	return false;
}

#ifdef CONFIG_IOMMU_MM_DATA
static inline void mm_pasid_init(struct mm_struct *mm)
{
	
	mm->iommu_mm = NULL;
}

static inline bool mm_valid_pasid(struct mm_struct *mm)
{
	return READ_ONCE(mm->iommu_mm);
}

static inline u32 mm_get_enqcmd_pasid(struct mm_struct *mm)
{
	struct iommu_mm_data *iommu_mm = READ_ONCE(mm->iommu_mm);

	if (!iommu_mm)
		return IOMMU_PASID_INVALID;
	return iommu_mm->pasid;
}

void mm_pasid_drop(struct mm_struct *mm);
struct iommu_sva *iommu_sva_bind_device(struct device *dev,
					struct mm_struct *mm);
void iommu_sva_unbind_device(struct iommu_sva *handle);
u32 iommu_sva_get_pasid(struct iommu_sva *handle);
#else
static inline struct iommu_sva *
iommu_sva_bind_device(struct device *dev, struct mm_struct *mm)
{
	return ERR_PTR(-ENODEV);
}

static inline void iommu_sva_unbind_device(struct iommu_sva *handle)
{
}

static inline u32 iommu_sva_get_pasid(struct iommu_sva *handle)
{
	return IOMMU_PASID_INVALID;
}
static inline void mm_pasid_init(struct mm_struct *mm) {}
static inline bool mm_valid_pasid(struct mm_struct *mm) { return false; }

static inline u32 mm_get_enqcmd_pasid(struct mm_struct *mm)
{
	return IOMMU_PASID_INVALID;
}

static inline void mm_pasid_drop(struct mm_struct *mm) {}
#endif 

#ifdef CONFIG_IOMMU_IOPF
int iopf_queue_add_device(struct iopf_queue *queue, struct device *dev);
void iopf_queue_remove_device(struct iopf_queue *queue, struct device *dev);
int iopf_queue_flush_dev(struct device *dev);
struct iopf_queue *iopf_queue_alloc(const char *name);
void iopf_queue_free(struct iopf_queue *queue);
int iopf_queue_discard_partial(struct iopf_queue *queue);
void iopf_free_group(struct iopf_group *group);
int iommu_report_device_fault(struct device *dev, struct iopf_fault *evt);
void iopf_group_response(struct iopf_group *group,
			 enum iommu_page_response_code status);
#else
static inline int
iopf_queue_add_device(struct iopf_queue *queue, struct device *dev)
{
	return -ENODEV;
}

static inline void
iopf_queue_remove_device(struct iopf_queue *queue, struct device *dev)
{
}

static inline int iopf_queue_flush_dev(struct device *dev)
{
	return -ENODEV;
}

static inline struct iopf_queue *iopf_queue_alloc(const char *name)
{
	return NULL;
}

static inline void iopf_queue_free(struct iopf_queue *queue)
{
}

static inline int iopf_queue_discard_partial(struct iopf_queue *queue)
{
	return -ENODEV;
}

static inline void iopf_free_group(struct iopf_group *group)
{
}

static inline int
iommu_report_device_fault(struct device *dev, struct iopf_fault *evt)
{
	return -ENODEV;
}

static inline void iopf_group_response(struct iopf_group *group,
				       enum iommu_page_response_code status)
{
}
#endif 
#endif 
