/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef __DRIVERS_PROVIDER_FRAMER_H
#define __DRIVERS_PROVIDER_FRAMER_H

#include <linux/export.h>
#include <linux/framer/framer.h>
#include <linux/types.h>

#define FRAMER_FLAG_POLL_STATUS  BIT(0)


struct framer_ops {
	int	(*init)(struct framer *framer);
	void	(*exit)(struct framer *framer);
	int	(*power_on)(struct framer *framer);
	int	(*power_off)(struct framer *framer);

	
	int	(*get_status)(struct framer *framer, struct framer_status *status);

	
	int	(*set_config)(struct framer *framer, const struct framer_config *config);

	
	int	(*get_config)(struct framer *framer, struct framer_config *config);

	u32 flags;
	struct module *owner;
};


struct framer_provider {
	struct device		*dev;
	struct module		*owner;
	struct list_head	list;
	struct framer * (*of_xlate)(struct device *dev,
				    const struct of_phandle_args *args);
};

static inline void framer_set_drvdata(struct framer *framer, void *data)
{
	dev_set_drvdata(&framer->dev, data);
}

static inline void *framer_get_drvdata(struct framer *framer)
{
	return dev_get_drvdata(&framer->dev);
}

#if IS_ENABLED(CONFIG_GENERIC_FRAMER)


struct framer *framer_create(struct device *dev, struct device_node *node,
			     const struct framer_ops *ops);
void framer_destroy(struct framer *framer);


struct framer *devm_framer_create(struct device *dev, struct device_node *node,
				  const struct framer_ops *ops);

struct framer *framer_provider_simple_of_xlate(struct device *dev,
					       const struct of_phandle_args *args);

struct framer_provider *
__framer_provider_of_register(struct device *dev, struct module *owner,
			      struct framer *(*of_xlate)(struct device *dev,
							 const struct of_phandle_args *args));

void framer_provider_of_unregister(struct framer_provider *framer_provider);

struct framer_provider *
__devm_framer_provider_of_register(struct device *dev, struct module *owner,
				   struct framer *(*of_xlate)(struct device *dev,
							      const struct of_phandle_args *args));

void framer_notify_status_change(struct framer *framer);

#else 

static inline struct framer *framer_create(struct device *dev, struct device_node *node,
					   const struct framer_ops *ops)
{
	return ERR_PTR(-ENOSYS);
}

static inline void framer_destroy(struct framer *framer)
{
}


static inline struct framer *devm_framer_create(struct device *dev, struct device_node *node,
						const struct framer_ops *ops)
{
	return ERR_PTR(-ENOSYS);
}

static inline struct framer *framer_provider_simple_of_xlate(struct device *dev,
							     const struct of_phandle_args *args)
{
	return ERR_PTR(-ENOSYS);
}

static inline struct framer_provider *
__framer_provider_of_register(struct device *dev, struct module *owner,
			      struct framer *(*of_xlate)(struct device *dev,
							 const struct of_phandle_args *args))
{
	return ERR_PTR(-ENOSYS);
}

void framer_provider_of_unregister(struct framer_provider *framer_provider)
{
}

static inline struct framer_provider *
__devm_framer_provider_of_register(struct device *dev, struct module *owner,
				   struct framer *(*of_xlate)(struct device *dev,
							      const struct of_phandle_args *args))
{
	return ERR_PTR(-ENOSYS);
}

void framer_notify_status_change(struct framer *framer)
{
}

#endif 

#define framer_provider_of_register(dev, xlate)		\
	__framer_provider_of_register((dev), THIS_MODULE, (xlate))

#define devm_framer_provider_of_register(dev, xlate)	\
	__devm_framer_provider_of_register((dev), THIS_MODULE, (xlate))

#endif 
