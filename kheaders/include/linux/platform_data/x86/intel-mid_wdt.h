/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __PLATFORM_X86_INTEL_MID_WDT_H_
#define __PLATFORM_X86_INTEL_MID_WDT_H_

#include <linux/platform_device.h>

struct intel_mid_wdt_pdata {
	int irq;
	int (*probe)(struct platform_device *pdev);
};

#endif	
