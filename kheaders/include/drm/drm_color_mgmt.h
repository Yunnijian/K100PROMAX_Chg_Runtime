

#ifndef __DRM_COLOR_MGMT_H__
#define __DRM_COLOR_MGMT_H__

#include <linux/ctype.h>
#include <linux/math64.h>
#include <drm/drm_property.h>

struct drm_crtc;
struct drm_plane;


static inline u32 drm_color_lut_extract(u32 user_input, int bit_precision)
{
	if (bit_precision > 16)
		return DIV_ROUND_CLOSEST_ULL(mul_u32_u32(user_input, (1 << bit_precision) - 1),
					     (1 << 16) - 1);
	else
		return DIV_ROUND_CLOSEST(user_input * ((1 << bit_precision) - 1),
					 (1 << 16) - 1);
}

u64 drm_color_ctm_s31_32_to_qm_n(u64 user_input, u32 m, u32 n);

void drm_crtc_enable_color_mgmt(struct drm_crtc *crtc,
				uint degamma_lut_size,
				bool has_ctm,
				uint gamma_lut_size);

int drm_mode_crtc_set_gamma_size(struct drm_crtc *crtc,
				 int gamma_size);


static inline int drm_color_lut_size(const struct drm_property_blob *blob)
{
	return blob->length / sizeof(struct drm_color_lut);
}

enum drm_color_encoding {
	DRM_COLOR_YCBCR_BT601,
	DRM_COLOR_YCBCR_BT709,
	DRM_COLOR_YCBCR_BT2020,
	DRM_COLOR_ENCODING_MAX,
};

enum drm_color_range {
	DRM_COLOR_YCBCR_LIMITED_RANGE,
	DRM_COLOR_YCBCR_FULL_RANGE,
	DRM_COLOR_RANGE_MAX,
};

int drm_plane_create_color_properties(struct drm_plane *plane,
				      u32 supported_encodings,
				      u32 supported_ranges,
				      enum drm_color_encoding default_encoding,
				      enum drm_color_range default_range);


enum drm_color_lut_tests {
	
	DRM_COLOR_LUT_EQUAL_CHANNELS = BIT(0),

	
	DRM_COLOR_LUT_NON_DECREASING = BIT(1),
};

int drm_color_lut_check(const struct drm_property_blob *lut, u32 tests);
#endif
