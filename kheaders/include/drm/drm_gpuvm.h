/* SPDX-License-Identifier: GPL-2.0-only OR MIT */

#ifndef __DRM_GPUVM_H__
#define __DRM_GPUVM_H__



#include <linux/dma-resv.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/types.h>

#include <drm/drm_device.h>
#include <drm/drm_gem.h>
#include <drm/drm_exec.h>

struct drm_gpuvm;
struct drm_gpuvm_bo;
struct drm_gpuvm_ops;


enum drm_gpuva_flags {
	
	DRM_GPUVA_INVALIDATED = (1 << 0),

	
	DRM_GPUVA_SPARSE = (1 << 1),

	
	DRM_GPUVA_USERBITS = (1 << 2),
};


struct drm_gpuva {
	
	struct drm_gpuvm *vm;

	
	struct drm_gpuvm_bo *vm_bo;

	
	enum drm_gpuva_flags flags;

	
	struct {
		
		u64 addr;

		
		u64 range;
	} va;

	
	struct {
		
		u64 offset;

		
		struct drm_gem_object *obj;

		
		struct list_head entry;
	} gem;

	
	struct {
		
		struct rb_node node;

		
		struct list_head entry;

		
		u64 __subtree_last;
	} rb;
};

int drm_gpuva_insert(struct drm_gpuvm *gpuvm, struct drm_gpuva *va);
void drm_gpuva_remove(struct drm_gpuva *va);

void drm_gpuva_link(struct drm_gpuva *va, struct drm_gpuvm_bo *vm_bo);
void drm_gpuva_unlink(struct drm_gpuva *va);

struct drm_gpuva *drm_gpuva_find(struct drm_gpuvm *gpuvm,
				 u64 addr, u64 range);
struct drm_gpuva *drm_gpuva_find_first(struct drm_gpuvm *gpuvm,
				       u64 addr, u64 range);
struct drm_gpuva *drm_gpuva_find_prev(struct drm_gpuvm *gpuvm, u64 start);
struct drm_gpuva *drm_gpuva_find_next(struct drm_gpuvm *gpuvm, u64 end);

static inline void drm_gpuva_init(struct drm_gpuva *va, u64 addr, u64 range,
				  struct drm_gem_object *obj, u64 offset)
{
	va->va.addr = addr;
	va->va.range = range;
	va->gem.obj = obj;
	va->gem.offset = offset;
}


static inline void drm_gpuva_invalidate(struct drm_gpuva *va, bool invalidate)
{
	if (invalidate)
		va->flags |= DRM_GPUVA_INVALIDATED;
	else
		va->flags &= ~DRM_GPUVA_INVALIDATED;
}


static inline bool drm_gpuva_invalidated(struct drm_gpuva *va)
{
	return va->flags & DRM_GPUVA_INVALIDATED;
}


enum drm_gpuvm_flags {
	
	DRM_GPUVM_RESV_PROTECTED = BIT(0),

	
	DRM_GPUVM_USERBITS = BIT(1),
};


struct drm_gpuvm {
	
	const char *name;

	
	enum drm_gpuvm_flags flags;

	
	struct drm_device *drm;

	
	u64 mm_start;

	
	u64 mm_range;

	
	struct {
		
		struct rb_root_cached tree;

		
		struct list_head list;
	} rb;

	
	struct kref kref;

	
	struct drm_gpuva kernel_alloc_node;

	
	const struct drm_gpuvm_ops *ops;

	
	struct drm_gem_object *r_obj;

	
	struct {
		
		struct list_head list;

		
		struct list_head *local_list;

		
		spinlock_t lock;
	} extobj;

	
	struct {
		
		struct list_head list;

		
		struct list_head *local_list;

		
		spinlock_t lock;
	} evict;
};

void drm_gpuvm_init(struct drm_gpuvm *gpuvm, const char *name,
		    enum drm_gpuvm_flags flags,
		    struct drm_device *drm,
		    struct drm_gem_object *r_obj,
		    u64 start_offset, u64 range,
		    u64 reserve_offset, u64 reserve_range,
		    const struct drm_gpuvm_ops *ops);


static inline struct drm_gpuvm *
drm_gpuvm_get(struct drm_gpuvm *gpuvm)
{
	kref_get(&gpuvm->kref);

	return gpuvm;
}

