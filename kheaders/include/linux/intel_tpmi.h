/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _INTEL_TPMI_H_
#define _INTEL_TPMI_H_

#include <linux/bitfield.h>

#define TPMI_VERSION_INVALID	0xff
#define TPMI_MINOR_VERSION(val)	FIELD_GET(GENMASK(4, 0), val)
#define TPMI_MAJOR_VERSION(val)	FIELD_GET(GENMASK(7, 5), val)


enum intel_tpmi_id {
	TPMI_ID_RAPL = 0,	
	TPMI_ID_PEM = 1,	
	TPMI_ID_UNCORE = 2,	
	TPMI_ID_SST = 5,	
	TPMI_ID_PLR = 0xc,	
	TPMI_CONTROL_ID = 0x80,	
	TPMI_INFO_ID = 0x81,	
};


struct intel_tpmi_plat_info {
	u16 cdie_mask;
	u8 package_id;
	u8 partition;
	u8 segment;
	u8 bus_number;
	u8 device_number;
	u8 function_number;
};

struct intel_tpmi_plat_info *tpmi_get_platform_data(struct auxiliary_device *auxdev);
struct resource *tpmi_get_resource_at_index(struct auxiliary_device *auxdev, int index);
int tpmi_get_resource_count(struct auxiliary_device *auxdev);
int tpmi_get_feature_status(struct auxiliary_device *auxdev, int feature_id, bool *read_blocked,
			    bool *write_blocked);
struct dentry *tpmi_get_debugfs_dir(struct auxiliary_device *auxdev);
#endif
