#ifndef _LINUX_GENERIC_RADIX_TREE_H
#define _LINUX_GENERIC_RADIX_TREE_H



#include <asm/page.h>
#include <linux/bug.h>
#include <linux/limits.h>
#include <linux/log2.h>
#include <linux/math.h>
#include <linux/slab.h>
#include <linux/types.h>

struct genradix_root;

#define GENRADIX_NODE_SHIFT	9
#define GENRADIX_NODE_SIZE	(1U << GENRADIX_NODE_SHIFT)

#define GENRADIX_ARY		(GENRADIX_NODE_SIZE / sizeof(struct genradix_node *))
#define GENRADIX_ARY_SHIFT	ilog2(GENRADIX_ARY)


#define GENRADIX_MAX_DEPTH	\
	DIV_ROUND_UP(BITS_PER_LONG - GENRADIX_NODE_SHIFT, GENRADIX_ARY_SHIFT)

#define GENRADIX_DEPTH_MASK				\
	((unsigned long) (roundup_pow_of_two(GENRADIX_MAX_DEPTH + 1) - 1))

static inline int genradix_depth_shift(unsigned depth)
{
	return GENRADIX_NODE_SHIFT + GENRADIX_ARY_SHIFT * depth;
}


static inline size_t genradix_depth_size(unsigned depth)
{
	return 1UL << genradix_depth_shift(depth);
}

static inline unsigned genradix_root_to_depth(struct genradix_root *r)
{
	return (unsigned long) r & GENRADIX_DEPTH_MASK;
}

static inline struct genradix_node *genradix_root_to_node(struct genradix_root *r)
{
	return (void *) ((unsigned long) r & ~GENRADIX_DEPTH_MASK);
}

struct __genradix {
	struct genradix_root		*root;
};

struct genradix_node {
	union {
		
		struct genradix_node	*children[GENRADIX_ARY];

		
		u8			data[GENRADIX_NODE_SIZE];
	};
};

static inline struct genradix_node *genradix_alloc_node(gfp_t gfp_mask)
{
	return kzalloc(GENRADIX_NODE_SIZE, gfp_mask);
}

static inline void genradix_free_node(struct genradix_node *node)
{
	kfree(node);
}



#define __GENRADIX_INITIALIZER					\
	{							\
		.tree = {					\
			.root = NULL,				\
		}						\
	}



#define GENRADIX(_type)						\
struct {							\
	struct __genradix	tree;				\
	_type			type[0] __aligned(1);		\
}

#define DEFINE_GENRADIX(_name, _type)				\
	GENRADIX(_type) _name = __GENRADIX_INITIALIZER


#define genradix_init(_radix)					\
do {								\
	*(_radix) = (typeof(*_radix)) __GENRADIX_INITIALIZER;	\
} while (0)

void __genradix_free(struct __genradix *);


#define genradix_free(_radix)	__genradix_free(&(_radix)->tree)

static inline size_t __idx_to_offset(size_t idx, size_t obj_size)
{
	if (__builtin_constant_p(obj_size))
		BUILD_BUG_ON(obj_size > GENRADIX_NODE_SIZE);
	else
		BUG_ON(obj_size > GENRADIX_NODE_SIZE);

	if (!is_power_of_2(obj_size)) {
		size_t objs_per_page = GENRADIX_NODE_SIZE / obj_size;

		return (idx / objs_per_page) * GENRADIX_NODE_SIZE +
			(idx % objs_per_page) * obj_size;
	} else {
		return idx * obj_size;
	}
}

#define __genradix_cast(_radix)		(typeof((_radix)->type[0]) *)
#define __genradix_obj_size(_radix)	sizeof((_radix)->type[0])
#define __genradix_objs_per_page(_radix)			\
	(GENRADIX_NODE_SIZE / sizeof((_radix)->type[0]))
#define __genradix_page_remainder(_radix)			\
	(GENRADIX_NODE_SIZE % sizeof((_radix)->type[0]))

#define __genradix_idx_to_offset(_radix, _idx)			\
	__idx_to_offset(_idx, __genradix_obj_size(_radix))

static inline void *__genradix_ptr_inlined(struct __genradix *radix, size_t offset)
{
	struct genradix_root *r = READ_ONCE(radix->root);
	struct genradix_node *n = genradix_root_to_node(r);
	unsigned level		= genradix_root_to_depth(r);
	unsigned shift		= genradix_depth_shift(level);

	if (unlikely(ilog2(offset) >= genradix_depth_shift(level)))
		return NULL;

	while (n && shift > GENRADIX_NODE_SHIFT) {
		shift -= GENRADIX_ARY_SHIFT;
		n = n->children[offset >> shift];
		offset &= (1UL << shift) - 1;
	}

	return n ? &n->data[offset] : NULL;
}

#define genradix_ptr_inlined(_radix, _idx)			\
	(__genradix_cast(_radix)				\
	 __genradix_ptr_inlined(&(_radix)->tree,		\
			__genradix_idx_to_offset(_radix, _idx)))

void *__genradix_ptr(struct __genradix *, size_t);


#define genradix_ptr(_radix, _idx)				\
	(__genradix_cast(_radix)				\
	 __genradix_ptr(&(_radix)->tree,			\
			__genradix_idx_to_offset(_radix, _idx)))

