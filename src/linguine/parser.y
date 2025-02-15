%{
/*
 * Horizon
 * Copyright (C) 2025, The Horizon Authors. All rights reserved.
 */
#include "ast.h"

#include <stdio.h>
#include <string.h>

#undef DEBUG
#ifdef DEBUG
static void print_debug(const char *s);
#define debug(s) print_debug(s)
#else
#define debug(s)
#endif

extern int ast_error_line;
extern int ast_error_column;

int ast_yylex(void *);
void ast_yyerror(void *, char *s);
%}

%{
#include "stdio.h"
extern void ast_yyerror(void *scanner, char *s);
%}

%parse-param { void *scanner }
%lex-param { scanner }

%code provides {
#define YY_DECL int ast_yylex(void *yyscanner)
}

%union {
	int ival;
	double fval;
	char *sval;

	struct ast_func_list *func_list;
	struct ast_func *func;
	struct ast_param_list *param_list;
	struct ast_stmt_list *stmt_list;
	struct ast_stmt *stmt;
	struct ast_expr *expr;
	struct ast_term *term;
	struct ast_arg_list *arg_list;
}

%token <sval> TOKEN_SYMBOL TOKEN_STR
%token <ival> TOKEN_INT
%token <fval> TOKEN_FLOAT
%token TOKEN_FUNC TOKEN_PLUS TOKEN_MINUS TOKEN_MUL TOKEN_DIV TOKEN_MOD TOKEN_ASSIGN
%token TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK TOKEN_LARR TOKEN_RARR
%token TOKEN_SEMICOLON TOKEN_DOT TOKEN_COMMA TOKEN_IF TOKEN_ELSE
%token TOKEN_WHILE TOKEN_FOR TOKEN_IN TOKEN_DOTDOT TOKEN_GT TOKEN_GTE TOKEN_LT
%token TOKEN_LTE TOKEN_EQ TOKEN_NEQ TOKEN_RETURN TOKEN_BREAK TOKEN_CONTINUE 
%token TOKEN_ARROW TOKEN_AND TOKEN_OR

%type <func_list> func_list;
%type <func> func;
%type <param_list> param_list;
%type <stmt_list> stmt_list;
%type <stmt> stmt;
%type <stmt> expr_stmt;
%type <stmt> assign_stmt;
%type <stmt> if_stmt;
%type <stmt> elif_stmt;
%type <stmt> else_stmt;
%type <stmt> while_stmt;
%type <stmt> for_stmt;
%type <stmt> return_stmt;
%type <stmt> break_stmt;
%type <stmt> continue_stmt;
%type <expr> expr;
%type <term> term;
%type <arg_list> arg_list;

%right TOKEN_LARR
%right TOKEN_LPAR
%left TOKEN_OR
%left TOKEN_AND
%left TOKEN_LT
%left TOKEN_LTE
%left TOKEN_GT
%left TOKEN_GTE
%left TOKEN_EQ
%left TOKEN_NEQ
%left TOKEN_PLUS
%left TOKEN_MINUS
%left TOKEN_MUL
%left TOKEN_DIV
%left TOKEN_MOD
%left TOKEN_DOT
%left TOKEN_ARROW

%locations

%initial-action {
	ast_yylloc.last_line = yylloc.first_line = 0;
	ast_yylloc.last_column = yylloc.first_column = 0;
}

%%
func_list	: func
		{
			$$ = ast_accept_func_list(NULL, $1);
			debug("func_list: class");
		}
		| func_list func
		{
			$$ = ast_accept_func_list($1, $2);
			debug("func_list: func_list func");
		}
		;
func		: TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_func($2, $4, $7);
			debug("func: func name(param_list) { stmt_list }");
		}
		| TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR param_list TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_func($2, $4, NULL);
			debug("func: func name(param_list) { empty }");
		}
		| TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_func($2, NULL, $6);
			debug("func: func name() { stmt_list }");
		}
		| TOKEN_FUNC TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_func($2, NULL, NULL);
			debug("func: func name() { empty }");
		}
		;
param_list	: TOKEN_SYMBOL
		{
			$$ = ast_accept_param_list(NULL, $1);
			debug("param_list: type symbol");
		}
		| param_list TOKEN_COMMA TOKEN_SYMBOL
		{
			$$ = ast_accept_param_list($1, $3);
			debug("param_list: param_list type symbol");
		}
		;
