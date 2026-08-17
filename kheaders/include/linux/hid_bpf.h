/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __HID_BPF_H
#define __HID_BPF_H

#include <linux/bpf.h>
#include <linux/mutex.h>
#include <linux/srcu.h>
#include <uapi/linux/hid.h>

struct hid_device;




struct hid_bpf_ctx {
	struct hid_device *hid;
	__u32 allocated_size;
	union {
		__s32 retval;
		__s32 size;
	};
};



#define HID_BPF_MAX_PROGS_PER_DEV 64
#define HID_BPF_FLAG_MASK (((HID_BPF_FLAG_MAX - 1) << 1) - 1)


struct hid_report_enum;

struct hid_ops {
	struct hid_report *(*hid_get_report)(struct hid_report_enum *report_enum, const u8 *data);
	int (*hid_hw_raw_request)(struct hid_device *hdev,
				  unsigned char reportnum, __u8 *buf,
				  size_t len, enum hid_report_type rtype,
				  enum hid_class_request reqtype,
				  u64 source, bool from_bpf);
	int (*hid_hw_output_report)(struct hid_device *hdev, __u8 *buf, size_t len,
				    u64 source, bool from_bpf);
	int (*hid_input_report)(struct hid_device *hid, enum hid_report_type type,
				u8 *data, u32 size, int interrupt, u64 source, bool from_bpf,
				bool lock_already_taken);
	struct module *owner;
	const struct bus_type *bus_type;
};

extern struct hid_ops *hid_ops;


struct hid_bpf_ops {
	
	int			hid_id;
	u32			flags;

	
	struct list_head	list;

	

	
	int (*hid_device_event)(struct hid_bpf_ctx *ctx, enum hid_report_type report_type,
				u64 source);

	
	int (*hid_rdesc_fixup)(struct hid_bpf_ctx *ctx);

	
	int (*hid_hw_request)(struct hid_bpf_ctx *ctx, unsigned char reportnum,
			       enum hid_report_type rtype, enum hid_class_request reqtype,
			       u64 source);

	
	int (*hid_hw_output_report)(struct hid_bpf_ctx *ctx, u64 source);


	
	struct hid_device *hdev;
};


struct hid_bpf {
	u8 *device_data;		
	u32 allocated_data;
	bool destroyed;			

	struct hid_bpf_ops *rdesc_ops;
	struct list_head prog_list;
	struct mutex prog_list_lock;	
	struct srcu_struct srcu;	
};

#ifdef CONFIG_HID_BPF
u8 *dispatch_hid_bpf_device_event(struct hid_device *hid, enum hid_report_type type, u8 *data,
				  u32 *size, int interrupt, u64 source, bool from_bpf);
int dispatch_hid_bpf_raw_requests(struct hid_device *hdev,
				  unsigned char reportnum, __u8 *buf,
				  u32 size, enum hid_report_type rtype,
				  enum hid_class_request reqtype,
				  u64 source, bool from_bpf);
int dispatch_hid_bpf_output_report(struct hid_device *hdev, __u8 *buf, u32 size,
				   u64 source, bool from_bpf);
int hid_bpf_connect_device(struct hid_device *hdev);
void hid_bpf_disconnect_device(struct hid_device *hdev);
void hid_bpf_destroy_device(struct hid_device *hid);
int hid_bpf_device_init(struct hid_device *hid);
u8 *call_hid_bpf_rdesc_fixup(struct hid_device *hdev, const u8 *rdesc, unsigned int *size);
#else 
static inline u8 *dispatch_hid_bpf_device_event(struct hid_device *hid, enum hid_report_type type,
						u8 *data, u32 *size, int interrupt,
						u64 source, bool from_bpf) { return data; }
static inline int dispatch_hid_bpf_raw_requests(struct hid_device *hdev,
						unsigned char reportnum, u8 *buf,
						u32 size, enum hid_report_type rtype,
						enum hid_class_request reqtype,
						u64 source, bool from_bpf) { return 0; }
static inline int dispatch_hid_bpf_output_report(struct hid_device *hdev, __u8 *buf, u32 size,
						 u64 source, bool from_bpf) { return 0; }
static inline int hid_bpf_connect_device(struct hid_device *hdev) { return 0; }
static inline void hid_bpf_disconnect_device(struct hid_device *hdev) {}
static inline void hid_bpf_destroy_device(struct hid_device *hid) {}
static inline int hid_bpf_device_init(struct hid_device *hid) { return 0; }

#define call_hid_bpf_rdesc_fixup(_hdev, _rdesc, _size)	\
		((u8 *)kmemdup(_rdesc, *(_size), GFP_KERNEL))

#endif 

#endif 
