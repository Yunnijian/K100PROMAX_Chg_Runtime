/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _LINUX_GUNYAH_H
#define _LINUX_GUNYAH_H

#include <linux/bitfield.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/mod_devicetable.h>
#include <linux/types.h>

#include <uapi/linux/gunyah.h>

struct gunyah_vm;

int __must_check gunyah_vm_get(struct gunyah_vm *ghvm);
void gunyah_vm_put(struct gunyah_vm *ghvm);
int gunyah_reclaim_fw_parcel(struct gunyah_vm *ghvm, u32 mem_handle);


struct gunyah_auth_vm_mgr_ops {
	u16 (*pre_alloc_vmid)(struct gunyah_vm *ghvm);
	int (*pre_vm_configure)(struct gunyah_vm *ghvm);
	int (*vm_authenticate)(struct gunyah_vm *ghvm);
	int (*pre_vm_init)(struct gunyah_vm *ghvm);
	int (*pre_vm_start)(struct gunyah_vm *ghvm);
	int (*pre_vm_reset)(struct gunyah_vm *ghvm);
	int (*post_vm_reset)(struct gunyah_vm *ghvm);
	void (*vm_start_fail)(struct gunyah_vm *ghvm);
};


struct gunyah_auth_vm_mgr {
	u32 type;
	const char *name;
	struct module *mod;
	long (*vm_attach)(struct gunyah_vm *ghvm, struct gunyah_auth_desc *d);
	void (*vm_detach)(struct gunyah_vm *ghvm);
};
int gunyah_auth_vm_mgr_register(struct gunyah_auth_vm_mgr *auth_vm);
void gunyah_auth_vm_mgr_unregister(struct gunyah_auth_vm_mgr *auth_vm);

struct gunyah_vm_function_instance;

struct gunyah_vm_function {
	u32 type;
	const char *name;
	struct module *mod;
	long (*bind)(struct gunyah_vm_function_instance *f);
	void (*unbind)(struct gunyah_vm_function_instance *f);
	bool (*compare)(const struct gunyah_vm_function_instance *f,
			const void *arg, size_t size);
};


struct gunyah_vm_function_instance {
	size_t arg_size;
	void *argp;
	struct gunyah_vm *ghvm;
	struct gunyah_rm *rm;
	struct gunyah_vm_function *fn;
	void *data;
	struct list_head vm_list;
};

int gunyah_vm_function_register(struct gunyah_vm_function *f);
void gunyah_vm_function_unregister(struct gunyah_vm_function *f);


#define MODULE_ALIAS_GUNYAH_VM_FUNCTION(_type, _idx)        \
	static inline void __maybe_unused __chk##_idx(void) \
	{                                                   \
		BUILD_BUG_ON(_type != _idx);                \
	}                                                   \
	MODULE_ALIAS("ghfunc:" __stringify(_idx))

#define DECLARE_GUNYAH_VM_FUNCTION(_name, _type, _bind, _unbind, _compare) \
	static struct gunyah_vm_function _name = {                         \
		.type = _type,                                             \
		.name = __stringify(_name),                                \
		.mod = THIS_MODULE,                                        \
		.bind = _bind,                                             \
		.unbind = _unbind,                                         \
		.compare = _compare,                                       \
	}

#define module_gunyah_vm_function(__gf)                  \
	module_driver(__gf, gunyah_vm_function_register, \
		      gunyah_vm_function_unregister)

#define DECLARE_GUNYAH_VM_FUNCTION_INIT(_name, _type, _idx, _bind, _unbind, \
					_compare)                           \
	DECLARE_GUNYAH_VM_FUNCTION(_name, _type, _bind, _unbind, _compare); \
	module_gunyah_vm_function(_name);                                   \
	MODULE_ALIAS_GUNYAH_VM_FUNCTION(_type, _idx)


enum gunyah_resource_type {
	
