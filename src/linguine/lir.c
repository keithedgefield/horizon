/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * LIR: Low-level Intermediate Representation Generator
 */

#include "lir.h"
#include "hir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define NEVER_COME_HERE		0

/*
 * Target LIR.
 */

#define BYTECODE_BUF_SIZE	65536

/* Bytecode array. */
static uint8_t bytecode[BYTECODE_BUF_SIZE];

/* Cuurent bytecode length. */
static int bytecode_top;

/*
 * Variable table.
 */

#define TMPVAR_MAX	1024

static int tmpvar_top;
static int tmpvar_count;

/*
 * Location table.
 */

#define LOC_MAX	1024

struct loc_entry {
	/* Location offset. */
	uint32_t offset;

	/* Branch target. */
	struct hir_block *block;
};

static struct loc_entry loc_tbl[LOC_MAX];
static int loc_count;

/*
 * Error position and message.
 */

static char *lir_file_name;
static int lir_error_line;
static char lir_error_message[65536];

/*
 * Forward declaration.
 */
static bool lir_visit_block(struct hir_block *block);
static bool lir_visit_basic_block(struct hir_block *block);
static bool lir_visit_if_block(struct hir_block *block);
static bool lir_visit_for_block(struct hir_block *block);
static bool lir_visit_for_range_block(struct hir_block *block);
static bool lir_visit_for_kv_block(struct hir_block *block);
static bool lir_visit_for_v_block(struct hir_block *block);
static bool lir_visit_while_block(struct hir_block *block);
static bool lir_visit_stmt(struct hir_stmt *stmt);
static bool lir_visit_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_unary_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_binary_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_dot_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_call_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_thiscall_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_array_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_dict_expr(int dst_tmpvar, struct hir_expr *expr);
static bool lir_visit_term(int dst_tmpvar, struct hir_term *term);
static bool lir_visit_symbol_term(int dst_tmpvar, struct hir_term *term);
static bool lir_visit_int_term(int dst_tmpvar, struct hir_term *term);
static bool lir_visit_float_term(int dst_tmpvar, struct hir_term *term);
static bool lir_visit_string_term(int dst_tmpvar, struct hir_term *term);
static bool lir_visit_empty_array_term(int dst_tmpvar, struct hir_term *term);
static bool lir_visit_empty_dict_term(int dst_tmpvar, struct hir_term *term);
static bool lir_increment_tmpvar(int *tmpvar_index);
static bool lir_decrement_tmpvar(int tmpvar_index);
static bool lir_put_opcode(uint8_t op);
static bool lir_put_tmpvar(uint16_t index);
static bool lir_put_imm8(uint8_t imm);
static bool lir_put_imm32(uint32_t imm);
static bool lir_put_string(const char *data);
static bool lir_put_branch_addr(struct hir_block *block);
static bool lir_put_u8(uint8_t b);
static bool lir_put_u16(uint16_t b);
static bool lir_put_u32(uint32_t b);
static void patch_block_address(void);
static void lir_fatal(const char *msg, ...);
static void lir_out_of_memory(void);

/*
 * Build
 */

bool
lir_build(
	struct hir_block *hir_func,
	struct lir_func **lir_func)
{
	struct hir_block *cur_block;
	int i;

	assert(hir_func != NULL);
	assert(hir_func->type == HIR_BLOCK_FUNC);

	/* Copy the file name. */
	lir_file_name = strdup(hir_func->val.func.file_name);
	if (lir_file_name == NULL) {
		lir_out_of_memory();
		return false;
	}

	/* Initialize the bytecode buffer. */
	bytecode_top = 0;
	memset(bytecode, 0, BYTECODE_BUF_SIZE);

	/* Initialize the tmpvars. */
	tmpvar_top = hir_func->val.func.param_count;
	tmpvar_count = tmpvar_top;

	/* Visit blocks. */
	cur_block = hir_func->val.func.inner;
	while (cur_block != NULL) {
		/* Visit a block. */
		lir_visit_block(cur_block);

		/* Move to a next. */
		cur_block = cur_block->succ;
		if (cur_block->type == HIR_BLOCK_END) {
			cur_block->addr = bytecode_top;
			break;
		}
	}

	/* Patch block address. */
	patch_block_address();

	/* Make an lir_func. */
	*lir_func = malloc(sizeof(struct lir_func));
	if (lir_func == NULL) {
		lir_out_of_memory();
		return false;
	}

	/* Copy the function name. */
	(*lir_func)->func_name = strdup(hir_func->val.func.name);
	if ((*lir_func)->func_name == NULL) {
		lir_out_of_memory();
		return false;
	}

	/* Copy the parameter names.  */
	(*lir_func)->param_count = hir_func->val.func.param_count;
	for (i = 0; i < hir_func->val.func.param_count; i++) {
		(*lir_func)->param_name[i] = strdup(hir_func->val.func.param_name[i]);
		if ((*lir_func)->param_name[i] == NULL) {
			lir_out_of_memory();
			return false;
		}
	}

	/* Copy the bytecode. */
	(*lir_func)->bytecode = malloc(bytecode_top);
	if ((*lir_func)->bytecode == NULL) {
		lir_out_of_memory();
		return false;
	}
	(*lir_func)->bytecode_size = bytecode_top;
	memcpy((*lir_func)->bytecode, bytecode, bytecode_top);

