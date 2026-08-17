

#ifndef __LINUX_LSM_HOOKS_H
#define __LINUX_LSM_HOOKS_H

#include <uapi/linux/lsm.h>
#include <linux/security.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/xattr.h>
#include <linux/static_call.h>
#include <linux/unroll.h>
#include <linux/jump_label.h>
#include <linux/lsm_count.h>

union security_list_options {
	#define LSM_HOOK(RET, DEFAULT, NAME, ...) RET (*NAME)(__VA_ARGS__);
	#include "lsm_hook_defs.h"
	#undef LSM_HOOK
	void *lsm_func_addr;
};


struct lsm_static_call {
	struct static_call_key *key;
	void *trampoline;
	struct security_hook_list *hl;
	
	struct static_key_false *active;
} __randomize_layout;


struct lsm_static_calls_table {
	#define LSM_HOOK(RET, DEFAULT, NAME, ...) \
		struct lsm_static_call NAME[MAX_LSM_COUNT];
	#include <linux/lsm_hook_defs.h>
	#undef LSM_HOOK
} __packed __randomize_layout;


struct lsm_id {
	const char *name;
	u64 id;
};


struct security_hook_list {
	struct lsm_static_call *scalls;
	union security_list_options hook;
	const struct lsm_id *lsmid;
} __randomize_layout;


struct lsm_blob_sizes {
	int lbs_cred;
	int lbs_file;
	int lbs_ib;
	int lbs_inode;
	int lbs_sock;
	int lbs_superblock;
	int lbs_ipc;
	int lbs_key;
	int lbs_msg_msg;
	int lbs_perf_event;
	int lbs_task;
	int lbs_xattr_count; 
	int lbs_tun_dev;
	int lbs_bdev;
};


#define LSM_RET_VOID ((void) 0)


#define LSM_HOOK_INIT(NAME, HOOK)			\
	{						\
		.scalls = static_calls_table.NAME,	\
		.hook = { .NAME = HOOK }		\
	}

extern void security_add_hooks(struct security_hook_list *hooks, int count,
			       const struct lsm_id *lsmid);

#define LSM_FLAG_LEGACY_MAJOR	BIT(0)
#define LSM_FLAG_EXCLUSIVE	BIT(1)

enum lsm_order {
	LSM_ORDER_FIRST = -1,	
	LSM_ORDER_MUTABLE = 0,
	LSM_ORDER_LAST = 1,	
};

struct lsm_info {
	const char *name;	
	enum lsm_order order;	
	unsigned long flags;	
	int *enabled;		
	int (*init)(void);	
	struct lsm_blob_sizes *blobs; 
};

#define DEFINE_LSM(lsm)							\
	static struct lsm_info __lsm_##lsm				\
		__used __section(".lsm_info.init")			\
		__aligned(sizeof(unsigned long))

#define DEFINE_EARLY_LSM(lsm)						\
	static struct lsm_info __early_lsm_##lsm			\
		__used __section(".early_lsm_info.init")		\
		__aligned(sizeof(unsigned long))


extern char *lsm_names;
extern struct lsm_static_calls_table static_calls_table __ro_after_init;
extern struct lsm_info __start_lsm_info[], __end_lsm_info[];
extern struct lsm_info __start_early_lsm_info[], __end_early_lsm_info[];


static inline struct xattr *lsm_get_xattr_slot(struct xattr *xattrs,
					       int *xattr_count)
{
	if (unlikely(!xattrs))
		return NULL;
	return &xattrs[(*xattr_count)++];
}

#endif 