	GUNYAH_RESOURCE_TYPE_BELL_TX	= 0,
	GUNYAH_RESOURCE_TYPE_BELL_RX	= 1,
	GUNYAH_RESOURCE_TYPE_MSGQ_TX	= 2,
	GUNYAH_RESOURCE_TYPE_MSGQ_RX	= 3,
	GUNYAH_RESOURCE_TYPE_VCPU	= 4,
	GUNYAH_RESOURCE_TYPE_MEM_EXTENT	= 9,
	GUNYAH_RESOURCE_TYPE_ADDR_SPACE	= 10,
	
};

struct gunyah_resource {
	enum gunyah_resource_type type;
	u64 capid;
	unsigned int irq;

	struct list_head list;
	u32 rm_label;
};


struct gunyah_vm_resource_ticket {
	struct list_head vm_list;
	struct list_head resources;
	enum gunyah_resource_type resource_type;
	u32 label;

	struct module *owner;
	bool (*populate)(struct gunyah_vm_resource_ticket *ticket,
			 struct gunyah_resource *ghrsc);
	void (*unpopulate)(struct gunyah_vm_resource_ticket *ticket,
			   struct gunyah_resource *ghrsc);
};

int gunyah_vm_add_resource_ticket(struct gunyah_vm *ghvm,
				  struct gunyah_vm_resource_ticket *ticket);
void gunyah_vm_remove_resource_ticket(struct gunyah_vm *ghvm,
				      struct gunyah_vm_resource_ticket *ticket);


struct gunyah_vm_io_handler {
	struct rb_node node;
	u64 addr;

	bool datamatch;
	u8 len;
	u64 data;
	struct gunyah_vm_io_handler_ops *ops;
};


struct gunyah_vm_io_handler_ops {
	int (*read)(struct gunyah_vm_io_handler *io_dev, u64 addr, u32 len,
		    u64 data);
	int (*write)(struct gunyah_vm_io_handler *io_dev, u64 addr, u32 len,
		     u64 data);
};

int gunyah_vm_add_io_handler(struct gunyah_vm *ghvm,
			     struct gunyah_vm_io_handler *io_dev);
void gunyah_vm_remove_io_handler(struct gunyah_vm *ghvm,
				 struct gunyah_vm_io_handler *io_dev);

#define GUNYAH_RM_ACL_X BIT(0)
#define GUNYAH_RM_ACL_W BIT(1)
#define GUNYAH_RM_ACL_R BIT(2)

struct gunyah_rm_mem_acl_entry {
	__le16 vmid;
	u8 perms;
	u8 reserved;
} __packed;

struct gunyah_rm_mem_entry {
	__le64 phys_addr;
	__le64 size;
} __packed;

enum gunyah_rm_mem_type {
	GUNYAH_RM_MEM_TYPE_NORMAL = 0,
	GUNYAH_RM_MEM_TYPE_IO = 1,
};


struct gunyah_rm_mem_parcel {
	enum gunyah_rm_mem_type mem_type;
	u32 label;
	size_t n_acl_entries;
	struct gunyah_rm_mem_acl_entry *acl_entries;
	size_t n_mem_entries;
	struct gunyah_rm_mem_entry *mem_entries;
	u32 mem_handle;
};

enum gunyah_pagetable_access {
	
	GUNYAH_PAGETABLE_ACCESS_NONE		= 0,
	GUNYAH_PAGETABLE_ACCESS_X		= 1,
	GUNYAH_PAGETABLE_ACCESS_W		= 2,
	GUNYAH_PAGETABLE_ACCESS_R		= 4,
	GUNYAH_PAGETABLE_ACCESS_RX		= 5,
	GUNYAH_PAGETABLE_ACCESS_RW		= 6,
	GUNYAH_PAGETABLE_ACCESS_RWX		= 7,
	
};

struct gunyah_rm_platform_ops {
	int (*pre_mem_share)(struct gunyah_rm *rm,
			     struct gunyah_rm_mem_parcel *mem_parcel);
	int (*post_mem_reclaim)(struct gunyah_rm *rm,
				struct gunyah_rm_mem_parcel *mem_parcel);

