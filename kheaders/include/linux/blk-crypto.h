/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __LINUX_BLK_CRYPTO_H
#define __LINUX_BLK_CRYPTO_H

#include <linux/types.h>

enum blk_crypto_mode_num {
	BLK_ENCRYPTION_MODE_INVALID,
	BLK_ENCRYPTION_MODE_AES_256_XTS,
	BLK_ENCRYPTION_MODE_AES_128_CBC_ESSIV,
	BLK_ENCRYPTION_MODE_ADIANTUM,
	BLK_ENCRYPTION_MODE_SM4_XTS,
	BLK_ENCRYPTION_MODE_MAX,
};


enum blk_crypto_key_type {
	
	BLK_CRYPTO_KEY_TYPE_STANDARD = 1 << 0,

	
	BLK_CRYPTO_KEY_TYPE_HW_WRAPPED = 1 << 1,
};


#define BLK_CRYPTO_MAX_STANDARD_KEY_SIZE	64
#define BLK_CRYPTO_MAX_HW_WRAPPED_KEY_SIZE	128


#define BLK_CRYPTO_MAX_ANY_KEY_SIZE \
	(BLK_CRYPTO_MAX_HW_WRAPPED_KEY_SIZE > \
	 BLK_CRYPTO_MAX_STANDARD_KEY_SIZE ? \
	 BLK_CRYPTO_MAX_HW_WRAPPED_KEY_SIZE : BLK_CRYPTO_MAX_STANDARD_KEY_SIZE)


#define BLK_CRYPTO_SW_SECRET_SIZE	32


struct blk_crypto_config {
	enum blk_crypto_mode_num crypto_mode;
	unsigned int data_unit_size;
	unsigned int dun_bytes;
	enum blk_crypto_key_type key_type;
};


struct blk_crypto_key {
	struct blk_crypto_config crypto_cfg;
	unsigned int data_unit_size_bits;
	unsigned int size;
	u8 raw[BLK_CRYPTO_MAX_ANY_KEY_SIZE];
};

#define BLK_CRYPTO_MAX_IV_SIZE		32
#define BLK_CRYPTO_DUN_ARRAY_SIZE	(BLK_CRYPTO_MAX_IV_SIZE / sizeof(u64))


struct bio_crypt_ctx {
	const struct blk_crypto_key	*bc_key;
	u64				bc_dun[BLK_CRYPTO_DUN_ARRAY_SIZE];
};

#include <linux/blk_types.h>
#include <linux/blkdev.h>

#ifdef CONFIG_BLK_INLINE_ENCRYPTION

static inline bool bio_has_crypt_ctx(struct bio *bio)
{
	return bio->bi_crypt_context;
}

void bio_crypt_set_ctx(struct bio *bio, const struct blk_crypto_key *key,
		       const u64 dun[BLK_CRYPTO_DUN_ARRAY_SIZE],
		       gfp_t gfp_mask);

bool bio_crypt_dun_is_contiguous(const struct bio_crypt_ctx *bc,
				 unsigned int bytes,
				 const u64 next_dun[BLK_CRYPTO_DUN_ARRAY_SIZE]);

int blk_crypto_init_key(struct blk_crypto_key *blk_key,
			const u8 *raw_key, size_t raw_key_size,
			enum blk_crypto_key_type key_type,
			enum blk_crypto_mode_num crypto_mode,
			unsigned int dun_bytes,
			unsigned int data_unit_size);

int blk_crypto_start_using_key(struct block_device *bdev,
			       const struct blk_crypto_key *key);

void blk_crypto_evict_key(struct block_device *bdev,
			  const struct blk_crypto_key *key);

bool blk_crypto_config_supported_natively(struct block_device *bdev,
					  const struct blk_crypto_config *cfg);
bool blk_crypto_config_supported(struct block_device *bdev,
				 const struct blk_crypto_config *cfg);

int blk_crypto_derive_sw_secret(struct block_device *bdev,
				const u8 *eph_key, size_t eph_key_size,
				u8 sw_secret[BLK_CRYPTO_SW_SECRET_SIZE]);

#else 

static inline bool bio_has_crypt_ctx(struct bio *bio)
{
	return false;
}

#endif 

static inline void bio_clone_skip_dm_default_key(struct bio *dst,
						 const struct bio *src);

int __bio_crypt_clone(struct bio *dst, struct bio *src, gfp_t gfp_mask);

static inline int bio_crypt_clone(struct bio *dst, struct bio *src,
				  gfp_t gfp_mask)
{
	bio_clone_skip_dm_default_key(dst, src);
	if (bio_has_crypt_ctx(src))
		return __bio_crypt_clone(dst, src, gfp_mask);
	return 0;
}

#if IS_ENABLED(CONFIG_DM_DEFAULT_KEY)
static inline void bio_set_skip_dm_default_key(struct bio *bio)
{
	bio->bi_skip_dm_default_key = true;
}

static inline bool bio_should_skip_dm_default_key(const struct bio *bio)
{
	return bio->bi_skip_dm_default_key;
}

static inline void bio_clone_skip_dm_default_key(struct bio *dst,
						 const struct bio *src)
{
	dst->bi_skip_dm_default_key = src->bi_skip_dm_default_key;
}
#else 
static inline void bio_set_skip_dm_default_key(struct bio *bio)
{
}

static inline bool bio_should_skip_dm_default_key(const struct bio *bio)
{
	return false;
}

static inline void bio_clone_skip_dm_default_key(struct bio *dst,
						 const struct bio *src)
{
}
#endif 

#endif 
