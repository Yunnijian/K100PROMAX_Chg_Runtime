/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MIN_HEAP_H
#define _LINUX_MIN_HEAP_H

#include <linux/bug.h>
#include <linux/string.h>
#include <linux/types.h>


#define MIN_HEAP_PREALLOCATED(_type, _name, _nr)	\
struct _name {	\
	int nr;	\
	int size;	\
	_type *data;	\
	_type preallocated[_nr];	\
}

#define DEFINE_MIN_HEAP(_type, _name) MIN_HEAP_PREALLOCATED(_type, _name, 0)

typedef DEFINE_MIN_HEAP(char, min_heap_char) min_heap_char;

#define __minheap_cast(_heap)		(typeof((_heap)->data[0]) *)
#define __minheap_obj_size(_heap)	sizeof((_heap)->data[0])


struct min_heap_callbacks {
	bool (*less)(const void *lhs, const void *rhs, void *args);
	void (*swp)(void *lhs, void *rhs, void *args);
};


static __always_inline
void __min_heap_init(min_heap_char *heap, void *data, int size)
{
	heap->nr = 0;
	heap->size = size;
	if (data)
		heap->data = data;
	else
		heap->data = heap->preallocated;
}

#define min_heap_init(_heap, _data, _size)	\
	__min_heap_init((min_heap_char *)_heap, _data, _size)


static __always_inline
void *__min_heap_peek(struct min_heap_char *heap)
{
	return heap->nr ? heap->data : NULL;
}

#define min_heap_peek(_heap)	\
	(__minheap_cast(_heap) __min_heap_peek((min_heap_char *)_heap))


static __always_inline
bool __min_heap_full(min_heap_char *heap)
{
	return heap->nr == heap->size;
}

#define min_heap_full(_heap)	\
	__min_heap_full((min_heap_char *)_heap)


static __always_inline
void __min_heap_sift_down(min_heap_char *heap, int pos, size_t elem_size,
		const struct min_heap_callbacks *func, void *args)
{
	void *left, *right;
	void *data = heap->data;
	void *root = data + pos * elem_size;
	int i = pos, j;

	
	for (;;) {
		if (i * 2 + 2 >= heap->nr)
			break;
		left = data + (i * 2 + 1) * elem_size;
		right = data + (i * 2 + 2) * elem_size;
		i = func->less(left, right, args) ? i * 2 + 1 : i * 2 + 2;
	}

	
	if (i * 2 + 2 == heap->nr)
		i = i * 2 + 1;

	
	while (i != pos && func->less(root, data + i * elem_size, args))
		i = (i - 1) / 2;

	
	j = i;
	while (i != pos) {
		i = (i - 1) / 2;
		func->swp(data + i * elem_size, data + j * elem_size, args);
	}
}

#define min_heap_sift_down(_heap, _pos, _func, _args)	\
	__min_heap_sift_down((min_heap_char *)_heap, _pos, __minheap_obj_size(_heap), _func, _args)


static __always_inline
void __min_heap_sift_up(min_heap_char *heap, size_t elem_size, size_t idx,
		const struct min_heap_callbacks *func, void *args)
{
	void *data = heap->data;
	size_t parent;

	while (idx) {
		parent = (idx - 1) / 2;
		if (func->less(data + parent * elem_size, data + idx * elem_size, args))
			break;
		func->swp(data + parent * elem_size, data + idx * elem_size, args);
		idx = parent;
	}
}

#define min_heap_sift_up(_heap, _idx, _func, _args)	\
	__min_heap_sift_up((min_heap_char *)_heap, __minheap_obj_size(_heap), _idx, _func, _args)


static __always_inline
void __min_heapify_all(min_heap_char *heap, size_t elem_size,
		const struct min_heap_callbacks *func, void *args)
{
	int i;

	for (i = heap->nr / 2 - 1; i >= 0; i--)
		__min_heap_sift_down(heap, i, elem_size, func, args);
}

#define min_heapify_all(_heap, _func, _args)	\
	__min_heapify_all((min_heap_char *)_heap, __minheap_obj_size(_heap), _func, _args)


static __always_inline
bool __min_heap_pop(min_heap_char *heap, size_t elem_size,
		const struct min_heap_callbacks *func, void *args)
{
	void *data = heap->data;

	if (WARN_ONCE(heap->nr <= 0, "Popping an empty heap"))
		return false;

	
	heap->nr--;
	memcpy(data, data + (heap->nr * elem_size), elem_size);
	__min_heap_sift_down(heap, 0, elem_size, func, args);

	return true;
}

#define min_heap_pop(_heap, _func, _args)	\
	__min_heap_pop((min_heap_char *)_heap, __minheap_obj_size(_heap), _func, _args)


static __always_inline
void __min_heap_pop_push(min_heap_char *heap,
		const void *element, size_t elem_size,
		const struct min_heap_callbacks *func,
		void *args)
{
	memcpy(heap->data, element, elem_size);
	__min_heap_sift_down(heap, 0, elem_size, func, args);
}

#define min_heap_pop_push(_heap, _element, _func, _args)	\
	__min_heap_pop_push((min_heap_char *)_heap, _element, __minheap_obj_size(_heap), _func, _args)


static __always_inline
bool __min_heap_push(min_heap_char *heap, const void *element, size_t elem_size,
		const struct min_heap_callbacks *func, void *args)
{
	void *data = heap->data;
	int pos;

	if (WARN_ONCE(heap->nr >= heap->size, "Pushing on a full heap"))
		return false;

	
	pos = heap->nr;
	memcpy(data + (pos * elem_size), element, elem_size);
	heap->nr++;

	
	__min_heap_sift_up(heap, elem_size, pos, func, args);

	return true;
}

#define min_heap_push(_heap, _element, _func, _args)	\
	__min_heap_push((min_heap_char *)_heap, _element, __minheap_obj_size(_heap), _func, _args)


static __always_inline
bool __min_heap_del(min_heap_char *heap, size_t elem_size, size_t idx,
		const struct min_heap_callbacks *func, void *args)
{
	void *data = heap->data;

	if (WARN_ONCE(heap->nr <= 0, "Popping an empty heap"))
		return false;

	
	heap->nr--;
	if (idx == heap->nr)
		return true;
	func->swp(data + (idx * elem_size), data + (heap->nr * elem_size), args);
	__min_heap_sift_up(heap, elem_size, idx, func, args);
	__min_heap_sift_down(heap, idx, elem_size, func, args);

	return true;
}

#define min_heap_del(_heap, _idx, _func, _args)	\
	__min_heap_del((min_heap_char *)_heap, __minheap_obj_size(_heap), _idx, _func, _args)

#endif 
