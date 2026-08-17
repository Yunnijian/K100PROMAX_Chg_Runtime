/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SYNC_CORE_H
#define _LINUX_SYNC_CORE_H

#ifdef CONFIG_ARCH_HAS_SYNC_CORE_BEFORE_USERMODE
#include <asm/sync_core.h>
#else

static inline void sync_core_before_usermode(void)
{
}
#endif

#ifdef CONFIG_ARCH_HAS_PREPARE_SYNC_CORE_CMD
#include <asm/sync_core.h>
#else

static inline void prepare_sync_core_cmd(struct mm_struct *mm)
{
}
#endif

#endif 