	/* Copy the file name. */
	(*lir_func)->file_name = strdup(hir_func->val.func.file_name);
	if ((*lir_func)->file_name == NULL) {
		lir_out_of_memory();
		return false;
	}

	(*lir_func)->tmpvar_size = tmpvar_count;

	return true;
}

static bool
lir_visit_block(
	struct hir_block *block)
{
	assert(block != NULL);

	switch (block->type) {
	case HIR_BLOCK_BASIC:
		if (!lir_visit_basic_block(block))
			return false;
		break;
	case HIR_BLOCK_IF:
		if (!lir_visit_if_block(block))
			return false;
		break;
	case HIR_BLOCK_FOR:
		if (!lir_visit_for_block(block))
			return false;
		break;
	case HIR_BLOCK_WHILE:
		if (!lir_visit_while_block(block))
			return false;
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	return true;
}

static bool
lir_visit_basic_block(
	struct hir_block *block)
{
	struct hir_stmt *stmt;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_BASIC);

	/* Store the block address. */
	block->addr = bytecode_top;

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(block->line))
		return false;

	/* Visit statements. */
	stmt = block->val.basic.stmt_list;
	while (stmt != NULL) {
		/* Visit a statement. */
		if (!lir_visit_stmt(stmt))
			return false;
		stmt = stmt->next;
	}

	return true;
}

static bool
lir_visit_if_block(
	struct hir_block *block)
{
	struct hir_stmt *stmt;
	int cond_tmpvar;
	bool is_else;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_IF);

	/* Store the block address. */
	block->addr = bytecode_top;

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(block->line))
		return false;

	/* Is an else-block? */
	if (block->val.if_.cond == NULL)
		is_else = true;
	else
		is_else = false;

	/* If this is not an else-block. */
	if (!is_else) {
		/* Skip this block if the condition is not met. */
		if (!lir_increment_tmpvar(&cond_tmpvar))
			return false;
		if (!lir_visit_expr(cond_tmpvar, block->val.if_.cond))
			return false;
		if (!lir_put_opcode(LOP_JMPIFFALSE))
			return false;
		if (!lir_put_tmpvar(cond_tmpvar))
			return false;
		if (block->val.if_.chain != NULL) {
			/* Jump to a chaining else-block. */
			if (!lir_put_branch_addr(block->val.if_.chain))
				return false;
		} else {
			/* Jump to a first non-if block. */
			if (!lir_put_branch_addr(block->succ))
				return false;
		}

	}

	/* Visit an inner block if exists. */
	if (block->val.if_.inner != NULL) {
		if (!lir_visit_block(block->val.if_.inner))
			return false;
	}

	/* Visit a chaining block if exists. */
	if (block->val.if_.chain != NULL) {
		if (!lir_visit_block(block->val.if_.chain))
			return false;
	}

	/* If this is an if-block or an else-if block. */
	if (!is_else) {
		/* Jump to a first non-if block. */
		if (!lir_put_opcode(LOP_JMP))
			return false;
		if (!lir_put_branch_addr(block->succ))
			return false;
	}

	return true;
}

static bool
lir_visit_for_block(
	struct hir_block *block)
{
	struct hir_stmt *stmt;
	int cond_tmpvar;
	bool is_else;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_FOR);

	/* Dispatch by type. */
	if (block->val.for_.is_ranged) {
		/* This is a ranged-for loop. */
		if (!lir_visit_for_range_block(block))
			return false;
	} else if (block->val.for_.key_symbol != NULL) {
		/* This is a for-each-key-and-value loop. */
		if (!lir_visit_for_kv_block(block))
			return false;
	} else {
		/* This is a for-each-value loop. */
		if (!lir_visit_for_v_block(block))
			return false;
	}

	return true;
}

static bool
lir_visit_for_range_block(
	struct hir_block *block)
{
	uint32_t loop_addr;
	int start_tmpvar, stop_tmpvar, loop_tmpvar, cmp_tmpvar;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_FOR);
	assert(block->val.for_.is_ranged);
	assert(block->val.for_.counter_symbol);
	assert(block->val.for_.start);
	assert(block->val.for_.stop);

	/* Store the block address. */
	block->addr = bytecode_top;

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(block->line))
		return false;

	/* Visit the start expr. */
	if (!lir_increment_tmpvar(&start_tmpvar))
		return false;
	if (!lir_visit_expr(start_tmpvar, block->val.for_.start))
		return false;

	/* Visit the stop expr. */
	if (!lir_increment_tmpvar(&stop_tmpvar))
		return false;
	if (!lir_visit_expr(stop_tmpvar, block->val.for_.stop))
		return false;

	/* Put the start value to a loop variable. */
	if (!lir_increment_tmpvar(&loop_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_ASSIGN))
		return false;
	if (!lir_put_tmpvar(loop_tmpvar))
		return false;
	if (!lir_put_tmpvar(start_tmpvar))
		return false;

	/* Put a loop header. */
	loop_addr = bytecode_top;
	if (!lir_increment_tmpvar(&cmp_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_EQ))
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_tmpvar(loop_tmpvar))
		return false;
	if (!lir_put_tmpvar(stop_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_JMPIFTRUE))
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_branch_addr(block->succ))
		return false;
	lir_decrement_tmpvar(cmp_tmpvar);

	/* Store a loop variable. */
	if (!lir_put_opcode(LOP_STORESYMBOL))
		return false;
	if (!lir_put_string(block->val.for_.counter_symbol))
		return false;
	if (!lir_put_tmpvar(loop_tmpvar))
		return false;

	/* Visit an inner block. */
	if (block->val.for_.inner != NULL) {
		if (!lir_visit_block(block->val.for_.inner))
			return false;
	}

	/* Increment the loop variable. */
	if (!lir_put_opcode(LOP_INC))
		return false;
	if (!lir_put_tmpvar(loop_tmpvar))
		return false;

	/* Put a back-edge jump. */
	if (!lir_put_opcode(LOP_JMP))
		return false;
	if (!lir_put_imm32(loop_addr))
		return false;

	lir_decrement_tmpvar(loop_tmpvar);
	lir_decrement_tmpvar(stop_tmpvar);
	lir_decrement_tmpvar(start_tmpvar);

	return true;
}

