/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */


#ifndef __SDW_AMD_H
#define __SDW_AMD_H

#include <linux/acpi.h>
#include <linux/soundwire/sdw.h>




#define AMD_SDW_CLK_STOP_MODE		1


#define AMD_SDW_POWER_OFF_MODE		2
#define ACP_SDW0	0
#define ACP_SDW1	1
#define AMD_SDW_MAX_MANAGER_COUNT	2

struct acp_sdw_pdata {
	u16 instance;
	
	struct mutex *acp_sdw_lock;
};


struct sdw_amd_dai_runtime {
	char *name;
	struct sdw_stream_runtime *stream;
	struct sdw_bus *bus;
	enum sdw_stream_type stream_type;
};


struct amd_sdw_manager {
	struct sdw_bus bus;
	struct device *dev;

	void __iomem *mmio;
	void __iomem *acp_mmio;

	struct work_struct amd_sdw_irq_thread;
	struct work_struct amd_sdw_work;
	
	struct mutex *acp_sdw_lock;

	enum sdw_slave_status status[SDW_MAX_DEVICES + 1];

	int num_din_ports;
	int num_dout_ports;

	int cols_index;
	int rows_index;

	u32 instance;
	u32 quirks;
	u32 wake_en_mask;
	u32 power_mode_mask;
	bool clk_stopped;

	struct sdw_amd_dai_runtime **dai_runtime_array;
};


struct sdw_amd_acpi_info {
	acpi_handle handle;
	int count;
	u32 link_mask;
};


struct sdw_amd_ctx {
	int count;
	int num_slaves;
	u32 link_mask;
	struct sdw_extended_slave_id *ids;
	struct platform_device *pdev[AMD_SDW_MAX_MANAGER_COUNT];
};


struct sdw_amd_res {
	u32 addr;
	u32 reg_range;
	u32 link_mask;
	int count;
	void __iomem *mmio_base;
	acpi_handle handle;
	struct device *parent;
	struct device *dev;
	
	struct mutex *acp_lock;
};

int sdw_amd_probe(struct sdw_amd_res *res, struct sdw_amd_ctx **ctx);

void sdw_amd_exit(struct sdw_amd_ctx *ctx);

int sdw_amd_get_slave_info(struct sdw_amd_ctx *ctx);

int amd_sdw_scan_controller(struct sdw_amd_acpi_info *info);
#endif
