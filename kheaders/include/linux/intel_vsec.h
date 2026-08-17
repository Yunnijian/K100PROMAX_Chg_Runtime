/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _INTEL_VSEC_H
#define _INTEL_VSEC_H

#include <linux/auxiliary_bus.h>
#include <linux/bits.h>

#define VSEC_CAP_TELEMETRY	BIT(0)
#define VSEC_CAP_WATCHER	BIT(1)
#define VSEC_CAP_CRASHLOG	BIT(2)
#define VSEC_CAP_SDSI		BIT(3)
#define VSEC_CAP_TPMI		BIT(4)


#define INTEL_DVSEC_ENTRIES		0xA
#define INTEL_DVSEC_SIZE		0xB
#define INTEL_DVSEC_TABLE		0xC
#define INTEL_DVSEC_TABLE_BAR(x)	((x) & GENMASK(2, 0))
#define INTEL_DVSEC_TABLE_OFFSET(x)	((x) & GENMASK(31, 3))
#define TABLE_OFFSET_SHIFT		3

struct pci_dev;
struct resource;

enum intel_vsec_id {
	VSEC_ID_TELEMETRY	= 2,
	VSEC_ID_WATCHER		= 3,
	VSEC_ID_CRASHLOG	= 4,
	VSEC_ID_SDSI		= 65,
	VSEC_ID_TPMI		= 66,
};


struct intel_vsec_header {
	u8	rev;
	u16	length;
	u16	id;
	u8	num_entries;
	u8	entry_size;
	u8	tbir;
	u32	offset;
};

enum intel_vsec_quirks {
	
	VSEC_QUIRK_NO_WATCHER	= BIT(0),

	
	VSEC_QUIRK_NO_CRASHLOG	= BIT(1),

	
	VSEC_QUIRK_TABLE_SHIFT	= BIT(2),

	
	VSEC_QUIRK_NO_DVSEC	= BIT(3),

	
	VSEC_QUIRK_EARLY_HW     = BIT(4),
};


struct pmt_callbacks {
	int (*read_telem)(struct pci_dev *pdev, u32 guid, u64 *data, loff_t off, u32 count);
};


struct intel_vsec_platform_info {
	struct device *parent;
	struct intel_vsec_header **headers;
	void *priv_data;
	unsigned long caps;
	unsigned long quirks;
	u64 base_addr;
};


struct intel_vsec_device {
	struct auxiliary_device auxdev;
	struct pci_dev *pcidev;
	struct resource *resource;
	struct ida *ida;
	int num_resources;
	int id; 
	void *priv_data;
	size_t priv_data_size;
	unsigned long quirks;
	u64 base_addr;
};

int intel_vsec_add_aux(struct pci_dev *pdev, struct device *parent,
		       struct intel_vsec_device *intel_vsec_dev,
		       const char *name);

static inline struct intel_vsec_device *dev_to_ivdev(struct device *dev)
{
	return container_of(dev, struct intel_vsec_device, auxdev.dev);
}

static inline struct intel_vsec_device *auxdev_to_ivdev(struct auxiliary_device *auxdev)
{
	return container_of(auxdev, struct intel_vsec_device, auxdev);
}

#if IS_ENABLED(CONFIG_INTEL_VSEC)
void intel_vsec_register(struct pci_dev *pdev,
			 struct intel_vsec_platform_info *info);
#else
static inline void intel_vsec_register(struct pci_dev *pdev,
				       struct intel_vsec_platform_info *info)
{
}
#endif
#endif