static bool
lir_visit_for_kv_block(
	struct hir_block *block)
{
	struct hir_stmt *stmt;
	uint32_t loop_addr;
	int col_tmpvar, size_tmpvar, i_tmpvar, key_tmpvar, val_tmpvar, cmp_tmpvar;
	bool is_else;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_FOR);
	assert(!block->val.for_.is_ranged);
	assert(block->val.for_.key_symbol != NULL);
	assert(block->val.for_.value_symbol != NULL);
	assert(block->val.for_.collection != NULL);

	/* Store the block address. */
	block->addr = bytecode_top;

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(block->line))
		return false;

	/* Visit a collection expr. */
	if (!lir_increment_tmpvar(&col_tmpvar))
		return false;
	if (!lir_visit_expr(col_tmpvar, block->val.for_.collection))
		return false;

	/* Get a collection size. */
	if (!lir_increment_tmpvar(&size_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_LEN))
		return false;
	if (!lir_put_tmpvar(size_tmpvar))
		return false;
	if (!lir_put_tmpvar(col_tmpvar))
		return false;

	/* Assign 0 to `i`. */
	if (!lir_increment_tmpvar(&i_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_ICONST))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_imm32(0))
		return false;

	/* Prepare a key and a value. */
	if (!lir_increment_tmpvar(&key_tmpvar))
		return false;
	if (!lir_increment_tmpvar(&val_tmpvar))
		return false;
	if (!lir_increment_tmpvar(&cmp_tmpvar))
		return false;

	/* Put a loop header. */
	loop_addr = bytecode_top;		/* LOOP: */
	if (!lir_put_opcode(LOP_GTE)) 		/*  if i >= size then break */
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_tmpvar(size_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_JMPIFTRUE)) 		/*  if i >= size then break */
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_branch_addr(block->succ))
		return false;
	if (!lir_put_opcode(LOP_GETDICTKEYBYINDEX))	/* key = dict.getKeyByIndex(i) */
		return false;
	if (!lir_put_tmpvar(key_tmpvar))
		return false;
	if (!lir_put_tmpvar(col_tmpvar))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_GETDICTVALBYINDEX)) 	/* val = dict.getValByIndex(i) */
		return false;
	if (!lir_put_tmpvar(val_tmpvar))
		return false;
	if (!lir_put_tmpvar(col_tmpvar))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_INC)) 		/* i++ */
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;

	/* Bind the key and value variables to local variables. */
	if (!lir_put_opcode(LOP_STORESYMBOL))
		return false;
	if (!lir_put_string(block->val.for_.key_symbol))
		return false;
	if (!lir_put_tmpvar(key_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_STORESYMBOL))
		return false;
	if (!lir_put_string(block->val.for_.value_symbol))
		return false;
	if (!lir_put_tmpvar(val_tmpvar))
		return false;

	/* Visit an inner block. */
	if (block->val.for_.inner != NULL) {
		if (!lir_visit_block(block->val.for_.inner))
			return false;
	}

	/* Put a back-edge jump. */
	if (!lir_put_opcode(LOP_JMP))
		return false;
	if (!lir_put_imm32(loop_addr))
		return false;

	lir_decrement_tmpvar(cmp_tmpvar);
	lir_decrement_tmpvar(val_tmpvar);
	lir_decrement_tmpvar(key_tmpvar);
	lir_decrement_tmpvar(i_tmpvar);
	lir_decrement_tmpvar(size_tmpvar);
	lir_decrement_tmpvar(col_tmpvar);

	return true;
}