	int (*pre_demand_page)(struct gunyah_rm *rm, u16 vmid,
			       enum gunyah_pagetable_access access,
			       struct folio *folio);
	int (*release_demand_page)(struct gunyah_rm *rm, u16 vmid,
				   enum gunyah_pagetable_access access,
				   struct folio *folio);
};

#if IS_ENABLED(CONFIG_GUNYAH_PLATFORM_HOOKS)
int gunyah_rm_register_platform_ops(
	const struct gunyah_rm_platform_ops *platform_ops);
void gunyah_rm_unregister_platform_ops(
	const struct gunyah_rm_platform_ops *platform_ops);
int devm_gunyah_rm_register_platform_ops(
	struct device *dev, const struct gunyah_rm_platform_ops *ops);
#else
static inline int gunyah_rm_register_platform_ops(
	const struct gunyah_rm_platform_ops *platform_ops)
{
	return 0;
}
static inline void gunyah_rm_unregister_platform_ops(
	const struct gunyah_rm_platform_ops *platform_ops)
{
}
static inline int
devm_gunyah_rm_register_platform_ops(struct device *dev,
				     const struct gunyah_rm_platform_ops *ops)
{
	return 0;
}
#endif



#define GUNYAH_CAPID_INVAL U64_MAX
#define GUNYAH_VMID_ROOT_VM 0xff

enum gunyah_error {
	
	GUNYAH_ERROR_OK				= 0,
	GUNYAH_ERROR_UNIMPLEMENTED		= -1,
	GUNYAH_ERROR_RETRY			= -2,

	GUNYAH_ERROR_ARG_INVAL			= 1,
	GUNYAH_ERROR_ARG_SIZE			= 2,
	GUNYAH_ERROR_ARG_ALIGN			= 3,

	GUNYAH_ERROR_NOMEM			= 10,

	GUNYAH_ERROR_ADDR_OVFL			= 20,
	GUNYAH_ERROR_ADDR_UNFL			= 21,
	GUNYAH_ERROR_ADDR_INVAL			= 22,

	GUNYAH_ERROR_DENIED			= 30,
	GUNYAH_ERROR_BUSY			= 31,
	GUNYAH_ERROR_IDLE			= 32,

	GUNYAH_ERROR_IRQ_BOUND			= 40,
	GUNYAH_ERROR_IRQ_UNBOUND		= 41,

	GUNYAH_ERROR_CSPACE_CAP_NULL		= 50,
	GUNYAH_ERROR_CSPACE_CAP_REVOKED		= 51,
	GUNYAH_ERROR_CSPACE_WRONG_OBJ_TYPE	= 52,
	GUNYAH_ERROR_CSPACE_INSUF_RIGHTS	= 53,
	GUNYAH_ERROR_CSPACE_FULL		= 54,

	GUNYAH_ERROR_MSGQUEUE_EMPTY		= 60,
	GUNYAH_ERROR_MSGQUEUE_FULL		= 61,
	
};


static inline int gunyah_error_remap(enum gunyah_error gunyah_error)
{
	switch (gunyah_error) {
	case GUNYAH_ERROR_OK:
		return 0;
	case GUNYAH_ERROR_NOMEM:
		return -ENOMEM;
	case GUNYAH_ERROR_DENIED:
	case GUNYAH_ERROR_CSPACE_CAP_NULL:
	case GUNYAH_ERROR_CSPACE_CAP_REVOKED:
	case GUNYAH_ERROR_CSPACE_WRONG_OBJ_TYPE:
	case GUNYAH_ERROR_CSPACE_INSUF_RIGHTS:
		return -EACCES;
	case GUNYAH_ERROR_CSPACE_FULL:
	case GUNYAH_ERROR_BUSY:
	case GUNYAH_ERROR_IDLE:
		return -EBUSY;
	case GUNYAH_ERROR_IRQ_BOUND:
	case GUNYAH_ERROR_IRQ_UNBOUND:
	case GUNYAH_ERROR_MSGQUEUE_FULL:
	case GUNYAH_ERROR_MSGQUEUE_EMPTY:
		return -EIO;
	case GUNYAH_ERROR_UNIMPLEMENTED:
		return -EOPNOTSUPP;
	case GUNYAH_ERROR_RETRY:
		return -EAGAIN;
	default:
		return -EINVAL;
	}
}

