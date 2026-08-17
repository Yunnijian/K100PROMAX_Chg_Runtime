/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __ACPI_NHLT_H__
#define __ACPI_NHLT_H__

#include <linux/acpi.h>
#include <linux/kconfig.h>
#include <linux/overflow.h>
#include <linux/types.h>

#define __acpi_nhlt_endpoint_config(ep)		((void *)((ep) + 1))
#define __acpi_nhlt_config_caps(cfg)		((void *)((cfg) + 1))


static inline struct acpi_nhlt_formats_config *
acpi_nhlt_endpoint_fmtscfg(const struct acpi_nhlt_endpoint *ep)
{
	struct acpi_nhlt_config *cfg = __acpi_nhlt_endpoint_config(ep);

	return (struct acpi_nhlt_formats_config *)((u8 *)(cfg + 1) + cfg->capabilities_size);
}

#define __acpi_nhlt_first_endpoint(tb) \
	((void *)(tb + 1))

#define __acpi_nhlt_next_endpoint(ep) \
	((void *)((u8 *)(ep) + (ep)->length))

#define __acpi_nhlt_get_endpoint(tb, ep, i) \
	((i) ? __acpi_nhlt_next_endpoint(ep) : __acpi_nhlt_first_endpoint(tb))

#define __acpi_nhlt_first_fmtcfg(fmts) \
	((void *)(fmts + 1))

#define __acpi_nhlt_next_fmtcfg(fmt) \
	((void *)((u8 *)((fmt) + 1) + (fmt)->config.capabilities_size))

#define __acpi_nhlt_get_fmtcfg(fmts, fmt, i) \
	((i) ? __acpi_nhlt_next_fmtcfg(fmt) : __acpi_nhlt_first_fmtcfg(fmts))




#define for_each_nhlt_endpoint(tb, ep)					\
	for (unsigned int __i = 0;					\
	     __i < (tb)->endpoints_count &&				\
		(ep = __acpi_nhlt_get_endpoint(tb, ep, __i));		\
	     __i++)


#define for_each_nhlt_fmtcfg(fmts, fmt)					\
	for (unsigned int __i = 0;					\
	     __i < (fmts)->formats_count &&				\
		(fmt = __acpi_nhlt_get_fmtcfg(fmts, fmt, __i));	\
	     __i++)


#define for_each_nhlt_endpoint_fmtcfg(ep, fmt) \
	for_each_nhlt_fmtcfg(acpi_nhlt_endpoint_fmtscfg(ep), fmt)

#if IS_ENABLED(CONFIG_ACPI_NHLT)



acpi_status acpi_nhlt_get_gbl_table(void);
void acpi_nhlt_put_gbl_table(void);

bool acpi_nhlt_endpoint_match(const struct acpi_nhlt_endpoint *ep,
			      int link_type, int dev_type, int dir, int bus_id);
struct acpi_nhlt_endpoint *
acpi_nhlt_tb_find_endpoint(const struct acpi_table_nhlt *tb,
			   int link_type, int dev_type, int dir, int bus_id);
struct acpi_nhlt_endpoint *
acpi_nhlt_find_endpoint(int link_type, int dev_type, int dir, int bus_id);
struct acpi_nhlt_format_config *
acpi_nhlt_endpoint_find_fmtcfg(const struct acpi_nhlt_endpoint *ep,
			       u16 ch, u32 rate, u16 vbps, u16 bps);
struct acpi_nhlt_format_config *
acpi_nhlt_tb_find_fmtcfg(const struct acpi_table_nhlt *tb,
			 int link_type, int dev_type, int dir, int bus_id,
			 u16 ch, u32 rate, u16 vpbs, u16 bps);
struct acpi_nhlt_format_config *
acpi_nhlt_find_fmtcfg(int link_type, int dev_type, int dir, int bus_id,
		      u16 ch, u32 rate, u16 vpbs, u16 bps);
int acpi_nhlt_endpoint_mic_count(const struct acpi_nhlt_endpoint *ep);

#else 

static inline acpi_status acpi_nhlt_get_gbl_table(void)
{
	return AE_NOT_FOUND;
}

static inline void acpi_nhlt_put_gbl_table(void)
{
}

static inline bool
acpi_nhlt_endpoint_match(const struct acpi_nhlt_endpoint *ep,
			 int link_type, int dev_type, int dir, int bus_id)
{
	return false;
}

static inline struct acpi_nhlt_endpoint *
acpi_nhlt_tb_find_endpoint(const struct acpi_table_nhlt *tb,
			   int link_type, int dev_type, int dir, int bus_id)
{
	return NULL;
}

static inline struct acpi_nhlt_format_config *
acpi_nhlt_endpoint_find_fmtcfg(const struct acpi_nhlt_endpoint *ep,
			       u16 ch, u32 rate, u16 vbps, u16 bps)
{
	return NULL;
}

static inline struct acpi_nhlt_format_config *
acpi_nhlt_tb_find_fmtcfg(const struct acpi_table_nhlt *tb,
			 int link_type, int dev_type, int dir, int bus_id,
			 u16 ch, u32 rate, u16 vpbs, u16 bps)
{
	return NULL;
}

static inline int acpi_nhlt_endpoint_mic_count(const struct acpi_nhlt_endpoint *ep)
{
	return 0;
}

static inline struct acpi_nhlt_endpoint *
acpi_nhlt_find_endpoint(int link_type, int dev_type, int dir, int bus_id)
{
	return NULL;
}

static inline struct acpi_nhlt_format_config *
acpi_nhlt_find_fmtcfg(int link_type, int dev_type, int dir, int bus_id,
		      u16 ch, u32 rate, u16 vpbs, u16 bps)
{
	return NULL;
}

#endif 

#endif 
