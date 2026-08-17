/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __TEE_DRV_H
#define __TEE_DRV_H

#include <linux/device.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/mod_devicetable.h>
#include <linux/tee.h>
#include <linux/types.h>



struct tee_device;


struct tee_context {
	struct tee_device *teedev;
	void *data;
	struct kref refcount;
	bool releasing;
	bool supp_nowait;
	bool cap_memref_null;
};


struct tee_shm {
	struct tee_context *ctx;
	phys_addr_t paddr;
	void *kaddr;
	size_t size;
	unsigned int offset;
	struct page **pages;
	size_t num_pages;
	refcount_t refcount;
	u32 flags;
	int id;
	u64 sec_world_id;
};

struct tee_param_memref {
	size_t shm_offs;
	size_t size;
	struct tee_shm *shm;
};

struct tee_param_value {
	u64 a;
	u64 b;
	u64 c;
};

struct tee_param {
	u64 attr;
	union {
		struct tee_param_memref memref;
		struct tee_param_value value;
	} u;
};


struct tee_shm *tee_shm_alloc_kernel_buf(struct tee_context *ctx, size_t size);


struct tee_shm *tee_shm_register_kernel_buf(struct tee_context *ctx,
					    void *addr, size_t length);


void tee_shm_free(struct tee_shm *shm);


void *tee_shm_get_va(struct tee_shm *shm, size_t offs);


int tee_shm_get_pa(struct tee_shm *shm, size_t offs, phys_addr_t *pa);


static inline size_t tee_shm_get_size(struct tee_shm *shm)
{
	return shm->size;
}


static inline struct page **tee_shm_get_pages(struct tee_shm *shm,
					      size_t *num_pages)
{
	*num_pages = shm->num_pages;
	return shm->pages;
}


static inline size_t tee_shm_get_page_offset(struct tee_shm *shm)
{
	return shm->offset;
}


struct tee_context *
tee_client_open_context(struct tee_context *start,
			int (*match)(struct tee_ioctl_version_data *,
				     const void *),
			const void *data, struct tee_ioctl_version_data *vers);


void tee_client_close_context(struct tee_context *ctx);


void tee_client_get_version(struct tee_context *ctx,
			    struct tee_ioctl_version_data *vers);


int tee_client_open_session(struct tee_context *ctx,
			    struct tee_ioctl_open_session_arg *arg,
			    struct tee_param *param);


int tee_client_close_session(struct tee_context *ctx, u32 session);


int tee_client_system_session(struct tee_context *ctx, u32 session);


int tee_client_invoke_func(struct tee_context *ctx,
			   struct tee_ioctl_invoke_arg *arg,
			   struct tee_param *param);


int tee_client_cancel_req(struct tee_context *ctx,
			  struct tee_ioctl_cancel_arg *arg);

extern const struct bus_type tee_bus_type;


struct tee_client_device {
	struct tee_client_device_id id;
	struct device dev;
};

#define to_tee_client_device(d) container_of(d, struct tee_client_device, dev)


struct tee_client_driver {
	const struct tee_client_device_id *id_table;
	struct device_driver driver;
};

#define to_tee_client_driver(d) \
		container_of_const(d, struct tee_client_driver, driver)

#endif 
