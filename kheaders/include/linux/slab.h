/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#include <linux/cache.h>
#include <linux/gfp.h>
#include <linux/overflow.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/percpu-refcount.h>
#include <linux/cleanup.h>
#include <linux/hash.h>

enum _slab_flag_bits {
	_SLAB_CONSISTENCY_CHECKS,
	_SLAB_RED_ZONE,
	_SLAB_POISON,
	_SLAB_KMALLOC,
	_SLAB_HWCACHE_ALIGN,
	_SLAB_CACHE_DMA,
	_SLAB_CACHE_DMA32,
	_SLAB_STORE_USER,
	_SLAB_PANIC,
	_SLAB_TYPESAFE_BY_RCU,
	_SLAB_TRACE,
#ifdef CONFIG_DEBUG_OBJECTS
	_SLAB_DEBUG_OBJECTS,
#endif
	_SLAB_NOLEAKTRACE,
	_SLAB_NO_MERGE,
#ifdef CONFIG_FAILSLAB
	_SLAB_FAILSLAB,
#endif
#ifdef CONFIG_MEMCG
	_SLAB_ACCOUNT,
#endif
#ifdef CONFIG_KASAN_GENERIC
	_SLAB_KASAN,
#endif
	_SLAB_NO_USER_FLAGS,
#ifdef CONFIG_KFENCE
	_SLAB_SKIP_KFENCE,
#endif
#ifndef CONFIG_SLUB_TINY
	_SLAB_RECLAIM_ACCOUNT,
#endif
	_SLAB_OBJECT_POISON,
	_SLAB_CMPXCHG_DOUBLE,
#ifdef CONFIG_SLAB_OBJ_EXT
	_SLAB_NO_OBJ_EXT,
#endif
	_SLAB_FLAGS_LAST_BIT
};

#define __SLAB_FLAG_BIT(nr)	((slab_flags_t __force)(1U << (nr)))
#define __SLAB_FLAG_UNUSED	((slab_flags_t __force)(0U))



#define SLAB_CONSISTENCY_CHECKS	__SLAB_FLAG_BIT(_SLAB_CONSISTENCY_CHECKS)

#define SLAB_RED_ZONE		__SLAB_FLAG_BIT(_SLAB_RED_ZONE)

#define SLAB_POISON		__SLAB_FLAG_BIT(_SLAB_POISON)

#define SLAB_KMALLOC		__SLAB_FLAG_BIT(_SLAB_KMALLOC)

#define SLAB_HWCACHE_ALIGN	__SLAB_FLAG_BIT(_SLAB_HWCACHE_ALIGN)

#define SLAB_CACHE_DMA		__SLAB_FLAG_BIT(_SLAB_CACHE_DMA)

#define SLAB_CACHE_DMA32	__SLAB_FLAG_BIT(_SLAB_CACHE_DMA32)

#define SLAB_STORE_USER		__SLAB_FLAG_BIT(_SLAB_STORE_USER)

#define SLAB_PANIC		__SLAB_FLAG_BIT(_SLAB_PANIC)


#define SLAB_TYPESAFE_BY_RCU	__SLAB_FLAG_BIT(_SLAB_TYPESAFE_BY_RCU)

#define SLAB_TRACE		__SLAB_FLAG_BIT(_SLAB_TRACE)


#ifdef CONFIG_DEBUG_OBJECTS
# define SLAB_DEBUG_OBJECTS	__SLAB_FLAG_BIT(_SLAB_DEBUG_OBJECTS)
#else
# define SLAB_DEBUG_OBJECTS	__SLAB_FLAG_UNUSED
#endif


#define SLAB_NOLEAKTRACE	__SLAB_FLAG_BIT(_SLAB_NOLEAKTRACE)


#define SLAB_NO_MERGE		__SLAB_FLAG_BIT(_SLAB_NO_MERGE)


#ifdef CONFIG_FAILSLAB
# define SLAB_FAILSLAB		__SLAB_FLAG_BIT(_SLAB_FAILSLAB)
#else
# define SLAB_FAILSLAB		__SLAB_FLAG_UNUSED
#endif

#ifdef CONFIG_MEMCG
# define SLAB_ACCOUNT		__SLAB_FLAG_BIT(_SLAB_ACCOUNT)
#else
# define SLAB_ACCOUNT		__SLAB_FLAG_UNUSED
#endif

