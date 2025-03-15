/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Linguine
 * Copyright (c) 2025, The Linguine Authors. All rights reserved.
 */

/*
 * The `linguine` Command
 */

#include "linguine/linguine.h"
#include "linguine/runtime.h"
#include "linguine/ast.h"
#include "linguine/hir.h"
#include "linguine/lir.h"
#include "linguine/cback.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

const char version[] =
	"Linguine CLI Version 0.0.1\n";

const char usage[] =
	"Usage: linguine [-c output-file] [-a output-file] [-d output-file] <source file>\n"
	"  -c   Compile to a bytecode file.\n"
	"  -a   Compile to an app C source.\n"
	"  -d   Compile to a DLL C source.\n"
	"  -i   Use interpreter instead of JIT compiler.\n"
	"  -h   Show this help.\n"
	"  -v   Show a version.\n";

const char options[] =
	"c:"	/* Compile to bytecode file. */
	"a:"	/* Compile to C app. */
	"d:"	/* Compile to C DLL. */
	"i"	/* Use interpreter instead of JIT compiler. */
	"h"	/* Show help. */
	"v";	/* Show version. */

bool opt_compile;
bool opt_compile_to_lsc;
bool opt_compile_to_app;
bool opt_compile_to_dll;
const char *opt_output;

/* Config */
extern bool linguine_conf_use_jit;

static const char *print_param[] = {"msg"};

static char source_text[1 * 1024 * 1024];

static void parse_options(int argc, char *argv[]);
static bool run_interpreter(int argc, char *argv[]);
static bool run_source_compiler(int argc, char *argv[]);
static bool run_binary_compiler(int argc, char *argv[]);
static bool load_source_file(char *fname);
static void print_error(struct rt_env *rt);
static bool cfunc_print(struct rt_env *rt);
static bool cfunc_readline(struct rt_env *rt);

int main(int argc, char *argv[])
{
	/* Parse command line options. */
	parse_options(argc, argv);

	/* Run. */
	if (opt_compile_to_dll || opt_compile_to_app) {
		/* Translate to a C source file. */
		if (!run_source_compiler(argc, argv))
			return 1;
	} else if (opt_compile_to_lsc) {
		/* Translate to a bytecode file. */
		if (!run_binary_compiler(argc, argv))
			return 1;
	} else {
		/* Run by the interpreter. */
		if (!run_interpreter(argc, argv))
			return 1;
	}

	return 0;
}

static void parse_options(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
		case 'c':
			opt_compile = true;
			opt_compile_to_lsc = true;
			opt_output = optarg;
			break;
		case 'a':
			opt_compile = true;
			opt_compile_to_app = true;
			opt_output = optarg;
			break;
		case 'd':
			opt_compile = true;
			opt_compile_to_dll = true;
			opt_output = optarg;
			break;
		case 'i':
			linguine_conf_use_jit = false;
			break;
		case 'h':
			printf("%s", usage);
			exit(0);
			break;
		case 'v':
			printf("%s", version);
			exit(0);
			break;
		default:
			printf("%s", usage);
			exit(1);
			break;
		}
	}

	if (optind >= argc) {
		printf("%s", usage);
		exit(1);
	}
}

static bool run_interpreter(int argc, char *argv[])
{
	struct rt_env *rt;
	struct rt_value ret;
	int i;

	/* Create a runtime. */
	if (!rt_create(&rt))
		return false;

	/* Register a print function. */
	if (!rt_register_cfunc(rt, "print", 1, print_param, cfunc_print))
		return false;

	for (i = optind; i < argc; i++) {
		/* Load a source file text. */
		if (!load_source_file(argv[i]))
			return false;

		/* Compile a source code. */
		if (!rt_register_source(rt, argv[i], source_text)) {
			print_error(rt);
			return false;
		}
	}

#if defined(CONF_DEBUGGER)
	rt->dbg_stop_flag = true;
#endif

	/* Run the main function. */
	if (!rt_call_with_name(rt, "main", NULL, 0, NULL, &ret)) {
		print_error(rt);
		return 1;
	}

	/* Destroy a runtime. */
	if (!rt_destroy(rt))
		return false;

	/* Return a result value. */
	return ret.val.i;
}