void *__genradix_ptr_alloc(struct __genradix *, size_t,
			   struct genradix_node **, gfp_t);

#define genradix_ptr_alloc_inlined(_radix, _idx, _gfp)			\
	(__genradix_cast(_radix)					\
	 (__genradix_ptr_inlined(&(_radix)->tree,			\
			__genradix_idx_to_offset(_radix, _idx)) ?:	\
	  __genradix_ptr_alloc(&(_radix)->tree,				\
			__genradix_idx_to_offset(_radix, _idx),		\
			NULL, _gfp)))

#define genradix_ptr_alloc_preallocated_inlined(_radix, _idx, _new_node, _gfp)\
	(__genradix_cast(_radix)					\
	 (__genradix_ptr_inlined(&(_radix)->tree,			\
			__genradix_idx_to_offset(_radix, _idx)) ?:	\
	  __genradix_ptr_alloc(&(_radix)->tree,				\
			__genradix_idx_to_offset(_radix, _idx),		\
			_new_node, _gfp)))


#define genradix_ptr_alloc(_radix, _idx, _gfp)			\
	(__genradix_cast(_radix)				\
	 __genradix_ptr_alloc(&(_radix)->tree,			\
			__genradix_idx_to_offset(_radix, _idx),	\
			NULL, _gfp))

#define genradix_ptr_alloc_preallocated(_radix, _idx, _new_node, _gfp)\
	(__genradix_cast(_radix)				\
	 __genradix_ptr_alloc(&(_radix)->tree,			\
			__genradix_idx_to_offset(_radix, _idx),	\
			_new_node, _gfp))

struct genradix_iter {
	size_t			offset;
	size_t			pos;
};


#define genradix_iter_init(_radix, _idx)			\
	((struct genradix_iter) {				\
		.pos	= (_idx),				\
		.offset	= __genradix_idx_to_offset((_radix), (_idx)),\
	})

void *__genradix_iter_peek(struct genradix_iter *, struct __genradix *, size_t);


#define genradix_iter_peek(_iter, _radix)			\
	(__genradix_cast(_radix)				\
	 __genradix_iter_peek(_iter, &(_radix)->tree,		\
			__genradix_objs_per_page(_radix)))

void *__genradix_iter_peek_prev(struct genradix_iter *, struct __genradix *,
				size_t, size_t);


#define genradix_iter_peek_prev(_iter, _radix)			\
	(__genradix_cast(_radix)				\
	 __genradix_iter_peek_prev(_iter, &(_radix)->tree,	\
			__genradix_objs_per_page(_radix),	\
			__genradix_obj_size(_radix) +		\
			__genradix_page_remainder(_radix)))

static inline void __genradix_iter_advance(struct genradix_iter *iter,
					   size_t obj_size)
{
	if (iter->offset + obj_size < iter->offset) {
		iter->offset	= SIZE_MAX;
		iter->pos	= SIZE_MAX;
		return;
	}

	iter->offset += obj_size;

	if (!is_power_of_2(obj_size) &&
	    (iter->offset & (GENRADIX_NODE_SIZE - 1)) + obj_size > GENRADIX_NODE_SIZE)
		iter->offset = round_up(iter->offset, GENRADIX_NODE_SIZE);

	iter->pos++;
}

#define genradix_iter_advance(_iter, _radix)			\
	__genradix_iter_advance(_iter, __genradix_obj_size(_radix))

static inline void __genradix_iter_rewind(struct genradix_iter *iter,
					  size_t obj_size)
{
	if (iter->offset == 0 ||
	    iter->offset == SIZE_MAX) {
		iter->offset = SIZE_MAX;
		return;
	}

	if ((iter->offset & (GENRADIX_NODE_SIZE - 1)) == 0)
		iter->offset -= GENRADIX_NODE_SIZE % obj_size;

	iter->offset -= obj_size;
	iter->pos--;
}

#define genradix_iter_rewind(_iter, _radix)			\
	__genradix_iter_rewind(_iter, __genradix_obj_size(_radix))

#define genradix_for_each_from(_radix, _iter, _p, _start)	\
	for (_iter = genradix_iter_init(_radix, _start);	\
	     (_p = genradix_iter_peek(&_iter, _radix)) != NULL;	\
	     genradix_iter_advance(&_iter, _radix))


#define genradix_for_each(_radix, _iter, _p)			\
	genradix_for_each_from(_radix, _iter, _p, 0)

#define genradix_last_pos(_radix)				\
	(SIZE_MAX / GENRADIX_NODE_SIZE * __genradix_objs_per_page(_radix) - 1)


#define genradix_for_each_reverse(_radix, _iter, _p)		\
	for (_iter = genradix_iter_init(_radix,	genradix_last_pos(_radix));\
	     (_p = genradix_iter_peek_prev(&_iter, _radix)) != NULL;\
	     genradix_iter_rewind(&_iter, _radix))

int __genradix_prealloc(struct __genradix *, size_t, gfp_t);


#define genradix_prealloc(_radix, _nr, _gfp)			\
	 __genradix_prealloc(&(_radix)->tree,			\
			__genradix_idx_to_offset(_radix, _nr + 1),\
			_gfp)


#endif 
