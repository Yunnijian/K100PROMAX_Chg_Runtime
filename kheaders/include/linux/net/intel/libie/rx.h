/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LIBIE_RX_H
#define __LIBIE_RX_H

#include <net/libeth/rx.h>




#define LIBIE_MAX_RX_BUF_LEN	9728U

#define LIBIE_RX_BUF_LEN(hr)	min_t(u32, LIBETH_RX_PAGE_LEN(hr),	\
				      LIBIE_MAX_RX_BUF_LEN)


#define __LIBIE_MAX_RX_FRM_LEN	16382U

#define LIBIE_MAX_RX_FRM_LEN(hr)					\
	min_t(u32, __LIBIE_MAX_RX_FRM_LEN, LIBIE_RX_BUF_LEN(hr) * 5)

#define LIBIE_MAX_MTU							\
	(LIBIE_MAX_RX_FRM_LEN(LIBETH_MAX_HEADROOM) - LIBETH_RX_LL_LEN)



#define LIBIE_RX_PT_NUM		154

extern const struct libeth_rx_pt libie_rx_pt_lut[LIBIE_RX_PT_NUM];


static inline struct libeth_rx_pt libie_rx_pt_parse(u32 pt)
{
	if (unlikely(pt >= LIBIE_RX_PT_NUM))
		pt = 0;

	return libie_rx_pt_lut[pt];
}

#endif 