static bool run_binary_compiler(int argc, char *argv[])
{
	return true;
}

static bool run_source_compiler(int argc, char *argv[])
{
	int i, j;

	if (!cback_init(opt_output))
		return false;

	for (i = optind; i < argc; i++) {
		int func_count;

		/* Load a file. */
		if (!load_source_file(argv[i]))
			return false;

		/* Do parse and build AST. */
		if (!ast_build(argv[i], source_text)) {
			printf("Error: %s: %d: %s",
			       ast_get_file_name(),
			       ast_get_error_line(),
			       ast_get_error_message());
			return false;
		}

		/* Transform AST to HIR. */
		if (!hir_build()) {
			printf("Error: %s: %d: %s",
			       hir_get_file_name(),
			       hir_get_error_line(),
			       hir_get_error_message());
			return false;
		}

		/* For each function. */
		func_count = hir_get_function_count();
		for (j = 0; j < func_count; j++) {
			struct hir_block *hfunc;
			struct lir_func *lfunc;

			/* Transform HIR to LIR (bytecode). */
			hfunc = hir_get_function(j);
			if (!lir_build(hfunc, &lfunc)) {
				printf("Error: %s: %d: %s",
				       lir_get_file_name(),
				       lir_get_error_line(),
				       lir_get_error_message());
				return false;;
			}

			/* Put C function. */
			if (!cback_translate_func(lfunc))
				return false;

			/* Free a LIR. */
			lir_free(lfunc);
		}

		/* Free HIR. */
		hir_free();
	}

	if (opt_compile_to_dll) {
		if (cback_finalize_dll())
			return false;
	} else if (opt_compile_to_app) {
		if (cback_finalize_standalone())
			return false;
	}

	return true;
}

static bool load_source_file(char *fname)
{
	FILE *fp;
	size_t len;

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		printf("Cannot open file \"%s\".\n", fname);
		return false;
	}

	len = fread(source_text, 1, sizeof(source_text) - 1, fp);
	if (len == 0) {
		printf("Cannot read file \"%s\".\n", fname);
		return false;
	}

	/* Terminate the string. */
	source_text[len] = '\0';

	fclose(fp);

	return true;
}

static void print_error(struct rt_env *rt)
{
	printf("%s:%d: error: %s\n",
	       rt_get_error_file(rt),
	       rt_get_error_line(rt),
	       rt_get_error_message(rt));
}

static bool cfunc_print(struct rt_env *rt)
{
	struct rt_value msg;
	const char *s;
	float f;
	int i;
	int type;

	if (!rt_get_local(rt, "msg", &msg))
		return false;

	if (!rt_get_value_type(rt, &msg, &type))
		return false;

	switch (type) {
	case RT_VALUE_INT:
		if (!rt_get_int(rt, &msg, &i))
			return false;
		printf("%i\n", i);
		break;
	case RT_VALUE_FLOAT:
		if (!rt_get_float(rt, &msg, &f))
			return false;
		printf("%f\n", f);
		break;
	case RT_VALUE_STRING:
		if (!rt_get_string(rt, &msg, &s))
			return false;
		printf("%s\n", s);
		break;
	default:
		printf("[object]\n");
		break;
	}

	return true;
}

static bool cfunc_readline(struct rt_env *rt)
{
	struct rt_value ret;
	char buf[1024];

	memset(buf, 0, sizeof(buf));

	fgets(buf, sizeof(buf) - 1, stdin);
	
	if (!rt_make_string(rt, &ret, buf))
		return false;
	if (!rt_set_local(rt, "$return", &ret))
		return false;

	return true;
}