static bool
lir_visit_for_v_block(
	struct hir_block *block)
{
	struct hir_stmt *stmt;
	uint32_t loop_addr;
	int arr_tmpvar, size_tmpvar, i_tmpvar, val_tmpvar, cmp_tmpvar;
	bool is_else;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_FOR);
	assert(!block->val.for_.is_ranged);
	assert(block->val.for_.value_symbol != NULL);
	assert(block->val.for_.collection != NULL);

	/* Store the block address. */
	block->addr = bytecode_top;

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(block->line))
		return false;

	/* Visit an array expr. */
	if (!lir_increment_tmpvar(&arr_tmpvar))
		return false;
	if (!lir_visit_expr(arr_tmpvar, block->val.for_.collection))
		return false;

	/* Get a collection size. */
	if (!lir_increment_tmpvar(&size_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_LEN))
		return false;
	if (!lir_put_tmpvar(size_tmpvar))
		return false;
	if (!lir_put_tmpvar(arr_tmpvar))
		return false;

	/* Assign 0 to `i`. */
	if (!lir_increment_tmpvar(&i_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_ICONST))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_imm32(0))
		return false;

	/* Prepare a value. */
	if (!lir_increment_tmpvar(&val_tmpvar))
		return false;
	if (!lir_increment_tmpvar(&cmp_tmpvar))
		return false;

	/* Put a loop header. */
	loop_addr = bytecode_top;		/* LOOP: */
	if (!lir_put_opcode(LOP_GTE)) 		/*  if i >= size then break */
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_tmpvar(size_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_JMPIFTRUE))
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_branch_addr(block->succ))
		return false;
	if (!lir_put_opcode(LOP_LOADARRAY)) 	/* val = array[i] */
		return false;
	if (!lir_put_tmpvar(val_tmpvar))
		return false;
	if (!lir_put_tmpvar(arr_tmpvar))
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;
	if (!lir_put_opcode(LOP_INC)) 		/* i++ */
		return false;
	if (!lir_put_tmpvar(i_tmpvar))
		return false;

	/* Bind the value variable to a local variable. */
	if (!lir_put_opcode(LOP_STORESYMBOL))
		return false;
	if (!lir_put_string(block->val.for_.value_symbol))
		return false;
	if (!lir_put_tmpvar(val_tmpvar))
		return false;

	/* Visit an inner block. */
	if (block->val.for_.inner != NULL) {
		if (!lir_visit_block(block->val.for_.inner))
			return false;
	}

	/* Put a back-edge jump. */
	if (!lir_put_opcode(LOP_JMP))
		return false;
	if (!lir_put_imm32(loop_addr))
		return false;

	lir_decrement_tmpvar(cmp_tmpvar);
	lir_decrement_tmpvar(val_tmpvar);
	lir_decrement_tmpvar(i_tmpvar);
	lir_decrement_tmpvar(size_tmpvar);
	lir_decrement_tmpvar(arr_tmpvar);

	return true;
}

static bool
lir_visit_while_block(
	struct hir_block *block)
{
	uint32_t loop_addr;
	int cmp_tmpvar;

	assert(block != NULL);
	assert(block->type == HIR_BLOCK_WHILE);

	/* Store the block address. */
	block->addr = bytecode_top;

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(block->line))
		return false;

	/* Put a loop header. */
	loop_addr = bytecode_top;
	if (!lir_increment_tmpvar(&cmp_tmpvar))
		return false;
	if (!lir_visit_expr(cmp_tmpvar, block->val.while_.cond))
		return false;
	if (!lir_put_opcode(LOP_JMPIFFALSE))
		return false;
	if (!lir_put_tmpvar(cmp_tmpvar))
		return false;
	if (!lir_put_branch_addr(block->succ))
		return false;
	lir_decrement_tmpvar(cmp_tmpvar);

	/* Visit an inner block. */
	if (block->val.for_.inner != NULL) {
		if (!lir_visit_block(block->val.for_.inner))
			return false;
	}

	/* Put a back-edge jump. */
	if (!lir_put_opcode(LOP_JMP))
		return false;
	if (!lir_put_imm32(loop_addr))
		return false;

	return true;
}

static bool
lir_visit_stmt(
	struct hir_stmt *stmt)
{
	int rhs_tmpvar, obj_tmpvar, access_tmpvar;

	assert(stmt != NULL);
	assert(stmt->rhs != NULL);

	/* Put a line number. */
	if (!lir_put_opcode(LOP_LINEINFO))
		return false;
	if (!lir_put_imm32(stmt->line))
		return false;

	/* Visit RHS. */
	if (!lir_increment_tmpvar(&rhs_tmpvar))
		return false;
	if (!lir_visit_expr(rhs_tmpvar, stmt->rhs))
		return false;

	/* Visit LHS. */
	if (stmt->lhs != NULL) {
		if (stmt->lhs->type == HIR_EXPR_TERM) {
			assert(stmt->lhs->val.term.term->type == HIR_TERM_SYMBOL);

			/* Put a storesymbol. */
			if (!lir_put_opcode(LOP_STORESYMBOL))
				return false;
			if (!lir_put_string(stmt->lhs->val.term.term->val.symbol))
				return false;
			if (!lir_put_tmpvar(rhs_tmpvar))
				return false;
		} else if (stmt->lhs->type == HIR_EXPR_SUBSCR) {
			assert(stmt->lhs->val.binary.expr[0] != NULL);
			assert(stmt->lhs->val.binary.expr[1] != NULL);

			/* Visit an array. */
			if (!lir_increment_tmpvar(&obj_tmpvar))
				return false;
			if (!lir_visit_expr(obj_tmpvar, stmt->lhs->val.binary.expr[0]))
				return false;

			/* Visit a subscript. */
			if (!lir_increment_tmpvar(&access_tmpvar))
				return false;
			if (!lir_visit_expr(access_tmpvar, stmt->lhs->val.binary.expr[1]))
				return false;

			/* Put a store. */
			if (!lir_put_opcode(LOP_STOREARRAY))
				return false;
			if (!lir_put_tmpvar(obj_tmpvar))
				return false;
			if (!lir_put_tmpvar(access_tmpvar))
				return false;
			if (!lir_put_tmpvar(rhs_tmpvar))
				return false;

			lir_decrement_tmpvar(access_tmpvar);
			lir_decrement_tmpvar(obj_tmpvar);
		} else if (stmt->lhs->type == HIR_EXPR_DOT) {
			assert(stmt->lhs->val.dot.obj != NULL);
			assert(stmt->lhs->val.dot.symbol != NULL);

			/* Visit an object. */
			if (!lir_increment_tmpvar(&obj_tmpvar))
				return false;
			if (!lir_visit_expr(obj_tmpvar, stmt->lhs->val.dot.obj))
				return false;

			/* Put a store. */
			if (!lir_put_opcode(LOP_STOREDOT))
				return false;
			if (!lir_put_tmpvar(obj_tmpvar))
				return false;
			if (!lir_put_string(stmt->lhs->val.dot.symbol))
				return false;
			if (!lir_put_tmpvar(rhs_tmpvar))
				return false;

			lir_decrement_tmpvar(obj_tmpvar);
		} else {
			lir_fatal("LHS is not a symbol or an array element.");
			return false;
		}
	}

