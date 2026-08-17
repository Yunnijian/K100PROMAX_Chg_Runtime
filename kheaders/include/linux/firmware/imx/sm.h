/* SPDX-License-Identifier: GPL-2.0+ */


#ifndef _SCMI_IMX_H
#define _SCMI_IMX_H

#include <linux/bitfield.h>
#include <linux/errno.h>
#include <linux/types.h>

#define SCMI_IMX_CTRL_PDM_CLK_SEL	0	
#define SCMI_IMX_CTRL_MQS1_SETTINGS	1	
#define SCMI_IMX_CTRL_SAI1_MCLK		2	
#define SCMI_IMX_CTRL_SAI3_MCLK		3	
#define SCMI_IMX_CTRL_SAI4_MCLK		4	
#define SCMI_IMX_CTRL_SAI5_MCLK		5	

#if IS_ENABLED(CONFIG_IMX_SCMI_MISC_DRV)
int scmi_imx_misc_ctrl_get(u32 id, u32 *num, u32 *val);
int scmi_imx_misc_ctrl_set(u32 id, u32 val);
#else
static inline int scmi_imx_misc_ctrl_get(u32 id, u32 *num, u32 *val)
{
	return -EOPNOTSUPP;
}

static inline int scmi_imx_misc_ctrl_set(u32 id, u32 val)
{
	return -EOPNOTSUPP;
}
#endif

#endif
