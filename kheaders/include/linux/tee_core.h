/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __TEE_CORE_H
#define __TEE_CORE_H

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/idr.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/tee.h>
#include <linux/tee_drv.h>
#include <linux/types.h>
#include <linux/uuid.h>



#define TEE_SHM_DYNAMIC		BIT(0)  
					
#define TEE_SHM_USER_MAPPED	BIT(1)  
#define TEE_SHM_POOL		BIT(2)  
#define TEE_SHM_PRIV		BIT(3)  

#define TEE_DEVICE_FLAG_REGISTERED	0x1
#define TEE_MAX_DEV_NAME_LEN		32


struct tee_device {
	char name[TEE_MAX_DEV_NAME_LEN];
	const struct tee_desc *desc;
	int id;
	unsigned int flags;

	struct device dev;
	struct cdev cdev;

	size_t num_users;
	struct completion c_no_users;
	struct mutex mutex;	

	struct idr idr;
	struct tee_shm_pool *pool;
};


struct tee_driver_ops {
	void (*get_version)(struct tee_device *teedev,
			    struct tee_ioctl_version_data *vers);
	int (*open)(struct tee_context *ctx);
	void (*release)(struct tee_context *ctx);
	int (*open_session)(struct tee_context *ctx,
			    struct tee_ioctl_open_session_arg *arg,
			    struct tee_param *param);
	int (*close_session)(struct tee_context *ctx, u32 session);
	int (*system_session)(struct tee_context *ctx, u32 session);
	int (*invoke_func)(struct tee_context *ctx,
			   struct tee_ioctl_invoke_arg *arg,
			   struct tee_param *param);
	int (*cancel_req)(struct tee_context *ctx, u32 cancel_id, u32 session);
	int (*supp_recv)(struct tee_context *ctx, u32 *func, u32 *num_params,
			 struct tee_param *param);
	int (*supp_send)(struct tee_context *ctx, u32 ret, u32 num_params,
			 struct tee_param *param);
	int (*shm_register)(struct tee_context *ctx, struct tee_shm *shm,
			    struct page **pages, size_t num_pages,
			    unsigned long start);
	int (*shm_unregister)(struct tee_context *ctx, struct tee_shm *shm);
};


#define TEE_DESC_PRIVILEGED	0x1
struct tee_desc {
	const char *name;
	const struct tee_driver_ops *ops;
	struct module *owner;
	u32 flags;
};


struct tee_device *tee_device_alloc(const struct tee_desc *teedesc,
				    struct device *dev,
				    struct tee_shm_pool *pool,
				    void *driver_data);


int tee_device_register(struct tee_device *teedev);


void tee_device_unregister(struct tee_device *teedev);


void tee_device_set_dev_groups(struct tee_device *teedev,
			       const struct attribute_group **dev_groups);


int tee_session_calc_client_uuid(uuid_t *uuid, u32 connection_method,
				 const u8 connection_data[TEE_IOCTL_UUID_LEN]);


struct tee_shm_pool {
	const struct tee_shm_pool_ops *ops;
	void *private_data;
};


struct tee_shm_pool_ops {
	int (*alloc)(struct tee_shm_pool *pool, struct tee_shm *shm,
		     size_t size, size_t align);
	void (*free)(struct tee_shm_pool *pool, struct tee_shm *shm);
	void (*destroy_pool)(struct tee_shm_pool *pool);
};


struct tee_shm_pool *tee_shm_pool_alloc_res_mem(unsigned long vaddr,
						phys_addr_t paddr, size_t size,
						int min_alloc_order);


static inline void tee_shm_pool_free(struct tee_shm_pool *pool)
{
	pool->ops->destroy_pool(pool);
}


void *tee_get_drvdata(struct tee_device *teedev);


struct tee_shm *tee_shm_alloc_priv_buf(struct tee_context *ctx, size_t size);

int tee_dyn_shm_alloc_helper(struct tee_shm *shm, size_t size, size_t align,
			     int (*shm_register)(struct tee_context *ctx,
						 struct tee_shm *shm,
						 struct page **pages,
						 size_t num_pages,
						 unsigned long start));
void tee_dyn_shm_free_helper(struct tee_shm *shm,
			     int (*shm_unregister)(struct tee_context *ctx,
						   struct tee_shm *shm));


static inline bool tee_shm_is_dynamic(struct tee_shm *shm)
{
	return shm && (shm->flags & TEE_SHM_DYNAMIC);
}


void tee_shm_put(struct tee_shm *shm);


static inline int tee_shm_get_id(struct tee_shm *shm)
{
	return shm->id;
}


struct tee_shm *tee_shm_get_from_id(struct tee_context *ctx, int id);

static inline bool tee_param_is_memref(struct tee_param *param)
{
	switch (param->attr & TEE_IOCTL_PARAM_ATTR_TYPE_MASK) {
	case TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT:
	case TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT:
	case TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT:
		return true;
	default:
		return false;
	}
}


struct tee_context *teedev_open(struct tee_device *teedev);


void teedev_close_context(struct tee_context *ctx);

#endif 
