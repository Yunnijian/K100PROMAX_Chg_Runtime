/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __LINUX_XHCI_SIDEBAND_H
#define __LINUX_XHCI_SIDEBAND_H

#include <linux/scatterlist.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

#define	EP_CTX_PER_DEV		31	

struct xhci_sideband;

enum xhci_sideband_type {
	XHCI_SIDEBAND_AUDIO,
	XHCI_SIDEBAND_VENDOR,
};

enum xhci_sideband_notify_type {
	XHCI_SIDEBAND_XFER_RING_FREE,
};


struct xhci_sideband_event {
	enum xhci_sideband_notify_type type;
	void *evt_data;
};


struct xhci_sideband {
	struct xhci_hcd                 *xhci;
	struct xhci_virt_device         *vdev;
	struct xhci_virt_ep             *eps[EP_CTX_PER_DEV];
	struct xhci_interrupter         *ir;
	enum xhci_sideband_type		type;

	
	struct mutex			mutex;

	struct usb_interface		*intf;
	int (*notify_client)(struct usb_interface *intf,
			     struct xhci_sideband_event *evt);
};

struct xhci_sideband *
xhci_sideband_register(struct usb_interface *intf, enum xhci_sideband_type type,
		       int (*notify_client)(struct usb_interface *intf,
				    struct xhci_sideband_event *evt));
void
xhci_sideband_unregister(struct xhci_sideband *sb);
int
xhci_sideband_add_endpoint(struct xhci_sideband *sb,
			   struct usb_host_endpoint *host_ep);
int
xhci_sideband_remove_endpoint(struct xhci_sideband *sb,
			      struct usb_host_endpoint *host_ep);
int
xhci_sideband_stop_endpoint(struct xhci_sideband *sb,
			    struct usb_host_endpoint *host_ep);
struct sg_table *
xhci_sideband_get_endpoint_buffer(struct xhci_sideband *sb,
				  struct usb_host_endpoint *host_ep);
struct sg_table *
xhci_sideband_get_event_buffer(struct xhci_sideband *sb);

#if IS_ENABLED(CONFIG_USB_XHCI_SIDEBAND)
bool xhci_sideband_check(struct usb_hcd *hcd);
#else
static inline bool xhci_sideband_check(struct usb_hcd *hcd)
{ return false; }
#endif 

int
xhci_sideband_create_interrupter(struct xhci_sideband *sb, int num_seg,
				 bool ip_autoclear, u32 imod_interval, int intr_num);
void
xhci_sideband_remove_interrupter(struct xhci_sideband *sb);
int
xhci_sideband_interrupter_id(struct xhci_sideband *sb);

#if IS_ENABLED(CONFIG_USB_XHCI_SIDEBAND)
void xhci_sideband_notify_ep_ring_free(struct xhci_sideband *sb,
				       unsigned int ep_index);
#else
static inline void xhci_sideband_notify_ep_ring_free(struct xhci_sideband *sb,
						     unsigned int ep_index)
{ }
#endif 
#endif 
