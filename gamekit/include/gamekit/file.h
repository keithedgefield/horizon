/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * file.h: "file_" component interface.
 */

#ifndef GAMEKIT_FILE_H
#define GAMEKIT_FILE_H

#include "compat.h"

/* File read stream. */
struct file;

/* Check whether a file exists. */
bool file_check_exist(const char *file);

/* Open a file stream. */
bool file_open(const char *file, struct file **f);

/* Get a file size. */
bool file_get_size(struct file *f, size_t *ret);

/* Read from a file stream. */
bool file_read(struct file *f, void *buf, size_t size, size_t *ret);

/* Read a u64 from a file stream. */
bool file_get_u64(struct file *f, uint64_t *data);

/* Read a u32 from a file stream. */
bool file_get_u32(struct file *f, uint32_t *data);

/* Read a u16 from a file stream. */
bool file_get_u16(struct file *f, uint16_t *data);

/* Read a u8 from a file stream. */
bool file_get_u8(struct file *f, uint8_t *data);

/* Read a string from a file stream. */
bool file_get_string(struct file *f, char *buf, size_t size);

/* Close a file stream. */
void file_close(struct file *r);

/* Rewind a file stream. */
void file_rewind(struct file *f);

#endif
