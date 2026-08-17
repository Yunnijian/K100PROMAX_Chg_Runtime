/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LIBETH_TYPES_H
#define __LIBETH_TYPES_H

#include <linux/types.h>


struct libeth_sq_napi_stats {
	union {
		struct {
							u32 packets;
							u32 bytes;
		};
		DECLARE_FLEX_ARRAY(u32, raw);
	};
};

#endif 
