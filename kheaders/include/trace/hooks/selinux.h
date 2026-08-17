/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM selinux

#define TRACE_INCLUDE_PATH trace/hooks
#if !defined(_TRACE_HOOK_SELINUX_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HOOK_SELINUX_H
#include <trace/hooks/vendor_hooks.h>

struct selinux_state;
DECLARE_RESTRICTED_HOOK(android_rvh_selinux_is_initialized,
	TP_PROTO(const struct selinux_state *state),
	TP_ARGS(state), 1);

#endif 

#include <trace/define_trace.h>
