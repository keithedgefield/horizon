/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stdfile.c: The standard implementation of the file_ interface.
 */

/*
 * [Archive File Format]
 *
 * struct header {
 *     u64 file_count;
 *     struct file_entry {
 *         u8  file_name[256]; // Obfuscated
 *         u64 file_size;
 *         u64 file_offset;
 *     } [file_count];
 * };
 * u8 file_body[file_count][file_length]; // Obfuscated
 */

#include "gamekit.h"
#include "stdfile.h"

/* Win32 */
#ifdef TARGET_WIN32
#include <fcntl.h>
#endif

/*
 * The "key" of obfuscation
 */

/* The key. */
#define OBFUSCATION_KEY	(0xabadcafedeadbeef)

/* The key is shuffled so that decompilers cannot read it directly. */
static volatile uint64_t key_obfuscated =
	~((((OBFUSCATION_KEY >> 56) & 0xff) << 0) |
	  (((OBFUSCATION_KEY >> 48) & 0xff) << 8) |
	  (((OBFUSCATION_KEY >> 40) & 0xff) << 16) |
	  (((OBFUSCATION_KEY >> 32) & 0xff) << 24) |
	  (((OBFUSCATION_KEY >> 24) & 0xff) << 32) |
	  (((OBFUSCATION_KEY >> 16) & 0xff) << 40) |
	  (((OBFUSCATION_KEY >> 8)  & 0xff) << 48) |
	  (((OBFUSCATION_KEY >> 0)  & 0xff) << 56));

/* Shuffled key. */
static volatile uint64_t key_reversed;

/* Use indirect addressing to avoid storing key_reserved to a register on x86. (no effect on Arm.) */
static volatile uint64_t *key_ref = &key_reversed;

/* These keys are not secret. */
static const uint64_t NEXT_MASK1 = 0xafcb8f2ff4fff33f;
static const uint64_t NEXT_MASK2 = 0xfcbfaff8f2f4f3f0;

/*
 * Package
 */

/* The package file. */
#define PACKAGE_FILE		"game.dat"

/* Maximum entries in a package. */
#define ENTRY_SIZE		(65536)

/* File name length for an entry. */
#define FILE_NAME_SIZE		(256)

/* Package file entry. */
struct file_entry {
	/* File name. */
	char name[FILE_NAME_SIZE];

	/* File size. */
	uint64_t size;

	/* Offset in the package file. */
	uint64_t offset;
};

/* Package file path. */
static char *file_package_path;

/* File entry count. */
static uint64_t file_entry_count;

/* File entry table. */
static struct file_entry file_entry[ENTRY_SIZE];

/*
 * File read stream
 */
struct file {
	/* Is a packaged file? */
	bool is_packaged;

	/* Is obfuscated? */
	bool is_obfuscated;

	/* stdio FILE pointer */
	FILE *fp;

	/* Obfuscation parameters */
	uint64_t next_random;
	uint64_t prev_random;

	/* Effective for a packaged file: */
	uint64_t index;
	uint64_t size;
	uint64_t offset;
	uint64_t pos;
};

/*
 * "file_make_path()" makes a real path to a specified file.
 * This function is implemented in the "sys" module.
 */
char *(*file_make_path)(const char *file);

/*
 * Forward declarations.
 */
static bool file_open_package(struct file *f, const char *path);
static bool file_open_real(struct file *f, const char *path);
static void file_ungetc(struct file *f, char c);
static void file_set_random_seed(uint64_t index, uint64_t *next_random);
static char file_get_next_random(uint64_t *next_random, uint64_t *prev_random);
static void file_rewind_random(uint64_t *next_random, uint64_t *prev_random);

/*
 * Initialize the stdfile module.
 */