void drm_gpuvm_put(struct drm_gpuvm *gpuvm);

bool drm_gpuvm_range_valid(struct drm_gpuvm *gpuvm, u64 addr, u64 range);
bool drm_gpuvm_interval_empty(struct drm_gpuvm *gpuvm, u64 addr, u64 range);

struct drm_gem_object *
drm_gpuvm_resv_object_alloc(struct drm_device *drm);


static inline bool
drm_gpuvm_resv_protected(struct drm_gpuvm *gpuvm)
{
	return gpuvm->flags & DRM_GPUVM_RESV_PROTECTED;
}


#define drm_gpuvm_resv(gpuvm__) ((gpuvm__)->r_obj->resv)


#define drm_gpuvm_resv_obj(gpuvm__) ((gpuvm__)->r_obj)

#define drm_gpuvm_resv_held(gpuvm__) \
	dma_resv_held(drm_gpuvm_resv(gpuvm__))

#define drm_gpuvm_resv_assert_held(gpuvm__) \
	dma_resv_assert_held(drm_gpuvm_resv(gpuvm__))

#define drm_gpuvm_resv_held(gpuvm__) \
	dma_resv_held(drm_gpuvm_resv(gpuvm__))

#define drm_gpuvm_resv_assert_held(gpuvm__) \
	dma_resv_assert_held(drm_gpuvm_resv(gpuvm__))


static inline bool
drm_gpuvm_is_extobj(struct drm_gpuvm *gpuvm,
		    struct drm_gem_object *obj)
{
	return obj && obj->resv != drm_gpuvm_resv(gpuvm);
}

static inline struct drm_gpuva *
__drm_gpuva_next(struct drm_gpuva *va)
{
	if (va && !list_is_last(&va->rb.entry, &va->vm->rb.list))
		return list_next_entry(va, rb.entry);

	return NULL;
}


#define drm_gpuvm_for_each_va_range(va__, gpuvm__, start__, end__) \
	for (va__ = drm_gpuva_find_first((gpuvm__), (start__), (end__) - (start__)); \
	     va__ && (va__->va.addr < (end__)); \
	     va__ = __drm_gpuva_next(va__))


#define drm_gpuvm_for_each_va_range_safe(va__, next__, gpuvm__, start__, end__) \
	for (va__ = drm_gpuva_find_first((gpuvm__), (start__), (end__) - (start__)), \
	     next__ = __drm_gpuva_next(va__); \
	     va__ && (va__->va.addr < (end__)); \
	     va__ = next__, next__ = __drm_gpuva_next(va__))


#define drm_gpuvm_for_each_va(va__, gpuvm__) \
	list_for_each_entry(va__, &(gpuvm__)->rb.list, rb.entry)


#define drm_gpuvm_for_each_va_safe(va__, next__, gpuvm__) \
	list_for_each_entry_safe(va__, next__, &(gpuvm__)->rb.list, rb.entry)


struct drm_gpuvm_exec {
	
	struct drm_exec exec;

	
	u32 flags;

	
	struct drm_gpuvm *vm;

	
	unsigned int num_fences;

	
	struct {
		
		int (*fn)(struct drm_gpuvm_exec *vm_exec);

		
		void *priv;
	} extra;
};

int drm_gpuvm_prepare_vm(struct drm_gpuvm *gpuvm,
			 struct drm_exec *exec,
			 unsigned int num_fences);

int drm_gpuvm_prepare_objects(struct drm_gpuvm *gpuvm,
			      struct drm_exec *exec,
			      unsigned int num_fences);

int drm_gpuvm_prepare_range(struct drm_gpuvm *gpuvm,
			    struct drm_exec *exec,
			    u64 addr, u64 range,
			    unsigned int num_fences);

int drm_gpuvm_exec_lock(struct drm_gpuvm_exec *vm_exec);

int drm_gpuvm_exec_lock_array(struct drm_gpuvm_exec *vm_exec,
			      struct drm_gem_object **objs,
			      unsigned int num_objs);

int drm_gpuvm_exec_lock_range(struct drm_gpuvm_exec *vm_exec,
			      u64 addr, u64 range);


static inline void
drm_gpuvm_exec_unlock(struct drm_gpuvm_exec *vm_exec)
{
	drm_exec_fini(&vm_exec->exec);
}

