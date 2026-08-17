/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ANDROID_VENDOR_H
#define _ANDROID_VENDOR_H


#ifdef CONFIG_ANDROID_VENDOR_OEM_DATA
#define ANDROID_VENDOR_DATA(n)		u64 android_vendor_data##n
#define ANDROID_VENDOR_DATA_ARRAY(n, s)	u64 android_vendor_data##n[s]

#define ANDROID_OEM_DATA(n)		u64 android_oem_data##n
#define ANDROID_OEM_DATA_ARRAY(n, s)	u64 android_oem_data##n[s]

#define android_init_vendor_data(p, n) \
	memset(&p->android_vendor_data##n, 0, sizeof(p->android_vendor_data##n))
#define android_init_oem_data(p, n) \
	memset(&p->android_oem_data##n, 0, sizeof(p->android_oem_data##n))
#else
#define ANDROID_VENDOR_DATA(n)
#define ANDROID_VENDOR_DATA_ARRAY(n, s)
#define ANDROID_OEM_DATA(n)
#define ANDROID_OEM_DATA_ARRAY(n, s)

#define android_init_vendor_data(p, n)
#define android_init_oem_data(p, n)
#endif

#endif 
