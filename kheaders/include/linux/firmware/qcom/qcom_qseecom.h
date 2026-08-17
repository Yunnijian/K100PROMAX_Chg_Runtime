/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef __QCOM_QSEECOM_H
#define __QCOM_QSEECOM_H

#include <linux/auxiliary_bus.h>
#include <linux/dma-mapping.h>
#include <linux/types.h>

#include <linux/firmware/qcom/qcom_scm.h>


struct qseecom_client {
	struct auxiliary_device aux_dev;
	u32 app_id;
};


static inline int qcom_qseecom_app_send(struct qseecom_client *client,
					void *req, size_t req_size,
					void *rsp, size_t rsp_size)
{
	return qcom_scm_qseecom_app_send(client->app_id, req, req_size, rsp, rsp_size);
}

#endif 
