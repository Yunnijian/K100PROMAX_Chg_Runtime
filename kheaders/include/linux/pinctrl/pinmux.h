/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __LINUX_PINCTRL_PINMUX_H
#define __LINUX_PINCTRL_PINMUX_H

#include <linux/types.h>

struct pinctrl_dev;
struct pinctrl_gpio_range;


struct pinmux_ops {
	int (*request) (struct pinctrl_dev *pctldev, unsigned int offset);
	int (*free) (struct pinctrl_dev *pctldev, unsigned int offset);
	int (*get_functions_count) (struct pinctrl_dev *pctldev);
	const char *(*get_function_name) (struct pinctrl_dev *pctldev,
					  unsigned int selector);
	int (*get_function_groups) (struct pinctrl_dev *pctldev,
				    unsigned int selector,
				    const char * const **groups,
				    unsigned int *num_groups);
	int (*set_mux) (struct pinctrl_dev *pctldev, unsigned int func_selector,
			unsigned int group_selector);
	int (*gpio_request_enable) (struct pinctrl_dev *pctldev,
				    struct pinctrl_gpio_range *range,
				    unsigned int offset);
	void (*gpio_disable_free) (struct pinctrl_dev *pctldev,
				   struct pinctrl_gpio_range *range,
				   unsigned int offset);
	int (*gpio_set_direction) (struct pinctrl_dev *pctldev,
				   struct pinctrl_gpio_range *range,
				   unsigned int offset,
				   bool input);
	bool strict;
};

#endif 
