/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2024, 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stdfile.h: The standard implementation of the file_ interface.
 */

#ifndef HORIZON_HAL_STDFILE_H
#define HORIZON_HAL_STDFILE_H

/* Initialize the stdfile module. */
bool stdfile_init(char *(*make_path_func)(const char *));

/* Cleanup the stdfile module. */
void stdfile_cleanup(void);

#endif