#ifdef CONFIG_KASAN_GENERIC
#define SLAB_KASAN		__SLAB_FLAG_BIT(_SLAB_KASAN)
#else
#define SLAB_KASAN		__SLAB_FLAG_UNUSED
#endif


#define SLAB_NO_USER_FLAGS	__SLAB_FLAG_BIT(_SLAB_NO_USER_FLAGS)

#ifdef CONFIG_KFENCE
#define SLAB_SKIP_KFENCE	__SLAB_FLAG_BIT(_SLAB_SKIP_KFENCE)
#else
#define SLAB_SKIP_KFENCE	__SLAB_FLAG_UNUSED
#endif



#ifndef CONFIG_SLUB_TINY
#define SLAB_RECLAIM_ACCOUNT	__SLAB_FLAG_BIT(_SLAB_RECLAIM_ACCOUNT)
#else
#define SLAB_RECLAIM_ACCOUNT	__SLAB_FLAG_UNUSED
#endif
#define SLAB_TEMPORARY		SLAB_RECLAIM_ACCOUNT	


#ifdef CONFIG_SLAB_OBJ_EXT
#define SLAB_NO_OBJ_EXT		__SLAB_FLAG_BIT(_SLAB_NO_OBJ_EXT)
#else
#define SLAB_NO_OBJ_EXT		__SLAB_FLAG_UNUSED
#endif


#define ZERO_SIZE_PTR ((void *)16)

#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= \
				(unsigned long)ZERO_SIZE_PTR)

#include <linux/kasan.h>

struct list_lru;
struct mem_cgroup;

bool slab_is_available(void);


struct kmem_cache_args {
	
	unsigned int align;
	
	unsigned int useroffset;
	
	unsigned int usersize;
	
	unsigned int freeptr_offset;
	
	bool use_freeptr_offset;
	
	void (*ctor)(void *);
};

struct kmem_cache *__kmem_cache_create_args(const char *name,
					    unsigned int object_size,
					    struct kmem_cache_args *args,
					    slab_flags_t flags);
static inline struct kmem_cache *
__kmem_cache_create(const char *name, unsigned int size, unsigned int align,
		    slab_flags_t flags, void (*ctor)(void *))
{
	struct kmem_cache_args kmem_args = {
		.align	= align,
		.ctor	= ctor,
	};

	return __kmem_cache_create_args(name, size, &kmem_args, flags);
}


static inline struct kmem_cache *
kmem_cache_create_usercopy(const char *name, unsigned int size,
			   unsigned int align, slab_flags_t flags,
			   unsigned int useroffset, unsigned int usersize,
			   void (*ctor)(void *))
{
	struct kmem_cache_args kmem_args = {
		.align		= align,
		.ctor		= ctor,
		.useroffset	= useroffset,
		.usersize	= usersize,
	};

	return __kmem_cache_create_args(name, size, &kmem_args, flags);
}


static inline struct kmem_cache *
__kmem_cache_default_args(const char *name, unsigned int size,
			  struct kmem_cache_args *args,
			  slab_flags_t flags)
{
	struct kmem_cache_args kmem_default_args = {};

	
	if (WARN_ON_ONCE(args))
		return ERR_PTR(-EINVAL);

	return __kmem_cache_create_args(name, size, &kmem_default_args, flags);
}


#define kmem_cache_create(__name, __object_size, __args, ...)           \
	_Generic((__args),                                              \
		struct kmem_cache_args *: __kmem_cache_create_args,	\
		void *: __kmem_cache_default_args,			\
		default: __kmem_cache_create)(__name, __object_size, __args, __VA_ARGS__)

void kmem_cache_destroy(struct kmem_cache *s);
int kmem_cache_shrink(struct kmem_cache *s);


#define KMEM_CACHE(__struct, __flags)                                   \
	__kmem_cache_create_args(#__struct, sizeof(struct __struct),    \
			&(struct kmem_cache_args) {			\
				.align	= __alignof__(struct __struct), \
			}, (__flags))


#define KMEM_CACHE_USERCOPY(__struct, __flags, __field)						\
	__kmem_cache_create_args(#__struct, sizeof(struct __struct),				\
			&(struct kmem_cache_args) {						\
				.align		= __alignof__(struct __struct),			\
				.useroffset	= offsetof(struct __struct, __field),		\
				.usersize	= sizeof_field(struct __struct, __field),	\
			}, (__flags))