int drm_gpuvm_validate(struct drm_gpuvm *gpuvm, struct drm_exec *exec);
void drm_gpuvm_resv_add_fence(struct drm_gpuvm *gpuvm,
			      struct drm_exec *exec,
			      struct dma_fence *fence,
			      enum dma_resv_usage private_usage,
			      enum dma_resv_usage extobj_usage);


static inline void
drm_gpuvm_exec_resv_add_fence(struct drm_gpuvm_exec *vm_exec,
			      struct dma_fence *fence,
			      enum dma_resv_usage private_usage,
			      enum dma_resv_usage extobj_usage)
{
	drm_gpuvm_resv_add_fence(vm_exec->vm, &vm_exec->exec, fence,
				 private_usage, extobj_usage);
}


static inline int
drm_gpuvm_exec_validate(struct drm_gpuvm_exec *vm_exec)
{
	return drm_gpuvm_validate(vm_exec->vm, &vm_exec->exec);
}


struct drm_gpuvm_bo {
	
	struct drm_gpuvm *vm;

	
	struct drm_gem_object *obj;

	
	bool evicted;

	
	struct kref kref;

	
	struct {
		
		struct list_head gpuva;

		
		struct {
			
			struct list_head gem;

			
			struct list_head extobj;

			
			struct list_head evict;
		} entry;
	} list;
};

struct drm_gpuvm_bo *
drm_gpuvm_bo_create(struct drm_gpuvm *gpuvm,
		    struct drm_gem_object *obj);

struct drm_gpuvm_bo *
drm_gpuvm_bo_obtain(struct drm_gpuvm *gpuvm,
		    struct drm_gem_object *obj);
struct drm_gpuvm_bo *
drm_gpuvm_bo_obtain_prealloc(struct drm_gpuvm_bo *vm_bo);


static inline struct drm_gpuvm_bo *
drm_gpuvm_bo_get(struct drm_gpuvm_bo *vm_bo)
{
	kref_get(&vm_bo->kref);
	return vm_bo;
}

bool drm_gpuvm_bo_put(struct drm_gpuvm_bo *vm_bo);

struct drm_gpuvm_bo *
drm_gpuvm_bo_find(struct drm_gpuvm *gpuvm,
		  struct drm_gem_object *obj);

void drm_gpuvm_bo_evict(struct drm_gpuvm_bo *vm_bo, bool evict);


static inline void
drm_gpuvm_bo_gem_evict(struct drm_gem_object *obj, bool evict)
{
	struct drm_gpuvm_bo *vm_bo;

	drm_gem_gpuva_assert_lock_held(obj);
	drm_gem_for_each_gpuvm_bo(vm_bo, obj)
		drm_gpuvm_bo_evict(vm_bo, evict);
}

void drm_gpuvm_bo_extobj_add(struct drm_gpuvm_bo *vm_bo);


#define drm_gpuvm_bo_for_each_va(va__, vm_bo__) \
	list_for_each_entry(va__, &(vm_bo)->list.gpuva, gem.entry)


#define drm_gpuvm_bo_for_each_va_safe(va__, next__, vm_bo__) \
	list_for_each_entry_safe(va__, next__, &(vm_bo)->list.gpuva, gem.entry)


enum drm_gpuva_op_type {
	
	DRM_GPUVA_OP_MAP,

	
	DRM_GPUVA_OP_REMAP,

	
	DRM_GPUVA_OP_UNMAP,

	
	DRM_GPUVA_OP_PREFETCH,
};


struct drm_gpuva_op_map {
	
	struct {
		
		u64 addr;

		
		u64 range;
	} va;

	
	struct {
		
		u64 offset;

		
		struct drm_gem_object *obj;
	} gem;
};


struct drm_gpuva_op_unmap {
	
	struct drm_gpuva *va;

	
	bool keep;
};


struct drm_gpuva_op_remap {
	
	struct drm_gpuva_op_map *prev;

	
	struct drm_gpuva_op_map *next;

	
	struct drm_gpuva_op_unmap *unmap;
};


struct drm_gpuva_op_prefetch {
	
	struct drm_gpuva *va;
};


struct drm_gpuva_op {
	
	struct list_head entry;

	
	enum drm_gpuva_op_type op;

	union {
		
