// SPDX-License-Identifier: GPL-2.0-only

#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ptrace.h>

#ifdef K100PM_CHG_RUNTIME_GENERATED_VERSIONS
#include "K100PM_CHG_RUNTIME_versions.h"
#endif

/* K100PM values are mA, mV, and deci-degrees C unless noted otherwise. */
#define K100PM_FG_VCUTOFF_FW_OFF	0xa90u
#define K100PM_FG_VCUTOFF_DELAY_OFF	0xa94u
#define K100PM_FG_VCUTOFF_SW_OFF	0xa98u

static bool armed;
static bool boost;
static bool pps;
static bool thermal;
static bool temperature_spoof;
static bool cutoff;
static bool observe;
static unsigned int temperature_ceiling = 360u;
static unsigned int cutoff_fw_mv = 2600u;
static unsigned int cutoff_delay_mv = 2600u;
static unsigned int cutoff_sw_mv = 2550u;

static unsigned int ichg_hits;
static unsigned int cv_hits;
static unsigned int curve_hits;
static unsigned int pps_hits;
static unsigned int thermal_hits;
static unsigned int temperature_hits;
static unsigned int cutoff_hits;

module_param(armed, bool, 0600);
MODULE_PARM_DESC(armed, "register K100PM charging hooks during module load");
module_param(boost, bool, 0600);
MODULE_PARM_DESC(boost, "enable K100PM JEITA, CV, CP and fast-charge curve targets");
module_param(pps, bool, 0600);
MODULE_PARM_DESC(pps, "raise K100PM third-party PPS current table targets only");
module_param(thermal, bool, 0600);
MODULE_PARM_DESC(thermal, "raise K100PM charge-strategy thermal voter targets");
module_param(temperature_spoof, bool, 0600);
MODULE_PARM_DESC(temperature_spoof, "clamp only K100PM charging FG temperature outputs");
module_param(cutoff, bool, 0600);
MODULE_PARM_DESC(cutoff, "rewrite K100PM FG shutdown thresholds");
module_param(observe, bool, 0600);
MODULE_PARM_DESC(observe, "log successful runtime rewrites");
module_param(temperature_ceiling, uint, 0600);
MODULE_PARM_DESC(temperature_ceiling, "charging FG temperature ceiling in deci-degrees C");
module_param(cutoff_fw_mv, uint, 0600);
module_param(cutoff_delay_mv, uint, 0600);
module_param(cutoff_sw_mv, uint, 0600);
module_param(ichg_hits, uint, 0444);
module_param(cv_hits, uint, 0444);
module_param(curve_hits, uint, 0444);
module_param(pps_hits, uint, 0444);
module_param(thermal_hits, uint, 0444);
module_param(temperature_hits, uint, 0444);
module_param(cutoff_hits, uint, 0444);

struct k100pm_probe_ctx {
	unsigned long arg0;
	unsigned long arg1;
};

static void k100pm_count(unsigned int *counter)
{
	WRITE_ONCE(*counter, READ_ONCE(*counter) + 1u);
}

static u32 k100pm_map_ichg(u32 value)
{
	switch (value) {
	case 12945u:
		return 14270u;
	case 17600u:
	case 18300u:
	case 17500u:
	case 18400u:
	case 4315u:
	case 3880u:
		return 19400u;
	default:
		return value;
	}
}

static u32 k100pm_map_cv(u32 value)
{
	switch (value) {
	case 4530u:
		return 4540u;
	case 4510u:
		return 4530u;
	case 4500u:
	case 4490u:
	case 4080u:
		return 4520u;
	default:
		return value;
	}
}

static u32 k100pm_map_fast_curve(u32 value)
{
	switch (value) {
	case 18300u:
		return 19400u;
	case 17600u:
		return 19400u;
	case 12400u:
		return 16000u;
	case 10356u:
		return 14000u;
	case 8630u:
		return 12000u;
	case 6904u:
		return 10000u;
	case 2958u:
	case 2107u:
	case 1445u:
		return 7000u;
	default:
		return value;
	}
}

