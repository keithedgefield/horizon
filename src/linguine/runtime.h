/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * RT: Language Runtime
 */

#ifndef LINGUINE_RUNTIME_H
#define LINGUINE_RUNTIME_H

#include "config.h"

#define RT_ARG_MAX	32

struct lir_func;
struct rt_env;

enum rt_value_type {
	RT_VALUE_INT,
	RT_VALUE_FLOAT,
	RT_VALUE_STRING,
	RT_VALUE_ARRAY,
	RT_VALUE_DICT,
	RT_VALUE_FUNC,
};

struct rt_env;
struct rt_frame;
struct rt_value;
struct rt_func;
struct rt_string;
struct rt_array;
struct rt_dict;
struct rt_bindglobal;
struct rt_bindlocal;

/* Runtime environment. */
struct rt_env {
	/* Global symbols. */
	struct rt_bindglobal *global;

	/* Stack. */
	struct rt_frame *frame;

	/* Heap usage in bytes. */
	size_t heap_usage;

	/* Deep object list. */
	struct rt_string *deep_str_list;
	struct rt_array *deep_arr_list;
	struct rt_dict *deep_dict_list;

	/* Garbage object list. */
	struct rt_string *garbage_str_list;
	struct rt_array *garbage_arr_list;
	struct rt_dict *garbage_dict_list;

	/* Execution file and line. */
	char file_name[1024];
	int line;

	/* Error message. */
	char error_message[4096];
};

/* Calling frame. */
struct rt_frame {
	/* function */
	struct rt_func *func;

	/* tmpvar */
	int tmpvar_size;
	struct rt_value *tmpvar;

	/* bindlocal. */
	struct rt_bindlocal *bindlocal;

	/* Shallow string list. */
	struct rt_string *shallow_str_list;

	/* Shallow array list. */
	struct rt_array *shallow_arr_list;

	/* Shallow dictionary list. */
	struct rt_dict *shallow_dict_list;

	/* Next frame. */
	struct rt_frame *next;
};

/*
 * Variable value.
 *  - If a value is zero-cleared, it shows an integer zero.
 */
struct rt_value {
	int type;
	union {
		int i;
		float f;
		struct rt_string *str;
		struct rt_array *arr;
		struct rt_dict *dict;
		struct rt_func *func;
	} val;
};

/* String object. */
struct rt_string {
	int ref_count;
	char *s;

	/* String list (shallow or deep). */
	struct rt_string *prev;
	struct rt_string *next;
	bool is_deep;

	/* Is marked? (for mark-and-sweep GC). */
	bool is_marked;
};

/* Array object */
struct rt_array {
	int ref_count;
	int alloc_size;
	int size;
	struct rt_value *table;

	/* Array list (shallow or deep). */
	struct rt_array *prev;
	struct rt_array *next;
	bool is_deep;

	/* Is marked? (for mark-and-sweep GC). */
	bool is_marked;
};

/* Dictionary object. */
struct rt_dict {
	int ref_count;
	int alloc_size;
	int size;
	char **key;
	struct rt_value *value;

	/* Dict list (shallow or deep). */
	struct rt_dict *prev;
	struct rt_dict *next;
	bool is_deep;

	/* Is marked? (for mark-and-sweep GC). */
	bool is_marked;
};

/* Function object. */
struct rt_func {
	char *name;
	int param_count;
	char *param_name[RT_ARG_MAX];

	char *file_name;

	/* Bytecode for a function. (if not a cfunc) */
	int bytecode_size;
	uint8_t *bytecode;
	int tmpvar_size;

	/* Function pointer. (if a cfunc) */
	bool (*cfunc)(struct rt_env *env);
};

/* Global variable entry. */
struct rt_bindglobal {
	char *name;
	struct rt_value val;

	/* XXX: */
	struct rt_bindglobal *next;
};

/* Local variable entry. */
struct rt_bindlocal {
	char *name;
	struct rt_value val;

	/* XXX: */
	struct rt_bindlocal *next;
};

/*
 * Runtime Environment
 */

/* Create a runtime environment. */
bool
rt_create(
	struct rt_env **rt);

/* Destroy a runtime environment. */
void
rt_destroy(
	struct rt_env *rt);

/* Get a file name. */
const char *
rt_get_file_name(
	struct rt_env *rt);

/* Get an error line number. */
int
rt_get_error_line(
	struct rt_env *rt);

/* Get an error message. */
const char *
rt_get_error_message(
	struct rt_env *rt);

/*
 * Function Registration
 */

