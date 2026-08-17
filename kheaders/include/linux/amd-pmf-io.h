/* SPDX-License-Identifier: GPL-2.0 */


#ifndef AMD_PMF_IO_H
#define AMD_PMF_IO_H

#include <linux/types.h>


enum sfh_message_type {
	MT_HPD,
	MT_ALS,
};


enum sfh_hpd_info {
	SFH_NOT_DETECTED,
	SFH_USER_PRESENT,
	SFH_USER_AWAY,
};


struct amd_sfh_info {
	u32 ambient_light;
	u8 user_present;
};

int amd_get_sfh_info(struct amd_sfh_info *sfh_info, enum sfh_message_type op);
#endif
