/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __CXL_MBOX_H__
#define __CXL_MBOX_H__
#include <linux/rcuwait.h>

struct cxl_mbox_cmd;


struct cxl_mailbox {
	struct device *host;
	size_t payload_size;
	struct mutex mbox_mutex; 
	struct rcuwait mbox_wait;
	int (*mbox_send)(struct cxl_mailbox *cxl_mbox, struct cxl_mbox_cmd *cmd);
};

int cxl_mailbox_init(struct cxl_mailbox *cxl_mbox, struct device *host);

#endif
