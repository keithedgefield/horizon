/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * AST: Abstract Syntax Tree Generator
 */

#ifndef LINGUINE_AST_H
#define LINGUINE_AST_H

#include "config.h"

/*
 * AST
 */

/* Statement Type */
enum ast_stmt_type {
	AST_STMT_EMPTY,
	AST_STMT_EXPR,
	AST_STMT_ASSIGN,
	AST_STMT_IF,
	AST_STMT_ELIF,
	AST_STMT_ELSE,
	AST_STMT_WHILE,
	AST_STMT_FOR,
	AST_STMT_RETURN,
	AST_STMT_BREAK,
	AST_STMT_CONTINUE,
};

/* Expression Type */
enum ast_expr_type {
	AST_EXPR_TERM,
	AST_EXPR_LT,
	AST_EXPR_LTE,
	AST_EXPR_GT,
	AST_EXPR_GTE,
	AST_EXPR_EQ,
	AST_EXPR_NEQ,
	AST_EXPR_PLUS,
	AST_EXPR_MINUS,
	AST_EXPR_MUL,
	AST_EXPR_DIV,
	AST_EXPR_MOD,
	AST_EXPR_AND,
	AST_EXPR_OR,
	AST_EXPR_NEG,
	AST_EXPR_PAR,
	AST_EXPR_SUBSCR,
	AST_EXPR_DOT,
	AST_EXPR_CALL,
	AST_EXPR_THISCALL,
	AST_EXPR_ARRAY,
	AST_EXPR_DICT,
};

/* Term Type */
enum ast_term_type {
	AST_TERM_INT,
	AST_TERM_FLOAT,
	AST_TERM_STRING,
	AST_TERM_SYMBOL,
	AST_TERM_EMPTY_ARRAY,
	AST_TERM_EMPTY_DICT,
};

/* Forward Declaration */
struct ast_func_list;
struct ast_func;
struct ast_param_list;
struct ast_param;
struct ast_stmt_list;
struct ast_stmt;
struct ast_expr;
struct ast_term;
struct ast_arg_list;

/* Function List */
struct ast_func_list {
	struct ast_func *list;
};

/* Function */
struct ast_func {
	/* Function name. */
	char *name;

	/* Parameter list. */
	struct ast_param_list *param_list;

	/* Statement list */
	struct ast_stmt_list *stmt_list;

	/* List next node. */
	struct ast_func *next;
};

/* AST Parameter List */
struct ast_param_list {
	struct ast_param *list;
};

/* AST Parameter */
struct ast_param {
	char *name;
	struct ast_param *next;
};

/* AST Statement List */
struct ast_stmt_list {
	struct ast_stmt *list;
};

/* AST Statement */
struct ast_stmt {
	/* Statement type. */
	int type;

	union {
		/* Expression Statement */
		struct {
			/* Expression */
			struct ast_expr *expr;
		} expr;

		/* Assignment Statement */
		struct {
			/* LHS and RHS */
			struct ast_expr *lhs;
			struct ast_expr *rhs;
		} assign;

		/* If Block */
		struct {
			/* Condition expression. */
			struct ast_expr *cond;

			/* Statement list. */
			struct ast_stmt_list *stmt_list;
		} if_;

		/* Else-If Block */
		struct {
			/* Condition expression. */
			struct ast_expr *cond;

			/* Statement list. */
			struct ast_stmt_list *stmt_list;
		} elif;

		/* Else Block */
		struct {
			/* Statement list. */
			struct ast_stmt_list *stmt_list;
		} else_;

		/* While Block */
		struct {
			/* Condition expression. */
			struct ast_expr *cond;

			/* Statement list. */
			struct ast_stmt_list *stmt_list;
		} while_;

		/* For Block */
		struct {
			/* Is ranged-for block? */
			bool is_range;

			/* Counter symbol. */
			char *counter_symbol;

			/* Counter start and stop for ranged-for. */
			struct ast_expr *start;
			struct ast_expr *stop;

			/* Key and value symbol. */
			char *key_symbol;
			char *value_symbol;

			/* Array expression. */
			struct ast_expr *collection;

			/* Statement list. */
			struct ast_stmt_list *stmt_list;
		} for_;

		/* Return Statement */
		struct {
			/* Return value expression. */
			struct ast_expr *expr;
		} return_;
	} val;

	/* Source code position. */
	int line;
	int column;

	/* Next list node. */
	struct ast_stmt *next;
};

/* AST Expression */
struct ast_expr {
	/* Expression type. */
	int type;

	union {
		/* Term Expression */
		struct {
			/* Term. */
			struct ast_term *term;
		} term;

		/* Binary Operator Expression */
		struct {
			/* Expressions. */
			struct ast_expr *expr[2];
		} binary;

		/* Unary Operator Expression */
		struct {
			/* Expression. */
			struct ast_expr *expr;
		} unary;

		/* Parensis Expression */
		struct {
			/* Expression. */
			struct ast_expr *expr;
		} par;