enum gunyah_api_feature {
	
	GUNYAH_FEATURE_DOORBELL		= 1,
	GUNYAH_FEATURE_MSGQUEUE		= 2,
	GUNYAH_FEATURE_VCPU		= 5,
	GUNYAH_FEATURE_MEMEXTENT	= 6,
	
};

bool arch_is_gunyah_guest(void);

enum gunyah_info_owner {
	
	GUNYAH_INFO_OWNER_INVALID	= 0,
	GUNYAH_INFO_OWNER_HYP		= 1,
	GUNYAH_INFO_OWNER_ROOTVM	= 2,
	GUNYAH_INFO_OWNER_RM		= 3,
	GUNYAH_INFO_OWNER_QCRM		= 16,
	
};

void *gunyah_get_info(u16 owner, u16 id, size_t *size);
int gunyah_map_addrspace_info_area(void);
void gunyah_unmap_addrspace_info_area(void);

#define GUNYAH_API_V1 1



#define GUNYAH_API_INFO_API_VERSION_MASK	GENMASK_ULL(13, 0)
#define GUNYAH_API_INFO_BIG_ENDIAN		BIT_ULL(14)
#define GUNYAH_API_INFO_IS_64BIT		BIT_ULL(15)
#define GUNYAH_API_INFO_VARIANT_MASK 		GENMASK_ULL(63, 56)


struct gunyah_hypercall_hyp_identify_resp {
	u64 api_info;
	u64 flags[3];
};

static inline u16
gunyah_api_version(const struct gunyah_hypercall_hyp_identify_resp *gunyah_api)
{
	return FIELD_GET(GUNYAH_API_INFO_API_VERSION_MASK,
			 gunyah_api->api_info);
}

void gunyah_hypercall_hyp_identify(
	struct gunyah_hypercall_hyp_identify_resp *hyp_identity);

enum gunyah_error gunyah_hypercall_bell_send(u64 capid, u64 new_flags,
					     u64 *old_flags);
enum gunyah_error gunyah_hypercall_bell_set_mask(u64 capid, u64 enable_mask,
						 u64 ack_mask);


#define GUNYAH_HYPERCALL_MSGQ_TX_FLAGS_PUSH BIT(0)

enum gunyah_error gunyah_hypercall_msgq_send(u64 capid, size_t size, void *buff,
					     u64 tx_flags, bool *ready);
enum gunyah_error gunyah_hypercall_msgq_recv(u64 capid, void *buff, size_t size,
					     size_t *recv_size, bool *ready);

#define GUNYAH_ADDRSPACE_SELF_CAP 0


#define GUNYAH_MEMEXTENT_MAPPING_USER_ACCESS		GENMASK_ULL(2, 0)
#define GUNYAH_MEMEXTENT_MAPPING_KERNEL_ACCESS		GENMASK_ULL(6, 4)
#define GUNYAH_MEMEXTENT_MAPPING_TYPE			GENMASK_ULL(23, 16)


enum gunyah_memextent_donate_type {
	
	GUNYAH_MEMEXTENT_DONATE_TO_CHILD		= 0,
	GUNYAH_MEMEXTENT_DONATE_TO_PARENT		= 1,
	GUNYAH_MEMEXTENT_DONATE_TO_SIBLING		= 2,
	GUNYAH_MEMEXTENT_DONATE_TO_PROTECTED		= 3,
	GUNYAH_MEMEXTENT_DONATE_FROM_PROTECTED		= 4,
	
};

enum gunyah_addrspace_map_flag_bits {
	
