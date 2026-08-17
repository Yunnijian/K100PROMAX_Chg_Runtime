/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

#include <linux/compiler_types.h>

#ifndef __ASSEMBLY__

#ifdef __KERNEL__


void ftrace_likely_update(struct ftrace_likely_data *f, int val,
			  int expect, int is_constant);
#if defined(CONFIG_TRACE_BRANCH_PROFILING) \
    && !defined(DISABLE_BRANCH_PROFILING) && !defined(__CHECKER__)
#define likely_notrace(x)	__builtin_expect(!!(x), 1)
#define unlikely_notrace(x)	__builtin_expect(!!(x), 0)

#define __branch_check__(x, expect, is_constant) ({			\
			long ______r;					\
			static struct ftrace_likely_data		\
				__aligned(4)				\
				__section("_ftrace_annotated_branch")	\
				______f = {				\
				.data.func = __func__,			\
				.data.file = __FILE__,			\
				.data.line = __LINE__,			\
			};						\
			______r = __builtin_expect(!!(x), expect);	\
			ftrace_likely_update(&______f, ______r,		\
					     expect, is_constant);	\
			______r;					\
		})


# ifndef likely
#  define likely(x)	(__branch_check__(x, 1, __builtin_constant_p(x)))
# endif
# ifndef unlikely
#  define unlikely(x)	(__branch_check__(x, 0, __builtin_constant_p(x)))
# endif

#ifdef CONFIG_PROFILE_ALL_BRANCHES

#define if(cond, ...) if ( __trace_if_var( !!(cond , ## __VA_ARGS__) ) )

#define __trace_if_var(cond) (__builtin_constant_p(cond) ? (cond) : __trace_if_value(cond))

#define __trace_if_value(cond) ({			\
	static struct ftrace_branch_data		\
		__aligned(4)				\
		__section("_ftrace_branch")		\
		__if_trace = {				\
			.func = __func__,		\
			.file = __FILE__,		\
			.line = __LINE__,		\
		};					\
	(cond) ?					\
		(__if_trace.miss_hit[1]++,1) :		\
		(__if_trace.miss_hit[0]++,0);		\
})

#endif 

#else
# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)
# define likely_notrace(x)	likely(x)
# define unlikely_notrace(x)	unlikely(x)
#endif


#ifndef barrier

# define barrier() __asm__ __volatile__("": : :"memory")
#endif

#ifndef barrier_data

# define barrier_data(ptr) __asm__ __volatile__("": :"r"(ptr) :"memory")
#endif


#ifndef barrier_before_unreachable
# define barrier_before_unreachable() do { } while (0)
#endif


#ifdef CONFIG_OBJTOOL

#define __annotate_jump_table __section(".data.rel.ro.c_jump_table")
#else 
#define __annotate_jump_table
#endif 


#define unreachable() do {		\
	barrier_before_unreachable();	\
	__builtin_unreachable();	\
} while (0)


#ifndef KENTRY
# define KENTRY(sym)						\
	extern typeof(sym) sym;					\
	static const unsigned long __kentry_##sym		\
	__used							\
	__attribute__((__section__("___kentry+" #sym)))		\
	= (unsigned long)&sym;
#endif

#ifndef RELOC_HIDE
# define RELOC_HIDE(ptr, off)					\
  ({ unsigned long __ptr;					\
     __ptr = (unsigned long) (ptr);				\
    (typeof(ptr)) (__ptr + (off)); })
#endif

#define absolute_pointer(val)	RELOC_HIDE((void *)(val), 0)

#ifndef OPTIMIZER_HIDE_VAR

#define OPTIMIZER_HIDE_VAR(var)						\
	__asm__ ("" : "=r" (var) : "0" (var))
#endif

#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)


#define data_race(expr)							\
({									\
	__kcsan_disable_current();					\
	__auto_type __v = (expr);					\
	__kcsan_enable_current();					\
	__v;								\
})

#endif 


static inline void *offset_to_ptr(const int *off)
{
	return (void *)((unsigned long)off + *off);
}

#endif 

#ifdef CONFIG_64BIT
#define ARCH_SEL(a,b) a
#else
#define ARCH_SEL(a,b) b
#endif


#define ___ADDRESSABLE(sym, __attrs)						\
	static void * __used __attrs						\
	__UNIQUE_ID(__PASTE(__addressable_,sym)) = (void *)(uintptr_t)&sym;

#define __ADDRESSABLE(sym) \
	___ADDRESSABLE(sym, __section(".discard.addressable"))


#define __must_be_array(a)	BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))


#define __must_be_cstr(p)	BUILD_BUG_ON_ZERO(__annotated(p, nonstring))


#define __is_constexpr(x) \
	(sizeof(int) == sizeof(*(8 ? ((void *)((long)(x) * 0l)) : (int *)8)))


#define is_signed_type(type) (((type)(-1)) < (__force type)1)
#define is_unsigned_type(type) (!is_signed_type(type))


#define statically_true(x) (__builtin_constant_p(x) && (x))


#define prevent_tail_call_optimization()	mb()

#include <asm/rwonce.h>

#endif 
