/* SPDX-License-Identifier: GPL-2.0 or MIT */



#ifndef __DRM_PANIC_H__
#define __DRM_PANIC_H__

#include <linux/module.h>
#include <linux/types.h>
#include <linux/iosys-map.h>

#include <drm/drm_device.h>
#include <drm/drm_fourcc.h>


struct drm_scanout_buffer {
	
	const struct drm_format_info *format;

	
	struct iosys_map map[DRM_FORMAT_MAX_PLANES];

	
	unsigned int width;

	
	unsigned int height;

	
	unsigned int pitch[DRM_FORMAT_MAX_PLANES];

	
	void (*set_pixel)(struct drm_scanout_buffer *sb, unsigned int x,
			  unsigned int y, u32 color);

};

#ifdef CONFIG_DRM_PANIC


#define drm_panic_trylock(dev, flags) \
	raw_spin_trylock_irqsave(&(dev)->mode_config.panic_lock, flags)



#define drm_panic_lock(dev, flags) \
	raw_spin_lock_irqsave(&(dev)->mode_config.panic_lock, flags)


#define drm_panic_unlock(dev, flags) \
	raw_spin_unlock_irqrestore(&(dev)->mode_config.panic_lock, flags)

#else

static inline bool drm_panic_trylock(struct drm_device *dev, unsigned long flags)
{
	return true;
}

static inline void drm_panic_lock(struct drm_device *dev, unsigned long flags) {}
static inline void drm_panic_unlock(struct drm_device *dev, unsigned long flags) {}

#endif

#endif 