	lir_decrement_tmpvar(rhs_tmpvar);

	return true;
}

static bool
lir_visit_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int opr1_tmpvar, opr2_tmpvar;

	assert(expr != NULL);

	switch (expr->type) {
	case HIR_EXPR_TERM:
		/* Visit a term inside the expr. */
		if (!lir_visit_term(dst_tmpvar, expr->val.term.term))
			return false;
		break;
	case HIR_EXPR_PAR:
		/* Visit an expr inside the expr. */
		if (!lir_visit_expr(dst_tmpvar, expr->val.unary.expr))
			return false;
		break;
	case HIR_EXPR_NEG:
		/* For the unary operator. (Currently NEG only) */
		if (!lir_visit_unary_expr(dst_tmpvar, expr))
			return false;
		break;
	case HIR_EXPR_LT:
	case HIR_EXPR_LTE:
	case HIR_EXPR_GT:
	case HIR_EXPR_GTE:
	case HIR_EXPR_EQ:
	case HIR_EXPR_NEQ:
	case HIR_EXPR_PLUS:
	case HIR_EXPR_MINUS:
	case HIR_EXPR_MUL:
	case HIR_EXPR_DIV:
	case HIR_EXPR_MOD:
	case HIR_EXPR_AND:
	case HIR_EXPR_OR:
	case HIR_EXPR_SUBSCR:
		/* For the binary operators. */
		if (!lir_visit_binary_expr(dst_tmpvar, expr))
			return false;
		break;
	case HIR_EXPR_DOT:
		/* For the dot operator. */
		if (!lir_visit_dot_expr(dst_tmpvar, expr))
			return false;
		break;
	case HIR_EXPR_CALL:
		/* For a function call. */
		if (!lir_visit_call_expr(dst_tmpvar, expr))
			return false;
		break;
	case HIR_EXPR_THISCALL:
		/* For a method call. */
		if (!lir_visit_thiscall_expr(dst_tmpvar, expr))
			return false;
		break;
	case HIR_EXPR_ARRAY:
		/* For an array expression. */
		if (!lir_visit_array_expr(dst_tmpvar, expr))
			return false;
		break;
	case HIR_EXPR_DICT:
		/* For a dictionary expression. */
		if (!lir_visit_dict_expr(dst_tmpvar, expr))
			return false;
		break;
	default:
		assert(NEVER_COME_HERE);
		abort();
		break;
	}

	return true;
}

static bool
lir_visit_unary_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int opr_tmpvar;

	assert(expr != NULL);
	assert(expr->type == HIR_EXPR_NEG);

	/* Visit the operand expr. */
	if (!lir_increment_tmpvar(&opr_tmpvar))
		return false;
	if (!lir_visit_expr(opr_tmpvar, expr->val.unary.expr))
		return false;

	/* Put an opcode. */
	switch (expr->type) {
	case HIR_EXPR_NEG:
		if (!lir_put_opcode(LOP_NEG))
			return false;
		if (!lir_put_tmpvar(dst_tmpvar))
			return false;
		if (!lir_put_tmpvar(opr_tmpvar))
			return false;
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	lir_decrement_tmpvar(opr_tmpvar);

	return true;
}

