/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stdimage.h: The standard implementation of the image_ interface.
 */

#ifndef HORIZON_HAL_STDIMAGE_H
#define HORIZON_HAL_STDIMAGE_H

#include "config.h"

/* Initialize the stdimage module. */
bool stdimage_init(void);

/* Cleanup the stdimage module. */
void stdimage_cleanup(void);

#endif