/* Register functions from a souce text. */
bool
rt_register_source(
	struct rt_env *rt,
	const char *file_name,
	const char *source_text);

/* Register functions from bytecode data. */
bool
rt_register_bytecode(
	struct rt_env *rt,
	uint32_t size,
	uint8_t *data);

/* Register a C function. */
bool
rt_register_cfunc(
	struct rt_env *rt,
	const char *name,
	int param_count,
	const char *param_name[],
	bool (*cfunc)(struct rt_env *env));

/*
 * Function Call
 */

/* Call a function with a name. */
bool
rt_call_with_name(
	struct rt_env *rt,
	const char *func_name,
	int arg_count,
	struct rt_value *arg,
	struct rt_value *ret);

/* Call a function object. */
bool
rt_call(
	struct rt_env *rt,
	struct rt_func *func,
	struct rt_value *thisptr,
	int arg_count,
	struct rt_value *arg,
	struct rt_value *ret);

/*
 * Value Manipulation
 */

/* Make an integer value. */
void
rt_make_int(
	struct rt_value *val,
	int i);

/* Make a floating-point value. */
void
rt_make_float(
	struct rt_value *val,
	float f);

/* Make a string value. */
bool
rt_make_string(
	struct rt_env *rt,
	struct rt_value *val,
	const char *s);

/* Make an empty array value. */
bool
rt_make_empty_array(
	struct rt_env *rt,
	struct rt_value *val);

/* Make an empty dictionary value */
bool
rt_make_empty_dict(
	struct rt_env *rt,
	struct rt_value *val);

/* Clone a value. */
bool
rt_copy_value(
	struct rt_env *rt,
	struct rt_value *dst,
	struct rt_value *src);

/* Get a value type. */
bool
rt_get_value_type(
	struct rt_env *rt,
	struct rt_value *val,
	int *type);

/* Retain a value. */
bool
rt_ref_value(
	struct rt_env *rt,
	struct rt_value *val);

/* Delete a value. */
bool
rt_unref_value(
	struct rt_env *rt,
	struct rt_value *val);

/* Get an integer value. */
bool
rt_get_int(
	struct rt_env *rt,
	struct rt_value *val,
	int *ret);

/* Get a floating-point value. */
bool
rt_get_float(
	struct rt_env *rt,
	struct rt_value *val,
	float *ret);

/* Get a string value. */
bool
rt_get_string(
	struct rt_env *rt,
	struct rt_value *val,
	const char **ret);

/* Get an array size. */
bool
rt_get_array_size(
	struct rt_env *rt,
	struct rt_value *val,
	int *size);

/* Get an array element. */
bool
rt_get_array_elem(
	struct rt_env *rt,
	struct rt_value *array,
	int index,
	struct rt_value *val);

/* Set an array element. */
bool
rt_set_array_elem(
	struct rt_env *rt,
	struct rt_value *array,
	int index,
	struct rt_value *val);

/* Get a dictionary size. */
bool
rt_get_dict_size(
	struct rt_env *rt,
	struct rt_value *dict,
	int *size);

/* Get a dictionary value by an index. */
bool
rt_get_dict_value_by_index(
	struct rt_env *rt,
	struct rt_value *dict,
	int index,
	struct rt_value *val);

/* Get a dictionary key by an index. */
bool
rt_get_dict_key_by_index(
	struct rt_env *rt,
	struct rt_value *dict,
	int index,
	const char **key);

/* Get a dictionary element. */
bool
rt_get_dict_elem(
	struct rt_env *rt,
	struct rt_value *dict,
	const char *key,
	struct rt_value *val);

/* Set a dictionary element. */
bool
rt_set_dict_elem(
	struct rt_env *rt,
	struct rt_value *dict,
	const char *key,
	struct rt_value *val);

/*
 * Local Variable
 */

/* Get a local variable value. (For C func implementation) */
bool
rt_get_local(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val);

/* Set a local variable value. (For C func implementation) */
bool
	rt_set_local(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val);

/*
 * Global Variable
 */

/* Register a global variable. */
bool
rt_register_global(
	struct rt_env *rt,
	const char *name);

/* Get a global variable. */
bool
rt_load_global(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val);

/* Set a global variable. */
bool
rt_store_global(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val);

/*
 * GC
 */

/* Do a shallow GC. (free already sweeped young generation objects) */
void
rt_shallow_gc(
	struct rt_env *rt);

/* Do a full GC. (without current frame objects) */
void
rt_deep_gc(
	struct rt_env *rt);

/* Get allocated object size in bytes. */
size_t
rt_get_heap_usage(
	struct rt_env *rt);

#endif
