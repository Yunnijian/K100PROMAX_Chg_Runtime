/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _LINUX_MISC_KEBA_H
#define _LINUX_MISC_KEBA_H

#include <linux/auxiliary_bus.h>

struct i2c_board_info;


struct keba_i2c_auxdev {
	struct auxiliary_device auxdev;
	struct resource io;
	int info_size;
	struct i2c_board_info *info;
};

#endif 
