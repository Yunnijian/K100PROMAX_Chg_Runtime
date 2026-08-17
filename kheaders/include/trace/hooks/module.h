/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM module

#define TRACE_INCLUDE_PATH trace/hooks
#if !defined(_TRACE_HOOK_MODULE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HOOK_MODULE_H
#include <trace/hooks/vendor_hooks.h>

struct module;
DECLARE_HOOK(android_vh_free_mod_mem,
		TP_PROTO(const struct module *mod),
		TP_ARGS(mod));

DECLARE_HOOK(android_vh_set_mod_perm_after_init,
		TP_PROTO(const struct module *mod),
		TP_ARGS(mod));

DECLARE_HOOK(android_vh_set_mod_perm_before_init,
		TP_PROTO(const struct module *mod),
		TP_ARGS(mod));

#endif 

#include <trace/define_trace.h>
