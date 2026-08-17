/* SPDX-License-Identifier: GPL-2.0+ */


#ifndef __LINUX_CMPXCHG_EMU_H
#define __LINUX_CMPXCHG_EMU_H

uintptr_t cmpxchg_emu_u8(volatile u8 *p, uintptr_t old, uintptr_t new);

#endif 
