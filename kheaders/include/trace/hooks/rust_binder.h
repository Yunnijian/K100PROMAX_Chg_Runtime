/* SPDX-License-Identifier: GPL-2.0-only */


#undef TRACE_SYSTEM
#undef TRACE_INCLUDE_FILE
#undef TRACE_INCLUDE_PATH
#define TRACE_SYSTEM rust_binder
#define TRACE_INCLUDE_FILE rust_binder
#define TRACE_INCLUDE_PATH trace/hooks

#if !defined(_RUST_BINDER_HOOK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _RUST_BINDER_HOOK_H

#include <trace/hooks/vendor_hooks.h>

typedef void *rust_binder_transaction;



DECLARE_HOOK(android_vh_rust_binder_set_priority,
	TP_PROTO(rust_binder_transaction t, struct task_struct *task),
	TP_ARGS(t, task));
DECLARE_HOOK(android_vh_rust_binder_restore_priority,
	TP_PROTO(struct task_struct *task),
	TP_ARGS(task));
DECLARE_HOOK(android_vh_rust_binder_looper_entry,
	TP_PROTO(rust_binder_thread thread, unsigned int looper_flags),
	TP_ARGS(thread, looper_flags));

#endif 


#include <trace/define_trace.h>