		/* Dot Expression */
		struct {
			/* Object expression. */
			struct ast_expr *obj;

			/* Member symbol. */
			char *symbol;
		} dot;

		/* Call Expression */
		struct {
			/* Function expression. */
			struct ast_expr *func;

			/* Argument list. */
			struct ast_arg_list *arg_list;
		} call;

		/* This-Call Expression */
		struct {
			/* Object expression. */
			struct ast_expr *obj;

			/* Function name. */
			char *func;

			/* Argument list. */
			struct ast_arg_list *arg_list;
		} thiscall;

		/* Array Literal Expression */
		struct {
			/* Element list. */
			struct ast_arg_list *elem_list;
		} array;

		/* Dictionary Literal Expression */
		struct {
			/* Element list. */
			struct ast_kv_list *kv_list;
		} dict;
	} val;

	/* Next expression node. */
	struct ast_expr *next;
};

/* Key-Value Pair */
struct ast_kv {
	char *key;
	struct ast_expr *value;
	struct ast_kv *next;
};

/* Key-Value List */
struct ast_kv_list {
	struct ast_kv *list;
};

/* AST Term */
struct ast_term {
	/* Term type. */
	int type;

	union {
		/* Value. */
		int i;
		double f;
		char *s;
		char *symbol;
	} val;
};

/* AST Argument List */
struct ast_arg_list {
	struct ast_expr *list;
};

/*
 * Public
 */
bool ast_build(const char *file_name, const char *text);
void ast_free(void);
struct ast_func_list *ast_get_func_list(void);
const char *ast_get_file_name(void);
const char *ast_get_error_message(void);
int ast_get_error_line(void);

/* Internal: called back from the parser. */
struct ast_func_list *ast_accept_func_list(struct ast_func_list *impl_list, struct ast_func *func);
struct ast_func *ast_accept_func(char *name, struct ast_param_list *param_list, struct ast_stmt_list *stmt_list);
struct ast_param_list *ast_accept_param_list(struct ast_param_list *param_list, char *name);
struct ast_stmt_list *ast_accept_stmt_list(struct ast_stmt_list *stmt_list, struct ast_stmt *stmt);
void ast_accept_stmt(struct ast_stmt *stmt, int line);
struct ast_stmt *ast_accept_empty_stmt(void);
struct ast_stmt *ast_accept_expr_stmt(struct ast_expr *expr);
struct ast_stmt *ast_accept_assign_stmt(struct ast_expr *lhs, struct ast_expr *rhs);
struct ast_stmt *ast_accept_if_stmt(struct ast_expr *cond, struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_elif_stmt(struct ast_expr *cond, struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_else_stmt(struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_while_stmt(struct ast_expr *cond, struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_for_kv_stmt(char *key_sym, char *val_sym, struct ast_expr *array, struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_for_v_stmt(char *iter_sym, struct ast_expr *array, struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_for_range_stmt(char *counter_sym, struct ast_expr *start, struct ast_expr *stop, struct ast_stmt_list *stmt_list);
struct ast_stmt *ast_accept_return_stmt(struct ast_expr *expr);
struct ast_stmt *ast_accept_break_stmt(void);
struct ast_stmt *ast_accept_continue_stmt(void);
struct ast_expr *ast_accept_term_expr(struct ast_term *term);
struct ast_expr *ast_accept_lt_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_lte_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_gt_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_gte_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_eq_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_neq_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_plus_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_minus_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_mul_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_div_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_mod_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_and_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_or_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_neg_expr(struct ast_expr *expr);
struct ast_expr *ast_accept_par_expr(struct ast_expr *expr);
struct ast_expr *ast_accept_subscr_expr(struct ast_expr *expr1, struct ast_expr *expr2);
struct ast_expr *ast_accept_dot_expr(struct ast_expr *obj, char *symbol);
struct ast_expr *ast_accept_call_expr(struct ast_expr *func, struct ast_arg_list *arg_list);
struct ast_expr *ast_accept_thiscall_expr(struct ast_expr *obj, char *func, struct ast_arg_list *arg_list);
struct ast_expr *ast_accept_array_expr(struct ast_arg_list *arg_list);
struct ast_expr *ast_accept_dict_expr(struct ast_kv_list *kv_list);
struct ast_kv_list *ast_accept_kv_list(struct ast_kv_list *kv_list, struct ast_kv *kv);
struct ast_kv *ast_accept_kv(char *key, struct ast_expr *value);
struct ast_term *ast_accept_int_term(int i);
struct ast_term *ast_accept_float_term(float f);
struct ast_term *ast_accept_str_term(char *s);
struct ast_term *ast_accept_symbol_term(char *symbol);
struct ast_term *ast_accept_empty_array_term(void);
struct ast_term *ast_accept_empty_dict_term(void);
struct ast_arg_list *ast_accept_arg_list(struct ast_arg_list *arg_list, struct ast_expr *expr);

#endif
