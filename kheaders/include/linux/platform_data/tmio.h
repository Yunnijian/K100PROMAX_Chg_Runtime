/* SPDX-License-Identifier: GPL-2.0 */
#ifndef MFD_TMIO_H
#define MFD_TMIO_H

#include <linux/platform_device.h>
#include <linux/types.h>




#define TMIO_MMC_BLKSZ_2BYTES		BIT(1)


#define TMIO_MMC_SDIO_IRQ		BIT(2)


#define TMIO_MMC_MIN_RCAR2		BIT(3)


#define TMIO_MMC_HAS_IDLE_WAIT		BIT(4)


#define TMIO_MMC_USE_BUSY_TIMEOUT	BIT(5)


#define TMIO_MMC_HAVE_CMD12_CTRL	BIT(7)


#define TMIO_MMC_SDIO_STATUS_SETBITS	BIT(8)


#define TMIO_MMC_32BIT_DATA_PORT	BIT(9)


#define TMIO_MMC_CLK_ACTUAL		BIT(10)


#define TMIO_MMC_HAVE_CBSY		BIT(11)

struct tmio_mmc_data {
	void				*chan_priv_tx;
	void				*chan_priv_rx;
	unsigned int			hclk;
	unsigned long			capabilities;
	unsigned long			capabilities2;
	unsigned long			flags;
	u32				ocr_mask;	
	dma_addr_t			dma_rx_offset;
	unsigned int			max_blk_count;
	unsigned short			max_segs;
};
#endif
