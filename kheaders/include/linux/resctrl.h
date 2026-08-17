/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _RESCTRL_H
#define _RESCTRL_H

#include <linux/cacheinfo.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/pid.h>


#define RESCTRL_RESERVED_CLOSID		0
#define RESCTRL_RESERVED_RMID		0

#define RESCTRL_PICK_ANY_CPU		-1

#ifdef CONFIG_PROC_CPU_RESCTRL

int proc_resctrl_show(struct seq_file *m,
		      struct pid_namespace *ns,
		      struct pid *pid,
		      struct task_struct *tsk);

#endif


#define MBA_MAX_MBPS   U32_MAX


enum resctrl_conf_type {
	CDP_NONE,
	CDP_CODE,
	CDP_DATA,
};

#define CDP_NUM_TYPES	(CDP_DATA + 1)


enum resctrl_event_id {
	QOS_L3_OCCUP_EVENT_ID		= 0x01,
	QOS_L3_MBM_TOTAL_EVENT_ID	= 0x02,
	QOS_L3_MBM_LOCAL_EVENT_ID	= 0x03,
};


struct resctrl_staged_config {
	u32			new_ctrl;
	bool			have_new_ctrl;
};

enum resctrl_domain_type {
	RESCTRL_CTRL_DOMAIN,
	RESCTRL_MON_DOMAIN,
};


struct rdt_domain_hdr {
	struct list_head		list;
	int				id;
	enum resctrl_domain_type	type;
	struct cpumask			cpu_mask;
};


struct rdt_ctrl_domain {
	struct rdt_domain_hdr		hdr;
	struct pseudo_lock_region	*plr;
	struct resctrl_staged_config	staged_config[CDP_NUM_TYPES];
	u32				*mbps_val;
};


struct rdt_mon_domain {
	struct rdt_domain_hdr		hdr;
	struct cacheinfo		*ci;
	unsigned long			*rmid_busy_llc;
	struct mbm_state		*mbm_total;
	struct mbm_state		*mbm_local;
	struct delayed_work		mbm_over;
	struct delayed_work		cqm_limbo;
	int				mbm_work_cpu;
	int				cqm_work_cpu;
};


struct resctrl_cache {
	unsigned int	cbm_len;
	unsigned int	min_cbm_bits;
	unsigned int	shareable_bits;
	bool		arch_has_sparse_bitmasks;
	bool		arch_has_per_cpu_cfg;
};


enum membw_throttle_mode {
	THREAD_THROTTLE_UNDEFINED = 0,
	THREAD_THROTTLE_MAX,
	THREAD_THROTTLE_PER_THREAD,
};


struct resctrl_membw {
	u32				min_bw;
	u32				bw_gran;
	u32				delay_linear;
	bool				arch_needs_linear;
	enum membw_throttle_mode	throttle_mode;
	bool				mba_sc;
	u32				*mb_map;
};

struct rdt_parse_data;
struct resctrl_schema;

enum resctrl_scope {
	RESCTRL_L2_CACHE = 2,
	RESCTRL_L3_CACHE = 3,
	RESCTRL_L3_NODE,
};


struct rdt_resource {
	int			rid;
	bool			alloc_capable;
	bool			mon_capable;
	int			num_rmid;
	enum resctrl_scope	ctrl_scope;
	enum resctrl_scope	mon_scope;
	struct resctrl_cache	cache;
	struct resctrl_membw	membw;
	struct list_head	ctrl_domains;
	struct list_head	mon_domains;
	char			*name;
	int			data_width;
	u32			default_ctrl;
	const char		*format_str;
	int			(*parse_ctrlval)(struct rdt_parse_data *data,
						 struct resctrl_schema *s,
						 struct rdt_ctrl_domain *d);
	struct list_head	evt_list;
	unsigned long		fflags;
	bool			cdp_capable;
};


struct resctrl_schema {
	struct list_head		list;
	char				name[8];
	enum resctrl_conf_type		conf_type;
	struct rdt_resource		*res;
	u32				num_closid;
};


u32 resctrl_arch_get_num_closid(struct rdt_resource *r);
u32 resctrl_arch_system_num_rmid_idx(void);
int resctrl_arch_update_domains(struct rdt_resource *r, u32 closid);


int resctrl_arch_update_one(struct rdt_resource *r, struct rdt_ctrl_domain *d,
			    u32 closid, enum resctrl_conf_type t, u32 cfg_val);

u32 resctrl_arch_get_config(struct rdt_resource *r, struct rdt_ctrl_domain *d,
			    u32 closid, enum resctrl_conf_type type);
int resctrl_online_ctrl_domain(struct rdt_resource *r, struct rdt_ctrl_domain *d);
int resctrl_online_mon_domain(struct rdt_resource *r, struct rdt_mon_domain *d);
void resctrl_offline_ctrl_domain(struct rdt_resource *r, struct rdt_ctrl_domain *d);
void resctrl_offline_mon_domain(struct rdt_resource *r, struct rdt_mon_domain *d);
void resctrl_online_cpu(unsigned int cpu);
void resctrl_offline_cpu(unsigned int cpu);


int resctrl_arch_rmid_read(struct rdt_resource *r, struct rdt_mon_domain *d,
			   u32 closid, u32 rmid, enum resctrl_event_id eventid,
			   u64 *val, void *arch_mon_ctx);


static inline void resctrl_arch_rmid_read_context_check(void)
{
	if (!irqs_disabled())
		might_sleep();
}


void resctrl_arch_reset_rmid(struct rdt_resource *r, struct rdt_mon_domain *d,
			     u32 closid, u32 rmid,
			     enum resctrl_event_id eventid);


void resctrl_arch_reset_rmid_all(struct rdt_resource *r, struct rdt_mon_domain *d);

extern unsigned int resctrl_rmid_realloc_threshold;
extern unsigned int resctrl_rmid_realloc_limit;

#endif 