void * __must_check krealloc_noprof(const void *objp, size_t new_size,
				    gfp_t flags) __realloc_size(2);
#define krealloc(...)				alloc_hooks(krealloc_noprof(__VA_ARGS__))

void kfree(const void *objp);
void kfree_sensitive(const void *objp);
size_t __ksize(const void *objp);

DEFINE_FREE(kfree, void *, if (!IS_ERR_OR_NULL(_T)) kfree(_T))


size_t ksize(const void *objp);

#ifdef CONFIG_PRINTK
bool kmem_dump_obj(void *object);
#else
static inline bool kmem_dump_obj(void *object) { return false; }
#endif


#ifdef ARCH_HAS_DMA_MINALIGN
#if ARCH_DMA_MINALIGN > 8 && !defined(ARCH_KMALLOC_MINALIGN)
#define ARCH_KMALLOC_MINALIGN ARCH_DMA_MINALIGN
#endif
#endif

#ifndef ARCH_KMALLOC_MINALIGN
#define ARCH_KMALLOC_MINALIGN __alignof__(unsigned long long)
#elif ARCH_KMALLOC_MINALIGN > 8
#define KMALLOC_MIN_SIZE ARCH_KMALLOC_MINALIGN
#define KMALLOC_SHIFT_LOW ilog2(KMALLOC_MIN_SIZE)
#endif


#ifndef ARCH_SLAB_MINALIGN
#define ARCH_SLAB_MINALIGN __alignof__(unsigned long long)
#endif


#ifndef arch_slab_minalign
static inline unsigned int arch_slab_minalign(void)
{
	return ARCH_SLAB_MINALIGN;
}
#endif


#define __assume_kmalloc_alignment __assume_aligned(ARCH_KMALLOC_MINALIGN)
#define __assume_slab_alignment __assume_aligned(ARCH_SLAB_MINALIGN)
#define __assume_page_alignment __assume_aligned(PAGE_SIZE)




#define KMALLOC_SHIFT_HIGH	(PAGE_SHIFT + 1)
#define KMALLOC_SHIFT_MAX	(MAX_PAGE_ORDER + PAGE_SHIFT)
#ifndef KMALLOC_SHIFT_LOW
#define KMALLOC_SHIFT_LOW	3
#endif


#define KMALLOC_MAX_SIZE	(1UL << KMALLOC_SHIFT_MAX)

#define KMALLOC_MAX_CACHE_SIZE	(1UL << KMALLOC_SHIFT_HIGH)

#define KMALLOC_MAX_ORDER	(KMALLOC_SHIFT_MAX - PAGE_SHIFT)


#ifndef KMALLOC_MIN_SIZE
#define KMALLOC_MIN_SIZE (1 << KMALLOC_SHIFT_LOW)
#endif


#define SLAB_OBJ_MIN_SIZE      (KMALLOC_MIN_SIZE < 16 ? \
                               (KMALLOC_MIN_SIZE) : 16)

#ifdef CONFIG_RANDOM_KMALLOC_CACHES
#define RANDOM_KMALLOC_CACHES_NR	15 // # of cache copies
#else
#define RANDOM_KMALLOC_CACHES_NR	0
#endif


enum kmalloc_cache_type {
	KMALLOC_NORMAL = 0,
#ifndef CONFIG_ZONE_DMA
	KMALLOC_DMA = KMALLOC_NORMAL,
#endif
#ifndef CONFIG_MEMCG
	KMALLOC_CGROUP = KMALLOC_NORMAL,
#endif
	KMALLOC_RANDOM_START = KMALLOC_NORMAL,
	KMALLOC_RANDOM_END = KMALLOC_RANDOM_START + RANDOM_KMALLOC_CACHES_NR,
#ifdef CONFIG_SLUB_TINY
	KMALLOC_RECLAIM = KMALLOC_NORMAL,
#else
	KMALLOC_RECLAIM,
#endif
#ifdef CONFIG_ZONE_DMA
	KMALLOC_DMA,
#endif
#ifdef CONFIG_MEMCG
	KMALLOC_CGROUP,
#endif
	NR_KMALLOC_TYPES
};

typedef struct kmem_cache * kmem_buckets[KMALLOC_SHIFT_HIGH + 1];

extern kmem_buckets kmalloc_caches[NR_KMALLOC_TYPES];


