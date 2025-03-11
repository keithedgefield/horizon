/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stdfile.h: The standard implementation of the file_ interface.
 */

#ifndef GAMEKIT_STDFILE_H
#define GAMEKIT_STDFILE_H

/* Initialize the stdfile module. */
bool stdfile_init(char *(*make_path_func)(const char *));

/* Cleanup the stdfile module. */
void stdfile_cleanup(void);

#endif
