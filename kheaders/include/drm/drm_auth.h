#ifndef _DRM_AUTH_H_
#define _DRM_AUTH_H_



#include <linux/idr.h>
#include <linux/kref.h>
#include <linux/wait.h>

struct drm_file;


struct drm_master {
	struct kref refcount;
	struct drm_device *dev;
	
	char *unique;
	
	int unique_len;
	
	struct idr magic_map;
	void *driver_priv;

	
	struct drm_master *lessor;

	
	int	lessee_id;

	
	struct list_head lessee_list;

	
	struct list_head lessees;

	
	struct idr leases;

	
	struct idr lessee_idr;
};

struct drm_master *drm_master_get(struct drm_master *master);
struct drm_master *drm_file_get_master(struct drm_file *file_priv);
void drm_master_put(struct drm_master **master);
bool drm_is_current_master(struct drm_file *fpriv);

struct drm_master *drm_master_create(struct drm_device *dev);

#endif