		struct drm_gpuva_op_map map;

		
		struct drm_gpuva_op_remap remap;

		
		struct drm_gpuva_op_unmap unmap;

		
		struct drm_gpuva_op_prefetch prefetch;
	};
};


struct drm_gpuva_ops {
	
	struct list_head list;
};


#define drm_gpuva_for_each_op(op, ops) list_for_each_entry(op, &(ops)->list, entry)


#define drm_gpuva_for_each_op_safe(op, next, ops) \
	list_for_each_entry_safe(op, next, &(ops)->list, entry)


#define drm_gpuva_for_each_op_from_reverse(op, ops) \
	list_for_each_entry_from_reverse(op, &(ops)->list, entry)


#define drm_gpuva_for_each_op_reverse(op, ops) \
	list_for_each_entry_reverse(op, &(ops)->list, entry)


#define drm_gpuva_first_op(ops) \
	list_first_entry(&(ops)->list, struct drm_gpuva_op, entry)


#define drm_gpuva_last_op(ops) \
	list_last_entry(&(ops)->list, struct drm_gpuva_op, entry)


#define drm_gpuva_prev_op(op) list_prev_entry(op, entry)


#define drm_gpuva_next_op(op) list_next_entry(op, entry)

struct drm_gpuva_ops *
drm_gpuvm_sm_map_ops_create(struct drm_gpuvm *gpuvm,
			    u64 addr, u64 range,
			    struct drm_gem_object *obj, u64 offset);
struct drm_gpuva_ops *
drm_gpuvm_sm_unmap_ops_create(struct drm_gpuvm *gpuvm,
			      u64 addr, u64 range);

struct drm_gpuva_ops *
drm_gpuvm_prefetch_ops_create(struct drm_gpuvm *gpuvm,
				 u64 addr, u64 range);

struct drm_gpuva_ops *
drm_gpuvm_bo_unmap_ops_create(struct drm_gpuvm_bo *vm_bo);

void drm_gpuva_ops_free(struct drm_gpuvm *gpuvm,
			struct drm_gpuva_ops *ops);

static inline void drm_gpuva_init_from_op(struct drm_gpuva *va,
					  struct drm_gpuva_op_map *op)
{
	drm_gpuva_init(va, op->va.addr, op->va.range,
		       op->gem.obj, op->gem.offset);
}


struct drm_gpuvm_ops {
	
	void (*vm_free)(struct drm_gpuvm *gpuvm);

	
	struct drm_gpuva_op *(*op_alloc)(void);

	
	void (*op_free)(struct drm_gpuva_op *op);

	
	struct drm_gpuvm_bo *(*vm_bo_alloc)(void);

	
	void (*vm_bo_free)(struct drm_gpuvm_bo *vm_bo);

	
	int (*vm_bo_validate)(struct drm_gpuvm_bo *vm_bo,
			      struct drm_exec *exec);

	
	int (*sm_step_map)(struct drm_gpuva_op *op, void *priv);

	
	int (*sm_step_remap)(struct drm_gpuva_op *op, void *priv);

	
	int (*sm_step_unmap)(struct drm_gpuva_op *op, void *priv);
};

int drm_gpuvm_sm_map(struct drm_gpuvm *gpuvm, void *priv,
		     u64 addr, u64 range,
		     struct drm_gem_object *obj, u64 offset);

int drm_gpuvm_sm_unmap(struct drm_gpuvm *gpuvm, void *priv,
		       u64 addr, u64 range);

void drm_gpuva_map(struct drm_gpuvm *gpuvm,
		   struct drm_gpuva *va,
		   struct drm_gpuva_op_map *op);

void drm_gpuva_remap(struct drm_gpuva *prev,
		     struct drm_gpuva *next,
		     struct drm_gpuva_op_remap *op);

void drm_gpuva_unmap(struct drm_gpuva_op_unmap *op);


static inline void
drm_gpuva_op_remap_to_unmap_range(const struct drm_gpuva_op_remap *op,
				  u64 *start_addr, u64 *range)
{
	const u64 va_start = op->prev ?
			     op->prev->va.addr + op->prev->va.range :
			     op->unmap->va->va.addr;
	const u64 va_end = op->next ?
			   op->next->va.addr :
			   op->unmap->va->va.addr + op->unmap->va->va.range;

	if (start_addr)
		*start_addr = va_start;
	if (range)
		*range = va_end - va_start;
}

#endif 
