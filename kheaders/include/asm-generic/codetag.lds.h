/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __ASM_GENERIC_CODETAG_LDS_H
#define __ASM_GENERIC_CODETAG_LDS_H

#define SECTION_WITH_BOUNDARIES(_name)	\
	. = ALIGN(8);			\
	__start_##_name = .;		\
	KEEP(*(_name))			\
	__stop_##_name = .;

#define CODETAG_SECTIONS()		\
	SECTION_WITH_BOUNDARIES(alloc_tags)

#define MOD_SEPARATE_CODETAG_SECTION(_name)	\
	.codetag.##_name : {			\
		SECTION_WITH_BOUNDARIES(_name)	\
	}


#define MOD_SEPARATE_CODETAG_SECTIONS()		\
	MOD_SEPARATE_CODETAG_SECTION(alloc_tags)

#endif 
