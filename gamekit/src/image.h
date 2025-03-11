/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * image.h: "image_" component interface.
 */

#ifndef GAMEKIT_IMAGE_H
#define GAMEKIT_IMAGE_H

#include "compat.h"

/*
 * image structure
 */
struct image;

/*
 * RGBA-color pixel value.
 */
typedef uint32_t pixel_t;

/*
 * Pixel value manipulation
 */

#if defined(TARGET_WINDOWS) || defined(TARGET_MACOS) || defined(TARGET_IOS)
/* Use RGBA on Direct3D and Metal */
#define ORDER_RGBA
#else
/* Use BGRA o OpenGL */
#define ORDER_BGRA
#endif

/* Compose a pixel value. */
static INLINE pixel_t make_pixel(uint32_t a, uint32_t r, uint32_t g, uint32_t b)
{
#ifdef ORDER_RGBA
	return (((pixel_t)a) << 24) | (((pixel_t)r) << 16) | (((pixel_t)g) << 8) | ((pixel_t)b);
#else
	return (((pixel_t)a) << 24) | (((pixel_t)b) << 16) | (((pixel_t)g) << 8) | ((pixel_t)r);
#endif
}

/* Get an alpha component value from a pixel value. */
static INLINE uint32_t get_pixel_a(pixel_t p)
{
	return (p >> 24) & 0xff;
}

/* Get a red component value from a pixel value. */
static INLINE uint32_t get_pixel_r(pixel_t p)
{
#ifdef ORDER_RGBA
	return (p >> 16) & 0xff;
#else
	return p & 0xff;
#endif
}

/* Get a green component value from a pixel value. */
static INLINE uint32_t get_pixel_g(pixel_t p)
{
#ifdef ORDER_RGBA
	return (p >> 8) & 0xff;
#else
	return (p >> 8) & 0xff;
#endif
}

/* Get a blue component value from a pixel value. */
static INLINE uint32_t get_pixel_b(pixel_t p)
{
#ifdef ORDER_RGBA
	return p & 0xff;
#else
	return (p >> 16) & 0xff;
#endif
}

#undef ORDER_RGBA
#undef ORDER_BGRA

/* Create an image. */
bool image_create(int w, int h, struct image **img);

/* Create an image with a PNG file. */
bool image_create_with_png(const uint8_t *data, size_t size, struct image **img);

/* Create an image with a JPEG file. */
bool image_create_with_jpeg(const uint8_t *data, size_t size, struct image **img);

/* Create an image with a WebP file. */
bool image_create_with_webp(const uint8_t *data, size_t size, struct image **img);

/* Destroy an image. */
void image_destroy(struct image *img);

/* Get an image width. */
int image_get_width(struct image *img);

/* Get an image height. */
int image_get_height(struct image *img);

/* Get image pixels. */
pixel_t *image_get_pixels(struct image *img);

/* Clear an image with a uniform color. */
void image_clear(struct image *img, pixel_t color);

/* Clear an image rectangle with a uniform color. */
void image_clear_rect(struct image *img, int x, int y, int w, int h,
		      pixel_t color);

/* Draw an image on an image. (copy) */
void image_draw_copy(struct image *dst_image, int dst_left, int dst_top,
		     struct image *src_image, int width, int height,
		     int src_left, int src_top);

/* Draw an image on an image. (alpha-blending, dst_alpha=255) */
void image_draw_alpha(struct image *dst_image, int dst_left, int dst_top,
		      struct image *src_image, int width, int height,
		      int src_left, int src_top, int alpha);

/* Draw an image on an image. (add-blending) */
void image_draw_add(struct image *dst_image, int dst_left, int dst_top,
		    struct image *src_image, int width, int height,
		    int src_left, int src_top, int alpha);

/* Draw an image on an image. (add-blending) */
void image_draw_sub(struct image *dst_image, int dst_left, int dst_top,
		    struct image *src_image, int width, int height,
		    int src_left, int src_top, int alpha);

/* Clip a rectangle by a source size. */
bool image_clip_by_source(int src_cx,
			  int src_cy,
			  int *cx,
			  int *cy,
			  int *dst_x,
			  int *dst_y,
			  int *src_x,
			  int *src_y);

/* Clip a rectangle by a destination size. */
bool image_clip_by_dest(int dst_cx,
			int dst_cy,
			int *cx,
			int *cy,
			int *dst_x,
			int *dst_y,
			int *src_x,
			int *src_y);

#endif
