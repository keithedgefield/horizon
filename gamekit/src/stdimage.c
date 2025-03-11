/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * stdimage.c: The standard implementation of the image_ interface.
 */

#include "gamekit/gamekit.h"

#if defined(TARGET_WIN32)
#include <malloc.h>	/* _aligned_malloc() */
#endif

/* 512-bit alignment. */
#define ALIGN_BYTES	(64)

/*
 * The body of the image structure.
 */

struct image {
	int width;
	int height;
	pixel_t *pixels;
};

/* Forward declaration. */
static bool image_check_draw(struct image *dst_image, int *dst_left, int *dst_top, struct image *src_image, int *width, int *height, int *src_left, int *src_top, int alpha);

/*
 * Initialize the stdimage module.
 */
bool stdimage_init(void)
{
	return true;
}

/*
 * Cleanup the stdimage module.
 */
void stdimage_cleanup(void)
{
}

/*
 * Create an image.
 */
bool image_create(int w, int h, struct image **ret)
{
	struct image *img;
	pixel_t *pixels;

	assert(w > 0 && h > 0);

	/* Allocate a memory for a struct image. */
	img = malloc(sizeof(struct image));
	if (img == NULL) {
		sys_out_of_memory();
		return NULL;
	}

	/* Allocate a pixel buffer. */
#if defined(TARGET_WIN32)
	pixels = _aligned_malloc((size_t)w * (size_t)h * sizeof(pixel_t), ALIGN_BYTES);
	if (pixels == NULL) {
		system_out_of_memory();
		free(img);
		return NULL;
	}
#else
	if (posix_memalign((void **)&pixels, ALIGN_BYTES, (size_t)w * (size_t)h * sizeof(pixel_t)) != 0) {
		sys_out_of_memory();
		free(img);
		return NULL;
	}
#endif

	/* Setup members */
	img->width = w;
	img->height = h;
	img->pixels = pixels;

	return img;
}

/*
 * Destroy an image.
 */
void image_destroy(struct image *img)
{
	assert(img != NULL);
	assert(img->width > 0 && img->height > 0);
	assert(img->pixels != NULL);

	/* Free the pixel buffer. */
#if defined(OPENNOVEL_TARGET_WIN32)
	_aligned_free(img->pixels);
#else
	free(img->pixels);
#endif
	img->pixels = NULL;

	/* Free the structure. */
	free(img);
}

/*
 * Get an image width.
 */
int image_get_width(struct image *img)
{
	assert(img != NULL);

	return img->width;
}

/*
 * Get an image height.
 */
int image_get_height(struct image *img)
{
	assert(img != NULL);

	return img->height;
}

/*
 * Get image pixels.
 */
pixel_t *image_get_pixels(struct image *img)
{
	assert(img != NULL);

	return img->pixels;
}

/*
 * Clear an image with a uniform color.
 */
void image_clear(struct image *img, pixel_t color)
{
	image_clear_rect(img, 0, 0, img->width, img->height, color);
}

/*
 * Clear an image rectangle with a uniform color.
 */
void image_clear_rect(struct image *img, int x, int y, int w, int h, pixel_t color)
{
	pixel_t *pixels;
	int i, j, sx, sy;

	assert(img != NULL);
	assert(img->width > 0 && img->height > 0);
	assert(img->pixels != NULL);

	/* Check if we have to draw. */
	if(w == 0 || h == 0)
		return;
	sx = sy = 0;
	if(!image_clip_by_dest(img->width, img->height, &w, &h, &x, &y, &sx, &sy))
		return;

	assert(x >= 0 && x < img->width);
	assert(w >= 0 && x + w <= img->width);
	assert(y >= 0 && y < img->height);
	assert(h >= 0 && y + h <= img->height);

	/* Fill pixels. */
	pixels = img->pixels;
	for (i = y; i < y + h; i++)
		for (j = x; j < x + w; j++)
			pixels[img->width * i + j] = color;
}

/*
 * Draw an image on an image. (copy)
 */
