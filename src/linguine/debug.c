/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * Debugger
 */

#include "runtime.h"

#include <stdio.h>
#include <string.h>

void
dbg_pre_hook(
	struct rt_env *rt)
{
	char buf[1024];

	if (!rt->dbg_stop_flag) {
		/* Continue. */
		return;
	}

	if (rt->dbg_error_flag) {
		printf("%s:%d: error: %s\n",
		       rt_get_error_file(rt),
		       rt_get_error_line(rt),
		       rt_get_error_message(rt));
	}

	while (true) {
		printf("(lindbg) ");
		if (fgets(buf, sizeof(buf) - 1, stdin) == NULL)
			continue;

		if (buf[0] == 'c') {
			rt->dbg_stop_flag = false;
			break;
		}
		if (buf[0] == 's') {
			rt->dbg_stop_flag = false;
			rt->dbg_single_step_flag = true;
			break;
		}
	}
}

void
dbg_post_hook(
	struct rt_env *rt)
{
	if (rt->dbg_single_step_flag) {
		if (strcmp(rt->dbg_last_file_name, rt->file_name) != 0) {
			rt->dbg_stop_flag = true;
			return;
		}
		if (rt->dbg_last_line != rt->line) {
			rt->dbg_stop_flag = true;
			return;
		}
	}
}