static bool
lir_visit_binary_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int opr1_tmpvar, opr2_tmpvar;
	int opcode;

	assert(expr != NULL);

	/* Visit the operand1 expr. */
	if (!lir_increment_tmpvar(&opr1_tmpvar))
		return false;
	if (!lir_visit_expr(opr1_tmpvar, expr->val.binary.expr[0]))
		return false;

	/* Visit the operand2 expr. */
	if (!lir_increment_tmpvar(&opr2_tmpvar))
		return false;
	if (!lir_visit_expr(opr2_tmpvar, expr->val.binary.expr[1]))
		return false;

	/* Put an opcode. */
	switch (expr->type) {
	case HIR_EXPR_LT:
		opcode = LOP_LT;
		break;
	case HIR_EXPR_LTE:
		opcode = LOP_LTE;
		break;
	case HIR_EXPR_EQ:
		opcode = LOP_EQ;
		break;
	case HIR_EXPR_NEQ:
		opcode = LOP_NEQ;
		break;
	case HIR_EXPR_GTE:
		opcode = LOP_GTE;
		break;
	case HIR_EXPR_GT:
		opcode = LOP_GT;
		break;
	case HIR_EXPR_PLUS:
		opcode = LOP_ADD;
		break;
	case HIR_EXPR_MINUS:
		opcode = LOP_SUB;
		break;
	case HIR_EXPR_MUL:
		opcode = LOP_MUL;
		break;
	case HIR_EXPR_DIV:
		opcode = LOP_DIV;
		break;
	case HIR_EXPR_MOD:
		opcode = LOP_MOD;
		break;
	case HIR_EXPR_AND:
		opcode = LOP_AND;
		break;
	case HIR_EXPR_OR:
		opcode = LOP_OR;
		break;
	case HIR_EXPR_SUBSCR:
		opcode = LOP_LOADARRAY;
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	if (!lir_put_opcode(opcode))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_tmpvar(opr1_tmpvar))
		return false;
	if (!lir_put_tmpvar(opr2_tmpvar))
		return false;

	lir_decrement_tmpvar(opr2_tmpvar);
	lir_decrement_tmpvar(opr1_tmpvar);

	return true;
}

static bool
lir_visit_dot_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int opr_tmpvar;

	assert(expr != NULL);
	assert(expr->type == HIR_EXPR_DOT);
	assert(expr->val.dot.obj != NULL);
	assert(expr->val.dot.symbol != NULL);

	/* Visit the operand expr. */
	if (!lir_increment_tmpvar(&opr_tmpvar))
		return false;
	if (!lir_visit_expr(opr_tmpvar, expr->val.dot.obj))
		return false;

	/* Put a bytecode sequence. */
	if (!lir_put_opcode(LOP_LOADDOT))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_tmpvar(opr_tmpvar))
		return false;
	if (!lir_put_string(expr->val.dot.symbol))
		return false;

	lir_decrement_tmpvar(opr_tmpvar);

	return true;
}

static bool
lir_visit_call_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int arg_tmpvar[HIR_PARAM_SIZE];
	int arg_count;
	int func_tmpvar;
	int i;

	assert(expr != NULL);
	assert(expr->type == HIR_EXPR_CALL);
	assert(expr->val.call.func != NULL);
	assert(expr->val.call.arg_count >= 0);
	assert(expr->val.call.arg_count < HIR_PARAM_SIZE);

	arg_count = expr->val.call.arg_count;
	
	/* Visit the func expr. */
	if (!lir_increment_tmpvar(&func_tmpvar))
		return false;
	if (!lir_visit_expr(func_tmpvar, expr->val.call.func))
		return false;

	/* Visit the arg exprs. */
	for (i = 0; i < arg_count; i++) {
		if (!lir_increment_tmpvar(&arg_tmpvar[i]))
			return false;
		if (!lir_visit_expr(arg_tmpvar[i], expr->val.call.arg[i]))
			return false;
	}

	/* Put a bytecode sequence. */
	if (!lir_put_opcode(LOP_CALL))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_tmpvar(func_tmpvar))
		return false;
	if (!lir_put_imm8(arg_count))
		return false;
	for (i = 0; i < arg_count; i++) {
		if (!lir_put_tmpvar(arg_tmpvar[i]))
			return false;
	}

	for (i = arg_count - 1; i >= 0; i--)
		lir_decrement_tmpvar(arg_tmpvar[i]);
	lir_decrement_tmpvar(func_tmpvar);

	return true;
}

static bool
lir_visit_thiscall_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int arg_tmpvar[HIR_PARAM_SIZE];
	int arg_count;
	int obj_tmpvar;
	int i;

	assert(expr != NULL);
	assert(expr->type == HIR_EXPR_THISCALL);
	assert(expr->val.thiscall.func != NULL);
	assert(expr->val.thiscall.arg_count >= 0);
	assert(expr->val.thiscall.arg_count < HIR_PARAM_SIZE);

	arg_count = expr->val.thiscall.arg_count;
	
	/* Visit the object expr. */
	if (!lir_increment_tmpvar(&obj_tmpvar))
		return false;
	if (!lir_visit_expr(obj_tmpvar, expr->val.thiscall.obj))
		return false;

	/* Visit the arg exprs. */
	for (i = 0; i < arg_count; i++) {
		if (!lir_increment_tmpvar(&arg_tmpvar[i]))
			return false;
		if (!lir_visit_expr(arg_tmpvar[i], expr->val.thiscall.arg[i]))
			return false;
	}

	/* Put a bytecode sequence. */
	if (!lir_put_opcode(LOP_THISCALL))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_tmpvar(obj_tmpvar))
		return false;
	if (!lir_put_string(expr->val.thiscall.func))
		return false;
	if (!lir_put_imm8(arg_count))
		return false;
	for (i = 0; i < arg_count; i++) {
		if (!lir_put_tmpvar(arg_tmpvar[i]))
			return false;
	}

	for (i = arg_count - 1; i >= 0; i--)
		lir_decrement_tmpvar(arg_tmpvar[i]);
	lir_decrement_tmpvar(obj_tmpvar);

	return true;
}

