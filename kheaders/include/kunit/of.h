/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _KUNIT_OF_H
#define _KUNIT_OF_H

#include <kunit/test.h>

struct device_node;

#ifdef CONFIG_OF

void of_node_put_kunit(struct kunit *test, struct device_node *node);

#else

static inline
void of_node_put_kunit(struct kunit *test, struct device_node *node)
{
	kunit_skip(test, "requires CONFIG_OF");
}

#endif 

#if defined(CONFIG_OF) && defined(CONFIG_OF_OVERLAY) && defined(CONFIG_OF_EARLY_FLATTREE)

int of_overlay_fdt_apply_kunit(struct kunit *test, void *overlay_fdt,
			       u32 overlay_fdt_size, int *ovcs_id);
#else

static inline int
of_overlay_fdt_apply_kunit(struct kunit *test, void *overlay_fdt,
			   u32 overlay_fdt_size, int *ovcs_id)
{
	kunit_skip(test, "requires CONFIG_OF and CONFIG_OF_OVERLAY and CONFIG_OF_EARLY_FLATTREE for root node");
	return -EINVAL;
}

#endif


static inline int __of_overlay_apply_kunit(struct kunit *test,
					   u8 *overlay_begin,
					   const u8 *overlay_end)
{
	int unused;

	return of_overlay_fdt_apply_kunit(test, overlay_begin,
					  overlay_end - overlay_begin,
					  &unused);
}


#define of_overlay_apply_kunit(test, overlay_name)		\
({								\
	extern uint8_t __dtbo_##overlay_name##_begin[];		\
	extern uint8_t __dtbo_##overlay_name##_end[];		\
								\
	__of_overlay_apply_kunit((test),			\
			__dtbo_##overlay_name##_begin,		\
			__dtbo_##overlay_name##_end);		\
})

#endif