static u32 k100pm_map_pps(u32 value)
{
	switch (value) {
	case 4500u:
		return 5000u;
	case 11000u:
		return 12000u;
	case 18400u:
		return 19400u;
	default:
		return value;
	}
}

static u32 k100pm_map_thermal(u32 value)
{
	switch (value) {
	case 13500u:
	case 11000u:
	case 9000u:
	case 8000u:
	case 7000u:
	case 6000u:
	case 5000u:
	case 4000u:
	case 3500u:
	case 3000u:
	case 2500u:
	case 2000u:
	case 1500u:
	case 1000u:
	case 500u:
		return 15000u;
	default:
		return value;
	}
}

static int k100pm_ichg_entry(struct kretprobe_instance *instance,
				      struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[1];
	u32 value;

	if (!READ_ONCE(boost))
		return 0;
	value = k100pm_map_ichg(old);
	if (value == old)
		return 0;
	regs->regs[1] = value;
	k100pm_count(&ichg_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: ichg %u -> %u mA\n", old, value);
	return 0;
}

static int k100pm_cv_entry(struct kretprobe_instance *instance,
			    struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[1];
	u32 value;

	if (!READ_ONCE(boost))
		return 0;
	value = k100pm_map_cv(old);
	if (value == old)
		return 0;
	regs->regs[1] = value;
	k100pm_count(&cv_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: cv %u -> %u mV\n", old, value);
	return 0;
}

static int k100pm_fast_curve_entry(struct kretprobe_instance *instance,
					  struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[2];
	u32 value = old;

	if (READ_ONCE(boost))
		value = k100pm_map_fast_curve(value);
	if (READ_ONCE(pps))
		value = k100pm_map_pps(value);
	if (value == old)
		return 0;
	regs->regs[2] = value;
	if (READ_ONCE(pps) && k100pm_map_pps(old) != old)
		k100pm_count(&pps_hits);
	else
		k100pm_count(&curve_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: fast curve %u -> %u mA\n", old, value);
	return 0;
}

static int k100pm_div_single_entry(struct kretprobe_instance *instance,
					  struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[2];
	u32 value = old;

	if (READ_ONCE(boost)) {
		if (value == 4500u)
			value = 5000u;
		else if (value == 11000u)
			value = 12000u;
		else if (value == 18400u)
			value = 20500u;
	} else if (READ_ONCE(pps)) {
		value = k100pm_map_pps(value);
	}
	if (value == old)
		return 0;
	regs->regs[2] = value;
	if (READ_ONCE(boost))
		k100pm_count(&curve_hits);
	else
		k100pm_count(&pps_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: CP single %u -> %u mA\n", old, value);
	return 0;
}

static int k100pm_div_multi_entry(struct kretprobe_instance *instance,
					 struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[2];
	u32 value = old;

	if (READ_ONCE(boost)) {
		if (value == 4500u)
			value = 5000u;
		else if (value == 11000u)
			value = 12000u;
		else if (value == 18400u)
			value = 22000u;
	} else if (READ_ONCE(pps)) {
		value = k100pm_map_pps(value);
	}
	if (value == old)
		return 0;
	regs->regs[2] = value;
	if (READ_ONCE(boost))
		k100pm_count(&curve_hits);
	else
		k100pm_count(&pps_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: CP multi %u -> %u mA\n", old, value);
	return 0;
}

static int k100pm_curr_max_entry(struct kretprobe_instance *instance,
					 struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[2];
	u32 value = old;

	if (READ_ONCE(boost) && value == 17500u)
		value = 19400u;
	else if (READ_ONCE(pps))
		value = k100pm_map_pps(value);
	if (value == old)
		return 0;
	regs->regs[2] = value;
	if (READ_ONCE(boost))
		k100pm_count(&curve_hits);
	else
		k100pm_count(&pps_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: current cap %u -> %u mA\n", old, value);
	return 0;
}

static int k100pm_thermal_entry(struct kretprobe_instance *instance,
					 struct pt_regs *regs)
{
	u32 old = (u32)regs->regs[2];
	u32 value;

	if (!READ_ONCE(thermal))
		return 0;
	value = k100pm_map_thermal(old);
	if (value == old)
		return 0;
	regs->regs[2] = value;
	k100pm_count(&thermal_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: thermal %u -> %u mA\n", old, value);
	return 0;
}

static int k100pm_fg_temp_entry(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	struct k100pm_probe_ctx *ctx = (struct k100pm_probe_ctx *)instance->data;

	ctx->arg1 = regs->regs[1];
	return 0;
}

static int k100pm_fg_temp_return(struct kretprobe_instance *instance,
					 struct pt_regs *regs)
{
	struct k100pm_probe_ctx *ctx = (struct k100pm_probe_ctx *)instance->data;
	int *temperature = (int *)ctx->arg1;
	unsigned int ceiling = READ_ONCE(temperature_ceiling);

	if (!READ_ONCE(temperature_spoof) || !temperature || !ceiling)
		return 0;
	if (READ_ONCE(*temperature) <= (int)ceiling)
		return 0;
	WRITE_ONCE(*temperature, (int)ceiling);
	k100pm_count(&temperature_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: charging FG temperature -> %u deci-C\n",
			ceiling);
	return 0;
}

static bool k100pm_cutoff_values_valid(u32 fw, u32 delay, u32 sw)
{
	return fw >= 2500u && fw <= 3300u && delay >= 2500u &&
		delay <= 3300u && sw >= 2500u && sw <= 3300u;
}

static void k100pm_apply_cutoff(unsigned long context)
{
	u8 *base = (u8 *)context;
	u32 *fw;
	u32 *delay;
	u32 *sw;
	u32 old_fw;
	u32 old_delay;
	u32 old_sw;

	if (!READ_ONCE(cutoff) || !base)
		return;
	fw = (u32 *)(base + K100PM_FG_VCUTOFF_FW_OFF);
	delay = (u32 *)(base + K100PM_FG_VCUTOFF_DELAY_OFF);
	sw = (u32 *)(base + K100PM_FG_VCUTOFF_SW_OFF);
	old_fw = READ_ONCE(*fw);
	old_delay = READ_ONCE(*delay);
	old_sw = READ_ONCE(*sw);
	if (!k100pm_cutoff_values_valid(old_fw, old_delay, old_sw))
		return;
	if (old_fw == READ_ONCE(cutoff_fw_mv) &&
	    old_delay == READ_ONCE(cutoff_delay_mv) &&
	    old_sw == READ_ONCE(cutoff_sw_mv))
		return;
	WRITE_ONCE(*fw, READ_ONCE(cutoff_fw_mv));
	WRITE_ONCE(*delay, READ_ONCE(cutoff_delay_mv));
	WRITE_ONCE(*sw, READ_ONCE(cutoff_sw_mv));
	k100pm_count(&cutoff_hits);
	if (READ_ONCE(observe))
		pr_info("K100PM_CHG_RUNTIME: cutoff %u/%u/%u -> %u/%u/%u mV\n",
			old_fw, old_delay, old_sw, READ_ONCE(cutoff_fw_mv),
			READ_ONCE(cutoff_delay_mv), READ_ONCE(cutoff_sw_mv));
}

static int k100pm_cutoff_entry(struct kretprobe_instance *instance,
				       struct pt_regs *regs)
{
	struct k100pm_probe_ctx *ctx = (struct k100pm_probe_ctx *)instance->data;

	ctx->arg0 = regs->regs[0];
	return 0;
}

static int k100pm_cutoff_return(struct kretprobe_instance *instance,
					struct pt_regs *regs)
{
	struct k100pm_probe_ctx *ctx = (struct k100pm_probe_ctx *)instance->data;

	k100pm_apply_cutoff(ctx->arg0);
	return 0;
}

static int k100pm_noop_return(struct kretprobe_instance *instance,
			       struct pt_regs *regs)
{
	return 0;
}

static struct kretprobe k100pm_probes[] = {
	{
		.kp.symbol_name = "platform_class_buckchg_ops_set_ichg",
		.entry_handler = k100pm_ichg_entry,
	},
	{
		.kp.symbol_name = "platform_class_buckchg_ops_set_term_volt",
		.entry_handler = k100pm_cv_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_div1_single_voter_cb",
		.entry_handler = k100pm_div_single_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_div2_single_voter_cb",
		.entry_handler = k100pm_div_single_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_div4_single_voter_cb",
		.entry_handler = k100pm_div_single_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_div1_multi_voter_cb",
		.entry_handler = k100pm_div_multi_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_div2_multi_voter_cb",
		.entry_handler = k100pm_div_multi_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_div4_multi_voter_cb",
		.entry_handler = k100pm_div_multi_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_curr_max_voter_cb",
		.entry_handler = k100pm_curr_max_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_single_chg_cur_voter_cb",
		.entry_handler = k100pm_fast_curve_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_multi_chg_cur_voter_cb",
		.entry_handler = k100pm_fast_curve_entry,
	},
	{
		.kp.symbol_name = "mca_quick_charge_thermal_flip_voter_cb",
		.entry_handler = k100pm_thermal_entry,
	},
	{
		.kp.symbol_name = "strategy_fg_ops_get_temp",
		.entry_handler = k100pm_fg_temp_entry,
		.handler = k100pm_fg_temp_return,
		.data_size = sizeof(struct k100pm_probe_ctx),
	},
	{
		.kp.symbol_name = "strategy_fg_ops_get_thermal_temp",
		.entry_handler = k100pm_fg_temp_entry,
		.handler = k100pm_fg_temp_return,
		.data_size = sizeof(struct k100pm_probe_ctx),
	},
	{
		.kp.symbol_name = "fg_update_status",
		.entry_handler = k100pm_cutoff_entry,
		.handler = k100pm_cutoff_return,
		.data_size = sizeof(struct k100pm_probe_ctx),
	},
	{
		.kp.symbol_name = "mca_battery_shutdown_update_vcutoff_para",
		.entry_handler = k100pm_cutoff_entry,
		.handler = k100pm_cutoff_return,
		.data_size = sizeof(struct k100pm_probe_ctx),
	},
};

static unsigned int k100pm_registered;

static int __init k100pm_chg_runtime_init(void)
{
	unsigned int i;
	int ret;

	if (!READ_ONCE(armed)) {
		pr_info("K100PM_CHG_RUNTIME: runtime rewrite build loaded inert\n");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(k100pm_probes); i++) {
		if (!k100pm_probes[i].handler)
			k100pm_probes[i].handler = k100pm_noop_return;
		k100pm_probes[i].maxactive = 32;
		ret = register_kretprobe(&k100pm_probes[i]);
		if (ret) {
			pr_err("K100PM_CHG_RUNTIME: register %s failed: %d\n",
			       k100pm_probes[i].kp.symbol_name, ret);
			while (k100pm_registered)
				unregister_kretprobe(&k100pm_probes[--k100pm_registered]);
			return ret;
		}
		k100pm_registered++;
	}

	pr_info("K100PM_CHG_RUNTIME: ready boost=%u pps=%u thermal=%u temp=%u cutoff=%u\n",
		READ_ONCE(boost), READ_ONCE(pps), READ_ONCE(thermal),
		READ_ONCE(temperature_spoof), READ_ONCE(cutoff));
	return 0;
}

static void __exit k100pm_chg_runtime_exit(void)
{
	while (k100pm_registered)
		unregister_kretprobe(&k100pm_probes[--k100pm_registered]);
	pr_info("K100PM_CHG_RUNTIME: removed ichg=%u cv=%u curve=%u pps=%u thermal=%u temp=%u cutoff=%u\n",
		READ_ONCE(ichg_hits), READ_ONCE(cv_hits), READ_ONCE(curve_hits),
		READ_ONCE(pps_hits), READ_ONCE(thermal_hits),
		READ_ONCE(temperature_hits), READ_ONCE(cutoff_hits));
}

module_init(k100pm_chg_runtime_init);
module_exit(k100pm_chg_runtime_exit);

MODULE_DESCRIPTION("K100 Pro Max runtime charging, thermal, PPS and FG cutoff rewrite");
MODULE_LICENSE("GPL v2");
