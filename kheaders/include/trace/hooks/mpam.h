/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM mpam
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH trace/hooks
#if !defined(_TRACE_HOOK_MPAM_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HOOK_MPAM_H
#include <trace/hooks/vendor_hooks.h>

struct task_struct;
DECLARE_HOOK(android_vh_mpam_set,
	TP_PROTO(struct task_struct *prev, struct task_struct *next),
	TP_ARGS(prev, next));

#endif 

#include <trace/define_trace.h>
