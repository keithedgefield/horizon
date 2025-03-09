/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Linguine
 * Copyright (c) 2025, The Linguine Authors. All rights reserved.
 */

/*
 * JIT: Just-In-Time native code generation
 */

#include "compat.h"

#if defined(ARCH_X86_64)

#include "runtime.h"

/* Generate a JIT-compiled code for a function. */
bool
jit_build(
	struct rt_env *rt,
	struct rt_func *func)
{
	return true;
}

/* Free a JIT-compiled code for a function. */
void
jit_free(
	struct rt_env *rt,
	struct rt_func *func)
{
}

#endif /* ARCH_X86_64 */
