/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LINUX_PECI_CPU_H
#define __LINUX_PECI_CPU_H

#include <linux/types.h>


#define X86_VENDOR_INTEL       0


#define VFM_MODEL_BIT	0
#define VFM_FAMILY_BIT	8
#define VFM_VENDOR_BIT	16
#define VFM_RSVD_BIT	24

#define	VFM_MODEL_MASK	GENMASK(VFM_FAMILY_BIT - 1, VFM_MODEL_BIT)
#define	VFM_FAMILY_MASK	GENMASK(VFM_VENDOR_BIT - 1, VFM_FAMILY_BIT)
#define	VFM_VENDOR_MASK	GENMASK(VFM_RSVD_BIT - 1, VFM_VENDOR_BIT)

#define VFM_MODEL(vfm)	(((vfm) & VFM_MODEL_MASK) >> VFM_MODEL_BIT)
#define VFM_FAMILY(vfm)	(((vfm) & VFM_FAMILY_MASK) >> VFM_FAMILY_BIT)
#define VFM_VENDOR(vfm)	(((vfm) & VFM_VENDOR_MASK) >> VFM_VENDOR_BIT)

#define	VFM_MAKE(_vendor, _family, _model) (	\
	((_model) << VFM_MODEL_BIT) |		\
	((_family) << VFM_FAMILY_BIT) |		\
	((_vendor) << VFM_VENDOR_BIT)		\
)


#include "../../arch/x86/include/asm/intel-family.h"

#define PECI_PCS_PKG_ID			0  
#define  PECI_PKG_ID_CPU_ID		0x0000  
#define  PECI_PKG_ID_PLATFORM_ID	0x0001  
#define  PECI_PKG_ID_DEVICE_ID		0x0002  
#define  PECI_PKG_ID_MAX_THREAD_ID	0x0003  
#define  PECI_PKG_ID_MICROCODE_REV	0x0004  
#define  PECI_PKG_ID_MCA_ERROR_LOG	0x0005  
#define PECI_PCS_MODULE_TEMP		9  
#define PECI_PCS_THERMAL_MARGIN		10 
#define PECI_PCS_DDR_DIMM_TEMP		14 
#define PECI_PCS_TEMP_TARGET		16 
#define PECI_PCS_TDP_UNITS		30 

struct peci_device;

int peci_temp_read(struct peci_device *device, s16 *temp_raw);

int peci_pcs_read(struct peci_device *device, u8 index,
		  u16 param, u32 *data);

int peci_pci_local_read(struct peci_device *device, u8 bus, u8 dev,
			u8 func, u16 reg, u32 *data);

int peci_ep_pci_local_read(struct peci_device *device, u8 seg,
			   u8 bus, u8 dev, u8 func, u16 reg, u32 *data);

int peci_mmio_read(struct peci_device *device, u8 bar, u8 seg,
		   u8 bus, u8 dev, u8 func, u64 address, u32 *data);

#endif 
