/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_CXL_EVENT_H
#define _LINUX_CXL_EVENT_H

#include <linux/types.h>
#include <linux/uuid.h>
#include <linux/workqueue_types.h>


struct cxl_event_record_hdr {
	u8 length;
	u8 flags[3];
	__le16 handle;
	__le16 related_handle;
	__le64 timestamp;
	u8 maint_op_class;
	u8 reserved[15];
} __packed;

struct cxl_event_media_hdr {
	struct cxl_event_record_hdr hdr;
	__le64 phys_addr;
	u8 descriptor;
	u8 type;
	u8 transaction_type;
	
	u8 validity_flags[2];
	u8 channel;
	u8 rank;
} __packed;

#define CXL_EVENT_RECORD_DATA_LENGTH 0x50
struct cxl_event_generic {
	struct cxl_event_record_hdr hdr;
	u8 data[CXL_EVENT_RECORD_DATA_LENGTH];
} __packed;


#define CXL_EVENT_GEN_MED_COMP_ID_SIZE	0x10
struct cxl_event_gen_media {
	struct cxl_event_media_hdr media_hdr;
	u8 device[3];
	u8 component_id[CXL_EVENT_GEN_MED_COMP_ID_SIZE];
	u8 reserved[46];
} __packed;


#define CXL_EVENT_DER_CORRECTION_MASK_SIZE	0x20
struct cxl_event_dram {
	struct cxl_event_media_hdr media_hdr;
	u8 nibble_mask[3];
	u8 bank_group;
	u8 bank;
	u8 row[3];
	u8 column[2];
	u8 correction_mask[CXL_EVENT_DER_CORRECTION_MASK_SIZE];
	u8 reserved[0x17];
} __packed;


struct cxl_get_health_info {
	u8 health_status;
	u8 media_status;
	u8 add_status;
	u8 life_used;
	u8 device_temp[2];
	u8 dirty_shutdown_cnt[4];
	u8 cor_vol_err_cnt[4];
	u8 cor_per_err_cnt[4];
} __packed;


struct cxl_event_mem_module {
	struct cxl_event_record_hdr hdr;
	u8 event_type;
	struct cxl_get_health_info info;
	u8 reserved[0x3d];
} __packed;

union cxl_event {
	struct cxl_event_generic generic;
	struct cxl_event_gen_media gen_media;
	struct cxl_event_dram dram;
	struct cxl_event_mem_module mem_module;
	
	struct cxl_event_media_hdr media_hdr;
} __packed;


struct cxl_event_record_raw {
	uuid_t id;
	union cxl_event event;
} __packed;

enum cxl_event_type {
	CXL_CPER_EVENT_GENERIC,
	CXL_CPER_EVENT_GEN_MEDIA,
	CXL_CPER_EVENT_DRAM,
	CXL_CPER_EVENT_MEM_MODULE,
};

#define CPER_CXL_DEVICE_ID_VALID		BIT(0)
#define CPER_CXL_DEVICE_SN_VALID		BIT(1)
#define CPER_CXL_COMP_EVENT_LOG_VALID		BIT(2)
struct cxl_cper_event_rec {
	struct {
		u32 length;
		u64 validation_bits;
		struct cper_cxl_event_devid {
			u16 vendor_id;
			u16 device_id;
			u8 func_num;
			u8 device_num;
			u8 bus_num;
			u16 segment_num;
			u16 slot_num; 
			u8 reserved;
		} __packed device_id;
		struct cper_cxl_event_sn {
			u32 lower_dw;
			u32 upper_dw;
		} __packed dev_serial_num;
	} __packed hdr;

	union cxl_event event;
} __packed;

struct cxl_cper_work_data {
	enum cxl_event_type event_type;
	struct cxl_cper_event_rec rec;
};

#ifdef CONFIG_ACPI_APEI_GHES
int cxl_cper_register_work(struct work_struct *work);
int cxl_cper_unregister_work(struct work_struct *work);
int cxl_cper_kfifo_get(struct cxl_cper_work_data *wd);
#else
static inline int cxl_cper_register_work(struct work_struct *work)
{
	return 0;
}

static inline int cxl_cper_unregister_work(struct work_struct *work)
{
	return 0;
}
static inline int cxl_cper_kfifo_get(struct cxl_cper_work_data *wd)
{
	return 0;
}
#endif

#endif 
