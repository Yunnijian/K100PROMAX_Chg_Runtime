/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_MMU_CONTEXT_H
#define __ASM_MMU_CONTEXT_H

#ifndef __ASSEMBLY__

#include <linux/compiler.h>
#include <linux/sched.h>
#include <linux/sched/hotplug.h>
#include <linux/mm_types.h>
#include <linux/pgtable.h>
#include <linux/pkeys.h>

#include <asm/cacheflush.h>
#include <asm/cpufeature.h>
#include <asm/daifflags.h>
#include <asm/proc-fns.h>
#include <asm/cputype.h>
#include <asm/sysreg.h>
#include <asm/tlbflush.h>

extern bool rodata_full;

static inline void contextidr_thread_switch(struct task_struct *next)
{
	if (!IS_ENABLED(CONFIG_PID_IN_CONTEXTIDR))
		return;

	write_sysreg(task_pid_nr(next), contextidr_el1);
	isb();
}


static inline void cpu_set_reserved_ttbr0_nosync(void)
{
	unsigned long ttbr = phys_to_ttbr(__pa_symbol(reserved_pg_dir));

	write_sysreg(ttbr, ttbr0_el1);
}

static inline void cpu_set_reserved_ttbr0(void)
{
	cpu_set_reserved_ttbr0_nosync();
	isb();
}

void cpu_do_switch_mm(phys_addr_t pgd_phys, struct mm_struct *mm);

static inline void cpu_switch_mm(pgd_t *pgd, struct mm_struct *mm)
{
	BUG_ON(pgd == swapper_pg_dir);
	cpu_do_switch_mm(virt_to_phys(pgd),mm);
}


#define idmap_t0sz	TCR_T0SZ(IDMAP_VA_BITS)


static inline void __cpu_set_tcr_t0sz(unsigned long t0sz)
{
	unsigned long tcr = read_sysreg(tcr_el1);

	if ((tcr & TCR_T0SZ_MASK) == t0sz)
		return;

	tcr &= ~TCR_T0SZ_MASK;
	tcr |= t0sz;
	write_sysreg(tcr, tcr_el1);
	isb();
}

#define cpu_set_default_tcr_t0sz()	__cpu_set_tcr_t0sz(TCR_T0SZ(vabits_actual))
#define cpu_set_idmap_tcr_t0sz()	__cpu_set_tcr_t0sz(idmap_t0sz)


static inline void cpu_uninstall_idmap(void)
{
	struct mm_struct *mm = current->active_mm;

	cpu_set_reserved_ttbr0();
	local_flush_tlb_all();
	cpu_set_default_tcr_t0sz();

	if (mm != &init_mm && !system_uses_ttbr0_pan())
		cpu_switch_mm(mm->pgd, mm);
}

static inline void cpu_install_idmap(void)
{
	cpu_set_reserved_ttbr0();
	local_flush_tlb_all();
	cpu_set_idmap_tcr_t0sz();

	cpu_switch_mm(lm_alias(idmap_pg_dir), &init_mm);
}


static inline void cpu_install_ttbr0(phys_addr_t ttbr0, unsigned long t0sz)
{
	cpu_set_reserved_ttbr0();
	local_flush_tlb_all();
	__cpu_set_tcr_t0sz(t0sz);

	
	write_sysreg(ttbr0, ttbr0_el1);
	isb();
}

void __cpu_replace_ttbr1(pgd_t *pgdp, bool cnp);

static inline void cpu_enable_swapper_cnp(void)
{
	__cpu_replace_ttbr1(lm_alias(swapper_pg_dir), true);
}

static inline void cpu_replace_ttbr1(pgd_t *pgdp)
{
	
	WARN_ON(system_capabilities_finalized());
	__cpu_replace_ttbr1(pgdp, false);
}


void check_and_switch_context(struct mm_struct *mm);

#define init_new_context(tsk, mm) init_new_context(tsk, mm)
static inline int
init_new_context(struct task_struct *tsk, struct mm_struct *mm)
{
	atomic64_set(&mm->context.id, 0);
	refcount_set(&mm->context.pinned, 0);

	
	mm->context.pkey_allocation_map = BIT(0);

	return 0;
}

static inline void arch_dup_pkeys(struct mm_struct *oldmm,
				  struct mm_struct *mm)
{
	
	mm->context.pkey_allocation_map = oldmm->context.pkey_allocation_map;
}

static inline int arch_dup_mmap(struct mm_struct *oldmm, struct mm_struct *mm)
{
	arch_dup_pkeys(oldmm, mm);

	return 0;
}

static inline void arch_exit_mmap(struct mm_struct *mm)
{
}

static inline void arch_unmap(struct mm_struct *mm,
			unsigned long start, unsigned long end)
{
}

#ifdef CONFIG_ARM64_SW_TTBR0_PAN
static inline void update_saved_ttbr0(struct task_struct *tsk,
				      struct mm_struct *mm)
{
	u64 ttbr;

	if (!system_uses_ttbr0_pan())
		return;

	if (mm == &init_mm)
		ttbr = phys_to_ttbr(__pa_symbol(reserved_pg_dir));
	else
		ttbr = phys_to_ttbr(virt_to_phys(mm->pgd)) | ASID(mm) << 48;

	WRITE_ONCE(task_thread_info(tsk)->ttbr0, ttbr);
}
#else
static inline void update_saved_ttbr0(struct task_struct *tsk,
				      struct mm_struct *mm)
{
}
#endif

#define enter_lazy_tlb enter_lazy_tlb
static inline void
enter_lazy_tlb(struct mm_struct *mm, struct task_struct *tsk)
{
	
	update_saved_ttbr0(tsk, &init_mm);
}

static inline void __switch_mm(struct mm_struct *next)
{
	
	if (next == &init_mm) {
		cpu_set_reserved_ttbr0();
		return;
	}

	check_and_switch_context(next);
}

static inline void
switch_mm(struct mm_struct *prev, struct mm_struct *next,
	  struct task_struct *tsk)
{
	if (prev != next)
		__switch_mm(next);

	
	update_saved_ttbr0(tsk, next);
}

static inline const struct cpumask *
task_cpu_possible_mask(struct task_struct *p)
{
	if (!static_branch_unlikely(&arm64_mismatched_32bit_el0))
		return cpu_possible_mask;

	if (!is_compat_thread(task_thread_info(p)))
		return cpu_possible_mask;

	return system_32bit_el0_cpumask();
}
#define task_cpu_possible_mask	task_cpu_possible_mask

void verify_cpu_asid_bits(void);
void post_ttbr_update_workaround(void);

unsigned long arm64_mm_context_get(struct mm_struct *mm);
void arm64_mm_context_put(struct mm_struct *mm);

#define mm_untag_mask mm_untag_mask
static inline unsigned long mm_untag_mask(struct mm_struct *mm)
{
	return -1UL >> 8;
}


static inline bool arch_vma_access_permitted(struct vm_area_struct *vma,
		bool write, bool execute, bool foreign)
{
	if (!system_supports_poe())
		return true;

	
	if (foreign || vma_is_foreign(vma))
		return true;

	return por_el0_allows_pkey(vma_pkey(vma), write, execute);
}

#include <asm-generic/mmu_context.h>

#endif 

#endif 