#define KMALLOC_NOT_NORMAL_BITS					\
	(__GFP_RECLAIMABLE |					\
	(IS_ENABLED(CONFIG_ZONE_DMA)   ? __GFP_DMA : 0) |	\
	(IS_ENABLED(CONFIG_MEMCG) ? __GFP_ACCOUNT : 0))

extern unsigned long random_kmalloc_seed;

static __always_inline enum kmalloc_cache_type kmalloc_type(gfp_t flags, unsigned long caller)
{
	
	if (likely((flags & KMALLOC_NOT_NORMAL_BITS) == 0))
#ifdef CONFIG_RANDOM_KMALLOC_CACHES
		
		return KMALLOC_RANDOM_START + hash_64(caller ^ random_kmalloc_seed,
						      ilog2(RANDOM_KMALLOC_CACHES_NR + 1));
#else
		return KMALLOC_NORMAL;
#endif

	
	if (IS_ENABLED(CONFIG_ZONE_DMA) && (flags & __GFP_DMA))
		return KMALLOC_DMA;
	if (!IS_ENABLED(CONFIG_MEMCG) || (flags & __GFP_RECLAIMABLE))
		return KMALLOC_RECLAIM;
	else
		return KMALLOC_CGROUP;
}


static __always_inline unsigned int __kmalloc_index(size_t size,
						    bool size_is_constant)
{
	if (!size)
		return 0;

	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;

	if (KMALLOC_MIN_SIZE <= 32 && size > 64 && size <= 96)
		return 1;
	if (KMALLOC_MIN_SIZE <= 64 && size > 128 && size <= 192)
		return 2;
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
	if (size <=   4 * 1024) return 12;
	if (size <=   8 * 1024) return 13;
	if (size <=  16 * 1024) return 14;
	if (size <=  32 * 1024) return 15;
	if (size <=  64 * 1024) return 16;
	if (size <= 128 * 1024) return 17;
	if (size <= 256 * 1024) return 18;
	if (size <= 512 * 1024) return 19;
	if (size <= 1024 * 1024) return 20;
	if (size <=  2 * 1024 * 1024) return 21;

	if (!IS_ENABLED(CONFIG_PROFILE_ALL_BRANCHES) && size_is_constant)
		BUILD_BUG_ON_MSG(1, "unexpected size in kmalloc_index()");
	else
		BUG();

	
	return -1;
}
static_assert(PAGE_SHIFT <= 20);
#define kmalloc_index(s) __kmalloc_index(s, true)

#include <linux/alloc_tag.h>


void *kmem_cache_alloc_noprof(struct kmem_cache *cachep,
			      gfp_t flags) __assume_slab_alignment __malloc;
#define kmem_cache_alloc(...)			alloc_hooks(kmem_cache_alloc_noprof(__VA_ARGS__))

void *kmem_cache_alloc_lru_noprof(struct kmem_cache *s, struct list_lru *lru,
			    gfp_t gfpflags) __assume_slab_alignment __malloc;
#define kmem_cache_alloc_lru(...)	alloc_hooks(kmem_cache_alloc_lru_noprof(__VA_ARGS__))


bool kmem_cache_charge(void *objp, gfp_t gfpflags);
void kmem_cache_free(struct kmem_cache *s, void *objp);

kmem_buckets *kmem_buckets_create(const char *name, slab_flags_t flags,
				  unsigned int useroffset, unsigned int usersize,
				  void (*ctor)(void *));


void kmem_cache_free_bulk(struct kmem_cache *s, size_t size, void **p);

int kmem_cache_alloc_bulk_noprof(struct kmem_cache *s, gfp_t flags, size_t size, void **p);
#define kmem_cache_alloc_bulk(...)	alloc_hooks(kmem_cache_alloc_bulk_noprof(__VA_ARGS__))

static __always_inline void kfree_bulk(size_t size, void **p)
{
	kmem_cache_free_bulk(NULL, size, p);
}

void *kmem_cache_alloc_node_noprof(struct kmem_cache *s, gfp_t flags,
				   int node) __assume_slab_alignment __malloc;
#define kmem_cache_alloc_node(...)	alloc_hooks(kmem_cache_alloc_node_noprof(__VA_ARGS__))


#ifdef CONFIG_SLAB_BUCKETS
#define DECL_BUCKET_PARAMS(_size, _b)	size_t (_size), kmem_buckets *(_b)
#define PASS_BUCKET_PARAMS(_size, _b)	(_size), (_b)
#define PASS_BUCKET_PARAM(_b)		(_b)
#else
#define DECL_BUCKET_PARAMS(_size, _b)	size_t (_size)
#define PASS_BUCKET_PARAMS(_size, _b)	(_size)
#define PASS_BUCKET_PARAM(_b)		NULL
#endif



