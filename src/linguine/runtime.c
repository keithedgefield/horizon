/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * Language Runtime
 */

/*
 * [configuration]
 *  - CONF_RUNTIME_ONLY ... No source compilation features.
 */

#include "runtime.h"

#if !defined(CONF_RUNTIME_ONLY)
#include "ast.h"
#include "hir.h"
#include "lir.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define NOT_IMPLEMENTED		0
#define NEVER_COME_HERE		0
#define BROKEN_BYTECODE		"Broken bytecode."

/* Text format buffer. */
static char text_buf[65536];

/* Forward declarations. */
static bool rt_register_lir(struct rt_env *rt, struct lir_func *lir);
static bool rt_enter_frame(struct rt_env *rt, struct rt_func *func);
static void rt_leave_frame(struct rt_env *rt);
static bool rt_expand_array(struct rt_env *rt, struct rt_value *array, int size);
static bool rt_expand_dict(struct rt_env *rt, struct rt_value *dict, int size);
static void rt_make_deep_reference(struct rt_env *rt, struct rt_value *val);
static void rt_recursively_mark_object(struct rt_env *rt, struct rt_value *val);
static void rt_free_string(struct rt_env *rt, struct rt_string *str);
static void rt_free_array(struct rt_env *rt, struct rt_array *array);
static void rt_free_dict(struct rt_env *rt, struct rt_dict *dict);
static bool rt_visit_bytecode(struct rt_env *rt, struct rt_func *func);
static bool rt_visit_op(struct rt_env *rt, struct rt_func *func, int *pc);
static bool rt_add_local(struct rt_env *rt, const char *name, struct rt_bindlocal **local);
static bool rt_find_local(struct rt_env *rt, const char *name, struct rt_bindlocal **local);
static bool rt_add_global(struct rt_env *rt, const char *name, struct rt_bindglobal **global);
static bool rt_find_global(struct rt_env *rt, const char *name, struct rt_bindglobal **global);
static void rt_error(struct rt_env *rt, const char *msg, ...);
static void rt_out_of_memory(struct rt_env *rt);
static bool rt_register_intrinsics(struct rt_env *rt);

/*
 * Create a runtime environment.
 */
bool
rt_create(
	struct rt_env **rt)
{
	struct rt_env *env;

	/* Allocate. */
	env = malloc(sizeof(struct rt_env));
	if (env == NULL)
		return false;
	memset(env, 0, sizeof(struct rt_env));

	/* Register the intrinsics. */
	if (!rt_register_intrinsics(env)) {
		free(env);
		return false;
	}

	*rt = env;
	return true;
}

/*
 *  Destroy a runtime environment.
 */
bool
rt_destroy(
	struct rt_env *rt)
{
	struct rt_string *str, *next_str;
	struct rt_array *arr, *next_arr;
	struct rt_dict *dict, *next_dict;

	/* Free frames. */
	while (rt->frame != NULL)
		rt_leave_frame(rt);

	/* Sweep garbages */
	rt_shallow_gc(rt);

	/* Free strongly-referenced strings. */
	str = rt->deep_str_list;
	while (str != NULL) {
		next_str = rt->deep_str_list->next;
		rt_free_string(rt, str);
		str = next_str;
	}
	
	/* Free strongly-referenced arrays. */
	arr = rt->deep_arr_list;
	while (arr != NULL) {
		next_arr = arr->next;
		rt_free_array(rt, arr);
		arr = next_arr;
	}

	/* Free strongly-referenced dictionaries. */
	dict = rt->deep_dict_list;
	while (dict != NULL) {
		next_dict = dict->next;
		rt_free_dict(rt, rt->deep_dict_list);
		dict = next_dict;
	}

	free(rt);

	return true;
}

/*
 * Get an error message.
 */
const char *rt_get_error_message(struct rt_env *rt)
{
	return &rt->error_message[0];
}

/*
 * Get an error file name.
 */
const char *rt_get_error_file(struct rt_env *rt)
{
	return &rt->file_name[0];
}

/* Get an error line number. */
int rt_get_error_line(struct rt_env *rt)
{
	return rt->line;
}

/*
 * Register functions from a souce text.
 */
bool
rt_register_source(
	struct rt_env *rt,
	const char *file_name,
	const char *source_text)
{
	struct hir_block *hfunc;
	struct lir_func *lfunc;
	struct rt_func *rfunc;
	int i, func_count;
	bool is_succeeded;

	is_succeeded = false;
	do {
		/* Do parse and build AST. */
		if (!ast_build(file_name, source_text)) {
			strncpy(rt->file_name, ast_get_file_name(), sizeof(rt->file_name));
			rt->line = ast_get_error_line();
			rt_error(rt, "%s", ast_get_error_message());
			break;
		}

		/* Transform AST to HIR. */
		if (!hir_build()) {
			strncpy(rt->file_name, hir_get_file_name(), sizeof(rt->file_name));
			rt->line = hir_get_error_line();
			rt_error(rt, "%s", hir_get_error_message());
			break;
		}

		/* For each function. */
		func_count = hir_get_function_count();
		for (i = 0; i < func_count; i++) {
			/* Transform HIR to LIR (bytecode). */
			hfunc = hir_get_function(i);
			if (!lir_build(hfunc, &lfunc)) {
				strncpy(rt->file_name, lir_get_file_name(), sizeof(rt->file_name));
				rt->line = lir_get_error_line();
				rt_error(rt, "%s", lir_get_error_message());
				break;
			}

			/* Make a function object. */
			if (!rt_register_lir(rt, lfunc))
				break;

			/* Free a LIR. */
			lir_free(lfunc);
		}

		is_succeeded = true;
	} while (0);

	/* Free intermediates. */
	hir_free();
	ast_free();

	/* If failed. */
	if (!is_succeeded)
		return false;

	/* Succeeded. */
	return true;
}

/* Register a function from LIR. */
static bool
rt_register_lir(
	struct rt_env *rt,
	struct lir_func *lir)
{
	struct rt_func *func;
	struct rt_bindglobal *global;
	int i;

