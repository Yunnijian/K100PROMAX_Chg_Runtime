/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef _LINUX_SKBUFF_REF_H
#define _LINUX_SKBUFF_REF_H

#include <linux/skbuff.h>


static inline void __skb_frag_ref(skb_frag_t *frag)
{
	get_page(skb_frag_page(frag));
}


static inline void skb_frag_ref(struct sk_buff *skb, int f)
{
	__skb_frag_ref(&skb_shinfo(skb)->frags[f]);
}

bool napi_pp_put_page(netmem_ref netmem);

static inline void skb_page_unref(netmem_ref netmem, bool recycle)
{
#ifdef CONFIG_PAGE_POOL
	if (recycle && napi_pp_put_page(netmem))
		return;
#endif
	put_page(netmem_to_page(netmem));
}


static inline void __skb_frag_unref(skb_frag_t *frag, bool recycle)
{
	skb_page_unref(skb_frag_netmem(frag), recycle);
}


static inline void skb_frag_unref(struct sk_buff *skb, int f)
{
	struct skb_shared_info *shinfo = skb_shinfo(skb);

	if (!skb_zcopy_managed(skb))
		__skb_frag_unref(&shinfo->frags[f], skb->pp_recycle);
}

#endif	
