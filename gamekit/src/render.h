/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * render.h: "render_" component interface.
 */

#ifndef GAMEKIT_RENDER_H
#define GAMEKIT_RENDER_H

#include "compat.h"
#include "image.h"

struct render_pipeline;
struct render_vertex_buffer;
struct render_index_buffer;
struct render_constant_buffer;
struct render_texture;

/*
 * Pipeline
 */

/*
 * While we have our own shader language, it is separated from this
 * "render" module.  The shader language is implemented in another
 * module, "shader", and the "render" module implements only a few
 * functions to assemble platform's shader program.  This separation
 * mechanism will reduce code changes of the "render" module when our
 * shader language is updated in the future.
 */

#define RENDER_FLOAT		"float"
#define RENDER_VEC2		"vec2"
#define RENDER_VEC3		"vec3"
#define RENDER_VEC4		"vec4"
#define RENDER_MAT2		"mat2"
#define RENDER_MAT3		"mat3"
#define RENDER_MAT4		"mat4"

#define RENDER_POSITION0	"position0"
#define RENDER_POSITION1	"position1"
#define RENDER_POSITION2	"position2"
#define RENDER_SVPOSITION	"sv_position"
#define RENDER_TEXCOORD0	"texcoord0"
#define RENDER_TEXCOORD1	"texcoord1"
#define RENDER_TEXCOORD2	"texcoord2"
#define RENDER_COLOR0		"color0"
#define RENDER_COLOR1		"color1"
#define RENDER_COLOR2		"color2"
#define RENDER_NORMAL0		"normal0"
#define RENDER_NORMAL1		"normal1"
#define RENDER_NORMAL2		"normal2"

struct render_pipeline;

/*
 * Define a pipeline.
 */

bool render_begin_pipeline(void);
bool render_end_pipeline(struct render_pipeline **pipeline);

bool render_begin_constant(void);
bool render_add_constant(const char *type, const char *name, const char *note);
bool render_end_constant(void);

bool render_begin_sampler(void);
bool render_add_sampler(const char *name, const char *note);
bool render_end_sampler(void);

bool render_begin_vertex_shader_input(void);
bool render_add_vertex_shader_input(const char *type, const char *name, const char *note);
bool render_end_vertex_shader_input(void);

bool render_begin_pixel_shader_input(void);
bool render_add_pixel_shader_input(const char *type, const char *name, const char *note);
bool render_end_pixel_shader_input(void);

bool render_begin_vertex_shader(void);
bool render_vertex_shader_assign_constant(const char *type, const char *lhs_var, const char *rhs_var);
bool render_vertex_shader_assign_input(const char *type, const char *lhs_var, const char *rhs_var);
bool render_vertex_shader_assign_tmp(const char *type, const char *lhs_var, const char *rhs_expr);
bool render_vertex_shader_assign_output(const char *lhs_var, const char *rhs_expr);
bool render_vertex_shader_begin_if(const char *cond);
bool render_vertex_shader_begin_else_if(const char *cond);
bool render_vertex_shader_begin_else(void);
bool render_vertex_shader_end_if(void);
bool render_end_vertex_shader(void);

bool render_begin_pixel_shader(void);
bool render_pixel_shader_assign_constant(const char *type, const char *lhs_var, const char *rhs_var);
bool render_pixel_shader_assign_input(const char *type, const char *lhs_var, const char *rhs_var);
bool render_pixel_shader_assign_tmp(const char *type, const char *lhs, const char *rhs);
bool render_pixel_shader_return(const char *expr);
bool render_pixel_shader_begin_if(const char *cond);
bool render_pixel_shader_begin_else_if(const char *cond);
bool render_pixel_shader_begin_else(void);
bool render_pixel_shader_end_if(void);
bool render_end_pixel_shader(void);

/* Destroy a pipeline. */
void render_destroy_pipeline(struct render_pipeline *pipeline);

/* Bind a program. */
void render_bind_pipeline(struct render_pipeline *pipeline);

/*
 * Texture
 */

/* Create a texture. */
bool render_create_texture(int width, int height, int miplevel, struct render_texture **tex);

/* Destroy a texture. */
void render_destroy_texture(struct render_texture *tex);

/* Upload pixels to a texture. */
void render_upload_texture(struct render_texture *tex, int miplevel, struct image *img);

/*
 * Vertex Buffer
 */

/* Create a vertex buffer. */
bool render_create_vertex_buffer(int size, struct render_vertex_buffer **buf);

/* Destroy a vertex buffer. */
void render_destroy_vertex_buffer(struct render_vertex_buffer *buf);

/* Upload data to a vertex buffer. */
void render_upload_vertex_buffer(struct render_vertex_buffer *buf, const float *src);

/*
 * Index Buffer
 */

/* Create an index buffer. */
bool render_create_index_buffer(int size, struct render_index_buffer **buf);

/* Destroy an index buffer. */
void render_destroy_index_buffer(struct render_index_buffer *buf);

/* Copy data to an index buffer. */
void render_upload_index_buffer(struct render_index_buffer *buf, const short *src);

/*
 * Constant
 */

/* Update a pipeline constant. */
bool render_update_constant(struct render_pipeline *pipeline, struct render_constant_buffer *buf, const char *name, void *src);

/*
 * Rendering
 */

/* Start a frame. */
void render_begin_frame(void);

/* Finish a frame. */
void render_end_frame(void);

/* Bind a vertex buffer. */
void render_bind_vertex_buffer(struct render_vertex_buffer *buf);

/* Bind an index buffer. */
void render_bind_index_buffer(struct render_index_buffer *buf);

/* Bind an index buffer. */
void render_bind_constant_buffer(struct render_constant_buffer *buf);

/* Bind a texture. */
void render_bind_texture(int index, struct render_texture *tex);

/* Draw triangles. */
void render_draw_triangle_strip(int offset, int count);

#endif