void *__kmalloc_noprof(size_t size, gfp_t flags)
				__assume_kmalloc_alignment __alloc_size(1);

void *__kmalloc_node_noprof(DECL_BUCKET_PARAMS(size, b), gfp_t flags, int node)
				__assume_kmalloc_alignment __alloc_size(1);

void *__kmalloc_cache_noprof(struct kmem_cache *s, gfp_t flags, size_t size)
				__assume_kmalloc_alignment __alloc_size(3);

void *__kmalloc_cache_node_noprof(struct kmem_cache *s, gfp_t gfpflags,
				  int node, size_t size)
				__assume_kmalloc_alignment __alloc_size(4);

void *__kmalloc_large_noprof(size_t size, gfp_t flags)
				__assume_page_alignment __alloc_size(1);

void *__kmalloc_large_node_noprof(size_t size, gfp_t flags, int node)
				__assume_page_alignment __alloc_size(1);


static __always_inline __alloc_size(1) void *kmalloc_noprof(size_t size, gfp_t flags)
{
	if (__builtin_constant_p(size) && size) {
		unsigned int index;

		if (size > KMALLOC_MAX_CACHE_SIZE)
			return __kmalloc_large_noprof(size, flags);

		index = kmalloc_index(size);
		return __kmalloc_cache_noprof(
				kmalloc_caches[kmalloc_type(flags, _RET_IP_)][index],
				flags, size);
	}
	return __kmalloc_noprof(size, flags);
}
#define kmalloc(...)				alloc_hooks(kmalloc_noprof(__VA_ARGS__))

#define kmem_buckets_alloc(_b, _size, _flags)	\
	alloc_hooks(__kmalloc_node_noprof(PASS_BUCKET_PARAMS(_size, _b), _flags, NUMA_NO_NODE))

#define kmem_buckets_alloc_track_caller(_b, _size, _flags)	\
	alloc_hooks(__kmalloc_node_track_caller_noprof(PASS_BUCKET_PARAMS(_size, _b), _flags, NUMA_NO_NODE, _RET_IP_))

static __always_inline __alloc_size(1) void *kmalloc_node_noprof(size_t size, gfp_t flags, int node)
{
	if (__builtin_constant_p(size) && size) {
		unsigned int index;

		if (size > KMALLOC_MAX_CACHE_SIZE)
			return __kmalloc_large_node_noprof(size, flags, node);

		index = kmalloc_index(size);
		return __kmalloc_cache_node_noprof(
				kmalloc_caches[kmalloc_type(flags, _RET_IP_)][index],
				flags, node, size);
	}
	return __kmalloc_node_noprof(PASS_BUCKET_PARAMS(size, NULL), flags, node);
}
#define kmalloc_node(...)			alloc_hooks(kmalloc_node_noprof(__VA_ARGS__))


static inline __alloc_size(1, 2) void *kmalloc_array_noprof(size_t n, size_t size, gfp_t flags)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;
	if (__builtin_constant_p(n) && __builtin_constant_p(size))
		return kmalloc_noprof(bytes, flags);
	return kmalloc_noprof(bytes, flags);
}
#define kmalloc_array(...)			alloc_hooks(kmalloc_array_noprof(__VA_ARGS__))


static inline __realloc_size(2, 3) void * __must_check krealloc_array_noprof(void *p,
								       size_t new_n,
								       size_t new_size,
								       gfp_t flags)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(new_n, new_size, &bytes)))
		return NULL;

	return krealloc_noprof(p, bytes, flags);
}
#define krealloc_array(...)			alloc_hooks(krealloc_array_noprof(__VA_ARGS__))


#define kcalloc(n, size, flags)		kmalloc_array(n, size, (flags) | __GFP_ZERO)

void *__kmalloc_node_track_caller_noprof(DECL_BUCKET_PARAMS(size, b), gfp_t flags, int node,
					 unsigned long caller) __alloc_size(1);
#define kmalloc_node_track_caller_noprof(size, flags, node, caller) \
	__kmalloc_node_track_caller_noprof(PASS_BUCKET_PARAMS(size, NULL), flags, node, caller)
