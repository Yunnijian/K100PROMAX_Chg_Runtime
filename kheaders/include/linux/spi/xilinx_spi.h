/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_SPI_XILINX_SPI_H
#define __LINUX_SPI_XILINX_SPI_H

#include <linux/types.h>

struct spi_board_info;


struct xspi_platform_data {
	struct spi_board_info *devices;
	u8 num_devices;
	u8 num_chipselect;
	u8 bits_per_word;
	bool force_irq;
};

#endif 
