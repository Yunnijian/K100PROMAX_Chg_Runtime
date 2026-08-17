/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _KUNIT_DEVICE_H
#define _KUNIT_DEVICE_H

#if IS_ENABLED(CONFIG_KUNIT)

#include <kunit/test.h>

struct device;
struct device_driver;


struct device_driver *kunit_driver_create(struct kunit *test, const char *name);


struct device *kunit_device_register(struct kunit *test, const char *name);


struct device *kunit_device_register_with_driver(struct kunit *test,
						 const char *name,
						 const struct device_driver *drv);


void kunit_device_unregister(struct kunit *test, struct device *dev);

#endif

#endif
