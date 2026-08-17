/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __TSM_H
#define __TSM_H

#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/uuid.h>

#define TSM_INBLOB_MAX 64
#define TSM_OUTBLOB_MAX SZ_32K


#define TSM_PRIVLEVEL_MAX 3


struct tsm_desc {
	unsigned int privlevel;
	size_t inblob_len;
	u8 inblob[TSM_INBLOB_MAX];
	char *service_provider;
	guid_t service_guid;
	unsigned int service_manifest_version;
};


struct tsm_report {
	struct tsm_desc desc;
	size_t outblob_len;
	u8 *outblob;
	size_t auxblob_len;
	u8 *auxblob;
	size_t manifestblob_len;
	u8 *manifestblob;
};


enum tsm_attr_index {
	TSM_REPORT_GENERATION,
	TSM_REPORT_PROVIDER,
	TSM_REPORT_PRIVLEVEL,
	TSM_REPORT_PRIVLEVEL_FLOOR,
	TSM_REPORT_SERVICE_PROVIDER,
	TSM_REPORT_SERVICE_GUID,
	TSM_REPORT_SERVICE_MANIFEST_VER,
};


enum tsm_bin_attr_index {
	TSM_REPORT_INBLOB,
	TSM_REPORT_OUTBLOB,
	TSM_REPORT_AUXBLOB,
	TSM_REPORT_MANIFESTBLOB,
};


struct tsm_ops {
	const char *name;
	unsigned int privlevel_floor;
	int (*report_new)(struct tsm_report *report, void *data);
	bool (*report_attr_visible)(int n);
	bool (*report_bin_attr_visible)(int n);
};

int tsm_register(const struct tsm_ops *ops, void *priv);
int tsm_unregister(const struct tsm_ops *ops);
#endif 