bool stdfile_init(char *(*make_path_func)(const char *))
{
	FILE *fp;
	uint64_t i, next_random;
	int j;

	/* Save a function pointer. */
	file_make_path = make_path_func;

	/* Get a real path to a package file. */
	file_package_path = file_make_path(PACKAGE_FILE);
	if (file_package_path == NULL)
		return false;

	/* Try opening a package file. */
#ifdef TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(utf8_to_utf16(file_package_path), L"r");
#else
	fp = fopen(file_package_path, "r");
#endif
	if (fp == NULL) {
		/* Failed to open. */
#if defined(TARGET_IOS) || defined(TARGET_WASM)
		/* Fail: On iOS and Emscripten, we need a package file. */
		return false;
#else
		/* On other platforms, we won't use a package file. */
		free(file_package_path);
		file_package_path = NULL;
		return true;
#endif
	}

	/* Read the number of the file entries. */
	if (fread(&file_entry_count, sizeof(uint64_t), 1, fp) < 1) {
		sys_error("Corrupted package file.");
		fclose(fp);
		return false;
	}
	if (file_entry_count > ENTRY_SIZE) {
		sys_error("Corrupted package file.");
		fclose(fp);
		return false;
	}

	/* Read the file entries. */
	for (i = 0; i < file_entry_count; i++) {
		if (fread(&file_entry[i].name, FILE_NAME_SIZE, 1, fp) < 1)
			break;
		file_set_random_seed(i, &next_random);
		for (j = 0; j < FILE_NAME_SIZE; j++)
			file_entry[i].name[j] ^= file_get_next_random(&next_random, NULL);
		if (fread(&file_entry[i].size, sizeof(uint64_t), 1, fp) < 1)
			break;
		if (fread(&file_entry[i].offset, sizeof(uint64_t), 1, fp) < 1)
			break;
	}
	if (i != file_entry_count) {
		sys_error("Package file corrupted.");
		fclose(fp);
		return false;
	}

	/*
	 * Close the package for now;
	 * we will reopen a FILE pointer per an input stream.
	 */
	fclose(fp);

	return true;
}

/*
 * Cleanup the stdfile module.
 */
void stdfile_cleanup(void)
{
	if (file_package_path != NULL) {
		free(file_package_path);
		file_package_path = NULL;
	}
}

/*
 * Check whether a file exists.
 */
bool file_check_exist(const char *file)
{
	char entry_name[FILE_NAME_SIZE];
	FILE *fp;
	uint64_t i;

	/* If we're using a package file. */
	if (file_package_path != NULL) {
		/* Check whether a file entry exists in the package. */
		for (i = 0; i < file_entry_count; i++) {
			if (strcasecmp(file_entry[i].name, entry_name) == 0) {
				/* Entry exists. */
				return true;
			}
		}
	}

#if defined(TARGET_IOS) || defined(TARGET_WASM)
	return false;
#else
	/* Make a real file path. */
	char *real_path;
	real_path = file_make_path(file);
	if (real_path == NULL)
		return false;

	/* Open a FILE pointer. */
#ifdef TARGET_WIN32
	_fmode = _O_BINARY;
	fp = _wfopen(conv_utf8_to_utf16(real_path), L"r");
#else
	fp = fopen(real_path, "r");
#endif

	/* Check if file exists. */
	free(real_path);
	if (fp == NULL) {
		/* Doesn't exist. */
		return false;
	}

	/* File exists. */
	fclose(fp);
	return true;
#endif
}

/*
 * Open a read file stream.
 */
bool file_open(const char *path, struct file **f)
{
	char entry_name[FILE_NAME_SIZE];
	char *real_path;
	struct file *fs;
	uint64_t i;

	/* Allocate a file struct. */
	fs = malloc(sizeof(struct file));
	if (fs == NULL) {
		sys_out_of_memory();
		return NULL;
	}

	/* If we're using a package file. */
	if (file_package_path != NULL) {
		/* Open a package file. */
		if (!file_open_package(fs, path)) {
			free(fs);
			return false;
		}

		/* Succeeded. */
		*f = fs;
		return true;
	}

#if defined(TARGET_IOS) || defined(TARGET_WASM)
	return false;
#else
	/* Open a real file. */
	if (!file_open_real(fs, path)) {
		free(fs);
		return false;
	}

	/* Succeeded. */
	*f = fs;
	return true;
#endif
}

