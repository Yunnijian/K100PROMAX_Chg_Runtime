/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _LINUX_USB_LJCA_H_
#define _LINUX_USB_LJCA_H_

#include <linux/auxiliary_bus.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#define LJCA_MAX_GPIO_NUM 64

#define auxiliary_dev_to_ljca_client(auxiliary_dev)			\
		container_of(auxiliary_dev, struct ljca_client, auxdev)

struct ljca_adapter;


typedef void (*ljca_event_cb_t)(void *context, u8 cmd, const void *evt_data, int len);


struct ljca_client {
	u8 type;
	u8 id;
	struct list_head link;
	struct auxiliary_device auxdev;
	struct ljca_adapter *adapter;

	void *context;
	ljca_event_cb_t event_cb;
	
	spinlock_t event_cb_lock;
};


struct ljca_gpio_info {
	unsigned int num;
	DECLARE_BITMAP(valid_pin_map, LJCA_MAX_GPIO_NUM);
};


struct ljca_i2c_info {
	u8 id;
	u8 capacity;
	u8 intr_pin;
};


struct ljca_spi_info {
	u8 id;
	u8 capacity;
};


int ljca_register_event_cb(struct ljca_client *client, ljca_event_cb_t event_cb, void *context);


void ljca_unregister_event_cb(struct ljca_client *client);


int ljca_transfer(struct ljca_client *client, u8 cmd, const u8 *obuf,
		  u8 obuf_len, u8 *ibuf, u8 ibuf_len);


int ljca_transfer_noack(struct ljca_client *client, u8 cmd, const u8 *obuf,
			u8 obuf_len);

#endif