#define kmalloc_node_track_caller(...)		\
	alloc_hooks(kmalloc_node_track_caller_noprof(__VA_ARGS__, _RET_IP_))


#define kmalloc_track_caller(...)		kmalloc_node_track_caller(__VA_ARGS__, NUMA_NO_NODE)

#define kmalloc_track_caller_noprof(...)	\
		kmalloc_node_track_caller_noprof(__VA_ARGS__, NUMA_NO_NODE, _RET_IP_)

static inline __alloc_size(1, 2) void *kmalloc_array_node_noprof(size_t n, size_t size, gfp_t flags,
							  int node)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;
	if (__builtin_constant_p(n) && __builtin_constant_p(size))
		return kmalloc_node_noprof(bytes, flags, node);
	return __kmalloc_node_noprof(PASS_BUCKET_PARAMS(bytes, NULL), flags, node);
}
#define kmalloc_array_node(...)			alloc_hooks(kmalloc_array_node_noprof(__VA_ARGS__))

#define kcalloc_node(_n, _size, _flags, _node)	\
	kmalloc_array_node(_n, _size, (_flags) | __GFP_ZERO, _node)


#define kmem_cache_zalloc(_k, _flags)		kmem_cache_alloc(_k, (_flags)|__GFP_ZERO)


static inline __alloc_size(1) void *kzalloc_noprof(size_t size, gfp_t flags)
{
	return kmalloc_noprof(size, flags | __GFP_ZERO);
}
#define kzalloc(...)				alloc_hooks(kzalloc_noprof(__VA_ARGS__))
#define kzalloc_node(_size, _flags, _node)	kmalloc_node(_size, (_flags)|__GFP_ZERO, _node)

void *__kvmalloc_node_noprof(DECL_BUCKET_PARAMS(size, b), gfp_t flags, int node) __alloc_size(1);
#define kvmalloc_node_noprof(size, flags, node)	\
	__kvmalloc_node_noprof(PASS_BUCKET_PARAMS(size, NULL), flags, node)
#define kvmalloc_node(...)			alloc_hooks(kvmalloc_node_noprof(__VA_ARGS__))

#define kvmalloc(_size, _flags)			kvmalloc_node(_size, _flags, NUMA_NO_NODE)
#define kvmalloc_noprof(_size, _flags)		kvmalloc_node_noprof(_size, _flags, NUMA_NO_NODE)
#define kvzalloc(_size, _flags)			kvmalloc(_size, (_flags)|__GFP_ZERO)

#define kvzalloc_node(_size, _flags, _node)	kvmalloc_node(_size, (_flags)|__GFP_ZERO, _node)
#define kmem_buckets_valloc(_b, _size, _flags)	\
	alloc_hooks(__kvmalloc_node_noprof(PASS_BUCKET_PARAMS(_size, _b), _flags, NUMA_NO_NODE))

static inline __alloc_size(1, 2) void *
kvmalloc_array_node_noprof(size_t n, size_t size, gfp_t flags, int node)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;

	return kvmalloc_node_noprof(bytes, flags, node);
}

#define kvmalloc_array_noprof(...)		kvmalloc_array_node_noprof(__VA_ARGS__, NUMA_NO_NODE)
#define kvcalloc_node_noprof(_n,_s,_f,_node)	kvmalloc_array_node_noprof(_n,_s,(_f)|__GFP_ZERO,_node)
#define kvcalloc_noprof(...)			kvcalloc_node_noprof(__VA_ARGS__, NUMA_NO_NODE)

#define kvmalloc_array(...)			alloc_hooks(kvmalloc_array_noprof(__VA_ARGS__))
#define kvcalloc_node(...)			alloc_hooks(kvcalloc_node_noprof(__VA_ARGS__))
#define kvcalloc(...)				alloc_hooks(kvcalloc_noprof(__VA_ARGS__))

void *kvrealloc_noprof(const void *p, size_t size, gfp_t flags)
		__realloc_size(2);
#define kvrealloc(...)				alloc_hooks(kvrealloc_noprof(__VA_ARGS__))

extern void kvfree(const void *addr);
DEFINE_FREE(kvfree, void *, if (!IS_ERR_OR_NULL(_T)) kvfree(_T))

extern void kvfree_sensitive(const void *addr, size_t len);

unsigned int kmem_cache_size(struct kmem_cache *s);


size_t kmalloc_size_roundup(size_t size);

void __init kmem_cache_init_late(void);

#endif	
