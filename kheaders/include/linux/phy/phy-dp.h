/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __PHY_DP_H_
#define __PHY_DP_H_

#include <linux/types.h>

#define PHY_SUBMODE_DP	0
#define PHY_SUBMODE_EDP	1


struct phy_configure_opts_dp {
	
	unsigned int link_rate;

	
	unsigned int lanes;

	
	unsigned int voltage[4];

	
	unsigned int pre[4];

	
	u8 ssc : 1;

	
	u8 set_rate : 1;

	
	u8 set_lanes : 1;

	
	u8 set_voltages : 1;
};

#endif 
