/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __QCOM_SOCINFO_H__
#define __QCOM_SOCINFO_H__

#include <linux/types.h>


#define SMEM_HW_SW_BUILD_ID		137

#define SMEM_SOCINFO_BUILD_ID_LENGTH	32
#define SMEM_SOCINFO_CHIP_ID_LENGTH	32


#define SOCINFO_MAJOR(ver) (((ver) >> 16) & 0xffff)
#define SOCINFO_MINOR(ver) ((ver) & 0xffff)
#define SOCINFO_VERSION(maj, min)  ((((maj) & 0xffff) << 16)|((min) & 0xffff))


struct socinfo {
	__le32 fmt;
	__le32 id;
	__le32 ver;
	char build_id[SMEM_SOCINFO_BUILD_ID_LENGTH];
	
	__le32 raw_id;
	__le32 raw_ver;
	
	__le32 hw_plat;
	
	__le32 plat_ver;
	
	__le32 accessory_chip;
	
	__le32 hw_plat_subtype;
	
	__le32 pmic_model;
	__le32 pmic_die_rev;
	
	__le32 pmic_model_1;
	__le32 pmic_die_rev_1;
	__le32 pmic_model_2;
	__le32 pmic_die_rev_2;
	
	__le32 foundry_id;
	
	__le32 serial_num;
	
	__le32 num_pmics;
	__le32 pmic_array_offset;
	
	__le32 chip_family;
	__le32 raw_device_family;
	__le32 raw_device_num;
	
	__le32 nproduct_id;
	char chip_id[SMEM_SOCINFO_CHIP_ID_LENGTH];
	
	__le32 num_clusters;
	__le32 ncluster_array_offset;
	__le32 num_subset_parts;
	__le32 nsubset_parts_array_offset;
	
	__le32 nmodem_supported;
	
	__le32  feature_code;
	__le32  pcode;
	__le32  npartnamemap_offset;
	__le32  nnum_partname_mapping;
	
	__le32 oem_variant;
	
	__le32 num_kvps;
	__le32 kvps_offset;
	
	__le32 num_func_clusters;
	__le32 boot_cluster;
	__le32 boot_core;
};


enum qcom_socinfo_feature_code {
	
	SOCINFO_FC_UNKNOWN = 0x0,
	SOCINFO_FC_AA,
	SOCINFO_FC_AB,
	SOCINFO_FC_AC,
	SOCINFO_FC_AD,
	SOCINFO_FC_AE,
	SOCINFO_FC_AF,
	SOCINFO_FC_AG,
	SOCINFO_FC_AH,
};



#define SOCINFO_FC_Yn(n)		(0xf1 + (n))
#define SOCINFO_FC_INT_MAX		SOCINFO_FC_Yn(0xf)


#define SOCINFO_PC_UNKNOWN		0
#define SOCINFO_PCn(n)			((n) + 1)
#define SOCINFO_PC_RESERVE		(BIT(31) - 1)

#endif
