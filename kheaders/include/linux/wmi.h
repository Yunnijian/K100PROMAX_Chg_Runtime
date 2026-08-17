/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _LINUX_WMI_H
#define _LINUX_WMI_H

#include <linux/device.h>
#include <linux/acpi.h>
#include <linux/mod_devicetable.h>


struct wmi_device {
	struct device dev;
	bool setable;
	const char *driver_override;
};


#define to_wmi_device(device)	container_of(device, struct wmi_device, dev)

extern acpi_status wmidev_evaluate_method(struct wmi_device *wdev,
					  u8 instance, u32 method_id,
					  const struct acpi_buffer *in,
					  struct acpi_buffer *out);

extern union acpi_object *wmidev_block_query(struct wmi_device *wdev,
					     u8 instance);

acpi_status wmidev_block_set(struct wmi_device *wdev, u8 instance, const struct acpi_buffer *in);

u8 wmidev_instance_count(struct wmi_device *wdev);


struct wmi_driver {
	struct device_driver driver;
	const struct wmi_device_id *id_table;
	bool no_notify_data;
	bool no_singleton;

	int (*probe)(struct wmi_device *wdev, const void *context);
	void (*remove)(struct wmi_device *wdev);
	void (*notify)(struct wmi_device *device, union acpi_object *data);
};

extern int __must_check __wmi_driver_register(struct wmi_driver *driver,
					      struct module *owner);
extern void wmi_driver_unregister(struct wmi_driver *driver);


#define wmi_driver_register(driver) __wmi_driver_register((driver), THIS_MODULE)


#define module_wmi_driver(__wmi_driver) \
	module_driver(__wmi_driver, wmi_driver_register, \
		      wmi_driver_unregister)

#endif
