/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef STM32_FIREWALL_DEVICE_H
#define STM32_FIREWALL_DEVICE_H

#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#define STM32_FIREWALL_MAX_EXTRA_ARGS		5


struct stm32_firewall_controller;


struct stm32_firewall {
	struct stm32_firewall_controller *firewall_ctrl;
	u32 extra_args[STM32_FIREWALL_MAX_EXTRA_ARGS];
	const char *entry;
	size_t extra_args_size;
	u32 firewall_id;
};

#if IS_ENABLED(CONFIG_STM32_FIREWALL)

int stm32_firewall_get_firewall(struct device_node *np, struct stm32_firewall *firewall,
				unsigned int nb_firewall);


int stm32_firewall_grant_access(struct stm32_firewall *firewall);


void stm32_firewall_release_access(struct stm32_firewall *firewall);


int stm32_firewall_grant_access_by_id(struct stm32_firewall *firewall, u32 subsystem_id);


void stm32_firewall_release_access_by_id(struct stm32_firewall *firewall, u32 subsystem_id);

#else 

static inline int stm32_firewall_get_firewall(struct device_node *np,
					      struct stm32_firewall *firewall,
					      unsigned int nb_firewall)
{
	return -ENODEV;
}

static inline int stm32_firewall_grant_access(struct stm32_firewall *firewall)
{
	return -ENODEV;
}

static inline void stm32_firewall_release_access(struct stm32_firewall *firewall)
{
}

static inline int stm32_firewall_grant_access_by_id(struct stm32_firewall *firewall,
						    u32 subsystem_id)
{
	return -ENODEV;
}

static inline void stm32_firewall_release_access_by_id(struct stm32_firewall *firewall,
						       u32 subsystem_id)
{
}

#endif 
#endif 