void image_draw_copy(struct image *dst_image, int dst_left, int dst_top,
		     struct image *src_image, int width, int height, int src_left,
		     int src_top)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	int x, y, sw, dw;

	if (!image_check_draw(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, 255))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++)
			*(dst_ptr + x) = *(src_ptr + x);
		src_ptr += sw;
		dst_ptr += dw;
	}
}

/*
 * Draw an image on an image. (alpha-blending, dst_alpha=255)
 */
void draw_image_alpha(struct image *dst_image, int dst_left, int dst_top,
		      struct image *src_image, int width, int height,
		      int src_left, int src_top, int alpha)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	float a, src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;
	uint32_t src_pix, dst_pix;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	if (!image_check_draw(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, alpha))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* Get the source and destination pixel values. */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* Calc alpha values. */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);
			dst_a = 1.0f - src_a;

			/* Multiply the alpha value and the source pixel value. */
			src_r = src_a * (float)get_pixel_r(src_pix);
			src_g = src_a * (float)get_pixel_g(src_pix);
			src_b = src_a * (float)get_pixel_b(src_pix);

			/* Multiply the alpha value and the destination pixel value. */
			dst_r = dst_a * (float)get_pixel_r(dst_pix);
			dst_g = dst_a * (float)get_pixel_g(dst_pix);
			dst_b = dst_a * (float)get_pixel_b(dst_pix);

			/* Store to the destination. */
			*dst_ptr++ = make_pixel(0xff,
						(uint32_t)(src_r + dst_r),
						(uint32_t)(src_g + dst_g),
						(uint32_t)(src_b + dst_b));
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}

/*
 * Draw an image on an image. (add-blending)
 */
void image_draw_image_add(struct image *dst_image, int dst_left, int dst_top,
			  struct image *src_image, int width, int height,
			  int src_left, int src_top, int alpha)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	float a, src_a;
	uint32_t src_pix, src_r, src_g, src_b;
	uint32_t dst_pix, dst_r, dst_g, dst_b;
	uint32_t add_r, add_g, add_b;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	if (!image_check_draw(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, alpha))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* Get the source and destination pixel values. */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* Calc alpha values. */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);

			/* Multiply the alpha value and the source pixel value. */
			src_r = (uint32_t)(src_a * ((float)get_pixel_r(src_pix) / 255.0f) * 255.0f);
			src_g = (uint32_t)(src_a * ((float)get_pixel_g(src_pix) / 255.0f) * 255.0f);
			src_b = (uint32_t)(src_a * ((float)get_pixel_b(src_pix) / 255.0f) * 255.0f);

			/* Multiply the alpha value and the destination pixel value. */
			dst_r = get_pixel_r(dst_pix);
			dst_g = get_pixel_g(dst_pix);
			dst_b = get_pixel_b(dst_pix);

			/* Add. */
			add_r = src_r + dst_r;
			if (add_r > 255)
				add_r = 255;
			add_g = src_g + dst_g;
			if (add_g > 255)
				add_g = 255;
			add_b = src_b + dst_b;
			if (add_b > 255)
				add_b = 255;

			/* Store to the destination. */
			*dst_ptr++ = make_pixel(0xff, add_r, add_g, add_b);
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}

/*
 * Draw an image on an image. (sub-blending)
 */