static bool
lir_visit_array_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int elem_count;
	int elem_tmpvar;
	int index_tmpvar;
	int i;

	assert(expr != NULL);
	assert(expr->type == HIR_EXPR_ARRAY);
	assert(expr->val.array.elem_count > 0);
	assert(expr->val.array.elem_count < HIR_ARRAY_LITERAL_SIZE);

	elem_count = expr->val.array.elem_count;
	
	/* Create an array. */
	if (!lir_put_opcode(LOP_ACONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;

	/* Push the elements. */
	if (!lir_increment_tmpvar(&elem_tmpvar))
		return false;
	if (!lir_increment_tmpvar(&index_tmpvar))
		return false;
	for (i = 0; i < elem_count; i++) {
		/* Visit the element. */
		if (!lir_visit_expr(elem_tmpvar, expr->val.array.elem[i]))
			return false;

		/* Add to the array. */
		if (!lir_put_opcode(LOP_ICONST))
			return false;
		if (!lir_put_tmpvar(index_tmpvar))
			return false;
		if (!lir_put_imm32(i))
			return false;
		if (!lir_put_opcode(LOP_STOREARRAY))
			return false;
		if (!lir_put_tmpvar(dst_tmpvar))
			return false;
		if (!lir_put_tmpvar(index_tmpvar))
			return false;
		if (!lir_put_tmpvar(elem_tmpvar))
			return false;
	}

	lir_decrement_tmpvar(index_tmpvar);
	lir_decrement_tmpvar(elem_tmpvar);

	return true;
}

static bool
lir_visit_dict_expr(
	int dst_tmpvar,
	struct hir_expr *expr)
{
	int kv_count;
	int key_tmpvar;
	int value_tmpvar;
	int index_tmpvar;
	int i;

	assert(expr != NULL);
	assert(expr->type == HIR_EXPR_DICT);
	assert(expr->val.dict.kv_count > 0);
	assert(expr->val.dict.kv_count < HIR_DICT_LITERAL_SIZE);

	kv_count = expr->val.dict.kv_count;
	
	/* Create a dictionary. */
	if (!lir_put_opcode(LOP_DCONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;

	/* Push the elements. */
	if (!lir_increment_tmpvar(&key_tmpvar))
		return false;
	if (!lir_increment_tmpvar(&value_tmpvar))
		return false;
	if (!lir_increment_tmpvar(&index_tmpvar))
		return false;
	for (i = 0; i < kv_count; i++) {
		/* Visit the element. */
		if (!lir_visit_expr(value_tmpvar, expr->val.dict.value[i]))
			return false;

		/* Add to the dict. */
		if (!lir_put_opcode(LOP_SCONST))
			return false;
		if (!lir_put_tmpvar(key_tmpvar))
			return false;
		if (!lir_put_string(expr->val.dict.key[i]))
			return false;
		if (!lir_put_opcode(LOP_STOREARRAY))
			return false;
		if (!lir_put_tmpvar(dst_tmpvar))
			return false;
		if (!lir_put_tmpvar(key_tmpvar))
			return false;
		if (!lir_put_tmpvar(value_tmpvar))
			return false;
	}

	lir_decrement_tmpvar(index_tmpvar);
	lir_decrement_tmpvar(value_tmpvar);
	lir_decrement_tmpvar(key_tmpvar);

	return true;
}

static bool
lir_visit_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	assert(term != NULL);

	switch (term->type) {
	case HIR_TERM_SYMBOL:
		if (!lir_visit_symbol_term(dst_tmpvar, term))
			return false;
		break;
	case HIR_TERM_INT:
		if (!lir_visit_int_term(dst_tmpvar, term))
			return false;
		break;
	case HIR_TERM_FLOAT:
		if (!lir_visit_float_term(dst_tmpvar, term))
			return false;
		break;
	case HIR_TERM_STRING:
		if (!lir_visit_string_term(dst_tmpvar, term))
			return false;
		break;
	case HIR_TERM_EMPTY_ARRAY:
		if (!lir_visit_empty_array_term(dst_tmpvar, term))
			return false;
		break;
	case HIR_TERM_EMPTY_DICT:
		if (!lir_visit_empty_dict_term(dst_tmpvar, term))
			return false;
		break;
	default:
		assert(NEVER_COME_HERE);
		break;
	}

	return true;
}

static bool
lir_visit_symbol_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	assert(term != NULL);
	assert(term->type == HIR_TERM_SYMBOL);

	if (!lir_put_opcode(LOP_LOADSYMBOL))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_string(term->val.symbol))
		return false;

	return true;
}

static bool
lir_visit_int_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	assert(term != NULL);
	assert(term->type == HIR_TERM_INT);

	if (!lir_put_opcode(LOP_ICONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_imm32(term->val.i))
		return false;

	return true;
}

static bool
lir_visit_float_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	uint32_t data;

	assert(term != NULL);
	assert(term->type == HIR_TERM_FLOAT);

	data = *(uint32_t *)&term->val.f;

	if (!lir_put_opcode(LOP_FCONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_imm32(data))
		return false;

	return true;
}