	func = malloc(sizeof(struct rt_func));
	if (func == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(func, 0, sizeof(struct rt_func));

	func->name = strdup(lir->func_name);
	if (func->name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	func->param_count = lir->param_count;
	for (i = 0; i < lir->param_count; i++) {
		func->param_name[i] = strdup(lir->param_name[i]);
		if (func->param_name[i] == NULL) {
			rt_out_of_memory(rt);
			return false;
		}
	}
	func->bytecode_size = lir->bytecode_size;
	func->bytecode = malloc(lir->bytecode_size);
	if (func->bytecode == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memcpy(func->bytecode, lir->bytecode, lir->bytecode_size);
	func->tmpvar_size = lir->tmpvar_size;
	func->file_name = strdup(lir->file_name);
	if (func->file_name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}

	/* Insert a bindglobal. */
	global = malloc(sizeof(struct rt_bindglobal));
	if (global == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	global->name = strdup(func->name);
	if (global->name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	global->val.type = RT_VALUE_FUNC;
	global->val.val.func = func;
	global->next = rt->global;
	rt->global = global;

	return true;
}

/*
 * Register functions from bytecode data.
 */
bool rt_register_bytecode(struct rt_env *rt, uint32_t size, uint8_t *data)
{
	assert(NOT_IMPLEMENTED);
	return false;
}

/*
 * Register a C function.
 */
bool
rt_register_cfunc(
	struct rt_env *rt,
	const char *name,
	int param_count,
	const char *param_name[],
	bool (*cfunc)(struct rt_env *env))
{
	struct rt_func *func;
	struct rt_bindglobal *global;
	int i;

	func = malloc(sizeof(struct rt_func));
	if (func == NULL) {
		rt_out_of_memory(rt);
		return false;
	}

	func->name = strdup(name);
	if (func->name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	func->param_count = param_count;
	for (i = 0; i < param_count; i++) {
		func->param_name[i] = strdup(param_name[i]);
		if (func->param_name[i] == NULL) {
			rt_out_of_memory(rt);
			return false;
		}
	}
	func->cfunc = cfunc;

	global = malloc(sizeof(struct rt_bindglobal));
	if (global == NULL) {
		rt_out_of_memory(rt);
		return false;
	}

	global->name = strdup(name);
	if (global->name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	global->val.type = RT_VALUE_FUNC;
	global->val.val.func = func;
	global->next = rt->global;
	rt->global = global;

	return true;
}

/*
 * Call a function with a name.
 */
bool
rt_call_with_name(
	struct rt_env *rt,
	const char *func_name,
	struct rt_value *thisptr,
	int arg_count,
	struct rt_value *arg,
	struct rt_value *ret)
{
	struct rt_bindglobal *bg;
	struct rt_func *func;

	/* Search a function. */
	bg = rt->global;
	while (bg != NULL) {
		if (bg->val.type == RT_VALUE_FUNC &&
		    strcmp(bg->val.val.func->name, func_name) == 0)
			break;
		bg = bg->next;
	}
	if (bg == NULL) {
		rt_error(rt, "Cannot find funcion.");
		return false;
	}
	func = bg->val.val.func;

	/* Call. */
	if (!rt_call(rt, func, thisptr, arg_count, arg, ret))
		return false;

	return true;
}

/*
 * Call a function.
 */
bool
rt_call(
	struct rt_env *rt,
	struct rt_func *func,
	struct rt_value *thisptr,
	int arg_count,
	struct rt_value *arg,
	struct rt_value *ret)
{
	struct rt_bindlocal *local;
	int i;

	/* Allocate a frame for this call. */
	if (!rt_enter_frame(rt, func))
		return false;

	/* Push this-pointer. */
	if (thisptr != NULL) {
		if (!rt_add_local(rt, "this", &local))
			return false;
		local->val = *thisptr;
	}

	/* Push args. */
	for (i = 0; i < arg_count; i++) {
		if (!rt_add_local(rt, func->param_name[i], &local))
			return false;
		local->val = arg[i];
	}

	/* Run. */
	if (func->cfunc != NULL) {
		if (!func->cfunc(rt))
			return false;
	} else {
		strncpy(rt->file_name, rt->frame->func->file_name, sizeof(rt->file_name));

		if (!rt_visit_bytecode(rt, func))
			return false;
	}

	/* Search a return value. */
	if (!rt_find_local(rt, "$return", &local)) {
		ret->type = RT_VALUE_INT;
		ret->val.i = 0;
	} else {
		*ret = local->val;
	}

	/* Succeeded. */
	rt_leave_frame(rt);

	return true;
}

/* Enter a new calling frame. */
static bool
rt_enter_frame(
	struct rt_env *rt,
	struct rt_func *func)
{
	struct rt_frame *frame;

	frame = malloc(sizeof(struct rt_frame));
	if (frame == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(frame, 0, sizeof(struct rt_frame));
	frame->func = func;
	frame->tmpvar_size = func->tmpvar_size;
	frame->tmpvar = malloc(sizeof(struct rt_value) * func->tmpvar_size);
	if (frame->tmpvar == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(frame->tmpvar, 0, sizeof(struct rt_value) * func->tmpvar_size);

	frame->next = rt->frame;
	rt->frame = frame;

	return true;
}

/* Leave a calling frame. */
static void
rt_leave_frame(
	struct rt_env *rt)
{
	struct rt_frame *frame;
	struct rt_string *str, *next_str;
	struct rt_array *arr, *next_arr;
	struct rt_dict *dict, *next_dict;

	/* Move shallow references to the garbage lists. */
	str = rt->frame->shallow_str_list;
	while (str != NULL) {
		next_str = str->next;
		str->next = rt->garbage_str_list;
		str->prev = NULL;
		if (rt->garbage_str_list != NULL)
			rt->garbage_str_list->prev = str;
		rt->garbage_str_list = str;
		str = next_str;
	}
	arr = rt->frame->shallow_arr_list;
	while (arr != NULL) {
		next_arr = arr->next;
		arr->next = rt->garbage_arr_list;
		arr->prev = NULL;
		if (rt->garbage_arr_list != NULL)
			rt->garbage_arr_list->prev = arr;
		rt->garbage_arr_list = arr;
		arr = next_arr;
	}
	dict = rt->frame->shallow_dict_list;
	while (dict != NULL) {
		next_dict = dict->next;
		dict->next = rt->garbage_dict_list;
		dict->prev = NULL;
		if (rt->garbage_dict_list != NULL)
			rt->garbage_dict_list->prev = dict;
		rt->garbage_dict_list = dict;
		dict = next_dict;
	}

	/* Unlink from the list. */
	frame = rt->frame;
	rt->frame = rt->frame->next;

	/* Free. */
	free(frame->tmpvar);
	free(frame);
}

/*
 * Make an integer value.
 */
void
rt_make_int(
	struct rt_value *val,
	int i)
{
	val->type = RT_VALUE_INT;
	val->val.i = i;
}

/*
 * Make an floating-point value.
 */
void
rt_make_float(
	struct rt_value *val,
	float f)
{
	val->type = RT_VALUE_FLOAT;
	val->val.f = f;
}

/*
 * Make a string value.
 */
bool
rt_make_string(
	struct rt_env *rt,
	struct rt_value *val,
	const char *s)
{
	struct rt_string *rts;

	/* Allocate a rt_string. */
	rts = malloc(sizeof(struct rt_string));
	if (rts == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(rts, 0, sizeof(struct rt_string));
	rts->s = strdup(s);
	if (rts->s == NULL) {
		rt_out_of_memory(rt);
		free(rts);
		return false;
	}

	/* Add to the shallow string list. */
	if (rt->frame != NULL) {
		rts->next = rt->frame->shallow_str_list;
		if (rt->frame->shallow_str_list != NULL)
			rt->frame->shallow_str_list->prev = rts;
		rt->frame->shallow_str_list = rts;
	} else {
		rts->next = rt->deep_str_list;
		if (rt->deep_str_list != NULL)
			rt->deep_str_list->prev = rts;
		rt->deep_str_list = rts;
		rts->is_deep = true;
	}

	/* Setup a value. */
	val->type = RT_VALUE_STRING;
	val->val.str = rts;

	/* Increment the heap usage. */
	rt->heap_usage += strlen(s);

	return true;
}

/*
 * Make a string value.
 */
bool
rt_make_string_format(
	struct rt_env *rt,
	struct rt_value *val,
	const char *s,
	...)
{
	va_list ap;

	va_start(ap, s);
	vsnprintf(text_buf, sizeof(text_buf), s, ap);
	va_end(ap);

	if (!rt_make_string(rt, val, text_buf))
		return false;

	return true;
}

/*
 * Make an empty array value.
 */
bool
rt_make_empty_array(struct rt_env *rt, struct rt_value *val)
{
	struct rt_array *arr;

	const int START_SIZE = 16;

	/* Alloc a rt_array. */
	arr = malloc(sizeof(struct rt_array));
	if (arr == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(arr, 0, sizeof(struct rt_array));

	/* Start from size 16. */
	arr->alloc_size = START_SIZE;
	arr->table = malloc(sizeof(struct rt_value) * START_SIZE);
	if (arr->table == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(arr->table, 0, sizeof(struct rt_value) * START_SIZE);
	arr->size = 0;

	val->type = RT_VALUE_ARRAY;
	val->val.arr = arr;

	/* Add to the shallow array list. */
	if (rt->frame != NULL) {
		arr->next = rt->frame->shallow_arr_list;
		if (rt->frame->shallow_arr_list != NULL)
			rt->frame->shallow_arr_list->prev = arr;
		rt->frame->shallow_arr_list = arr;
	} else {
		arr->next = rt->deep_arr_list;
		if (rt->deep_arr_list != NULL)
			rt->deep_arr_list->prev = arr;
		rt->deep_arr_list = arr;
		arr->is_deep = true;
	}

	/* Increment the heap usage. */
	rt->heap_usage += arr->alloc_size * sizeof(struct rt_value);

	return true;
}

/*
 * Make an empty dictionary value.
 */
bool
rt_make_empty_dict(struct rt_env *rt, struct rt_value *val)
{
	struct rt_dict *dict;

	const int START_SIZE = 16;

	/* Alloc a rt_dict. */
	dict = malloc(sizeof(struct rt_dict));
	if (dict == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(dict, 0, sizeof(struct rt_dict));

	/* Start from size 16. */
	dict->alloc_size = START_SIZE;
	dict->key = malloc(sizeof(char *) * START_SIZE);
	if (dict->key == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	dict->value = malloc(sizeof(struct rt_value) * START_SIZE);
	if (dict->value == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	memset(dict->value, 0, sizeof(struct rt_value) * START_SIZE);
	dict->size = 0;

	val->type = RT_VALUE_DICT;
	val->val.dict = dict;

	/* Add to the shallow array list. */
	if (rt->frame != NULL) {
		dict->next = rt->frame->shallow_dict_list;
		if (rt->frame->shallow_dict_list != NULL)
			rt->frame->shallow_dict_list->prev = dict;
		rt->frame->shallow_dict_list = dict;
	} else {
		dict->next = rt->deep_dict_list;
		if (rt->deep_dict_list != NULL)
			rt->deep_dict_list->prev = dict;
		rt->deep_dict_list = dict;
		dict->is_deep = true;
	}

	/* Increment the heap usage. */
	rt->heap_usage += dict->alloc_size * sizeof(struct rt_value);

	return true;
}

/*
 * Clone a value.
 */
bool
rt_copy_value(
	struct rt_env *rt,
	struct rt_value *dst,
	struct rt_value *src)
{
	*dst = *src;
	return true;
}

/*
 * Get a value type.
 */
bool
rt_get_value_type(
	struct rt_env *rt,
	struct rt_value *val,
	int *type)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type == RT_VALUE_INT ||
	       val->type == RT_VALUE_FLOAT ||
	       val->type == RT_VALUE_STRING ||
	       val->type == RT_VALUE_ARRAY ||
	       val->type == RT_VALUE_DICT);
	assert(type != NULL);

	*type = val->type;

	return true;
}

/* Get an integer value. */
bool
rt_get_int(
	struct rt_env *rt,
	struct rt_value *val,
	int *ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type == RT_VALUE_INT);

	*ret = val->val.i;

	return true;
}

/* Get a floating-point value. */
bool
rt_get_float(
	struct rt_env *rt,
	struct rt_value *val,
	float *ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type == RT_VALUE_FLOAT);

	*ret = val->val.f;

	return true;
}

/* Get a string value. */
bool
rt_get_string(
	struct rt_env *rt,
	struct rt_value *val,
	const char **ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type == RT_VALUE_STRING);
	assert(val->val.str != NULL);
	assert(val->val.str->s != NULL);

	*ret = val->val.str->s;

	return true;
}

/* Get a function value. */
bool
rt_get_func(
	struct rt_env *rt,
	struct rt_value *val,
	struct rt_func **ret)
{
	assert(rt != NULL);
	assert(val != NULL);
	assert(val->type == RT_VALUE_FUNC);
	assert(val->val.func != NULL);

	*ret = val->val.func;

	return true;
}

/*
 * Get an array size.
 */
bool
rt_get_array_size(struct rt_env *rt, struct rt_value *array, int *size)
{
	assert(rt != NULL);
	assert(array != NULL);
	assert(array->type == RT_VALUE_ARRAY);

	if (array->type != RT_VALUE_ARRAY) {
		rt_error(rt, "Not an array.");
		return false;
	}

	*size = array->val.arr->size;

	return true;
}

/*
 * Get an array element.
 */
bool
rt_get_array_elem(struct rt_env *rt, struct rt_value *array, int index, struct rt_value *val)
{
	assert(rt != NULL);
	assert(array != NULL);
	assert(array->type == RT_VALUE_ARRAY);
	assert(index < array->val.arr->size);

	/* Check the array boundary. */
	if (index < 0) {
		rt_error(rt, "Array index %d is negative value.", index);
		return false;
	}
	if (index >= array->val.arr->size) {
		rt_error(rt, "Array index %d is out-of-range.", index);
		return false;
	}

	/* Load. */
	*val = array->val.arr->table[index];

	return true;
}

/*
 * Set an array element.
 */
bool
rt_set_array_elem(struct rt_env *rt, struct rt_value *array, int index, struct rt_value *val)
{
	assert(rt != NULL);
	assert(array != NULL);
	assert(array->type == RT_VALUE_ARRAY);

	/* Expand the array if needed. */
	if (!rt_expand_array(rt, array, index))
		return false;
	if (array->val.arr->size < index + 1)
		array->val.arr->size = index + 1;

	/* Store. */
	array->val.arr->table[index] = *val;

	/* Mark the references of the array and its element as strong. */
	rt_make_deep_reference(rt, array);
	rt_make_deep_reference(rt, val);

	return true;
}

/*
 * Expand an array.
 */
static bool
rt_expand_array(
	struct rt_env *rt,
	struct rt_value *array,
	int size)
{
	struct rt_array *arr;
	struct rt_value *new_tbl;

	assert(rt != NULL);
	assert(array->type == RT_VALUE_ARRAY);

	arr = array->val.arr;

	/* Expand the table. */
	if (arr->alloc_size < size) {
		/* Decrement the heap usage. */
		rt->heap_usage -= arr->alloc_size * sizeof(struct rt_value);

		/* Realloc the table. */
		new_tbl = malloc(sizeof(struct rt_value) * size);
		if (new_tbl == NULL) {
			rt_out_of_memory(rt);
			return false;
		}
		memset(new_tbl, 0, sizeof(struct rt_value) * size);
		memcpy(new_tbl, arr->table, sizeof(struct rt_value) * arr->alloc_size);
		free(arr->table);
		arr->table = new_tbl;
		arr->alloc_size = size;

		/* Increment the heap usage. */
		rt->heap_usage += arr->alloc_size * sizeof(struct rt_value);
	}

	return true;
}

static bool
rt_resize_array(
	struct rt_env *rt,
	struct rt_value *arr,
	int size)
{
	struct rt_array *a;
	struct rt_value *new_tbl;

	assert(rt != NULL);
	assert(arr->type == RT_VALUE_ARRAY);

	a = arr->val.arr;

	/* Expand the array size if needed. */
	if (!rt_expand_array(rt, arr, size))
		return false;

	/* If we shrink the array: */
	if (size <= a->size) {
		/* Remove the reminder. */
		memset(&a->table[size], 0, sizeof(struct rt_value) * (a->size - size - 1));
	}

	/* Set the element count. */
	a->size = size;

	return true;
}

/*
 * Get a dictionary size.
 */
bool
rt_get_dict_size(struct rt_env *rt, struct rt_value *dict, int *size)
{
	assert(rt != NULL);
	assert(dict != NULL);
	assert(dict->type == RT_VALUE_DICT);
	assert(size != NULL);

	if (dict->type != RT_VALUE_DICT)
		return false;

	*size = dict->val.dict->size;

	return true;
}

/*
 * Get a dictionary value by an index.
 */
bool
rt_get_dict_value_by_index(struct rt_env *rt, struct rt_value *dict, int index, struct rt_value *val)
{
	assert(rt != NULL);
	assert(dict != NULL);
	assert(dict->type == RT_VALUE_DICT);
	assert(index < dict->val.dict->size);

	if (dict->type != RT_VALUE_DICT)
		return false;
	
	*val = dict->val.dict->value[index];
		
	return true;
}

/*
 * Get a dictionary key by an index.
 */
bool
rt_get_dict_key_by_index(struct rt_env *rt, struct rt_value *dict, int index, const char **key)
{
	assert(rt != NULL);
	assert(dict != NULL);
	assert(dict->type == RT_VALUE_DICT);
	assert(index < dict->val.dict->size);

	if (dict->type != RT_VALUE_DICT)
		return false;
	
	*key = dict->val.dict->key[index];
		
	return false;
}

/* Get a dictionary element. */
bool
rt_get_dict_elem(struct rt_env *rt, struct rt_value *dict, const char *key, struct rt_value *val)
{
	int i;

	assert(rt != NULL);
	assert(dict != NULL);
	assert(dict->type == RT_VALUE_DICT);
	assert(key != NULL);
	assert(val != NULL);

	for (i = 0; i < dict->val.dict->size; i++) {
		if (strcmp(dict->val.dict->key[i], key) == 0) {
			*val = dict->val.dict->value[i];
			return true;
		}
	}

	rt_error(rt, "Dictionary key \"%s\" not found.", key);

	return false;
}

/* Set a dictionary element. */
bool
rt_set_dict_elem(struct rt_env *rt, struct rt_value *dict, const char *key, struct rt_value *val)
{
	int i;

	assert(rt != NULL);
	assert(dict != NULL);
	assert(dict->type == RT_VALUE_DICT);
	assert(key != NULL);
	assert(val != NULL);

	/* Search for the key. */
	for (i = 0; i < dict->val.dict->size; i++) {
		if (strcmp(dict->val.dict->key[i], key) == 0) {
			dict->val.dict->value[i] = *val;
			return true;
		}
	}

	/* Expand the size. */
	if (!rt_expand_dict(rt, dict, dict->val.dict->size + 1))
		return false;

	/* Append the key. */
	dict->val.dict->key[dict->val.dict->size] = strdup(key);
	if (dict->val.dict->key[dict->val.dict->size] == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	dict->val.dict->value[dict->val.dict->size] = *val;
	dict->val.dict->size++;

	/* Mark the references of the dictionary and its element as strong. */
	rt_make_deep_reference(rt, dict);
	rt_make_deep_reference(rt, val);

	return true;
}

/*
 * Expand an array.
 */
static bool
rt_expand_dict(
	struct rt_env *rt,
	struct rt_value *dict,
	int size)
{
	struct rt_dict *d;
	char **new_key;
	struct rt_value *new_value;

	assert(rt != NULL);
	assert(dict->type == RT_VALUE_DICT);

	d = dict->val.dict;

	/* Expand the table. */
	if (d->alloc_size < size) {
		/* Decrement the heap usage. */
		rt->heap_usage -= d->alloc_size * (sizeof(char *) + sizeof(struct rt_value));

		/* Realloc the key table. */
		new_key = malloc(sizeof(const char *) * size);
		if (new_key == NULL) {
			rt_out_of_memory(rt);
			return false;
		}
		memcpy(new_key, d->key, sizeof(struct rt_value) * d->alloc_size);
		free(d->key);
		d->key = new_key;

		/* Realloc the value table. */
		new_value = malloc(sizeof(struct rt_value) * size);
		if (new_value == NULL) {
			rt_out_of_memory(rt);
			return false;
		}
		memcpy(new_value, d->value, sizeof(struct rt_value) * d->alloc_size);
		free(d->value);
		d->value = new_value;

		d->alloc_size = size;

		/* Increment the heap usage. */
		rt->heap_usage += d->alloc_size * (sizeof(char *) + sizeof(struct rt_value));
	}

	return true;
}

/* Remove a dictionary element. */
bool
rt_remove_dict_elem(
	struct rt_env *rt,
	struct rt_value *dict,
	const char *key)
{
	int i;

	assert(rt != NULL);
	assert(dict != NULL);
	assert(dict->type == RT_VALUE_DICT);
	assert(key != NULL);

	/* Search for the key. */
	for (i = 0; i < dict->val.dict->size; i++) {
		if (strcmp(dict->val.dict->key[i], key) == 0) {
			/* Remove the key and value. */
			free(dict->val.dict->key[i]);
			memmove(&dict->val.dict->key[i],
				&dict->val.dict->key[i + 1],
				sizeof(const char *) * (dict->val.dict->size - i - 1));
			memmove(&dict->val.dict->value[i],
				&dict->val.dict->value[i + 1],
				sizeof(struct rt_value) * (dict->val.dict->size - i - 1));
			dict->val.dict->size--;
			return true;
		}
	}

	rt_error(rt, "Key \"%s\" not found.", key);
	return false;
}

/*
 * Get a local variable value. (For C func implementation)
 */
bool rt_get_local(struct rt_env *rt, const char *name, struct rt_value *val)
{
	struct rt_bindlocal *local;

	if (!rt_find_local(rt, name, &local)) {
		rt_error(rt, "Local variable \"%s\" not found.", name);
		return false;
	}

	*val = local->val;

	return true;
}

/*
 * Set a local variable value. (For C func implementation)
 */
bool
rt_set_local(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val)
{
	struct rt_bindlocal *local;

	if (!rt_find_local(rt, name, &local)) {
		/* If the name is $return, add a local variable. Otherwise, fail. */
		if (strcmp(name, "$return") != 0)
			return false;
		if (!rt_add_local(rt, "$return", &local))
			return false;
	}

	local->val = *val;

	return true;
}

static bool
rt_add_local(
	struct rt_env *rt,
	const char *name,
	struct rt_bindlocal **local)
{
	*local = malloc(sizeof(struct rt_bindlocal));
	if (*local == NULL) {
		rt_out_of_memory(rt);
		return false;
	}

	(*local)->name = strdup(name);
	if ((*local)->name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}

	(*local)->next = rt->frame->local;
	rt->frame->local = (*local);

	return true;
}

static bool
rt_find_local(
	struct rt_env *rt,
	const char *name,
	struct rt_bindlocal **local)
{
	struct rt_bindlocal *l;

	l = rt->frame->local;
	while (l != NULL) {
		if (strcmp(l->name, name) == 0) {
			*local = l;
			return true;
		}
		l = l->next;
	}

	*local = NULL;

	return false;
}

/*
 * Get a global variable.
 */
bool
rt_get_global(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val)
{
	struct rt_bindglobal *global;

	global = rt->global;
	while (global != NULL) {
		if (strcmp(global->name, name) == 0)
			break;
		global = global->next;
	}
	if (global == NULL) {
		rt_error(rt, "Global variable %d not found.", name);
		return false;
	}

	*val = global->val;

	return true;
}

static bool
rt_find_global(
	struct rt_env *rt,
	const char *name,
	struct rt_bindglobal **global)
{
	struct rt_bindglobal *g;

	g = rt->global;
	while (g != NULL) {
		if (strcmp(g->name, name) == 0) {
			*global = g;
			return true;
		}
		g = g->next;
	}

	*global = NULL;

	return false;
}


/*
 * Set a global variable.
 */
bool
rt_set_global(
	struct rt_env *rt,
	const char *name,
	struct rt_value *val)
{
	struct rt_bindglobal *global;

	if (!rt_find_global(rt, name, &global)) {
		if (!rt_add_global(rt, name, &global))
			return false;
	}

	global->val = *val;

	return true;
}

static bool
rt_add_global(
	struct rt_env *rt,
	const char *name,
	struct rt_bindglobal **global)
{
	struct rt_bindglobal *g;

	g = malloc(sizeof(struct rt_bindglobal));
	if (g == NULL) {
		rt_out_of_memory(rt);
		return false;
	}

	g->name = strdup(name);
	if (g->name == NULL) {
		rt_out_of_memory(rt);
		return false;
	}
	g->val.type = RT_VALUE_INT;
	g->val.val.i = 0;

	g->next = rt->global;
	rt->global = g;

	*global = g;

	return true;
}

/*
 * GC
 */

/*
 * Do a shallow GC for nursery space.
 */
bool
rt_shallow_gc(
	struct rt_env *rt)
{
	struct rt_string *str, *next_str;
	struct rt_array *arr, *next_arr;
	struct rt_dict *dict, *next_dict;

	/*
	 * A nursery space belongs to a calling frame.
	 * An object first created in a nursery space,
	 * and when it get a strong reference,
	 * it is moved to the tenured space.
	 * Objects in a nursery space are released by rt_leave_frame(),
	 * and are moved to the garbage list.
	 * The shallow GC sweeps such objects in the garbage list.
	 */

	str = rt->garbage_str_list;
	while (str != NULL) {
		next_str = str->next;
		rt_free_string(rt, str);
		str = next_str;
	}
	rt->garbage_str_list = NULL;

	arr = rt->garbage_arr_list;
	while (arr != NULL) {
		next_arr = arr->next;
		rt_free_array(rt, arr);
		arr = next_arr;
	}
	rt->garbage_arr_list = NULL;

	dict = rt->garbage_dict_list;
	while (dict != NULL) {
		next_dict = dict->next;
		rt_free_dict(rt, dict);
		dict = next_dict;
	}
	rt->garbage_arr_list = NULL;

	return true;
}

/*
 * Do a deep GC. (tenured space GC)
 */
bool
rt_deep_gc(
	struct rt_env *rt)
{
	struct rt_string *str, *next_str;
	struct rt_array *arr, *next_arr;
	struct rt_dict *dict, *next_dict;
	struct rt_bindglobal *global;

	/*
	 * We do a full mark-and-sweep GC for objects in the tenured space.
	 * For now, objects in nersery spaces are not affected by this deep GC.
	 */

	/* First, do a shallow GC and sweep objects in the garbage lists. */
	rt_shallow_gc(rt);

	/* Clear marks of strings with strong references. */
	str = rt->deep_str_list;
	while (str != NULL) {
		str->is_marked = false;
		str = str->next;
	}
	
	/* Clear marks of arrays with a strong references. */
	arr = rt->deep_arr_list;
	while (arr != NULL) {
		arr->is_marked = false;
		arr = arr->next;
	}

	/* Clear marks of dictionaries with strong references. */
	dict = rt->deep_dict_list;
	while (dict != NULL) {
		dict->is_marked = false;
		dict = dict->next;
	}

	/* Recursively mark all objects that are referenced by the global variables. */
	global = rt->global;
	while (global != NULL) {
		rt_recursively_mark_object(rt, &global->val);
		global = global->next;
	}

	/* Sweep strings without marks. */
	str = rt->deep_str_list;
	while (str != NULL) {
		next_str = str->next;
		if (!str->is_marked) {
			/* Unlink. */
			if (str->prev != NULL) {
				str->prev->next = str->next;
				str->next->prev = str->prev;
			} else {
				str->next->prev = NULL;
				rt->deep_str_list = str->next;
			}

			/* Remove. */
			rt_free_string(rt, str);
		}
		str = next_str;
	}

	/* Sweep arrays without marks. */
	arr = rt->deep_arr_list;
	while (arr != NULL) {
		next_arr = arr->next;
		if (!arr->is_marked) {
			/* Unlink. */
			if (arr->prev != NULL) {
				arr->prev->next = arr->next;
				arr->next->prev = arr->prev;
			} else {
				arr->next->prev = NULL;
				rt->deep_arr_list = arr->next;
			}

			/* Remove. */
			rt_free_array(rt, arr);
		}
		arr = next_arr;
	}

	/* Sweep dictionaries without marks. */
	dict = rt->deep_dict_list;
	while (dict != NULL) {
		next_dict = dict->next;
		if (!dict->is_marked) {
			/* Unlink. */
			if (dict->prev != NULL) {
				dict->prev->next = dict->next;
				dict->next->prev = dict->prev;
			} else {
				dict->next->prev = NULL;
				rt->deep_arr_list = arr->next;
			}

			/* Remove. */
			rt_free_dict(rt, dict);
		}
		dict = next_dict;
	}

	return true;
}

/* Mark objects recursively as used. */
static void
rt_recursively_mark_object(
	struct rt_env *rt,
	struct rt_value *val)
{
	int i;

	switch (val->type) {
	case RT_VALUE_INT:
	case RT_VALUE_FLOAT:
		break;
	case RT_VALUE_STRING:
		val->val.str->is_marked = true;
		break;
	case RT_VALUE_ARRAY:
		for (i = 0; i < val->val.arr->size; i++)
			rt_recursively_mark_object(rt, &val->val.arr->table[i]);
		break;
	case RT_VALUE_DICT:
		for (i = 0; i < val->val.dict->size; i++)
			rt_recursively_mark_object(rt, &val->val.dict->value[i]);
		break;
	case RT_VALUE_FUNC:
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}
}

/* Set the reference of a value's object as strong.  */
static void
rt_make_deep_reference(
	struct rt_env *rt,
	struct rt_value *val)
{
	if (rt->frame != NULL)
		return;

	switch (val->type) {
	case RT_VALUE_INT:
	case RT_VALUE_FLOAT:
	case RT_VALUE_FUNC:
		break;
	case RT_VALUE_STRING:
		if (!val->val.str->is_deep) {
			/* Unlink from the shallow list. */
			if (val->val.str->prev != NULL) {
				val->val.str->prev->next = val->val.str->next;
				val->val.str->next->prev = val->val.str->prev;
			} else {
				val->val.str->next->prev = NULL;
				rt->frame->shallow_str_list = val->val.str->next;
			}

			/* Link to the deep list. */
			val->val.str->next = rt->deep_str_list;
			rt->deep_str_list->prev = val->val.str;
			rt->deep_str_list = val->val.str;

			/* Make deep. */
			val->val.str->is_deep = true;
		}
		break;
	case RT_VALUE_ARRAY:
		if (!val->val.arr->is_deep) {
			/* Unlink from the shallow list. */
			if (val->val.arr->prev != NULL) {
				val->val.arr->prev->next = val->val.arr->next;
				val->val.arr->next->prev = val->val.arr->prev;
			} else {
				val->val.arr->next->prev = NULL;
				rt->frame->shallow_arr_list = val->val.arr->next;
			}

			/* Link to the deep list. */
			val->val.arr->next = rt->deep_arr_list;
			rt->deep_arr_list->prev = val->val.arr;
			rt->deep_arr_list = val->val.arr;

			/* Make deep. */
			val->val.arr->is_deep = true;
		}
		break;
	case RT_VALUE_DICT:
		if (!val->val.dict->is_deep) {
			/* Unlink from the shallow list. */
			if (val->val.dict->prev != NULL) {
				val->val.dict->prev->next = val->val.dict->next;
				val->val.dict->next->prev = val->val.dict->prev;
			} else {
				val->val.dict->next->prev = NULL;
				rt->frame->shallow_dict_list = val->val.dict->next;
			}

			/* Link to the deep list. */
			val->val.dict->next = rt->deep_dict_list;
			rt->deep_dict_list->prev = val->val.dict;
			rt->deep_dict_list = val->val.dict;

			/* Make deep. */
			val->val.dict->is_deep = true;
		}
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}
}

/* Free a string. */
static void
rt_free_string(
	struct rt_env *rt,
	struct rt_string *str)
{
	free(str->s);
	free(str);
}

/* Free an array. */
static void
rt_free_array(
	struct rt_env *rt,
	struct rt_array *array)
{
	free(array->table);
	free(array);
}

/* Free a dictionary. */
static void
rt_free_dict(
	struct rt_env *rt,
	struct rt_dict *dict)
{
	free(dict->key);
	free(dict->value);
	free(dict);
}

/* Get an approximate memory usage in bytes. */
bool
rt_get_heap_usage(
	struct rt_env *rt,
	size_t *ret)
{
	*ret = rt->heap_usage;
	return true;
}

/*
 * Instruction Interpretation
 */

/* Visit a bytecode array. */
static bool
rt_visit_bytecode(
	struct rt_env *rt,
	struct rt_func *func)
{
	int pc;

	pc = 0;
	while (pc < func->bytecode_size) {
		if (!rt_visit_op(rt, func, &pc))
			return false;
	}

	return true;
}

/* Visit an ROP_ASSIGN instruction. */
static inline bool
rt_visit_assign_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src;

	assert(func->bytecode[*pc] == ROP_ASSIGN);

	if (*pc + 1 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	src = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	rt->frame->tmpvar[dst] = rt->frame->tmpvar[src];

	*pc += 1 + 2 + 2;

	return true;
}

/* Visit an ROP_ICONST instruction. */
static inline bool
rt_visit_iconst_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t val;

	assert(func->bytecode[*pc] == ROP_ICONST);

	if (*pc + 1 + 2 + 4 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	val = (func->bytecode[*pc + 3] << 24) |
	       (func->bytecode[*pc + 4] << 16) |
	       (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];

	rt->frame->tmpvar[dst].type = RT_VALUE_INT;
	rt->frame->tmpvar[dst].val.i = val;

	*pc += 1 + 2 + 4;

	return true;
}

/* Visit an ROP_FCONST instruction. */
static inline bool
rt_visit_fconst_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t raw;
	float val;

	assert(func->bytecode[*pc] == ROP_FCONST);

	if (*pc + 1 + 2 + 4 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	raw = (func->bytecode[*pc + 3] << 24) |
	       (func->bytecode[*pc + 4] << 16) |
	       (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];

	val = *(float *)&raw;

	rt->frame->tmpvar[dst].type = RT_VALUE_FLOAT;
	rt->frame->tmpvar[dst].val.f = val;

	*pc += 1 + 2 + 4;

	return true;
}

/* Visit an ROP_SCONST instruction. */
static inline bool
rt_visit_sconst_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	const char *s;
	int len;

	assert(func->bytecode[*pc] == ROP_SCONST);

	if (*pc + 1 + 2  > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
	      func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	s = (const char *)&func->bytecode[*pc + 3];
	len = strlen(s);
	if (*pc + 1 + 2 + len + 1 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (!rt_make_string(rt, &rt->frame->tmpvar[dst], s))
		return false;

	*pc += 1 + 2 + len + 1;

	return true;
}

/* Visit an ROP_ACONST instruction. */
static inline bool
rt_visit_aconst_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;

	assert(func->bytecode[*pc] == ROP_ACONST);

	if (*pc + 1 + 2  > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
	      func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (!rt_make_empty_array(rt, &rt->frame->tmpvar[dst]))
		return false;

	*pc += 1 + 2;

	return true;
}

/* Visit an ROP_DCONST instruction. */
static inline bool
rt_visit_dconst_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;

	assert(func->bytecode[*pc] == ROP_DCONST);

	if (*pc + 1 + 2  > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
	      func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (!rt_make_empty_dict(rt, &rt->frame->tmpvar[dst]))
		return false;

	*pc += 1 + 2;

	return true;
}

/* Visit an ROP_INC instruction. */
static inline bool
rt_visit_inc_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	struct rt_value *val;
	uint32_t dst;

	assert(func->bytecode[*pc] == ROP_INC);

	if (*pc + 1 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	val = &rt->frame->tmpvar[dst];
	if (val->type != RT_VALUE_INT) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	val->val.i++;

	*pc += 1 + 2;

	return true;
}

/* Visit an ROP_ADD instruction. */
static inline bool
rt_visit_add_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_ADD);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i + src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (float)src1_val->val.i + src2_val->val.f;
			break;
		case RT_VALUE_STRING:
			if (!rt_make_string_format(rt, dst_val, "%d%s", src1_val->val.i, src2_val->val.str->s))
				return false;
			break;
		default:
			rt_error(rt, "Value is not a number or a string.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f + (float)src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f + src2_val->val.f;
			break;
		case RT_VALUE_STRING:
			if (!rt_make_string_format(rt, dst_val, "%f%s", src1_val->val.i, src2_val->val.str->s))
				return false;
			break;
		default:
			rt_error(rt, "Value is not a number or a string.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			if (!rt_make_string_format(rt, dst_val, "%s%d", src1_val->val.str->s, src2_val->val.i))
				return false;
			break;
		case RT_VALUE_FLOAT:
			if (!rt_make_string_format(rt, dst_val, "%s%f", src1_val->val.str->s, src2_val->val.f))
				return false;
			break;
		case RT_VALUE_STRING:
			if (!rt_make_string_format(rt, dst_val, "%s%s", src1_val->val.str->s, src2_val->val.str->s))
				return false;
			break;
		default:
			rt_error(rt, "Value is not a number or a string.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_SUB instruction. */
static inline bool
rt_visit_sub_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_SUB);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i - src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (float)src1_val->val.i - src2_val->val.f;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f - (float)src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f - src2_val->val.f;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not a number.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_MUL instruction. */
static inline bool
rt_visit_mul_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_MUL);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i * src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (float)src1_val->val.i * src2_val->val.f;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f * (float)src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f * src2_val->val.f;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not a number.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_DIV instruction. */
static inline bool
rt_visit_div_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_DIV);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i / src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (float)src1_val->val.i / src2_val->val.f;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f / (float)src2_val->val.i;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = src1_val->val.f / src2_val->val.f;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not a number.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_MOD instruction. */
static inline bool
rt_visit_mod_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_MOD);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i % src2_val->val.i;
			break;
		default:
			rt_error(rt, "Value is not an integer.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not an integer.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_AND instruction. */
static inline bool
rt_visit_and_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_AND);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i & src2_val->val.i;
			break;
		default:
			rt_error(rt, "Value is not an integer.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not an integer.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_OR instruction. */
static inline bool
rt_visit_or_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_OR);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i | src2_val->val.i;
			break;
		default:
			rt_error(rt, "Value is not an integer.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not an integer.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_XOR instruction. */
static inline bool
rt_visit_xor_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_XOR);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = src1_val->val.i ^ src2_val->val.i;
			break;
		default:
			rt_error(rt, "Value is not an integer.");
			return false;
		}
		break;
	default:
		rt_error(rt, "Value is not an integer.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_NEG instruction. */
static inline bool
rt_visit_neg_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src;
	struct rt_value *dst_val;
	struct rt_value *src_val;

	assert(func->bytecode[*pc] == ROP_NEG);

	if (*pc + 1 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src_val = &rt->frame->tmpvar[src];

	switch (src_val->type) {
	case RT_VALUE_INT:
		dst_val->type = RT_VALUE_INT;
		dst_val->val.i = -src_val->val.i;
		break;
	default:
		rt_error(rt, "Value is not an integer.");
		return false;
	}

	*pc += 1 + 2 + 2;
	
	return true;
}

/* Visit an ROP_LT instruction. */
static inline bool
rt_visit_lt_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_LT);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = (src1_val->val.i < src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = ((float)src1_val->val.i < src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f < (float)src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f < src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_STRING:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = strcmp(src1_val->val.str->s, src2_val->val.str->s) < 0 ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a string.");
			break;
		}
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_LTE instruction. */
static inline bool
rt_visit_lte_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_LTE);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = (src1_val->val.i <= src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = ((float)src1_val->val.i <= src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f <= (float)src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f <= src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_STRING:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = strcmp(src1_val->val.str->s, src2_val->val.str->s) <= 0 ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a string.");
			break;
		}
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_GT instruction. */
static inline bool
rt_visit_gt_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_GT);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = (src1_val->val.i > src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = ((float)src1_val->val.i > src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f > (float)src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f > src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_STRING:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = strcmp(src1_val->val.str->s, src2_val->val.str->s) > 0 ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a string.");
			break;
		}
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_GTE instruction. */
static inline bool
rt_visit_gte_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_GTE);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = (src1_val->val.i >= src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = ((float)src1_val->val.i >= src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f >= (float)src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f >= src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_STRING:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = strcmp(src1_val->val.str->s, src2_val->val.str->s) >= 0 ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a string.");
			break;
		}
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_EQ instruction. */
static inline bool
rt_visit_eq_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_EQ);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = (src1_val->val.i == src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = ((float)src1_val->val.i == src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f == (float)src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f == src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_STRING:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = strcmp(src1_val->val.str->s, src2_val->val.str->s) == 0 ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a string.");
			break;
		}
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_NEQ instruction. */
static inline bool
rt_visit_neq_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src1;
	uint32_t src2;
	struct rt_value *dst_val;
	struct rt_value *src1_val;
	struct rt_value *src2_val;

	assert(func->bytecode[*pc] == ROP_NEQ);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src1 = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src1 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src1_val = &rt->frame->tmpvar[src1];

	src2 = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src2 >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src2_val = &rt->frame->tmpvar[src2];

	switch (src1_val->type) {
	case RT_VALUE_INT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_INT;
			dst_val->val.i = (src1_val->val.i != src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = ((float)src1_val->val.i != src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_FLOAT:
		switch (src2_val->type) {
		case RT_VALUE_INT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f != (float)src2_val->val.i) ? 1 : 0;
			break;
		case RT_VALUE_FLOAT:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = (src1_val->val.f != src2_val->val.f) ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a number.");
			return false;
		}
		break;
	case RT_VALUE_STRING:
		switch (src2_val->type) {
		case RT_VALUE_STRING:
			dst_val->type = RT_VALUE_FLOAT;
			dst_val->val.f = strcmp(src1_val->val.str->s, src2_val->val.str->s) != 0 ? 1 : 0;
			break;
		default:
			rt_error(rt, "Value is not a string.");
			break;
		}
	default:
		rt_error(rt, "Value is not a number or a string.");
		return false;
	}

	*pc += 1 + 2 + 2 + 2;
	
	return true;
}

/* Visit an ROP_STOREARRAY instruction. */
static inline bool
rt_visit_storearray_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t arr_index;
	uint32_t subscr_index;
	uint32_t src_index;
	struct rt_value *arr_val;
	struct rt_value *subscr_val;
	struct rt_value *src_val;
	struct rt_array *array;
	int subscript;
	const char *key;
	bool is_dict;

	assert(func->bytecode[*pc] == ROP_STOREARRAY);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Get an array value. */
	arr_index = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (arr_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	arr_val = &rt->frame->tmpvar[arr_index];
	if (arr_val->type == RT_VALUE_ARRAY) {
		is_dict = false;
	} else if (arr_val->type == RT_VALUE_DICT) {
		is_dict = true;
	} else {
		rt_error(rt, "Not an array or a dictionary.");
		return false;
	}

	/* Get a subscript value. */
	subscr_index = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (subscr_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	subscr_val = &rt->frame->tmpvar[subscr_index];
	if (!is_dict) {
		if (subscr_val->type != RT_VALUE_INT) {
			rt_error(rt, "Subscript not an integer.");
			return false;
		}
		subscript = subscr_val->val.i;
		key = NULL;
	} else {
		if (subscr_val->type != RT_VALUE_STRING) {
			rt_error(rt, "Subscript not a string.");
			return false;
		}
		subscript = -1;
		key = subscr_val->val.str->s;
	}

	/* Get a source value. */
	src_index = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (src_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src_val = &rt->frame->tmpvar[src_index];

	/* Store the element. */
	if (!is_dict) {
		if (!rt_set_array_elem(rt, arr_val, subscript, src_val))
			return false;
	} else {
		if (!rt_set_dict_elem(rt, arr_val, key, src_val))
			return false;
	}

	*pc += 1 + 2 + 2 + 2;

	return true;
}

/* Visit an ROP_LOADARRAY instruction. */
static inline bool
rt_visit_loadarray_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst_index;
	uint32_t arr_index;
	uint32_t subscr_index;
	struct rt_value *dst_val;
	struct rt_value *arr_val;
	struct rt_value *subscr_val;
	int subscript;
	const char *key;
	bool is_dict;

	assert(func->bytecode[*pc] == ROP_LOADARRAY);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Get a destination. */
	dst_index = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst_index];

	/* Get an array value. */
	arr_index = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (arr_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	arr_val = &rt->frame->tmpvar[arr_index];
	if (arr_val->type == RT_VALUE_ARRAY) {
		is_dict = false;
	} else if (arr_val->type == RT_VALUE_DICT) {
		is_dict = true;
	} else {
		rt_error(rt, "Not an array or a dictionary.");
		return false;
	}

	/* Get a subscript value. */
	subscr_index = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (subscr_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	subscr_val = &rt->frame->tmpvar[subscr_index];
	if (!is_dict) {
		if (subscr_val->type != RT_VALUE_INT) {
			rt_error(rt, "Subscript not an integer.");
			return false;
		}
		subscript = subscr_val->val.i;
		key = NULL;
	} else {
		if (subscr_val->type != RT_VALUE_STRING) {
			rt_error(rt, "Subscript not a string.");
			return false;
		}
		subscript = -1;;
		key = subscr_val->val.str->s;
	}

	/* Load the element. */
	if (!is_dict) {
		if (!rt_get_array_elem(rt, arr_val, subscript, dst_val))
			return false;
	} else {
		if (!rt_get_dict_elem(rt, arr_val, key, dst_val))
			return false;
	}

	*pc += 1 + 2 + 2 + 2;

	return true;
}

/* Visit an ROP_LEN instruction. */
static inline bool
rt_visit_len_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t src;
	struct rt_value *dst_val;
	struct rt_value *src_val;

	assert(func->bytecode[*pc] == ROP_LEN);

	if (*pc + 1 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst];

	src = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (src >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	src_val = &rt->frame->tmpvar[src];

	switch (src_val->type) {
	case RT_VALUE_STRING:
		dst_val->type = RT_VALUE_INT;
		dst_val->val.i = strlen(src_val->val.str->s);
		break;
	case RT_VALUE_ARRAY:
		dst_val->type = RT_VALUE_INT;
		dst_val->val.i = src_val->val.arr->size;
		break;
	case RT_VALUE_DICT:
		dst_val->type = RT_VALUE_INT;
		dst_val->val.i = src_val->val.dict->size;
		break;
	default:
		rt_error(rt, "Value is not a string, an array, or a dictionary.");
		return false;
	}

	*pc += 1 + 2 + 2;
	
	return true;
}

/* Visit an ROP_GETDICTKEYBYINDEX instruction. */
static inline bool
rt_visit_getdictkeybyindex_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst_index;
	uint32_t dict_index;
	uint32_t subscr_index;
	struct rt_value *dst_val;
	struct rt_value *dict_val;
	struct rt_value *subscr_val;
	int subscript;
	const char *key;
	bool is_dict;

	assert(func->bytecode[*pc] == ROP_GETDICTKEYBYINDEX);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Get a destination. */
	dst_index = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst_index];

	/* Get a dictionary value. */
	dict_index = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (dict_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dict_val = &rt->frame->tmpvar[dict_index];
	if (dict_val->type != RT_VALUE_DICT) {
		rt_error(rt, "Not a dictionary.");
		return false;
	}

	/* Get a subscript value. */
	subscr_index = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (subscr_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	subscr_val = &rt->frame->tmpvar[subscr_index];
	if (subscr_val->type != RT_VALUE_INT) {
		rt_error(rt, "Subscript not an integer.");
		return false;
	}
	if (subscr_val->val.i >= dict_val->val.dict->size) {
		rt_error(rt, "Dictionary index out-of-range.");
		return false;
	}

	/* Load the element. */
	if (!rt_make_string(rt, dst_val, dict_val->val.dict->key[subscr_val->val.i]))
		return false;

	*pc += 1 + 2 + 2 + 2;

	return true;
}

/* Visit an ROP_GETDICTVALBYINDEX instruction. */
static inline bool
rt_visit_getdictvalbyindex_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst_index;
	uint32_t dict_index;
	uint32_t subscr_index;
	struct rt_value *dst_val;
	struct rt_value *dict_val;
	struct rt_value *subscr_val;
	int subscript;
	const char *key;
	bool is_dict;

	assert(func->bytecode[*pc] == ROP_GETDICTVALBYINDEX);

	if (*pc + 1 + 2 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Get a destination. */
	dst_index = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (dst_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dst_val = &rt->frame->tmpvar[dst_index];

	/* Get a dictionary value. */
	dict_index = (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (dict_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	dict_val = &rt->frame->tmpvar[dict_index];
	if (dict_val->type != RT_VALUE_DICT) {
		rt_error(rt, "Not a dictionary.");
		return false;
	}

	/* Get a subscript value. */
	subscr_index = (func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (subscr_index >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	subscr_val = &rt->frame->tmpvar[subscr_index];
	if (subscr_val->type != RT_VALUE_INT) {
		rt_error(rt, "Subscript not an integer.");
		return false;
	}
	if (subscr_val->val.i >= dict_val->val.dict->size) {
		rt_error(rt, "Dictionary index out-of-range.");
		return false;
	}

	/* Load the element. */
	*dst_val = dict_val->val.dict->value[subscr_val->val.i];

	*pc += 1 + 2 + 2 + 2;

	return true;
}

/* Visit an ROP_LOADYMBOL instruction. */
static inline bool
rt_visit_loadsymbol_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	const char *symbol;
	struct rt_bindlocal *local;
	struct rt_bindglobal *global;
	int len;

	assert(func->bytecode[*pc] == ROP_LOADSYMBOL);

	if (*pc + 1 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
	      (func->bytecode[*pc + 2]);

	symbol = (const char *)&func->bytecode[*pc + 3];
	len = strlen(symbol);
	if (*pc + 2 + len + 1 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Search local. */
	if (rt_find_local(rt, symbol, &local)) {
		rt->frame->tmpvar[dst] = local->val;
	} else {
		/* Search global. */
		if (rt_find_global(rt, symbol, &global)) {
			rt->frame->tmpvar[dst] = global->val;
		} else {
			rt_error(rt, "Symbol \"%s\" not found.", symbol);
			return false;
		}
	}

	*pc += 1 + 2 + len + 1;

	return true;
}

/* Visit an ROP_STORESYMBOL instruction. */
static inline bool
rt_visit_storesymbol_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	const char *symbol;
	uint32_t src;
	struct rt_bindlocal *local;
	struct rt_bindglobal *global;
	int len;

	symbol = (const char *)&func->bytecode[*pc + 1];
	len = strlen(symbol);
	if (*pc + 1 + len + 1 > func->bytecode_size) {
		rt_error(rt, "Bytecode too short.");
		return false;
	}

	src = (func->bytecode[*pc + 1 + len + 1] << 8) |
	      (func->bytecode[*pc + 1 + len + 1 + 1]);

	/* Search local. */
	if (rt_find_local(rt, symbol, &local)) {
		/* Found. */
		local->val = rt->frame->tmpvar[src];
	} else {
		/* Not found. Search global. */
		if (rt_find_global(rt, symbol, &global)) {
			/* Found. */
			global->val = rt->frame->tmpvar[src];
			rt_make_deep_reference(rt, &global->val);
		} else {
			/* Not found. Bind a local variable. */
			if (local == NULL && global == NULL) {
				if (!rt_add_local(rt, symbol, &local))
					return false;
				local->val = rt->frame->tmpvar[src];
			}
		}
	}

	*pc += 1 + len + 1 + 2;

	return true;
}

/* Visit an ROP_LOADDOT instruction. */
static inline bool
rt_visit_loaddot_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t dst;
	uint32_t dict;
	const char *field;
	int len;

	assert(func->bytecode[*pc] == ROP_LOADDOT);

	if (*pc + 1 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst = (func->bytecode[*pc + 1] << 8) |
		(func->bytecode[*pc + 2]);
	if (dst >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dict = (func->bytecode[*pc + 3] << 8) |
		(func->bytecode[*pc + 4]);
	if (dict >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	field = (const char *)&func->bytecode[*pc + 5];
	len = strlen(field);
	if (*pc + 1 + 2  + 2 + len + 1 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Load. */
	if (!rt_get_dict_elem(rt, &rt->frame->tmpvar[dict], field, &rt->frame->tmpvar[dst]))
		return false;

	*pc += 1 + 2 + 2 + len + 1;

	return true;
}

/* Visit an ROP_STOREDOT instruction. */
static inline bool
rt_visit_storedot_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t src;
	uint32_t dict;
	const char *field;
	int len;

	assert(func->bytecode[*pc] == ROP_STOREDOT);

	if (*pc + 1 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dict = (func->bytecode[*pc + 1] << 8) |
		(func->bytecode[*pc + 2]);
	if (dict >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	field = (const char *)&func->bytecode[*pc + 3];
	len = strlen(field);
	if (*pc + 1 + 2  + 2 + len + 1 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	src = (func->bytecode[*pc + 1 + 2 + len + 1] << 8) |
		(func->bytecode[*pc + 1 + 2 + len + 1 + 1]);
	if (src >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	/* Store. */
	if (!rt_set_dict_elem(rt, &rt->frame->tmpvar[dict], field, &rt->frame->tmpvar[src]))
		return false;

	*pc += 1 + 2 + 2 + len + 1;

	return true;
}

/* Visit an ROP_CALL instruction. */
static inline bool
rt_visit_call_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	struct rt_func *callee;
	struct rt_value arg[RT_ARG_MAX];
	struct rt_value ret;
	int arg_tmpvar;
	int arg_count;
	int dst_tmpvar;
	int func_tmpvar;
	int i;

	assert(func->bytecode[*pc] == ROP_CALL);

	dst_tmpvar = (func->bytecode[*pc + 1] << 8) |
		      func->bytecode[*pc + 2];
	if (dst_tmpvar >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	func_tmpvar = (func->bytecode[*pc + 3] << 8) |
		      func->bytecode[*pc + 4];
	if (func_tmpvar >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	if (rt->frame->tmpvar[func_tmpvar].type != RT_VALUE_FUNC) {
		rt_error(rt, "Not a function.");
		return false;
	}
	callee = rt->frame->tmpvar[func_tmpvar].val.func;

	arg_count = func->bytecode[*pc + 5];
	for (i = 0; i < arg_count; i++) {
		arg_tmpvar = (func->bytecode[*pc + 6 + i * 2] << 8 ) | 
			     func->bytecode[*pc + 6 + i * 2 + 1];
		arg[i] = rt->frame->tmpvar[arg_tmpvar];
	}

	if (!rt_enter_frame(rt, callee))
		return false;

	/* Call. */
	if (!rt_call(rt, callee, NULL, arg_count, arg, &ret))
		return false;

	rt_leave_frame(rt);

	rt->frame->tmpvar[dst_tmpvar] = ret;

	*pc += 6 + arg_count * 2;

	return true;
}

/* Visit an ROP_THISCALL instruction. */
static inline bool
rt_visit_thiscall_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	struct rt_value callee;
	struct rt_value arg[RT_ARG_MAX];
	struct rt_value ret;
	struct rt_value *this_obj;
	int arg_tmpvar;
	int arg_count;
	int dst_tmpvar;
	int obj_tmpvar;
	const char *name;
	int len, i;

	assert(func->bytecode[*pc] == ROP_THISCALL);

	if (*pc + 1 + 2 + 2 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	dst_tmpvar = (func->bytecode[*pc + 1] << 8) |
		      func->bytecode[*pc + 2];
	if (dst_tmpvar >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	obj_tmpvar = (func->bytecode[*pc + 3] << 8) |
		      func->bytecode[*pc + 4];
	if (obj_tmpvar >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}
	this_obj = &rt->frame->tmpvar[obj_tmpvar];

	name = (const char *)&func->bytecode[*pc + 5];
	len = strlen(name);
	if (*pc + 1 + 2 + 2 + len + 1 + 1 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (!rt_get_dict_elem(rt, &rt->frame->tmpvar[obj_tmpvar], name, &callee))
		return false;
	if (callee.type != RT_VALUE_FUNC) {
		rt_error(rt, "Not a function.");
		return false;
	}

	arg_count = func->bytecode[*pc + 1 + 2 + 2 + len + 1];
	for (i = 0; i < arg_count; i++) {
		arg_tmpvar = (func->bytecode[*pc + 1 + 2 + 2 + len + 1 + 1 + i * 2] << 8 ) | 
			     func->bytecode[*pc + 1 + 2 + 2 + len + 1 + 1 + i * 2 + 1];
		arg[i] = rt->frame->tmpvar[arg_tmpvar];
	}

	if (!rt_enter_frame(rt, callee.val.func))
		return false;

	/* Call. */
	if (!rt_call(rt, callee.val.func, this_obj, arg_count, arg, &ret))
		return false;

	rt_leave_frame(rt);

	rt->frame->tmpvar[dst_tmpvar] = ret;

	*pc += 1 + 2 + 2 + len + 1 + 1 + arg_count * 2;

	return true;
}

/* Visit an ROP_JMP instruction. */
static inline bool
rt_visit_jmp_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t target;

	assert(func->bytecode[*pc] == ROP_JMP);

	if (*pc + 1 + 4 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	target = (func->bytecode[*pc + 1] << 24) |
		(func->bytecode[*pc + 2] << 16) |
		(func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];
	if (target >= func->bytecode_size + 1) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	*pc = target;

	return true;
}

/* Visit an ROP_JMPIFTRUE instruction. */
static inline bool
rt_visit_jmpiftrue_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t target;
	uint32_t src;

	assert(func->bytecode[*pc] == ROP_JMPIFTRUE);

	if (*pc + 1 + 2 + 4 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	src = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (src >= func->tmpvar_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	target = (func->bytecode[*pc + 3] << 24) |
		(func->bytecode[*pc + 4] << 16) |
		(func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (target >= func->bytecode_size + 1) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (rt->frame->tmpvar[src].type != RT_VALUE_INT) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (rt->frame->tmpvar[src].val.i == 1)
		*pc = target;
	else
		*pc += 1 + 2 + 4;

	return true;
}

/* Visit an ROP_JMPIFFALSE instruction. */
static inline bool
rt_visit_jmpiffalse_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	uint32_t target;
	uint32_t src;

	assert(func->bytecode[*pc] == ROP_JMPIFFALSE);

	if (*pc + 1 + 2 + 4 > func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	src = (func->bytecode[*pc + 1] << 8) |
		func->bytecode[*pc + 2];
	if (src >= func->tmpvar_size + 1) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	target = (func->bytecode[*pc + 3] << 24) |
		(func->bytecode[*pc + 4] << 16) |
		(func->bytecode[*pc + 5] << 8) |
		func->bytecode[*pc + 6];
	if (target >= func->bytecode_size) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (rt->frame->tmpvar[src].type != RT_VALUE_INT) {
		rt_error(rt, BROKEN_BYTECODE);
		return false;
	}

	if (rt->frame->tmpvar[src].val.i == 0)
		*pc = target;
	else
		*pc += 1 + 2 + 4;

	return true;
}

/* Visit an ROP_LINEINFO instruction. */
static inline bool
rt_visit_lineinfo_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	int line;

	assert(func->bytecode[*pc] == ROP_LINEINFO);

	line = (func->bytecode[*pc + 1] << 24) |
	       (func->bytecode[*pc + 2] << 16) |
	       (func->bytecode[*pc + 3] << 8) |
		func->bytecode[*pc + 4];

	rt->line = line;

	*pc += 5;

	return true;
}

/* Visit an instruction. */
static bool
rt_visit_op(
	struct rt_env *rt,
	struct rt_func *func,
	int *pc)
{
	switch (func->bytecode[*pc]) {
	case ROP_NOP:
		/* NOP */
		(*pc)++;
		break;
	case ROP_ASSIGN:
		if (!rt_visit_assign_op(rt, func, pc))
			return false;
		break;
	case ROP_ICONST:
		if (!rt_visit_iconst_op(rt, func, pc))
			return false;
		break;
	case ROP_FCONST:
		if (!rt_visit_fconst_op(rt, func, pc))
			return false;
		break;
	case ROP_SCONST:
		if (!rt_visit_sconst_op(rt, func, pc))
			return false;
		break;
	case ROP_ACONST:
		if (!rt_visit_aconst_op(rt, func, pc))
			return false;
		break;
	case ROP_DCONST:
		if (!rt_visit_dconst_op(rt, func, pc))
			return false;
		break;
	case ROP_INC:
		if (!rt_visit_inc_op(rt, func, pc))
			return false;
		break;
	case ROP_ADD:
		if (!rt_visit_add_op(rt, func, pc))
			return false;
		break;
	case ROP_SUB:
		if (!rt_visit_sub_op(rt, func, pc))
			return false;
		break;
	case ROP_MUL:
		if (!rt_visit_mul_op(rt, func, pc))
			return false;
		break;
	case ROP_DIV:
		if (!rt_visit_div_op(rt, func, pc))
			return false;
		break;
	case ROP_MOD:
		if (!rt_visit_mod_op(rt, func, pc))
			return false;
		break;
	case ROP_AND:
		if (!rt_visit_and_op(rt, func, pc))
			return false;
		break;
	case ROP_OR:
		if (!rt_visit_or_op(rt, func, pc))
			return false;
		break;
	case ROP_XOR:
		if (!rt_visit_xor_op(rt, func, pc))
			return false;
		break;
	case ROP_NEG:
		if (!rt_visit_neg_op(rt, func, pc))
			return false;
		break;
	case ROP_LT:
		if (!rt_visit_lt_op(rt, func, pc))
			return false;
		break;
	case ROP_LTE:
		if (!rt_visit_lte_op(rt, func, pc))
			return false;
		break;
	case ROP_GT:
		if (!rt_visit_gt_op(rt, func, pc))
			return false;
		break;
	case ROP_GTE:
		if (!rt_visit_gte_op(rt, func, pc))
			return false;
		break;
	case ROP_EQ:
		if (!rt_visit_eq_op(rt, func, pc))
			return false;
		break;
	case ROP_NEQ:
		if (!rt_visit_neq_op(rt, func, pc))
			return false;
		break;
	case ROP_STOREARRAY:
		if (!rt_visit_storearray_op(rt, func, pc))
			return false;
		break;
	case ROP_LOADARRAY:
		if (!rt_visit_loadarray_op(rt, func, pc))
			return false;
		break;
	case ROP_LEN:
		if (!rt_visit_len_op(rt, func, pc))
			return false;
		break;
	case ROP_GETDICTKEYBYINDEX:
		if (!rt_visit_getdictkeybyindex_op(rt, func, pc))
			return false;
		break;
	case ROP_GETDICTVALBYINDEX:
		if (!rt_visit_getdictvalbyindex_op(rt, func, pc))
			return false;
		break;
	case ROP_LOADSYMBOL:
		if (!rt_visit_loadsymbol_op(rt, func, pc))
			return false;
		break;
	case ROP_STORESYMBOL:
		if (!rt_visit_storesymbol_op(rt, func, pc))
			return false;
		break;
	case ROP_LOADDOT:
		if (!rt_visit_loaddot_op(rt, func, pc))
			return false;
		break;
	case ROP_STOREDOT:
		if (!rt_visit_storedot_op(rt, func, pc))
			return false;
		break;
	case ROP_CALL:
		if (!rt_visit_call_op(rt, func, pc))
			return false;
		break;
	case ROP_THISCALL:
		if (!rt_visit_thiscall_op(rt, func, pc))
			return false;
		break;
	case ROP_JMP:
		if (!rt_visit_jmp_op(rt, func, pc))
			return false;
		break;
	case ROP_JMPIFTRUE:
		if (!rt_visit_jmpiftrue_op(rt, func, pc))
			return false;
		break;
	case ROP_JMPIFFALSE:
		if (!rt_visit_jmpiffalse_op(rt, func, pc))
			return false;
		break;
	case ROP_LINEINFO:
		if (!rt_visit_lineinfo_op(rt, func, pc))
			return false;
		break;
	default:
		rt_error(rt, "Unknow opcode.");
		return false;
	}

	return true;
}

/*
 * Intrinsics
 */

static bool rt_intrin_len(struct rt_env *rt);
static bool rt_intrin_push(struct rt_env *rt);
static bool rt_intrin_unset(struct rt_env *rt);
static bool rt_intrin_resize(struct rt_env *rt);

static bool
rt_register_intrinsics(
	struct rt_env *rt)
{
	struct item {
		const char *name;
		int param_count;
		const char *param[HIR_PARAM_SIZE];
		bool (*cfunc)(struct rt_env *rt);
	} items[] = {
		{"len", 1, {"val"}, rt_intrin_len},
		{"push", 2, {"arr", "val"}, rt_intrin_push},
		{"unset", 2, {"dict", "key"}, rt_intrin_unset},
		{"resize", 2, {"arr", "size"}, rt_intrin_resize},
	};
	int i;

	for (i = 0; i < sizeof(items) / sizeof(struct item); i++) {
		if (!rt_register_cfunc(rt,
				       items[i].name,
				       items[i].param_count,
				       items[i].param,
				       items[i].cfunc))
			return false;
	}

	return true;
}

/* len() */
static bool
rt_intrin_len(
	struct rt_env *rt)
{
	struct rt_value val, ret;

	if (!rt_get_local(rt, "val", &val))
		return false;

	switch (val.type) {
	case RT_VALUE_INT:
	case RT_VALUE_FLOAT:
	case RT_VALUE_FUNC:
		ret.type = RT_VALUE_INT;
		ret.val.i = 0;
		break;
	case RT_VALUE_STRING:
		ret.type = RT_VALUE_INT;
		ret.val.i = strlen(val.val.str->s);
		break;
	case RT_VALUE_ARRAY:
		ret.type = RT_VALUE_INT;
		ret.val.i = val.val.arr->size;
		break;
	case RT_VALUE_DICT:
		ret.type = RT_VALUE_INT;
		ret.val.i = val.val.dict->size;
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	if (!rt_set_local(rt, "$return", &ret))
		return false;

	return true;
}

/* push() */
static bool
rt_intrin_push(
	struct rt_env *rt)
{
	struct rt_value arr, val;

	if (!rt_get_local(rt, "arr", &arr))
		return false;
	if (!rt_get_local(rt, "val", &val))
		return false;

	switch (arr.type) {
	case RT_VALUE_INT:
	case RT_VALUE_FLOAT:
	case RT_VALUE_FUNC:
	case RT_VALUE_STRING:
	case RT_VALUE_DICT:
		rt_error(rt, "Not an array.");
		break;
	case RT_VALUE_ARRAY:
		if (!rt_set_array_elem(rt, &arr, arr.val.arr->size, &val))
			return false;
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	if (!rt_set_local(rt, "$return", &arr))
		return false;

	return true;
}

/* unset() */
static bool
rt_intrin_unset(
	struct rt_env *rt)
{
	struct rt_value arr, val;

	if (!rt_get_local(rt, "dict", &arr))
		return false;
	if (!rt_get_local(rt, "key", &val))
		return false;

	switch (arr.type) {
	case RT_VALUE_INT:
	case RT_VALUE_FLOAT:
	case RT_VALUE_FUNC:
	case RT_VALUE_STRING:
	case RT_VALUE_DICT:
		rt_error(rt, "Not a dictionary.");
		break;
	case RT_VALUE_ARRAY:
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	if (val.type != RT_VALUE_STRING) {
		rt_error(rt, "Key not a string.");
		return false;
	}

	if (!rt_remove_dict_elem(rt, &arr, val.val.str->s))
		return false;

	return true;
}

/* resize() */
static bool
rt_intrin_resize(
	struct rt_env *rt)
{
	struct rt_value arr, size;

	if (!rt_get_local(rt, "arr", &arr))
		return false;
	if (!rt_get_local(rt, "size", &size))
		return false;

	switch (arr.type) {
	case RT_VALUE_INT:
	case RT_VALUE_FLOAT:
	case RT_VALUE_FUNC:
	case RT_VALUE_STRING:
	case RT_VALUE_DICT:
		rt_error(rt, "Not an array.");
		break;
	case RT_VALUE_ARRAY:
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	if (size.type != RT_VALUE_INT) {
		rt_error(rt, "Size not an integer.");
		return false;
	}

	if (!rt_resize_array(rt, &arr, size.val.i))
		return false;

	return true;
}

/*
 * Error Handling
 */

/* Output an error message.*/
static void
rt_error(
	struct rt_env *rt,
	const char *msg,
	...)
{
	va_list ap;

	va_start(ap, msg);
	vsnprintf(rt->error_message, sizeof(rt->error_message), msg, ap);
	va_end(ap);
}

/* Output an out-of-memory message. */
static void
rt_out_of_memory(
	struct rt_env *rt)
{
	rt_error(rt, "Out of memory.");
}

