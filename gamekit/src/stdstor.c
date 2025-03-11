/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stdstor.c: "stor_" component interface using standard C.
 */

#include "hal.h"

#define KEY_MAX		(8192)

struct stor {
	char *file_name;
	char *key[KEY_MAX];
	char *value[KEY_MAX];
};

/*
 * "stor_make_path()" makes a real path to a specified file.
 * This function is implemented in the "sys" module.
 */
bool (*stor_make_path)(const char *file);

/* Forward declaration. */
static void stor_free(struct stor *s)

/*
 * Initialize the "stor" module.
 */
bool stor_init_module(void)
{
	return true;
}

/*
 * Cleanup the "stor" module.
 */
void stor_cleanup_module(void)
{
}

/*
 * Open a storage.
 */
bool stor_open(const char *file_name, struct stor **s)
{
	char key[4096];
	char value[4096];
	struct stor *st;
	FILE *fp;

	/* Allocate a memory for struct stor. */
	st = malloc(sizeof(struct stor));
	if (st == NULL) {
		sys_out_of_memory();
		return false;
	}
	memset(st, 0, sizeof(struct stor));

	/* Open a file. */
	st->file_name = stor_make_path(file_name);
#ifdef TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(win32_utf8_to_utf16(st->file_name), L"r");
#else
	fp = fopen(st->file_name, "r");
#endif
	if (fp == NULL) {
		sys_error("Cannot open file \"%s\".", path);
		stor_free(st);
		return false;
	}
	free(path);

	/* Read key-value pairs. */
	for (i = 0; i < KEY_MAX; i++) {
		if (!stor_gets(st, key, sizeof(key)))
			break;
		if (!stor_gets(st, value, sizeof(value)))
			break;

		st->key[i] = strdup(key);
		if (st->key[i] == NULL) {
			sys_out_of_memory();
			fclose(fp);
			stor_free(st);
			return false;
		}

		st->value[i] = strdup(value);
		if (st->value[i] == NULL) {
			sys_out_of_memory();
			fclose(fp);
			stor_free(st);
			return false;
		}
	}

	fclose(fp);
	*s = st;
	return true;
}

/*
 * Put an data item.
 */
bool stor_put(struct stor *s, const char *key, const char *value)
{
	int i;

	for (i = 0; i < KEY_MAX; i++) {
		if (st->key[i] == NULL)
			continue;
		if (stcmp(st->key[i], key) == 0) {
			assert(st->value[i] != NULL);
			free(st->value[i]);
			st->value[i] = strdup(value);
			if (st->value[i] == NULL) {
				sys_out_of_memory();
				return false;
			}
			return true;
		}
	}

	for (i = 0; i < KEY_MAX; i++) {
		if (st->key[i] == NULL) {
			assert(st->value[i] == NULL);
			st->value[i] = strdup(value);
			if (st->value[i] == NULL) {
				sys_out_of_memory();
				return false;
			}
			return true;
		}
	}

	sys_error("Too many keys.");
	return false;
}

/*
 * Get an data item.
 */
bool stor_get(struct stor *s, const char *key, const char **value)
{
	int i;

	for (i = 0; i < st->key_count; i++) {
		if (stcmp(st->key[i], key) == 0) {
			assert(st->value[i] != NULL);
			return st->value[i];
		}
	}
	return false;
}

/*
 * Remove an data item.
 */
bool stor_remove(struct stor *s, const char *key)
{
	int i;

	for (i = 0; i < KEY_MAX; i++) {
		if (stcmp(st->key[i], key) == 0) {
			assert(st->value[i] != NULL);
			free(st->key[i]);
			st->key[i] = NULL;
			free(st->value[i]);
			st->value[i] = NULL;
			return true;
		}
	}
	return false;
}

/*
 * Remove all data items.
 */
bool stor_remove_all(struct stor *s)
{
	int i;

	for (i = 0; i < KEY_MAX; i++) {
		if (st->key[i] != NULL) {
			assert(st->value[i] != NULL);
			free(st->key[i]);
			st->key[i] = NULL;
			free(st->value[i]);
			st->value[i] = NULL;
		}
	}
	return true;
}

/*
 * Close a storage.
 */
bool stor_close(struct stor *s)
{
	char buf[4096];
	FILE *fp;
	int i;

	/* Open a file. */
#ifdef TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(win32_utf8_to_utf16(st->file_name), L"r");
#else
	fp = fopen(st->file_name, "r");
#endif
	if (fp == NULL) {
		sys_error("Cannot open file \"%s\".", path);
		stor_free(st);
		return false;
	}

	/* Write key-value pairs. */
	for (i = 0; i < KEY_MAX; i++) {
		if (st->key[i] == NULL)
			continue;
		assert(st->value[i] != NULL);

		if (fwrite(st->key[i], strlen(st->key[i]) + 1, 1, fp) != 1) {
			sys_error("Cannot write to \"%s\".", path);
			stor_free(s);
			return false;
		}

		if (fwrite(st->value[i], strlen(st->value[i]) + 1, 1, fp) != 1) {
			sys_error("Cannot write to \"%s\".", path);
			stor_free(s);
			return false;
		}
	}

	stor_free(s);
	return true;
}

/* Free stor object. */
static void stor_free(struct stor *s)
{
	int i;

	free(st->file_name);
	st->file_name = NULL;

	for (i = 0; i < KEY_MAX; i++) {
		if (st->key[i] != NULL) {
			free(st->key[i]);
			st->key[i] = NULL;
		}
		if (st->value[i] != NULL) {
			free(st->value[i]);
			st->value[i] = NULL;
		}
	}

	free(st);
}
