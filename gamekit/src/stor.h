/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stor.h: "stor" component interface.
 */

#ifndef GAMEKIT_STOR_H
#define GAMEKIT_STOR_H

#include "config.h"

struct stor;

/* Initialize the "stor" module. */
bool stor_init_module(void);

/* Cleanup the "stor" module. */
void stor_cleanup_module(void);

/* Open a storage. */
bool stor_open(const char *file_name, struct stor **s);

/* Put an data item. */
bool stor_put(struct stor *s, const char *key, const char *value);

/* Get an data item. */
bool stor_get(struct stor *s, const char *key, const char **value);

/* Remove an data item. */
bool stor_remove(struct stor *s, const char *key);

/* Remove all data items. */
bool stor_remove_all(struct stor *s);

/* Close a storage. */
bool stor_close(struct stor *s);

#endif
