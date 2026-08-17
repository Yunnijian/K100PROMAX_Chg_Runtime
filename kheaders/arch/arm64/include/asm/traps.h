/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_TRAP_H
#define __ASM_TRAP_H

#include <linux/list.h>
#include <asm/esr.h>
#include <asm/ptrace.h>
#include <asm/sections.h>

#ifdef CONFIG_ARMV8_DEPRECATED
bool try_emulate_armv8_deprecated(struct pt_regs *regs, u32 insn);
#else
static inline bool
try_emulate_armv8_deprecated(struct pt_regs *regs, u32 insn)
{
	return false;
}
#endif 

void force_signal_inject(int signal, int code, unsigned long address, unsigned long err);
void arm64_notify_segfault(unsigned long addr);
void arm64_force_sig_fault(int signo, int code, unsigned long far, const char *str);
void arm64_force_sig_fault_pkey(unsigned long far, const char *str, int pkey);
void arm64_force_sig_mceerr(int code, unsigned long far, short lsb, const char *str);
void arm64_force_sig_ptrace_errno_trap(int errno, unsigned long far, const char *str);

int early_brk64(unsigned long addr, unsigned long esr, struct pt_regs *regs);


void arm64_skip_faulting_instruction(struct pt_regs *regs, unsigned long size);

static inline int __in_irqentry_text(unsigned long ptr)
{
	return ptr >= (unsigned long)&__irqentry_text_start &&
	       ptr < (unsigned long)&__irqentry_text_end;
}

static inline int in_entry_text(unsigned long ptr)
{
	return ptr >= (unsigned long)&__entry_text_start &&
	       ptr < (unsigned long)&__entry_text_end;
}


static inline bool arm64_is_ras_serror(unsigned long esr)
{
	WARN_ON(preemptible());

	if (esr & ESR_ELx_IDS)
		return false;

	if (this_cpu_has_cap(ARM64_HAS_RAS_EXTN))
		return true;
	else
		return false;
}


static inline unsigned long arm64_ras_serror_get_severity(unsigned long esr)
{
	unsigned long aet = esr & ESR_ELx_AET;

	if (!arm64_is_ras_serror(esr)) {
		
		return ESR_ELx_AET_UC;
	}

	
	if ((esr & ESR_ELx_FSC) != ESR_ELx_FSC_SERROR) {
		
		return ESR_ELx_AET_UC;
	}

	return aet;
}

bool arm64_is_fatal_ras_serror(struct pt_regs *regs, unsigned long esr);
void __noreturn arm64_serror_panic(struct pt_regs *regs, unsigned long esr);

static inline void arm64_mops_reset_regs(struct user_pt_regs *regs, unsigned long esr)
{
	bool wrong_option = esr & ESR_ELx_MOPS_ISS_WRONG_OPTION;
	bool option_a = esr & ESR_ELx_MOPS_ISS_OPTION_A;
	int dstreg = ESR_ELx_MOPS_ISS_DESTREG(esr);
	int srcreg = ESR_ELx_MOPS_ISS_SRCREG(esr);
	int sizereg = ESR_ELx_MOPS_ISS_SIZEREG(esr);
	unsigned long dst, size;

	dst = regs->regs[dstreg];
	size = regs->regs[sizereg];

	
	if (esr & ESR_ELx_MOPS_ISS_MEM_INST) {
		
		if (option_a ^ wrong_option) {
			
			regs->regs[dstreg] = dst + size;
			regs->regs[sizereg] = -size;
		}
	} else {
		
		unsigned long src = regs->regs[srcreg];
		if (!(option_a ^ wrong_option)) {
			
			if (regs->pstate & PSR_N_BIT) {
				
				regs->regs[dstreg] = dst - size;
				regs->regs[srcreg] = src - size;
			}
		} else {
			
			if (size & BIT(63)) {
				
				regs->regs[dstreg] = dst + size;
				regs->regs[srcreg] = src + size;
				regs->regs[sizereg] = -size;
			}
		}
	}

	if (esr & ESR_ELx_MOPS_ISS_FROM_EPILOGUE)
		regs->pc -= 8;
	else
		regs->pc -= 4;
}
#endif
