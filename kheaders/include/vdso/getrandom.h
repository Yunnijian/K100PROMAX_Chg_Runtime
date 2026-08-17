/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _VDSO_GETRANDOM_H
#define _VDSO_GETRANDOM_H

#include <linux/types.h>

#define CHACHA_KEY_SIZE         32
#define CHACHA_BLOCK_SIZE       64


struct vgetrandom_state {
	union {
		struct {
			u8	batch[CHACHA_BLOCK_SIZE * 3 / 2];
			u32	key[CHACHA_KEY_SIZE / sizeof(u32)];
		};
		u8		batch_key[CHACHA_BLOCK_SIZE * 2];
	};
	u64			generation;
	u8			pos;
	bool 			in_use;
};


extern void __arch_chacha20_blocks_nostack(u8 *dst_bytes, const u32 *key, u32 *counter, size_t nblocks);


extern ssize_t __vdso_getrandom(void *buffer, size_t len, unsigned int flags, void *opaque_state, size_t opaque_len);

#endif 