/* Open a file in the package. */
static bool file_open_package(struct file *f, const char *path)
{
	int i;

	/* Search a file entry on the package. */
	for (i = 0; i < file_entry_count; i++) {
		if (strcasecmp(file_entry[i].name, path) == 0)
			break;
	}
	if (i == file_entry_count) {
		/* Not found. */
		sys_error("Cannot open file \"%s\".", path);
		return false;
	}

	/* Open a new FILE pointer to the package file. */
#ifdef TARGET_WIN32
	_fmode = _O_BINARY;
	f->fp = _wfopen(utf8_to_utf16(file_package_path), L"r");
#else
	f->fp = fopen(file_package_path, "r");
#endif
	if (f->fp == NULL) {
		sys_error("Cannot open file \"%s\".", PACKAGE_FILE);
		return false;
	}

	/* Seek to the offset. */
	if (fseek(f->fp, (long)file_entry[i].offset, SEEK_SET) != 0) {
		sys_error("Cannot read file \"%s\".", PACKAGE_FILE);
		fclose(f->fp);
		return false;
	}

	/* Setup the file struct. */
	f->is_packaged = true;
	f->is_obfuscated = true;
	f->index = i;
	f->size = file_entry[i].size;
	f->offset = file_entry[i].offset;
	f->pos = 0;
	file_set_random_seed(i, &f->next_random);
	f->prev_random = 0;

	return true;
}

/* Open a real file on a file system. */
static bool file_open_real(struct file *f, const char *path)
{
	char *real_path;

	/* Make a real path on the OS's file system. */
	real_path = file_make_path(path);
	if (real_path == NULL)
		return false;

	/* Open a real file. */
#ifdef TARGET_WIN32
	_fmode = _O_BINARY;
	f->fp = _wfopen(file_utf8_to_utf16(real_path), L"r");
#else
	f->fp = fopen(real_path, "r");
#endif
	free(real_path);
	if (f->fp == NULL)
		return false;

	f->is_packaged = false;
	return true;
}

/*
 * Get a file size.
 */
bool file_get_size(struct file *f, size_t *ret)
{
	long pos, len;

	/* If f points to a package entry. */
	if (f->is_packaged) {
		*ret = (size_t)f->size;
		return true;
	}

	/* Return a real file size. */
	pos = ftell(f->fp);
	fseek(f->fp, 0, SEEK_END);
	len = ftell(f->fp);
	fseek(f->fp, pos, SEEK_SET);
	*ret = (size_t)len;
	return true;
}

/*
 * Read bytes from a read file stream.
 */
bool file_read(struct file *f, void *buf, size_t size, size_t *ret)
{
	size_t len, obf;

	assert(f != NULL);
	assert(f->fp != NULL);

	if (f->is_packaged) {
		/*
		 * For the case f points to a package entry.
		 */

		/* Read. */
		if (f->pos + size > f->size)
			size = (size_t)(f->size - f->pos);
		if (size == 0)
			return false;
		len = fread(buf, 1, size, f->fp);
		f->pos += len;

		/* Do obfuscation decode. */
		for (obf = 0; obf < len; obf++)
			*(((char *)buf) + obf) ^= file_get_next_random(&f->next_random, &f->prev_random);
	} else {
		/*
		 * For the case f points to a real file.
		 */

		/* Read. */
		len = fread(buf, 1, size, f->fp);
	}

	*ret = len;
	if (len == 0)
		return false;

	return true;
}

/*
 * Read a u64 from a file stream.
 */
bool file_get_u64(struct file *f, uint64_t *data)
{
	uint64_t val;
	size_t ret;

	if (!file_read(f, &val, 8, &ret))
		return false;
	if (ret != 8)
		return false;

	*data = LETOHOST64(val);
	return true;
}

/* Read a u32 from a file stream. */
bool file_get_u32(struct file *f, uint32_t *data)
{
	uint32_t val;
	size_t ret;

	if (!file_read(f, &val, 4, &ret))
		return false;
	if (ret != 4)
		return false;

	*data = LETOHOST32(val);
	return true;
}

/* Read a u16 from a file stream. */
bool file_get_u16(struct file *f, uint16_t *data)
{
	uint16_t val;
	size_t ret;

	if (!file_read(f, &val, 2, &ret))
		return false;
	if (ret != 2)
		return false;

	*data = LETOHOST16(val);
	return true;
}

