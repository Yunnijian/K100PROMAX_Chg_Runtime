/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __PEF2256_H__
#define __PEF2256_H__

#include <linux/types.h>

struct pef2256;
struct regmap;


struct regmap *pef2256_get_regmap(struct pef2256 *pef2256);


enum pef2256_version {
	PEF2256_VERSION_UNKNOWN,
	PEF2256_VERSION_1_2,
	PEF2256_VERSION_2_1,
	PEF2256_VERSION_2_2,
};


enum pef2256_version pef2256_get_version(struct pef2256 *pef2256);

#endif 
