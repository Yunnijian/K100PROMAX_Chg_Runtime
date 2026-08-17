/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _KUNIT_ASSERT_H
#define _KUNIT_ASSERT_H

#include <linux/err.h>
#include <linux/printk.h>
#include <linux/android_kabi.h>

struct kunit;
struct string_stream;


enum kunit_assert_type {
	KUNIT_ASSERTION,
	KUNIT_EXPECTATION,
};


struct kunit_loc {
	int line;
	const char *file;

	ANDROID_KABI_RESERVE(1);
};

#define KUNIT_CURRENT_LOC { .file = __FILE__, .line = __LINE__ }


struct kunit_assert {
	ANDROID_KABI_RESERVE(1);
};

typedef void (*assert_format_t)(const struct kunit_assert *assert,
				const struct va_format *message,
				struct string_stream *stream);

void kunit_assert_prologue(const struct kunit_loc *loc,
			   enum kunit_assert_type type,
			   struct string_stream *stream);


struct kunit_fail_assert {
	struct kunit_assert assert;
};

void kunit_fail_assert_format(const struct kunit_assert *assert,
			      const struct va_format *message,
			      struct string_stream *stream);


struct kunit_unary_assert {
	struct kunit_assert assert;
	const char *condition;
	bool expected_true;
};

void kunit_unary_assert_format(const struct kunit_assert *assert,
			       const struct va_format *message,
			       struct string_stream *stream);


struct kunit_ptr_not_err_assert {
	struct kunit_assert assert;
	const char *text;
	const void *value;
};

void kunit_ptr_not_err_assert_format(const struct kunit_assert *assert,
				     const struct va_format *message,
				     struct string_stream *stream);


struct kunit_binary_assert_text {
	const char *operation;
	const char *left_text;
	const char *right_text;
};


struct kunit_binary_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	long long left_value;
	long long right_value;
};

void kunit_binary_assert_format(const struct kunit_assert *assert,
				const struct va_format *message,
				struct string_stream *stream);


struct kunit_binary_ptr_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	const void *left_value;
	const void *right_value;
};

void kunit_binary_ptr_assert_format(const struct kunit_assert *assert,
				    const struct va_format *message,
				    struct string_stream *stream);


struct kunit_binary_str_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	const char *left_value;
	const char *right_value;
};

void kunit_binary_str_assert_format(const struct kunit_assert *assert,
				    const struct va_format *message,
				    struct string_stream *stream);


struct kunit_mem_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	const void *left_value;
	const void *right_value;
	const size_t size;
};

void kunit_mem_assert_format(const struct kunit_assert *assert,
			     const struct va_format *message,
			     struct string_stream *stream);

#if IS_ENABLED(CONFIG_KUNIT)
void kunit_assert_print_msg(const struct va_format *message,
			    struct string_stream *stream);
bool is_literal(const char *text, long long value);
bool is_str_literal(const char *text, const char *value);
void kunit_assert_hexdump(struct string_stream *stream,
			  const void *buf,
			  const void *compared_buf,
			  const size_t len);
#endif

#endif 