stmt_list	: stmt
		{
			$$ = ast_accept_stmt_list(NULL, $1);
			debug("stmt_list: stmt");
		}
		| stmt_list stmt
		{
			$$ = ast_accept_stmt_list($1, $2);
			debug("stmt_list: stmt_list stmt");
		}
		;
stmt		: expr_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: expr_stmt");
		}
		| assign_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: assign_stmt");
		}
		| if_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: if_stmt");
		}
		| elif_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: elif_stmt");
		}
		| else_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: else_stmt");
		}
		| while_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: while_stmt");
		}
		| for_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: for_stmt");
		}
		| return_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: return_stmt");
		}
		| break_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: break_stmt");
		}
		| continue_stmt
		{
			$$ = $1;
			ast_accept_stmt($1, ast_yylloc.first_line + 1);
			debug("stmt: continue_stmt");
		}
		;
expr_stmt	: expr TOKEN_SEMICOLON
		{
			$$ = ast_accept_expr_stmt($1);
			debug("expr_stmt");
		}
		;
assign_stmt	: expr TOKEN_ASSIGN expr TOKEN_SEMICOLON
		{
			$$ = ast_accept_assign_stmt($1, $3);
			debug("assign_stmt");
		}
		;
if_stmt		: TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_if_stmt($3, $6);
			debug("if_stmt: stmt_list");
		}
		| TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_if_stmt($3, NULL);
			debug("if_stmt: empty");
		}
		;
elif_stmt	: TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_elif_stmt($4, $7);
			debug("elif_stmt: stmt_list");
		}
		| TOKEN_ELSE TOKEN_IF TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_elif_stmt($4, NULL);
			debug("elif_stmt: empty");
		}
		;
else_stmt	: TOKEN_ELSE TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_else_stmt($3);
			debug("else_stmt: stmt_list");
		}
		| TOKEN_ELSE TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_else_stmt(NULL);
			debug("else_stmt: empty");
		}
		;
while_stmt	: TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_while_stmt($3, $6);
			debug("while_stmt: stmt_list");
		}
		| TOKEN_WHILE TOKEN_LPAR expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_while_stmt($3, NULL);
			debug("while_stmt: empty");
		}
		;
for_stmt	: TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_for_kv_stmt($3, $5, $7, $10);
			debug("for_stmt: for(k, v in array) { stmt_list }");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_COMMA TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_for_kv_stmt($3, $5, $7, NULL);
			debug("for_stmt: for(k, v in array) { empty }");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_for_v_stmt($3, $5, $8);
			debug("for_stmt: for(v in array) { stmt_list }");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_for_v_stmt($3, $5, NULL);
			debug("for_stmt: for(v in array) { empty }");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_DOTDOT expr TOKEN_RPAR TOKEN_LBLK stmt_list TOKEN_RBLK
		{
			$$ = ast_accept_for_range_stmt($3, $5, $7, $10);
			debug("for_stmt: for(i in x..y) { stmt_list }");
		}
		| TOKEN_FOR TOKEN_LPAR TOKEN_SYMBOL TOKEN_IN expr TOKEN_DOTDOT expr TOKEN_RPAR TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_for_range_stmt($3, $5, $7, NULL);
			debug("for_stmt: for(i in x..y) { empty}");
		}
		;
return_stmt	: TOKEN_RETURN expr TOKEN_SEMICOLON
		{
			$$ = ast_accept_return_stmt($2);
			debug("rerurn_stmt:");
		}
		;
break_stmt	: TOKEN_BREAK TOKEN_SEMICOLON
		{
			$$ = ast_accept_break_stmt();
			debug("break_stmt:");
		}
		;
continue_stmt	: TOKEN_CONTINUE TOKEN_SEMICOLON
		{
			$$ = ast_accept_continue_stmt();
			debug("continue_stmt");
		}
		;