void image_draw_sub(struct image *dst_image, int dst_left, int dst_top,
		    struct image *src_image, int width, int height,
		    int src_left, int src_top, int alpha)
{
	pixel_t * RESTRICT src_ptr;
	pixel_t * RESTRICT dst_ptr;
	float a, src_a;
	uint32_t src_pix, src_r, src_g, src_b;
	uint32_t dst_pix, dst_r, dst_g, dst_b;
	uint32_t sub_r, sub_g, sub_b;
	int src_line_inc, dst_line_inc, x, y, sw, dw;

	if (!image_check_draw(dst_image, &dst_left, &dst_top, src_image, &width, &height, &src_left, &src_top, alpha))
		return;

	sw = src_image->width;
	dw = dst_image->width;
	src_ptr = src_image->pixels + sw * src_top + src_left;
	dst_ptr = dst_image->pixels + dw * dst_top + dst_left;
	src_line_inc = sw - width;
	dst_line_inc = dw - width;
	a = (float)alpha / 255.0f;

	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			/* Get the source and destination pixel values. */
			src_pix	= *src_ptr++;
			dst_pix	= *dst_ptr;

			/* Calc alpha values. */
			src_a = a * ((float)get_pixel_a(src_pix) / 255.0f);

			/* Multiply the alpha value and the source pixel value. */
			src_r = (uint32_t)(src_a * ((float)get_pixel_r(src_pix) / 255.0f) * 255.0f);
			src_g = (uint32_t)(src_a * ((float)get_pixel_g(src_pix) / 255.0f) * 255.0f);
			src_b = (uint32_t)(src_a * ((float)get_pixel_b(src_pix) / 255.0f) * 255.0f);

			/* Multiply the alpha value and the destination pixel value. */
			dst_r = get_pixel_r(dst_pix);
			dst_g = get_pixel_g(dst_pix);
			dst_b = get_pixel_b(dst_pix);

			/* Add. */
			sub_r = dst_r - src_r;
			if (sub_r < 0)
				sub_r = 0;
			sub_g = dst_g - src_g;
			if (sub_g < 0)
				sub_g = 0;
			sub_b = dst_b - src_b;
			if (sub_b < 0)
				sub_b = 0;

			/* Store to the destination. */
			*dst_ptr++ = make_pixel(0xff, sub_r, sub_g, sub_b);
		}
		src_ptr += src_line_inc;
		dst_ptr += dst_line_inc;
	}
}

/* Check for draw_image_*() parameters. */
static bool image_check_draw(struct image *dst_image, int *dst_left,
			     int *dst_top, struct image *src_image,
			     int *width, int *height, int *src_left,
			     int *src_top, int alpha)
{
	assert(dst_image != NULL);
	assert(dst_image != src_image);
	assert(dst_image->width > 0 && dst_image->height > 0);
	assert(dst_image->pixels != NULL);
	assert(src_image != NULL);
	assert(src_image->width > 0 && src_image->height > 0);
	assert(src_image->pixels != NULL);

	/* Return false if no need for a draw. */
	if(alpha == 0 || *width == 0 || *height == 0)
		return false;
	if(!image_clip_by_source(src_image->width, src_image->height, width, height, dst_left, dst_top, src_left, src_top))
		return false;
	if(!image_clip_by_dest(dst_image->width, dst_image->height, width, height, dst_left, dst_top, src_left, src_top))
		return false;

	/* Need for a draw. */
	return true;
}

/*
 * Clip a transfer rectangle by a size of a source size.
 */
bool image_clip_by_source(int src_cx, int src_cy, int *cx, int *cy, int *dst_x,
			  int *dst_y, int *src_x, int *src_y)
{
	/* If the rectangle is completely out-of-scope.*/
	if(*src_x < 0 && -*src_x >= *cx)
		return false;	/* Left. */
	if(*src_y < 0 && -*src_y >= *cy)
		return false;	/* Top. */
	if(*src_x > 0 && *src_x >= src_cx)
		return false;	/* Right. */
	if(*src_y > 0 && *src_y >= src_cy)
		return false;	/* Bottom. */

	/* Cut by a left edge. */
	if(*src_x < 0) {
		*cx += *src_x;
		*dst_x -= *src_x;
		*src_x = 0;
	}

	/* Cut by a top edge. *.
	if(*src_y < 0) {
		*cy += *src_y;
		*dst_y -= *src_y;
		*src_y = 0;
	}

	/* Cut by a right edge. */
	if(*src_x + *cx > src_cx)
		*cx = src_cx - *src_x;

	/* Cut by a bottom edge. *.
	if(*src_y + *cy > src_cy)
		*cy = src_cy - *src_y;

	if (*cx <= 0 || *cy <= 0)
		return false;

	/* Need for a draw. */
	return true;
}

/*
 * Clip a transfer rectangle by a size of a destination size.
 */
