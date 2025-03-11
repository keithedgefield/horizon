/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Linguine
 * Copyright (c) 2025, The Linguine Authors. All rights reserved.
 */

/*
 * The `linguine` Command
 */

#include "linguine/runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

const char version[] =
	"Linguine CLI Version 0.0.1\n";

const char usage[] =
	"Usage: linguine <source file>\n";

const char options[] =
	"i"	/* Use interpreter instead of JIT. */
	"h"	/* Show help. */
	"v";	/* Show version. */

const char *opt_source = NULL;

/* Config */
extern bool linguine_conf_use_jit;

static const char *print_param[] = {"msg"};

static char source_text[1 * 1024 * 1024];

static void parse_options(int argc, char *argv[]);
static bool load_source_file(void);
static void print_error(struct rt_env *rt);
static bool cfunc_print(struct rt_env *rt);
static bool cfunc_readline(struct rt_env *rt);

int main(int argc, char *argv[])
{
	struct rt_env *rt;
	struct rt_value ret;

	/* Parse command line options. */
	parse_options(argc, argv);

	/* Load a source file text. */
	load_source_file();

	/* Create a runtime. */
	if (!rt_create(&rt))
		return 1;

	/* Register a print function. */
	if (!rt_register_cfunc(rt, "print", 1, print_param, cfunc_print))
		return false;

	/* Compile a source code. */
	if (!rt_register_source(rt, opt_source, source_text)) {
		print_error(rt);
		return 1;
	}

#if defined(CONF_DEBUGGER)
	rt->dbg_stop_flag = true;
#endif

	/* Run the main function. */
	if (!rt_call_with_name(rt, "main", NULL, 0, NULL, &ret)) {
		print_error(rt);
		return 1;
	}

	/* Return a result value. */
	return ret.val.i;
}

static void parse_options(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
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
