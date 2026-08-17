/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_MODULE_H
#define __ASM_MODULE_H

#include <asm-generic/module.h>

#ifdef CONFIG_KVM
struct pkvm_module_section {
	void *start;
	void *end;
};

typedef s32 kvm_nvhe_reloc_t;
struct pkvm_module_ops;

struct pkvm_el2_sym {
	char			*name;
	__le32			*rela_pos;
	struct list_head	node;
};

struct pkvm_el2_module {
	struct pkvm_module_section text;
	struct pkvm_module_section bss;
	struct pkvm_module_section rodata;
	struct pkvm_module_section data;
	struct pkvm_module_section event_ids;
	struct pkvm_module_section patchable_function_entries;
	struct pkvm_module_section sections;
	void *hyp_va;
	struct hyp_event *hyp_events;
	struct hyp_printk_fmt *hyp_printk_fmts;
	unsigned int nr_hyp_events;
	unsigned int nr_hyp_printk_fmts;
	kvm_nvhe_reloc_t *relocs;
	struct list_head node;
	struct list_head ext_symbols;
	unsigned int nr_relocs;
	int (*init)(const struct pkvm_module_ops *ops);
};

void kvm_apply_hyp_module_relocations(struct pkvm_el2_module *mod,
				      kvm_nvhe_reloc_t *begin,
				      kvm_nvhe_reloc_t *end);

#define ARM64_MODULE_KVM_ARCHDATA					\
					\
	struct pkvm_el2_module	hyp;
#else
#define ARM64_MODULE_KVM_ARCHDATA
#endif

struct mod_plt_sec {
	int			plt_shndx;
	int			plt_num_entries;
	int			plt_max_entries;
};

struct mod_arch_specific {
	struct mod_plt_sec	core;
	struct mod_plt_sec	init;

	
	struct plt_entry	*ftrace_trampolines;

	ARM64_MODULE_KVM_ARCHDATA
};

u64 module_emit_plt_entry(struct module *mod, Elf64_Shdr *sechdrs,
			  void *loc, const Elf64_Rela *rela,
			  Elf64_Sym *sym);

u64 module_emit_veneer_for_adrp(struct module *mod, Elf64_Shdr *sechdrs,
				void *loc, u64 val);

struct plt_entry {
	
	__le32	adrp;	
	__le32	add;	
	__le32	br;	
};

static inline bool is_forbidden_offset_for_adrp(void *place)
{
	return cpus_have_final_cap(ARM64_WORKAROUND_843419) &&
	       ((u64)place & 0xfff) >= 0xff8;
}

struct plt_entry get_plt_entry(u64 dst, void *pc);

static inline const Elf_Shdr *find_section(const Elf_Ehdr *hdr,
				    const Elf_Shdr *sechdrs,
				    const char *name)
{
	const Elf_Shdr *s, *se;
	const char *secstrs = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;

	for (s = sechdrs, se = sechdrs + hdr->e_shnum; s < se; s++) {
		if (strcmp(name, secstrs + s->sh_name) == 0)
			return s;
	}

	return NULL;
}

#endif 