	GUNYAH_ADDRSPACE_MAP_FLAG_PARTIAL	= 0,
	GUNYAH_ADDRSPACE_MAP_FLAG_PRIVATE	= 1,
	GUNYAH_ADDRSPACE_MAP_FLAG_VMMIO		= 2,
	GUNYAH_ADDRSPACE_MAP_FLAG_NOSYNC	= 31,
	
};

enum gunyah_error gunyah_hypercall_addrspace_map(u64 capid, u64 extent_capid,
						 u64 vbase, u32 extent_attrs,
						 u32 flags, u64 offset,
						 u64 size);
enum gunyah_error gunyah_hypercall_addrspace_unmap(u64 capid, u64 extent_capid,
						   u64 vbase, u32 flags,
						   u64 offset, u64 size);


#define GUNYAH_MEMEXTENT_OPTION_TYPE_MASK	GENMASK_ULL(7, 0)
#define GUNYAH_MEMEXTENT_OPTION_NOSYNC		BIT(31)


enum gunyah_error gunyah_hypercall_memextent_donate(u32 options, u64 from_capid,
						    u64 to_capid, u64 offset,
						    u64 size);

struct gunyah_hypercall_vcpu_run_resp {
	union {
		enum {
			
			
			GUNYAH_VCPU_STATE_READY			= 0,
			
			GUNYAH_VCPU_STATE_EXPECTS_WAKEUP	= 1,
			
			GUNYAH_VCPU_STATE_POWERED_OFF		= 2,
			
			GUNYAH_VCPU_STATE_BLOCKED		= 3,
			
			GUNYAH_VCPU_ADDRSPACE_VMMIO_READ	= 4,
			
			GUNYAH_VCPU_ADDRSPACE_VMMIO_WRITE	= 5,
			
			GUNYAH_VCPU_ADDRSPACE_PAGE_FAULT	= 7,
			
			GUNYAH_VCPU_STATE_SYSTEM_OFF		= 0x100,
			
		} state;
		u64 sized_state;
	};
	u64 state_data[3];
};

enum {
	GUNYAH_ADDRSPACE_VMMIO_ACTION_EMULATE = 0,
	GUNYAH_ADDRSPACE_VMMIO_ACTION_RETRY = 1,
	GUNYAH_ADDRSPACE_VMMIO_ACTION_FAULT = 2,
};


struct gunyah_vcpu {
	struct gunyah_vm_function_instance *f;
	struct gunyah_resource *rsc;
	struct mutex run_lock;
	struct gunyah_vm *ghvm;

	struct gunyah_vcpu_run *vcpu_run;

	
	enum {
		GUNYAH_VCPU_RUN_STATE_UNKNOWN = 0,
		GUNYAH_VCPU_RUN_STATE_READY,
		GUNYAH_VCPU_RUN_STATE_MMIO_READ,
		GUNYAH_VCPU_RUN_STATE_MMIO_WRITE,
		GUNYAH_VCPU_RUN_STATE_SYSTEM_DOWN,
	} state;
	u8 mmio_read_len;
	u64 mmio_addr;

	struct completion ready;

	struct notifier_block nb;
	struct gunyah_vm_resource_ticket ticket;
	struct kref kref;
};

enum gunyah_error
gunyah_hypercall_vcpu_run(u64 capid, unsigned long *resume_data,
			  struct gunyah_hypercall_vcpu_run_resp *resp);

#define GUNYAH_ADDRSPC_MODIFY_FLAG_UNLOCK_BIT		0
#define GUNYAH_ADDRSPC_MODIFY_FLAG_SANITIZE_BIT		1
enum gunyah_error
gunyah_hypercall_addrspc_modify_pages(u64 capid, u64 addr, u64 size, u64 flags);
enum gunyah_error
gunyah_hypercall_addrspace_find_info_area(unsigned long *ipa, unsigned long *size);
enum gunyah_error
#define GUNYAH_ADDRSPACE_VMMIO_CONFIGURE_OP_ADD_RANGE	0
gunyah_hypercall_addrspc_configure_vmmio_range(u64 capid, u64 base, u64 size, u64 op);
#endif
