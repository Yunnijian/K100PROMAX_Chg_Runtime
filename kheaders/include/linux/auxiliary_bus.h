/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _AUXILIARY_BUS_H_
#define _AUXILIARY_BUS_H_

#include <linux/device.h>
#include <linux/mod_devicetable.h>




struct auxiliary_device {
	struct device dev;
	const char *name;
	u32 id;
	struct {
		struct xarray irqs;
		struct mutex lock; 
		bool irq_dir_exists;
	} sysfs;
};


struct auxiliary_driver {
	int (*probe)(struct auxiliary_device *auxdev, const struct auxiliary_device_id *id);
	void (*remove)(struct auxiliary_device *auxdev);
	void (*shutdown)(struct auxiliary_device *auxdev);
	int (*suspend)(struct auxiliary_device *auxdev, pm_message_t state);
	int (*resume)(struct auxiliary_device *auxdev);
	const char *name;
	struct device_driver driver;
	const struct auxiliary_device_id *id_table;
};

static inline void *auxiliary_get_drvdata(struct auxiliary_device *auxdev)
{
	return dev_get_drvdata(&auxdev->dev);
}

static inline void auxiliary_set_drvdata(struct auxiliary_device *auxdev, void *data)
{
	dev_set_drvdata(&auxdev->dev, data);
}

static inline struct auxiliary_device *to_auxiliary_dev(struct device *dev)
{
	return container_of(dev, struct auxiliary_device, dev);
}

static inline const struct auxiliary_driver *to_auxiliary_drv(const struct device_driver *drv)
{
	return container_of(drv, struct auxiliary_driver, driver);
}

int auxiliary_device_init(struct auxiliary_device *auxdev);
int __auxiliary_device_add(struct auxiliary_device *auxdev, const char *modname);
#define auxiliary_device_add(auxdev) __auxiliary_device_add(auxdev, KBUILD_MODNAME)

#ifdef CONFIG_SYSFS
int auxiliary_device_sysfs_irq_add(struct auxiliary_device *auxdev, int irq);
void auxiliary_device_sysfs_irq_remove(struct auxiliary_device *auxdev,
				       int irq);
#else 
static inline int
auxiliary_device_sysfs_irq_add(struct auxiliary_device *auxdev, int irq)
{
	return 0;
}

static inline void
auxiliary_device_sysfs_irq_remove(struct auxiliary_device *auxdev, int irq) {}
#endif

static inline void auxiliary_device_uninit(struct auxiliary_device *auxdev)
{
	mutex_destroy(&auxdev->sysfs.lock);
	put_device(&auxdev->dev);
}

static inline void auxiliary_device_delete(struct auxiliary_device *auxdev)
{
	device_del(&auxdev->dev);
}

int __auxiliary_driver_register(struct auxiliary_driver *auxdrv, struct module *owner,
				const char *modname);
#define auxiliary_driver_register(auxdrv) \
	__auxiliary_driver_register(auxdrv, THIS_MODULE, KBUILD_MODNAME)

void auxiliary_driver_unregister(struct auxiliary_driver *auxdrv);


#define module_auxiliary_driver(__auxiliary_driver) \
	module_driver(__auxiliary_driver, auxiliary_driver_register, auxiliary_driver_unregister)

struct auxiliary_device *auxiliary_find_device(struct device *start,
					       const void *data,
					       device_match_t match);

#endif 
