/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PCA953X_H
#define _LINUX_PCA953X_H

#include <linux/types.h>
#include <linux/i2c.h>



struct pca953x_platform_data {
	
	unsigned	gpio_base;

	
	int		irq_base;
};

#endif 