bool image_clip_by_dest(int dst_cx, int dst_cy, int *cx, int *cy, int *dst_x,
			int *dst_y, int *src_x, int *src_y)
{
	/* If completely out-of-scope. */
	if(*dst_x < 0 && -*dst_x >= *cx)
		return false;	/* Left. */
	if(*dst_y < 0 && -*dst_y >= *cy)
		return false;	/* Top. */
	if(*dst_x > 0 && *dst_x >= dst_cx)
		return false;	/* Right. */
	if(*dst_y > 0 && *dst_y >= dst_cy)
		return false;	/* Bottom. */

	/* Cut by a left edge. */
	if(*dst_x < 0) {
		*cx += *dst_x;
		*src_x -= *dst_x;
		*dst_x = 0;
	}

	/* Cut by a top edge. */
	if(*dst_y < 0) {
		*cy += *dst_y;
		*src_y -= *dst_y;
		*dst_y = 0;
	}

	/* Cut by a right edge. */
	if(*dst_x + *cx > dst_cx)
		*cx = dst_cx - *dst_x;

	/* Cut by a bottom edge. */
	if(*dst_y + *cy > dst_cy)
		*cy = dst_cy - *dst_y;

	if (*cx <= 0 || *cy <= 0)
		return false;

	/* Need for a draw. */
	return true;
}

/*
 * PNG
 */

#define PNG_DEBUG 3
#if defined(OPENNOVEL_TARGET_WASM) || defined(OPENNOVEL_TARGET_ANDROID) || defined(OPENNOVEL_TARGET_POSIX)
#include <png.h>
#else
#include <png/png.h>
#endif

struct png_reader {
	const uint8_t *data;
	size_t size;
	size_t pos;
};

static void image_png_read_callback(png_structp png_ptr, png_bytep buf, png_size_t len);

/*
 * Create an image with a PNG file.
 */
bool image_create_with_png(const uint8_t *data, size_t size, struct image **img)
{
	struct png_reader reader;
	png_structp png_ptr;
	png_byte color_type, bit_depth;
	png_infop info_ptr;
	png_bytep *rows;
	int width;
	int height;
	int y;
	pixel_t *pixels;

	reader.data = data;
	reader.size = size;
	reader.pos = 0;

	/* Check a signature. */
	if (size < 8)
		return false;
	if (png_sig_cmp(data, 0, 8))
		return false;

	/* Create a png read struct. */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
		return false;

	/* Create a png info struct. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	/* Return here if failed. */
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		if (*img != NULL) {
			image_destroy(*img);
			*img = NULL;
		}
		if (rows != NULL)
			free(rows);
		return false;
	}

	/* Read a header. */
	png_set_read_fn(png_ptr, &reader, image_png_read_callback);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	/* Get metrics. */
	width = (int)png_get_image_width(png_ptr, info_ptr);
	height = (int)png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	/* Convert palette to RGBA. */
	switch(color_type) {
	case PNG_COLOR_TYPE_GRAY:
		png_set_gray_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
		break;
	case PNG_COLOR_TYPE_PALETTE:
#if defined(TARGET_WINDOWS)
		png_set_bgr(png_ptr);
#endif
		png_set_palette_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
		break;
	case PNG_COLOR_TYPE_RGB:
#if defined(TARGET_WINDOWS)
		png_set_bgr(png_ptr);
#endif
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			png_set_tRNS_to_alpha(png_ptr);
		} else {
			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			png_read_update_info(png_ptr, info_ptr);
		}
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
#if defined(TARGET_WINDOWS)
		png_set_bgr(png_ptr);
#endif
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		png_set_gray_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
		break;
	default:
		return false;
	}

	/* 16-bit to 8-bit. */
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	/* Allocate an image. */
	if (!image_create(width, height, img))
		return false;

	/* Allocate a rows buffer. */
	rows = malloc(sizeof(png_bytep) * (size_t)height);
	if (rows == NULL) {
		image_destroy(*img);
		sys_out_of_memory();
		return false;
	}
	assert(png_get_rowbytes(png_ptr, info_ptr) == (size_t)(width*4));

	/* Read an image. */
#ifdef _MSC_VER
#pragma warning(disable:6386)
#endif
	pixels = (*img)->pixels;
	for (y = 0; y < height; y++)
		rows[y] = (png_bytep)&pixels[width * y];
	png_read_image(png_ptr, rows);

	/* Cleanup. */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return true;
}