/* Read a u8 from a file stream. */
bool file_get_u8(struct file *f, uint8_t *data)
{
	uint8_t val;
	size_t ret;

	if (!file_read(f, &val, 1, &ret))
		return false;
	if (ret != 1)
		return false;

	*data = val;
	return true;
}

/*
 * Read a line from a read file stream.
 */
bool file_get_string(struct file *f, char *buf, size_t size)
{
	char *ptr;
	size_t len, read_size;
	char c;

	assert(f != NULL);
	assert(f->fp != NULL);
	assert(buf != NULL);
	assert(size > 0);

	ptr = buf;
	for (len = 0; len < size - 1; len++) {
		if (!file_read(f, &c, 1, &read_size)) {
			*ptr = '\0';
			if (len == 0)
				return false;
			return true;
		}
		if (c == '\n' || c == '\0') {
			*ptr = '\0';
			return true;
		}
		if (c == '\r') {
			if (!file_read(f, &c, 1, &read_size)) {
				*ptr = '\0';
				return true;
			}
			if (c == '\n') {
				*ptr = '\0';
				return true;
			}
			file_ungetc(f, c);
			*ptr = '\0';
			return true;
		}
		*ptr++ = c;
	}
	*ptr = '\0';

	if (len == 0)
		return false;

	return true;
}

/* Push back a character to a read file stream. */
static void file_ungetc(struct file *f, char c)
{
	assert(f != NULL);
	assert(f->fp != NULL);

	if (f->is_packaged) {
		/* If f points to a package entry. */
		assert(f->pos != 0);
		ungetc(c, f->fp);
		f->pos--;
		file_rewind_random(&f->next_random, &f->prev_random);
	} else {
		/* If f points to a real file. */
		ungetc(c, f->fp);
	}
}

/*
 * Close a read file stream.
 */
void file_close(struct file *f)
{
	assert(f != NULL);
	assert(f->fp != NULL);

	fclose(f->fp);
	free(f);
}

/*
 * Rewind a read file stream.
 */
void file_rewind(struct file *f)
{
	assert(f != NULL);
	assert(f->fp != NULL);

	if (f->is_packaged) {
		/* If f points to a package entry. */
		fseek(f->fp, (long)f->offset, SEEK_SET);
		f->pos = 0;
		file_set_random_seed(f->index, &f->next_random);
		f->prev_random = 0;
	} else {
		/* If f points to a real file. */
		rewind(f->fp);
		f->pos = 0;
	}
}

/* Set a random seed. */
static void file_set_random_seed(uint64_t index, uint64_t *next_random)
{
	uint64_t i, next, lsb;

	/* The key is shuffled so that decompilers cannot read it directly. */
	key_reversed = ((((key_obfuscated >> 56) & 0xff) << 0) |
			(((key_obfuscated >> 48) & 0xff) << 8) |
			(((key_obfuscated >> 40) & 0xff) << 16) |
			(((key_obfuscated >> 32) & 0xff) << 24) |
			(((key_obfuscated >> 24) & 0xff) << 32) |
			(((key_obfuscated >> 16) & 0xff) << 40) |
			(((key_obfuscated >> 8)  & 0xff) << 48) |
			(((key_obfuscated >> 0)  & 0xff) << 56));
	next = ~(*key_ref);
	for (i = 0; i < index; i++) {
		/* This XOR mask is not a secret. */
		next ^= NEXT_MASK1;
		lsb = next >> 63;
		next = (next << 1) | lsb;
	}

	*next_random = next;
}

/* Get a next random mask. */
static char file_get_next_random(uint64_t *next_random, uint64_t *prev_random)
{
	uint64_t next;
	char ret;

	/* For ungetc(). */
	if (prev_random != NULL)
		*prev_random = *next_random;

	ret = (char)(*next_random);
	next = *next_random;
	next = (((~(*key_ref) & 0xff00) * next + (~(*key_ref) & 0xff)) %
		~(*key_ref)) ^ NEXT_MASK2;
	*next_random = next;

	return ret;
}

/* Go back to the previous random mask. */
static void file_rewind_random(uint64_t *next_random, uint64_t *prev_random)
{
	*next_random = *prev_random;
	*prev_random = 0;
}
