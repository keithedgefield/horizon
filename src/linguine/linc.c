/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * The `linc` Command
 */

#include "linguine/ast.h"
#include "linguine/hir.h"
#include "linguine/lir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

const char lsc_header[] = "Linguine Bytecode";

const char version[] =
	"Linguine compiler version 0.0.1\n";

const char usage[] =
	"Usage: linc -h -v -b -c -C source-files\n";

const char options[] =
	"h"	/* Show help */
	"v"	/* Show version */
	"b"	/* Compile to bytecode */
	"c"	/* Compile to C */
	"p";	/* Generate C project (implies -c) */

extern const char source_runtime_h[];

#if !defined(TARGET_MACOS)
#define INCLUDE_BIN_FILE(symbol, file) \
	extern const char symbol[]; \
	__asm__(".global " #symbol "\n" \
		#symbol ":\n" \
		".incbin \"" file "\"\n" \
		".byte 0\n");
#else
#define INCLUDE_BIN_FILE(symbol, file) \
	extern const char symbol[]; \
	__asm__(".global _" #symbol "\n" \
		"_" #symbol ":\n" \
		".incbin \"" file "\"\n" \
		".byte 0\n");
#endif

INCLUDE_BIN_FILE(source_runtime_h, "linguine/runtime.h");
INCLUDE_BIN_FILE(source_runtime_c, "linguine/runtime.c");

bool opt_bytecode_backend = true;
bool opt_c_backend;
bool opt_generate_project;
const char *opt_source;

static char source_text[1 * 1024 * 1024];

static void parse_options(int argc, char *argv[]);
static bool load_source_file(void);
static void compile_to_bytecode(void);
static void compile_to_c(void);
static void generate_c_main(void);

int main(int argc, char *argv[])
{
	parse_options(argc, argv);

	load_source_file();

	if (opt_bytecode_backend) {
		compile_to_bytecode();
	}

	return 0;
}

static void parse_options(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
		case 'v':
			printf("%s", version);
			exit(0);
			break;
		case 'h':
			printf("%s", usage);
			exit(0);
			break;
		case 'b':
			opt_bytecode_backend = true;
			break;
		case 'c':
			opt_c_backend = true;
			break;
		case 'C':
			opt_c_backend = true;
			opt_generate_project = true;
			break;
		default:
			printf("%s", usage);
			exit(1);
			break;
		}
	}

	if (optind >= argc) {
		printf("Specify a source file.\n");
		exit(1);
	}

	opt_source = argv[optind];
}

static bool load_source_file(void)
{
	FILE *fp;
	size_t len;

	fp = fopen(opt_source, "rb");
	if (fp == NULL) {
		printf("Cannot open file \"%s\".\n", opt_source);
		exit(1);
	}

	len = fread(source_text, 1, sizeof(source_text) - 1, fp);
	if (len == 0) {
		printf("Cannot read file \"%s\".\n", opt_source);
		exit(1);
	}

	/* Terminate the string. */
	source_text[len] = '\0';

	fclose(fp);

	return true;
}

static void compile_to_bytecode(void)
{
	struct hir_block *hfunc;
	struct lir_func *lfunc;
	struct rt_func *rfunc;
	int i, func_count;

	/* Do parse and build AST. */
	if (!ast_build(opt_source, source_text)) {
		printf("%s:%d: error: %s\n",
		       ast_get_file_name(),
		       ast_get_error_line(),
		       ast_get_error_message());
		exit(1);
	}

	/* Transform AST to HIR. */
	if (!hir_build()) {
		printf("%s:%d: error: %s\n",
		       hir_get_file_name(),
		       hir_get_error_line(),
		       hir_get_error_message());
		exit(1);
	}
	func_count = hir_get_function_count();

	/* For each function. */
	for (i = 0; i < func_count; i++) {
		/* Transform HIR to LIR (bytecode). */
		hfunc = hir_get_function(i);
		if (!lir_build(hfunc, &lfunc)) {
			printf("%s:%d: error: %s",
			       lir_get_file_name(),
			       lir_get_error_line(),
			       lir_get_error_message());
			exit(1);
		}

		/* TODO: Put bytecode to a file. */

		/* Free a LIR. */
		lir_free(lfunc);
	}

	/* Free intermediates. */
	hir_free();
	ast_free();
}

static void compile_to_c(void)
{
	printf("C backend is not implemented yet.\n");
	exit(1);
}

static void generate_c_main(void)
{
	printf("C backend is not implemented yet.\n");
	exit(1);
}
