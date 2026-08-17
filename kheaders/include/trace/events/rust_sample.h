/* SPDX-License-Identifier: GPL-2.0-only */


#undef TRACE_SYSTEM
#define TRACE_SYSTEM rust_sample

#if !defined(_RUST_SAMPLE_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _RUST_SAMPLE_TRACE_H

#include <linux/tracepoint.h>

TRACE_EVENT(rust_sample_loaded,
	TP_PROTO(int magic_number),
	TP_ARGS(magic_number),
	TP_STRUCT__entry(
		__field(int, magic_number)
	),
	TP_fast_assign(
		__entry->magic_number = magic_number;
	),
	TP_printk("magic=%d", __entry->magic_number)
);

#endif 


#include <trace/define_trace.h>
