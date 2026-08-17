// SPDX-License-Identifier: GPL-2.0-only

#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ptrace.h>

#ifdef K100PM_CHG_RUNTIME_GENERATED_VERSIONS
#include "K100PM_CHG_RUNTIME_versions.h"
#endif

#define K100PM_OBSERVE_SYMBOL "platform_class_buckchg_ops_set_ichg"

static bool armed;
static bool observe;
static bool registered;
static unsigned int hits;

module_param(armed, bool, 0600);
module_param(observe, bool, 0600);
module_param(hits, uint, 0444);
MODULE_PARM_DESC(armed, "register the K100PM observe-only kretprobe during module load");
MODULE_PARM_DESC(observe, "emit sampled observe records when the kretprobe fires");
MODULE_PARM_DESC(hits, "kretprobe handler calls since module load");

static int k100pm_chg_observe_handler(struct kretprobe_instance *instance,
				      struct pt_regs *regs)
{
	unsigned int next = READ_ONCE(hits) + 1u;

	WRITE_ONCE(hits, next);
	if (READ_ONCE(observe) && (next <= 16u || (next & 127u) == 0u))
		pr_info("K100PM_CHG_RUNTIME: ichg call=%u x0=%px x1=%lx x2=%lx x3=%lx\n",
			next, (void *)regs->regs[0], regs->regs[1], regs->regs[2], regs->regs[3]);
	return 0;
}

static struct kretprobe k100pm_chg_probe = {
	.handler = k100pm_chg_observe_handler,
	.maxactive = 32,
	.kp.symbol_name = K100PM_OBSERVE_SYMBOL,
};

static int __init k100pm_chg_runtime_init(void)
{
	int ret;

	if (!READ_ONCE(armed)) {
		pr_info("K100PM_CHG_RUNTIME: inert observe build loaded\n");
		return 0;
	}

	ret = register_kretprobe(&k100pm_chg_probe);
	if (ret) {
		pr_err("K100PM_CHG_RUNTIME: register %s failed: %d\n",
		       K100PM_OBSERVE_SYMBOL, ret);
		return ret;
	}

	WRITE_ONCE(registered, true);
	pr_info("K100PM_CHG_RUNTIME: observing %s\n", K100PM_OBSERVE_SYMBOL);
	return 0;
}

static void __exit k100pm_chg_runtime_exit(void)
{
	if (READ_ONCE(registered))
		unregister_kretprobe(&k100pm_chg_probe);
	pr_info("K100PM_CHG_RUNTIME: unloaded, hits=%u\n", READ_ONCE(hits));
}

module_init(k100pm_chg_runtime_init);
module_exit(k100pm_chg_runtime_exit);

MODULE_DESCRIPTION("K100 Pro Max charging runtime observe bootstrap");
MODULE_LICENSE("GPL v2");
