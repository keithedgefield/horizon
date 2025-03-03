/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * hal.h: The main header of HAL.
 */

#ifndef HORIZON_HAL_HAL_H
#define HORIZON_HAL_HAL_H

#include "config.h"

/* Modules */
#include "sys.h"
#include "file.h"
#include "image.h"
#include "input.h"
#include "render.h"

/* C89 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/*
 * Callbacks
 */

/* Called after the "file" initializain and before the "render" initialization. */
bool on_hal_init_render(char **title, int *width, int *height);

/* Called after the whole HAL initialization and before the game loop. */
bool on_hal_ready(void);

/* Called every frame. */
bool on_hal_frame(void);

/*
 * Helpers
 */
#if defined(TARGET_WIN32)
const wchar_t *win32_utf8_to_utf16(const char *s);
#endif

#endif
