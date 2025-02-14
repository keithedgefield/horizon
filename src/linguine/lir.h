/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * LIR: Low-level Intermediate Representation Generator
 */

#ifndef LINGUINE_LIR_H
#define LINGUINE_LIR_H

#include "config.h"

#define LIR_PARAM_SIZE		32

struct hir_block;

struct lir_func {
	char *func_name;
	int param_count;
	char *param_name[LIR_PARAM_SIZE];
	int bytecode_size;
	uint8_t *bytecode;
	int tmpvar_size;
	char *file_name;
};

/* Build a LIR function from a HIR function. */
bool lir_build(struct hir_block *hir_func, struct lir_func **lir_func);

/* Free a constructed LIR. */
void lir_free(struct lir_func *func);

/* Get a file name. */
const char *lir_get_file_name(void);

/* Get an error line. */
int lir_get_error_line(void);

/* Get an error message. */
const char *lir_get_error_message(void);

#endif
