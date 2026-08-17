/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _ASM_ARM64_PKEYS_H
#define _ASM_ARM64_PKEYS_H

#define ARCH_VM_PKEY_FLAGS (VM_PKEY_BIT0 | VM_PKEY_BIT1 | VM_PKEY_BIT2)

#define arch_max_pkey() 8

int arch_set_user_pkey_access(struct task_struct *tsk, int pkey,
		unsigned long init_val);

static inline bool arch_pkeys_enabled(void)
{
	return system_supports_poe();
}

static inline int vma_pkey(struct vm_area_struct *vma)
{
	return (vma->vm_flags & ARCH_VM_PKEY_FLAGS) >> VM_PKEY_SHIFT;
}

static inline int arch_override_mprotect_pkey(struct vm_area_struct *vma,
		int prot, int pkey)
{
	if (pkey != -1)
		return pkey;

	return vma_pkey(vma);
}

static inline int execute_only_pkey(struct mm_struct *mm)
{
	// Execute-only mappings are handled by EPAN/FEAT_PAN3.
	return -1;
}

#define mm_pkey_allocation_map(mm)	(mm)->context.pkey_allocation_map
#define mm_set_pkey_allocated(mm, pkey) do {		\
	mm_pkey_allocation_map(mm) |= (1U << pkey);	\
} while (0)
#define mm_set_pkey_free(mm, pkey) do {			\
	mm_pkey_allocation_map(mm) &= ~(1U << pkey);	\
} while (0)

static inline bool mm_pkey_is_allocated(struct mm_struct *mm, int pkey)
{
	
	if (pkey < 0 || pkey >= arch_max_pkey())
		return false;

	return mm_pkey_allocation_map(mm) & (1U << pkey);
}


static inline int mm_pkey_alloc(struct mm_struct *mm)
{
	
	u8 all_pkeys_mask = GENMASK(arch_max_pkey() - 1, 0);
	int ret;

	if (!arch_pkeys_enabled())
		return -1;

	
	if (mm_pkey_allocation_map(mm) == all_pkeys_mask)
		return -1;

	ret = ffz(mm_pkey_allocation_map(mm));

	mm_set_pkey_allocated(mm, ret);

	return ret;
}

static inline int mm_pkey_free(struct mm_struct *mm, int pkey)
{
	if (!mm_pkey_is_allocated(mm, pkey))
		return -EINVAL;

	mm_set_pkey_free(mm, pkey);

	return 0;
}

#endif 