expr		: term
		{
			$$ = ast_accept_term_expr($1);
			debug("expr: term");
		}
		| expr TOKEN_OR expr
		{
			$$ = ast_accept_or_expr($1, $3);
			debug("expr: expr or expr");
		}
		| expr TOKEN_AND expr
		{
			$$ = ast_accept_and_expr($1, $3);
			debug("expr: expr and expr");
		}
		| expr TOKEN_LT expr
		{
			$$ = ast_accept_lt_expr($1, $3);
			debug("expr: expr lt expr");
		}
		| expr TOKEN_LTE expr
		{
			$$ = ast_accept_lte_expr($1, $3);
			debug("expr: expr lte expr");
		}
		| expr TOKEN_GT expr
		{
			$$ = ast_accept_gt_expr($1, $3);
			debug("expr: expr gt expr");
		}
		| expr TOKEN_GTE expr
		{
			$$ = ast_accept_gte_expr($1, $3);
			debug("expr: expr gte expr");
		}
		| expr TOKEN_EQ expr
		{
			$$ = ast_accept_eq_expr($1, $3);
			debug("expr: expr eq expr");
		}
		| expr TOKEN_NEQ expr
		{
			$$ = ast_accept_neq_expr($1, $3);
			debug("expr: expr neq expr");
		}
		| expr TOKEN_PLUS expr
		{
			$$ = ast_accept_plus_expr($1, $3);
			debug("expr: expr plus expr");
		}
		| expr TOKEN_MINUS expr
		{
			$$ = ast_accept_minus_expr($1, $3);
			debug("expr: expr sub expr");
		}
		| expr TOKEN_MUL expr
		{
			$$ = ast_accept_mul_expr($1, $3);
			debug("expr: expr mul expr");
		}
		| expr TOKEN_DIV expr
		{
			$$ = ast_accept_div_expr($1, $3);
			debug("expr: expr div expr");
		}
		| expr TOKEN_MOD expr
		{
			$$ = ast_accept_mod_expr($1, $3);
			debug("expr: expr div expr");
		}
		| TOKEN_MINUS expr
		{
			$$ = ast_accept_neg_expr($2);
			debug("expr: neg expr");
		}
		| TOKEN_LPAR expr TOKEN_RPAR
		{
			$$ = $2;
			debug("expr: (expr)");
		}
		| expr TOKEN_LARR expr TOKEN_RARR
		{
			$$ = ast_accept_subscr_expr($1, $3);
			debug("expr: array[subscript]");
		}
		| expr TOKEN_DOT TOKEN_SYMBOL
		{
			$$ = ast_accept_dot_expr($1, $3);
			debug("expr: expr.symbol");
		}
		| expr TOKEN_LPAR arg_list TOKEN_RPAR
		{
			$$ = ast_accept_call_expr($1, $3);
			debug("expr: call(param_list)");
		}
		| expr TOKEN_LPAR TOKEN_RPAR
		{
			$$ = ast_accept_call_expr($1, NULL);
			debug("expr: call()");
		}
		| expr TOKEN_ARROW TOKEN_SYMBOL TOKEN_LPAR arg_list TOKEN_RPAR
		{
			$$ = ast_accept_thiscall_expr($1, $3, $5);
			debug("expr: thiscall(param_list)");
		}
		| expr TOKEN_ARROW TOKEN_SYMBOL TOKEN_LPAR TOKEN_RPAR
		{
			$$ = ast_accept_thiscall_expr($1, $3, NULL);
			debug("expr: thiscall(param_list)");
		}
		| TOKEN_LARR arg_list TOKEN_RARR
		{
			$$ = ast_accept_array_expr($2);
			debug("expr: array");
		}
		;
arg_list	: expr
		{
			$$ = ast_accept_arg_list(NULL, $1);
			debug("arg_list: expr");
		}
		| arg_list TOKEN_COMMA expr
		{
			$$ = ast_accept_arg_list($1, $3);
			debug("arg_list: arg_list param_list");
		}
		;
term		: TOKEN_INT
		{
			$$ = ast_accept_int_term($1);
			debug("term: int");
		}
		| TOKEN_FLOAT
		{
			$$ = ast_accept_float_term($1);
			debug("term: float");
		}
		| TOKEN_STR
		{
			$$ = ast_accept_str_term($1);
			debug("term: string");
		}
		| TOKEN_SYMBOL
		{
			$$ = ast_accept_symbol_term($1);
			debug("term: symbol");
		}
		| TOKEN_LARR TOKEN_RARR
		{
			$$ = ast_accept_empty_array_term();
			debug("term: empty array symbol");
		}
		| TOKEN_LBLK TOKEN_RBLK
		{
			$$ = ast_accept_empty_dict_term();
			debug("term: empty dict symbol");
		}
		;
%%

#ifdef DEBUG
static void print_debug(const char *s)
{
	fprintf(stderr, "%s\n", s);
}
#endif

void ast_yyerror(void *scanner, char *s)
{
	extern int ast_error_line;
	extern int ast_error_column;
	extern char *ast_error_message;

	(void)scanner;
	(void)s;

	ast_error_line = ast_yylloc.last_line + 1;
	ast_error_column = ast_yylloc.last_column + 1;
	if (s != NULL)
		strcpy(ast_error_message, s);
	else
		strcpy(ast_error_message, "");
}