static bool
lir_visit_string_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	assert(term != NULL);
	assert(term->type == HIR_TERM_STRING);

	if (!lir_put_opcode(LOP_SCONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;
	if (!lir_put_string(term->val.s))
		return false;

	return true;
}

static bool
lir_visit_empty_array_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	assert(term != NULL);
	assert(term->type == HIR_TERM_EMPTY_ARRAY);

	if (!lir_put_opcode(LOP_ACONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;

	return true;
}

static bool
lir_visit_empty_dict_term(
	int dst_tmpvar,
	struct hir_term *term)
{
	assert(term != NULL);
	assert(term->type == HIR_TERM_EMPTY_DICT);

	if (!lir_put_opcode(LOP_DCONST))
		return false;
	if (!lir_put_tmpvar(dst_tmpvar))
		return false;

	return true;
}

static bool
lir_increment_tmpvar(
	int *tmpvar_index)
{
	if (tmpvar_top >= TMPVAR_MAX) {
		lir_fatal("Too much local variables.");
		return false;
	}

	*tmpvar_index = tmpvar_top;

	tmpvar_top++;
	if (tmpvar_top > tmpvar_count)
		tmpvar_count = tmpvar_top;

	return true;
}

static bool
lir_decrement_tmpvar(
	int tmpvar_index)
{
	assert(tmpvar_index == tmpvar_top - 1);
	assert(tmpvar_top > 0);

	tmpvar_top--;

	return true;
}

static bool
lir_put_opcode(
	uint8_t opcode)
{
	if (!lir_put_u8(opcode))
		return false;

	return true;
}

static bool
lir_put_tmpvar(
	uint16_t index)
{
	if (!lir_put_u16(index))
		return false;

	return true;
}

static bool
lir_put_imm8(
	uint8_t imm)
{
	if (!lir_put_u8(imm))
		return false;

	return true;
}


static bool
lir_put_imm32(
	uint32_t imm)
{
	if (!lir_put_u32(imm))
		return false;

	return true;
}

static bool lir_put_branch_addr(
	struct hir_block *block)
{
	if (loc_count >= LOC_MAX) {
		lir_fatal("Too many jumps.");
		return false;
	}

	loc_tbl[loc_count].offset = bytecode_top;
	loc_tbl[loc_count].block = block;
	loc_count++;

	bytecode[bytecode_top] = 0xff;
	bytecode[bytecode_top + 1] = 0xff;
	bytecode[bytecode_top + 2] = 0xff;
	bytecode[bytecode_top + 3] = 0xff;
	bytecode_top += 4;

	return true;
}

static bool
lir_put_string(
	const char *s)
{
	int i, len;

	len = strlen(s);
	for (i = 0; i < len; i++) {
		if (!lir_put_u8(*s++))
			return false;
	}
	if (!lir_put_u8('\0'))
		return false;

	return true;
}

static bool
lir_put_u8(
	uint8_t b)
{
	if (bytecode_top + 1 > BYTECODE_BUF_SIZE)
		return false;

	bytecode[bytecode_top] = b;

	bytecode_top++;

	return true;
}

static bool
lir_put_u16(
	uint16_t b)
{
	if (bytecode_top + 2 > BYTECODE_BUF_SIZE)
		return false;

	bytecode[bytecode_top] = (uint8_t)((b >> 8) & 0xff);
	bytecode[bytecode_top + 1] = (uint8_t)(b & 0xff);

	bytecode_top += 2;

	return true;
}

static bool
lir_put_u32(
	uint32_t b)
{
	if (bytecode_top + 4 > BYTECODE_BUF_SIZE)
		return false;

	bytecode[bytecode_top] = (uint8_t)((b >> 24) & 0xff);
	bytecode[bytecode_top + 1] = (uint8_t)((b >> 16) & 0xff);
	bytecode[bytecode_top + 2] = (uint8_t)((b >> 8) & 0xff);
	bytecode[bytecode_top + 3] = (uint8_t)(b & 0xff);

	bytecode_top += 4;

	return true;
}

static void
patch_block_address(void)
{
	uint32_t offset, addr;
	int i;

	for (i = 0; i < loc_count; i++) {
		offset = loc_tbl[i].offset;
		addr = loc_tbl[i].block->addr;
		bytecode[offset] = (addr >> 24) & 0xff;
		bytecode[offset + 1] = (addr >> 16) & 0xff;
		bytecode[offset + 2] = (addr >> 8) & 0xff;
		bytecode[offset + 3] = addr & 0xff;
	}
}

/*
 * Free a constructed LIR.
 */
void
lir_free(struct lir_func *func)
{
	int i;

	assert(func != NULL);

	free(func->func_name);
	for (i = 0; i < func->param_count; i++)
		free(func->param_name[i]);
	free(func->bytecode);
	memset(func, 0, sizeof(struct lir_func));
}

/*
 * Get a file name.
 */
const char *
lir_get_file_name(void)
{
	return lir_file_name;
}

/*
 * Get an error line.
 */
int
lir_get_error_line(void)
{
	return lir_error_line;
}

/*
 * Get an error message.
 */
const char *
lir_get_error_message(void)
{
	return lir_error_message;
}

/* Set an error message. */
static void
lir_fatal(
	const char *msg,
	...)
{
	va_list ap;

	va_start(ap, msg);
	vsnprintf(lir_error_message,
		  sizeof(lir_error_message),
		  msg,
		  ap);
	va_end(ap);
}

/* Set an out-of-memory error message. */
static void
lir_out_of_memory(void)
{
	snprintf(lir_error_message,
		 sizeof(lir_error_message),
		 "LIR: Out of memory error.");
}
