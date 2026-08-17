/* SPDX-License-Identifier: MIT */


#ifndef __DRM_UTILS_H__
#define __DRM_UTILS_H__

#include <linux/types.h>

struct drm_edid;

int drm_get_panel_orientation_quirk(int width, int height);

int drm_get_panel_min_brightness_quirk(const struct drm_edid *edid);

signed long drm_timeout_abs_to_jiffies(int64_t timeout_nsec);

#endif
