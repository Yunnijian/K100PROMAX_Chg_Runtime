/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __ASM_PTDUMP_H
#define __ASM_PTDUMP_H

#include <linux/ptdump.h>

#ifdef CONFIG_PTDUMP_CORE

#include <linux/mm_types.h>
#include <linux/seq_file.h>

struct addr_marker {
	unsigned long start_address;
	char *name;
};

struct ptdump_info {
	struct mm_struct		*mm;
	const struct addr_marker	*markers;
	unsigned long			base_addr;
};

struct ptdump_prot_bits {
	u64		mask;
	u64		val;
	const char	*set;
	const char	*clear;
};

struct ptdump_pg_level {
	const struct ptdump_prot_bits *bits;
	char name[4];
	int num;
	u64 mask;
};


struct ptdump_pg_state {
	struct ptdump_state ptdump;
	struct ptdump_pg_level *pg_level;
	struct seq_file *seq;
	const struct addr_marker *marker;
	const struct mm_struct *mm;
	unsigned long start_address;
	int level;
	u64 current_prot;
	bool check_wx;
	unsigned long wx_pages;
	unsigned long uxn_pages;
};

void ptdump_walk(struct seq_file *s, struct ptdump_info *info);
void note_page(struct ptdump_state *pt_st, unsigned long addr, int level,
	       u64 val);
#ifdef CONFIG_PTDUMP_DEBUGFS
#define EFI_RUNTIME_MAP_END	DEFAULT_MAP_WINDOW_64
void __init ptdump_debugfs_register(struct ptdump_info *info, const char *name);
#else
static inline void ptdump_debugfs_register(struct ptdump_info *info,
					   const char *name) { }
#endif 
#else
static inline void note_page(struct ptdump_state *pt_st, unsigned long addr,
			     int level, u64 val) { }
#endif 

#endif 
