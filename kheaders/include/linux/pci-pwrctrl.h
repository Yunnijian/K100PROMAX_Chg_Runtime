/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __PCI_PWRCTRL_H__
#define __PCI_PWRCTRL_H__

#include <linux/notifier.h>
#include <linux/workqueue.h>

struct device;
struct device_link;




struct pci_pwrctrl {
	struct device *dev;

	
	struct notifier_block nb;
	struct device_link *link;
	struct work_struct work;
};

void pci_pwrctrl_init(struct pci_pwrctrl *pwrctrl, struct device *dev);
int pci_pwrctrl_device_set_ready(struct pci_pwrctrl *pwrctrl);
void pci_pwrctrl_device_unset_ready(struct pci_pwrctrl *pwrctrl);
int devm_pci_pwrctrl_device_set_ready(struct device *dev,
				     struct pci_pwrctrl *pwrctrl);

#endif 
