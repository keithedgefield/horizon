/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

#ifndef HORIZON_HAL_SYS_H
#define HORIZON_HAL_SYS_H

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * Interface of the "system" module.
 *  - Implementations are:
 *    - "linuxsystem.c" ... For Linux
 *    - "winsystem.c"   ... For Windows
 *    - "nssystem.c"    ... For macOS
 *    - "uisystem.c"    ... For iOS
 *    - "ndksystem.c"   ... For Android NDK
 *    - "emsystem.c"    ... For Emscripten
 */

#include "config.h"

/* Print a log line. */
void sys_log(const char *format, ...);

/* Print an error line. */
void sys_error(const char *format, ...);

/* Print an out-of-memory log. */
void sys_out_of_memory(void);

/* Get a millisecond time. */
uint64_t system_get_tick(void);

/* Get a two-letter language code of a system. */
const char *system_get_language(void);

#endif