static void image_png_read_callback(png_structp png_ptr, png_bytep buf, png_size_t len)
{
	struct png_reader *reader;

	reader = png_get_io_ptr(png_ptr);

	if (reader->pos + len > reader->size)
		len = reader->size - reader->pos;

	memcpy(buf, reader->data + reader->pos, len);
}

/*
 * JPEG
 */

#if __has_include(<jpeglib.h>)
#include <jpeglib.h>
#else
#include <jpeg/jpeglib.h>
#endif

/*
 * Create an image with a JPEG file.
 */
bool image_create_with_jpeg(const uint8_t *data, size_t size, struct image **img)
{
	struct jpeg_decompress_struct jpeg;
	struct jpeg_error_mgr jerr;
	pixel_t *p;
	unsigned char *raw_data;
	unsigned char *line;
	unsigned int width, height, x, y;
	int components;

	/* Start decoding. */
	jpeg_create_decompress(&jpeg);
	jpeg_mem_src(&jpeg, data, size);
	jpeg.err = jpeg_std_error(&jerr);
	jpeg_read_header(&jpeg, TRUE);
	jpeg_start_decompress(&jpeg);

	/* Get metrics. */
	width = jpeg.output_width;
	height = jpeg.output_height;
	components = jpeg.out_color_components;
	if (components != 3) {
		jpeg_destroy_decompress(&jpeg);
		return false;
	}

	/* Create an image. */
	if (!image_create(width, height, img)) {
		jpeg_destroy_decompress(&jpeg);
		return false;
	}

	/* Allocate a line buffer. */
	line = malloc(width * height * 3);
	if (line == NULL) {
		sys_out_of_memory();
		jpeg_destroy_decompress(&jpeg);
		image_destroy(*img);
		*img = NULL;
		return false;
	}

	/* Decode each line. */
	p = (*img)->pixels;
#if defined(TARGET_WINDOWS)
	for (y = 0; y < height; y++) {
		jpeg_read_scanlines(&jpeg, &line, 1);
		for (x = 0; x < width; x++) {
			*p++ = make_pixel(255, line[x * 3 + 2], line[x * 3 + 1], line[x * 3 + 0]);
		}
	}
#else
	for (y = 0; y < height; y++) {
		jpeg_read_scanlines(&jpeg, &line, 1);
		for (x = 0; x < width; x++) {
			*p++ = make_pixel(255, line[x * 3], line[x * 3 + 1], line[x * 3 + 2]);
		}
	}
#endif

	/* Cleanup. */
	free(line);
	jpeg_destroy_decompress(&jpeg);

	return true;
}

/*
 * WebP
 */

#include <webp/decode.h>

/*
 * Create an image with a WebP file.
 */
bool image_create_with_webp(const uint8_t *data, size_t size, struct image **img)
{
	pixel_t *p;
	uint8_t *pixels;
	size_t file_size;
	int width, height;
	int x, y;

	/* Get metrics. */
	if (!WebPGetInfo(data, size, &width, &height))
		return false;

	/* Create an image. */
	if (!image_create(width, height, img))
		return false;

	/* Do decoding. */
	pixels = WebPDecodeRGBA(data, size, &width, &height);
	if (pixels == NULL) {
		image_destroy(*img);
		*img = NULL;
		return false;
	}

	/* Copy pixels. */
	p = (*img)->pixels;
#if defined(ORDER_RGBA)
	/* Use RGBA as is. */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*p++ = make_pixel(pixels[y * width * 4 + x * 4 + 3],
					  pixels[y * width * 4 + x * 4 + 0],
					  pixels[y * width * 4 + x * 4 + 1],
					  pixels[y * width * 4 + x * 4 + 2]);
		}
	}
#else
	/* Reverse the pixel order for OpenGL. (RGB -> BGR) */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*p++ = make_pixel(pixels[y * width * 4 + x * 4 + 3],
					  pixels[y * width * 4 + x * 4 + 2],
					  pixels[y * width * 4 + x * 4 + 1],
					  pixels[y * width * 4 + x * 4 + 0]);
		}
	}
#endif

	/* Cleanup. */
	WebPFree(pixels);

	return true;
}
