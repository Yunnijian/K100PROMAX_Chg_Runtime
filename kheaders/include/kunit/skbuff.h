/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _KUNIT_SKBUFF_H
#define _KUNIT_SKBUFF_H

#include <kunit/resource.h>
#include <linux/skbuff.h>

static void kunit_action_kfree_skb(void *p)
{
	kfree_skb((struct sk_buff *)p);
}


static inline struct sk_buff *kunit_zalloc_skb(struct kunit *test, int len,
					       gfp_t gfp)
{
	struct sk_buff *res = alloc_skb(len, gfp);

	if (!res || skb_pad(res, len))
		return NULL;

	if (kunit_add_action_or_reset(test, kunit_action_kfree_skb, res))
		return NULL;

	return res;
}


static inline void kunit_kfree_skb(struct kunit *test, struct sk_buff *skb)
{
	if (!skb)
		return;

	kunit_release_action(test, kunit_action_kfree_skb, (void *)skb);
}

#endif 
