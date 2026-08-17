/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _ANDROID_KABI_H
#define _ANDROID_KABI_H

#include <linux/args.h>
#include <linux/compiler.h>
#include <linux/compiler_attributes.h>
#include <linux/stringify.h>



#if defined(BUILD_VDSO) || defined(__DISABLE_EXPORTS)
#define __ANDROID_KABI_RULE(hint, target, value)
#else
#define __ANDROID_KABI_RULE(hint, target, value)			 \
	static const char CONCATENATE(__gendwarfksyms_rule_,		 \
				      __COUNTER__)[] __used __aligned(1) \
		__section(".discard.gendwarfksyms.kabi_rules") =	 \
			"1\0" #hint "\0" target "\0" value
#endif

#define _ANDROID_KABI_RULE(hint, target, value) \
	__ANDROID_KABI_RULE(hint, #target, #value)

#define _ANDROID_KABI_NORMAL_SIZE_ALIGN(_orig, _new)			\
	union {								\
		_Static_assert(						\
			sizeof(struct { _new; }) <=			\
				sizeof(struct { _orig; }),		\
			FILE_LINE ": " __stringify(_new)		\
				" is larger than " __stringify(_orig));	\
		_Static_assert(						\
			__alignof__(struct { _new; }) <=		\
				__alignof__(struct { _orig; }),		\
			FILE_LINE ": " __stringify(_orig)		\
				" is not aligned the same as "		\
				__stringify(_new));			\
	}

#define _ANDROID_KABI_REPLACE(_orig, _new)		      \
	union {						      \
		_new;					      \
		struct {				      \
			_orig;				      \
		};					      \
		_ANDROID_KABI_NORMAL_SIZE_ALIGN(_orig, _new); \
	}





#define ANDROID_KABI_RESERVE(number)		u64 __kabi_reserved##number
#define ANDROID_BACKPORT_RESERVE(number)	u64 __kabi_reserved_backport##number




#define ANDROID_KABI_DECLONLY(fqn)	_ANDROID_KABI_RULE(declonly, fqn, )


#define ANDROID_KABI_ENUMERATOR_IGNORE(fqn, field) \
	_ANDROID_KABI_RULE(enumerator_ignore, fqn field, )


#define ANDROID_KABI_ENUMERATOR_VALUE(fqn, field, value) \
	_ANDROID_KABI_RULE(enumerator_value, fqn field, value)


#define ANDROID_KABI_BYTE_SIZE(fqn, value) \
	_ANDROID_KABI_RULE(byte_size, fqn, value)


#define ANDROID_KABI_TYPE_STRING(type, str) \
	__ANDROID_KABI_RULE(type_string, type, str)


#define ANDROID_KABI_IGNORE(n, _new)		 \
	union {					 \
		_new;				 \
		unsigned char __kabi_ignored##n; \
	}


#define ANDROID_KABI_REPLACE(_oldtype, _oldname, _new) \
	_ANDROID_KABI_REPLACE(_oldtype __kabi_renamed##_oldname, struct { _new; })


#define ANDROID_KABI_USE(number, _new) \
	_ANDROID_KABI_REPLACE(ANDROID_KABI_RESERVE(number), _new)


#define ANDROID_KABI_USE2(number, _new1, _new2) \
	_ANDROID_KABI_REPLACE(ANDROID_KABI_RESERVE(number), struct{ _new1; _new2; })


#define ANDROID_BACKPORT_USE(number, _new) \
	_ANDROID_KABI_REPLACE(ANDROID_BACKPORT_RESERVE(number), _new)

#endif 
