/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * glrender.c: The OpenGL implementation for the "render" module interface.
 */

#include "hal/hal.h"
#include "hal/glrender.h"

/* Linux (OpenGL 3.2) */
#if defined(TARGET_LINUX)
#include <GL/gl.h>
#include "glhelper.h"
#endif

/* Android (OpenGL ES 3.0) */
#if defined(TARGET_ANDROID)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/* Emscripten (OpenGL ES 3.0) */
#if defined(TARGET_WASM)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * False assertion
 */

#define NEVER_COME_HERE		0

/*
 * Screen size.
 */

static int conf_game_width;
static int conf_game_height;

/*
 * Program
 */ 

#define PIPELINE_MAX	128
#define NAME_MAX	128
#define VARIABLE_MAX	32
#define TEXT_MAX	32768

struct render_pipeline {
	bool is_used;

	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vao;

	char vertex_shader_src[TEXT_MAX];
	char fragment_shader_src[TEXT_MAX];

	struct uniform {
		char type[NAME_MAX];
		char name[NAME_MAX];
		char note[NAME_MAX];
		bool is_sampler;
	} uniform[VARIABLE_MAX];
	int uniform_count;

	struct attribute {
		char type[NAME_MAX];
		char name[NAME_MAX];
		char note[NAME_MAX];
		int size;
	} attribute[VARIABLE_MAX];
	int attribute_count;
	int attribute_size;

	struct varying {
		char type[NAME_MAX];
		char name[NAME_MAX];
		char alt[NAME_MAX];
		char note[NAME_MAX];
	} varying[VARIABLE_MAX];
	int varying_count;
};

/* Pipelines. */
static struct render_pipeline render_pipeline[PIPELINE_MAX];

/* Pipline construction cursor. */
static int render_pipeline_cursor;

/* A binded pipeline. */
static struct render_pipeline *render_binded_pipeline;

/* Helpers to construct shader strings. */
#define STRNCPY(s1, s2)	strncpy((s1), s2, sizeof(s1) - 1)
#define STRNCAT(s1, s2)	strncat((s1), s2, sizeof(s1) - 1)

/*
 * Vertex Buffer
 */

struct render_vertex_buffer {
	bool is_used;
	GLuint buf;
	size_t size;
};

#define VERTEX_BUFFER_MAX	1024

static struct render_vertex_buffer render_vertex_buffer[VERTEX_BUFFER_MAX];

/*
 * Index Buffer
 */

struct render_index_buffer {
	bool is_used;
	GLuint buf;
	size_t size;
};

#define INDEX_BUFFER_MAX	1024

static struct render_index_buffer render_index_buffer[INDEX_BUFFER_MAX];

/*
 * Constant Buffer
 */

struct render_constant_buffer {
	bool is_used;
	GLuint buf;
	size_t size;
};

#define CONSTANT_BUFFER_MAX	1024

static struct render_constant_buffer render_constant_buffer[CONSTANT_BUFFER_MAX];

/*
 * Texture
 */

struct render_texture {
	bool is_used;
	GLuint tex;
	GLuint width;
	GLuint height;
};

#define TEXTURE_MAX	1024

static int render_texture_count;
static struct render_texture render_texture[TEXTURE_MAX];

/*
 * Re-initialization
 */

/* Indicates if the first rendering after re-init. */
static bool is_after_reinit;

/* Re-init count. */
static int reinit_count;

/*
 * Forward declaration
 */

static bool render_compile_vertex_shader(void);
static bool render_compile_fragment_shader(void);
static bool render_create_program(void);
static void render_setup_attributes(struct render_pipeline *p);
static void render_setup_samplers(struct render_pipeline *p);
static const char *render_translate_type(const char *type);

/*
 * Initialize the glrender module.
 */
bool glrender_init(int x, int y, int w, int h)
{
	glrender_cleanup();

	/* Set a viewport. */
	glViewport(x, y, w, h);

	/* Set a reinit state. */
	is_after_reinit = true;
	reinit_count++;

	return true;
}

/*
 * Cleanup the glreder module.
 */
void glrender_cleanup(void)
{
	int i;

	/* Delete programs. */
	for (i = 0; i < PIPELINE_MAX; i++) {
		glDeleteProgram(render_pipeline[i].program);
		glDeleteShader(render_pipeline[i].vertex_shader);
		glDeleteShader(render_pipeline[i].fragment_shader);
		glDeleteVertexArrays(1, &render_pipeline[i].vao);
		memset(&render_pipeline[i], 0, sizeof(struct render_pipeline));
	}

	/* Delete vertex buffers. */
	for (i = 0; i < VERTEX_BUFFER_MAX; i++) {
		glDeleteBuffers(1, &render_vertex_buffer[i].buf);
		memset(&render_vertex_buffer[i], 0, sizeof(struct render_vertex_buffer));
	}

	/* Delete index buffers. */
	for (i = 0; i < INDEX_BUFFER_MAX; i++) {
		glDeleteBuffers(1, &render_index_buffer[i].buf);
		memset(&render_index_buffer[i], 0, sizeof(struct render_index_buffer));
	}

	/* Delete constant buffers. */
	for (i = 0; i < CONSTANT_BUFFER_MAX; i++) {
		glDeleteBuffers(1, &render_constant_buffer[i].buf);
		memset(&render_constant_buffer[i], 0, sizeof(struct render_constant_buffer));
	}
}

/*
 * Define a pipeline.
 */

bool render_begin_pipeline(void)
{
	int index, i;

	/* Allocate a struct. */
	index = -1;
	for (i = 0; i < PIPELINE_MAX; i++) {
		if (!render_pipeline[i].is_used) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		sys_error("Too many pipelines.");
		return false;
	}

	/* Clear a struct. */
	memset(&render_pipeline[index], 0, sizeof(struct render_pipeline));

	/* Start a vertex shader. */
	STRNCPY(render_pipeline[index].vertex_shader_src, "#version 100\n");
	STRNCAT(render_pipeline[index].vertex_shader_src, "precision mediump float;\n");

	/* Start a fragment shader. */
	STRNCPY(render_pipeline[index].fragment_shader_src, "#version 100\n");
	STRNCAT(render_pipeline[index].fragment_shader_src, "precision mediump float;\n");

	render_pipeline_cursor = index;

	return true;
}

bool render_end_pipeline(struct render_pipeline **pipeline)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	/* Compile a vertex shader. */
	if (!render_compile_vertex_shader())
		return false;

	/* Compile a fragment shader. */
	if (!render_compile_fragment_shader())
		return false;

	/* Create a program. */
	if (!render_create_program())
		return false;

	/* Setup sampler locations. */
	render_setup_samplers(p);

	*pipeline = p;

	return true;
}

static void render_setup_samplers(struct render_pipeline *p)
{
	int sampler_count;
	int sampler_loc;
	int i;

	sampler_count = 0;
	for (i = 0; i < p->uniform_count; i++) {
		if (p->uniform[i].is_sampler) {
			sampler_loc = glGetUniformLocation(p->program, p->uniform[i].name);
			glUniform1i(sampler_loc, sampler_count);
			printf("sampler %d:%s:%d\n", sampler_loc, p->uniform[i].name, sampler_count);
			sampler_count++;
		}
	}
}

static bool render_compile_vertex_shader(void)
{
	struct render_pipeline *p;
	char buf[1024];
	const char *src;
	int len;
	GLuint is_succeeded;

	p = &render_pipeline[render_pipeline_cursor];

	p->vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	src = p->vertex_shader_src;
	glShaderSource(p->vertex_shader, 1, &src, NULL);
	glCompileShader(p->vertex_shader);

	glGetShaderiv(p->vertex_shader, GL_COMPILE_STATUS, &is_succeeded);
	if (!is_succeeded) {
		sys_error("Failed to compile a vertex shader.");
		glGetShaderInfoLog(p->vertex_shader, sizeof(buf), &len, &buf[0]);
		sys_error("%s", buf);
		return false;
	}

	printf("===\n%s\n", p->vertex_shader_src);

	return true;
}

static bool render_compile_fragment_shader(void)
{
	struct render_pipeline *p;
	char buf[1024];
	const char *src;
	int len;
	GLuint is_succeeded;

	p = &render_pipeline[render_pipeline_cursor];

	p->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	src = p->fragment_shader_src;
	glShaderSource(p->fragment_shader, 1, &src, NULL);
	glCompileShader(p->fragment_shader);

	glGetShaderiv(p->fragment_shader, GL_COMPILE_STATUS, &is_succeeded);
	if (!is_succeeded) {
		sys_error("Failed to compile a fragment shader.");
		glGetShaderInfoLog(p->fragment_shader, sizeof(buf), &len, &buf[0]);
		sys_error("%s", buf);
		return false;
	}

	printf("===\n%s\n", p->fragment_shader_src);

	return true;
}

static bool render_create_program(void)
{
	struct render_pipeline *p;
	char buf[1024];
	int len;
	GLuint is_succeeded;

	p = &render_pipeline[render_pipeline_cursor];

	p->program = glCreateProgram();
	glAttachShader(p->program, p->vertex_shader);
	glAttachShader(p->program, p->fragment_shader);
	glLinkProgram(p->program);

	glGetProgramiv(p->program, GL_LINK_STATUS, &is_succeeded);
	if (!is_succeeded) {
		sys_error("Failed to link a program.\n");
		glGetProgramInfoLog(p->program, sizeof(buf), &len, &buf[0]);
		sys_error("%s", buf);
		return false;
	}
	glUseProgram(p->program);

	glGenVertexArrays(1, &p->vao);
	glBindVertexArray(p->vao);

	return true;
}

bool render_begin_constant(void)
{
	return true;
}

bool render_add_constant(const char *type, const char *name, const char *note)
{
	struct render_pipeline *p;
	const char *ttype;
	int index;

	/* Alloc a struct. */
	p = &render_pipeline[render_pipeline_cursor];
	if (p->uniform_count >= VARIABLE_MAX) {
		sys_error("Too many constants.");
		return false;
	}
	index = p->uniform_count;
	p->uniform_count++;

	/* Translate a type name to GLSL style. */
	ttype = render_translate_type(type);
	if (ttype == NULL)
		return false;

	/* Copy a type and a name. */
	STRNCPY(p->uniform[index].type, ttype);
	STRNCPY(p->uniform[index].name, name);
	STRNCPY(p->uniform[index].note, note);
	p->uniform[index].is_sampler = false;

	/* Add a definition to the vertex shader source. */
	STRNCAT(p->vertex_shader_src, "uniform ");
	STRNCAT(p->vertex_shader_src, ttype);
	STRNCAT(p->vertex_shader_src, " ");
	STRNCAT(p->vertex_shader_src, name);
	STRNCAT(p->vertex_shader_src, ";\n");

	/* Add a definition to the fragment shader source. */
	STRNCAT(p->fragment_shader_src, "uniform ");
	STRNCAT(p->fragment_shader_src, ttype);
	STRNCAT(p->fragment_shader_src, " ");
	STRNCAT(p->fragment_shader_src, name);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_end_constant(void)
{
	return true;
}

bool render_begin_sampler(void)
{
	return true;
}

bool render_add_sampler(const char *name, const char *note)
{
	struct render_pipeline *p;
	const char *ttype;
	int index;

	/* Alloc a struct. */
	p = &render_pipeline[render_pipeline_cursor];
	if (p->uniform_count >= VARIABLE_MAX) {
		sys_error("Too many samplers.");
		return false;
	}
	index = p->uniform_count;
	p->uniform_count++;

	/* Copy a type and a name. */
	STRNCPY(p->uniform[index].type, "sampler2D");
	STRNCPY(p->uniform[index].name, name);
	STRNCPY(p->uniform[index].note, note);
	p->uniform[index].is_sampler = true;

	/* Add a definition to the fragment shader source. */
	STRNCAT(p->fragment_shader_src, "uniform ");
	STRNCAT(p->fragment_shader_src, "sampler2D");
	STRNCAT(p->fragment_shader_src, " ");
	STRNCAT(p->fragment_shader_src, name);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_end_sampler(void)
{
	return true;
}

bool render_begin_vertex_shader_input(void)
{
	return true;
}

bool render_add_vertex_shader_input(const char *type, const char *name, const char *note)
{
	struct render_pipeline *p;
	const char *ttype;
	int index;
	int size;

	/* Alloc a struct. */
	p = &render_pipeline[render_pipeline_cursor];
	if (p->attribute_count >= VARIABLE_MAX) {
		sys_error("Too many inputs.");
		return false;
	}
	index = p->attribute_count;
	p->attribute_count++;

	/* Translate a type name to GLSL style. */
	ttype = render_translate_type(type);
	if (ttype == NULL)
		return false;

	/* Get a size. */
	size = 0;
	if (strcmp(ttype, "float") == 0)
		p->attribute[index].size = 1;
	else if (strcmp(ttype, "vec2") == 0)
		p->attribute[index].size = 2;
	else if (strcmp(ttype, "vec3") == 0)
		p->attribute[index].size = 3;
	else if (strcmp(ttype, "vec4") == 0)
		p->attribute[index].size = 4;
	else if (strcmp(ttype, "mat2") == 0)
		p->attribute[index].size = 4;
	else if (strcmp(ttype, "mat3") == 0)
		p->attribute[index].size = 9;
	else if (strcmp(ttype, "mat16") == 0)
		p->attribute[index].size = 16;
	else
		assert(NEVER_COME_HERE);
	p->attribute_size += p->attribute[index].size;

	/* Copy a type and a name. */
	STRNCPY(p->attribute[index].type, ttype);
	STRNCPY(p->attribute[index].name, name);
	STRNCPY(p->attribute[index].note, note);

	/* Add a definition to the vertex shader source. */
	STRNCAT(p->vertex_shader_src, "attribute ");
	STRNCAT(p->vertex_shader_src, ttype);
	STRNCAT(p->vertex_shader_src, " ");
	STRNCAT(p->vertex_shader_src, name);
	STRNCAT(p->vertex_shader_src, ";\n");

	return true;
}

bool render_end_vertex_shader_input(void)
{
	return true;
}

bool render_begin_pixel_shader_input(void)
{
	return true;
}

bool render_add_pixel_shader_input(const char *type, const char *name, const char *note)
{
	struct render_pipeline *p;
	const char *ttype;
	int index;

	/* Alloc a struct. */
	p = &render_pipeline[render_pipeline_cursor];
	if (p->varying_count >= VARIABLE_MAX) {
		sys_error("Too many outputs.");
		return false;
	}
	index = p->varying_count;
	p->varying_count++;

	/* Translate a type name to GLSL style. */
	ttype = render_translate_type(type);
	if (ttype == NULL)
		return false;

	/* Copy a type and a name. */
	STRNCPY(p->varying[index].type, ttype);
	STRNCPY(p->varying[index].name, name);
	STRNCPY(p->varying[index].note, note);

	/* If this is a SV_POSITION, don't put to the shader source. */
	if (strcmp(note, RENDER_SVPOSITION) == 0)
		return true;

	/* Add a definition to the fragment shader source. */
	STRNCAT(p->vertex_shader_src, "varying ");
	STRNCAT(p->vertex_shader_src, ttype);
	STRNCAT(p->vertex_shader_src, " ");
	STRNCAT(p->vertex_shader_src, name);
	STRNCAT(p->vertex_shader_src, ";\n");

	/* Add a definition to the fragment shader source. */
	STRNCAT(p->fragment_shader_src, "varying ");
	STRNCAT(p->fragment_shader_src, ttype);
	STRNCAT(p->fragment_shader_src, " ");
	STRNCAT(p->fragment_shader_src, name);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_end_pixel_shader_input(void)
{
	return true;
}

bool render_begin_vertex_shader(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->vertex_shader_src, "void main() {\n");
	return true;
}

bool render_vertex_shader_assign_constant(const char *type, const char *lhs_var, const char *rhs_var)
{
	struct render_pipeline *p;
	int i, index;

	p = &render_pipeline[render_pipeline_cursor];

	/* Put a type if specified. */
	if (type != NULL) {
		const char *ttype;
		ttype = render_translate_type(type);
		if (ttype == NULL)
			return false;
		STRNCAT(p->vertex_shader_src, ttype);
		STRNCAT(p->vertex_shader_src, " ");
	}

	/* Put an assign statement */
	STRNCAT(p->vertex_shader_src, lhs_var);
	STRNCAT(p->vertex_shader_src, " = ");
	STRNCAT(p->vertex_shader_src, rhs_var);
	STRNCAT(p->vertex_shader_src, ";\n");

	return true;
}

bool render_vertex_shader_assign_input(const char *type, const char *lhs_var, const char *rhs_var)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	/* Put a type if specified. */
	if (type != NULL) {
		const char *ttype;
		ttype = render_translate_type(type);
		if (ttype == NULL)
			return false;
		STRNCAT(p->vertex_shader_src, ttype);
		STRNCAT(p->vertex_shader_src, " ");
	}

	/* Put an assign statement */
	STRNCAT(p->vertex_shader_src, lhs_var);
	STRNCAT(p->vertex_shader_src, " = ");
	STRNCAT(p->vertex_shader_src, rhs_var);
	STRNCAT(p->vertex_shader_src, ";\n");

	return true;
}

bool render_vertex_shader_assign_tmp(const char *type, const char *lhs_var, const char *rhs_expr)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	/* Put a type if specified. */
	if (type != NULL) {
		const char *ttype;
		ttype = render_translate_type(type);
		if (ttype == NULL)
			return false;
		STRNCAT(p->vertex_shader_src, ttype);
		STRNCAT(p->vertex_shader_src, " ");
	}

	/* Put an assign statement */
	STRNCAT(p->vertex_shader_src, lhs_var);
	STRNCAT(p->vertex_shader_src, " = ");
	STRNCAT(p->vertex_shader_src, rhs_expr);
	STRNCAT(p->vertex_shader_src, ";\n");

	return true;
}

bool render_vertex_shader_assign_output(const char *lhs_var, const char *rhs_expr)
{
	struct render_pipeline *p;
	int i;
	bool is_svposition;

	p = &render_pipeline[render_pipeline_cursor];

	/* Search a pixel shader input. */
	is_svposition = false;
	for (i = 0; i < VARIABLE_MAX; i++) {
		if (strcmp(p->varying[i].name, lhs_var) == 0) {
			if (strcmp(p->varying[i].note, RENDER_SVPOSITION) == 0)
				is_svposition = true;
			break;
		}
	}

	/* put an assign statement */
	if (is_svposition)
		STRNCAT(p->vertex_shader_src, "gl_Position");
	else
		STRNCAT(p->vertex_shader_src, lhs_var);
	STRNCAT(p->vertex_shader_src, " = ");
	STRNCAT(p->vertex_shader_src, rhs_expr);
	STRNCAT(p->vertex_shader_src, ";\n");

	return true;
}

bool render_vertex_shader_begin_if(const char *cond)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->vertex_shader_src, "if (");
	STRNCAT(p->vertex_shader_src, cond);
	STRNCAT(p->vertex_shader_src, ") {\n");

	return true;
}

bool render_vertex_shader_begin_else_if(const char *cond)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->vertex_shader_src, "} else if (");
	STRNCAT(p->vertex_shader_src, cond);
	STRNCAT(p->vertex_shader_src, ") {\n");

	return true;
}

bool render_vertex_shader_begin_else(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->vertex_shader_src, "} else {\n");

	return true;
}

bool render_vertex_shader_end_if(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->vertex_shader_src, "}\n");

	return true;
}

bool render_end_vertex_shader(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->vertex_shader_src, "}\n");

	return true;
}

bool render_begin_pixel_shader(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "void main() {\n");

	return true;
}

bool render_pixel_shader_assign_constant(const char *type, const char *lhs_var, const char *rhs_var)
{
	struct render_pipeline *p;
	int i, index;

	p = &render_pipeline[render_pipeline_cursor];

	/* Put a type if specified. */
	if (type != NULL) {
		const char *ttype;
		ttype = render_translate_type(type);
		if (ttype == NULL)
			return false;
		STRNCAT(p->fragment_shader_src, ttype);
		STRNCAT(p->fragment_shader_src, " ");
	}

	/* Put an assign statement */
	STRNCAT(p->fragment_shader_src, lhs_var);
	STRNCAT(p->fragment_shader_src, " = ");
	STRNCAT(p->fragment_shader_src, rhs_var);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_pixel_shader_assign_input(const char *type, const char *lhs_var, const char *rhs_var)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	/* Put a type if specified. */
	if (type != NULL) {
		const char *ttype;
		ttype = render_translate_type(type);
		if (ttype == NULL)
			return false;
		STRNCAT(p->fragment_shader_src, ttype);
		STRNCAT(p->fragment_shader_src, " ");
	}

	/* Put an assign statement */
	STRNCAT(p->fragment_shader_src, lhs_var);
	STRNCAT(p->fragment_shader_src, " = ");
	STRNCAT(p->fragment_shader_src, rhs_var);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_pixel_shader_assign_tmp(const char *type, const char *lhs_var, const char *rhs_expr)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	/* Put a type if specified. */
	if (type != NULL) {
		const char *ttype;
		ttype = render_translate_type(type);
		if (ttype == NULL)
			return false;
		STRNCAT(p->fragment_shader_src, ttype);
		STRNCAT(p->fragment_shader_src, " ");
	}

	/* Put an assign statement */
	STRNCAT(p->fragment_shader_src, lhs_var);
	STRNCAT(p->fragment_shader_src, " = ");
	STRNCAT(p->fragment_shader_src, rhs_expr);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_pixel_shader_return(const char *expr)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "gl_FragColor = ");
	STRNCAT(p->fragment_shader_src, expr);
	STRNCAT(p->fragment_shader_src, ";\n");

	return true;
}

bool render_pixel_shader_begin_if(const char *cond)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "if (");
	STRNCAT(p->fragment_shader_src, cond);
	STRNCAT(p->fragment_shader_src, ") {\n");

	return true;
}

bool render_pixel_shader_begin_else_if(const char *cond)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "} else if (");
	STRNCAT(p->fragment_shader_src, cond);
	STRNCAT(p->fragment_shader_src, ") {\n");

	return true;
}

bool render_pixel_shader_begin_else(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "} else {\n");

	return true;
}

bool render_pixel_shader_end_if(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "}\n");

	return true;
}

bool render_end_pixel_shader(void)
{
	struct render_pipeline *p;

	p = &render_pipeline[render_pipeline_cursor];

	STRNCAT(p->fragment_shader_src, "}\n");

	return true;
}

const char *render_translate_type(const char *type)
{
	return type;
}

/*
 * Bind a program.
 */
void render_bind_pipeline(struct render_pipeline *pipeline)
{
	render_binded_pipeline = pipeline;

	glUseProgram(pipeline->program);
	glBindVertexArray(pipeline->vao);
}

/*
 * Create a vertex buffer.
 */
bool render_create_vertex_buffer(int size, struct render_vertex_buffer **buf)
{
	int index, i;

	assert(buf != NULL);

	/* Allocate a struct. */
	index = -1;
	for (i = 0; i < VERTEX_BUFFER_MAX; i++) {
		if (!render_vertex_buffer[i].is_used) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		sys_error("Too many vertex buffers.");
		return false;
	}

	*buf = &render_vertex_buffer[index];
	(*buf)->is_used = true;
	(*buf)->size = size;

	/* Create a VBO. */
	glGenBuffers(1, &(*buf)->buf);

	return true;
}

/*
 * Bind a vertex buffer.
 */
void render_bind_vertex_buffer(struct render_vertex_buffer *buf)
{
	assert(buf != NULL);

	glBindBuffer(GL_ARRAY_BUFFER, buf->buf);

	if (render_binded_pipeline != NULL)
		render_setup_attributes(render_binded_pipeline);
}

static void render_setup_attributes(struct render_pipeline *p)
{
	int attr_ofs;
	int attr_loc;
	int i;

	attr_ofs = 0;
	for (i = 0; i < p->attribute_count; i++) {
		attr_loc = glGetAttribLocation(p->program, p->attribute[i].name);
		glVertexAttribPointer((GLuint)attr_loc,
				      p->attribute[i].size,
				      GL_FLOAT,
				      GL_FALSE,
				      p->attribute_size * sizeof(GLfloat),
				      (const GLvoid *)(attr_ofs * sizeof(GLfloat)));
		glEnableVertexAttribArray((GLuint)attr_loc);
		attr_ofs += p->attribute[i].size;
	}
}

/*
 * Upload data to a vertex buffer.
 */
void render_upload_vertex_buffer(struct render_vertex_buffer *buf, const float *src)
{
	assert(buf != NULL);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * buf->size, src, GL_STATIC_DRAW);
}

/*
 * Destroy a vertex buffer.
 */
void render_destroy_vertex_buffer(struct render_vertex_buffer *buf)
{
	assert(buf != NULL);

	glDeleteBuffers(1, (const GLuint *)&buf->buf);

	buf->is_used = false;
	buf->buf = -1;
}

/*
 * Create an index buffer.
 */
bool render_create_index_buffer(int size, struct render_index_buffer **buf)
{
	int index, i;

	assert(buf != NULL);

	/* Allocate a struct. */
	index = -1;
	for (i = 0; i < INDEX_BUFFER_MAX; i++) {
		if (!render_index_buffer[i].is_used) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		sys_error("Too many index buffers.");
		return false;
	}

	*buf = &render_index_buffer[index];
	(*buf)->is_used = true;
	(*buf)->size = size;

	/* Create an IBO. */
	glGenBuffers(1, &(*buf)->buf);

	return true;
}

/*
 * Bind an index buffer.
 */
void render_bind_index_buffer(struct render_index_buffer *buf)
{
	assert(buf != NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->buf);
}

/*
 * Upload data to a vertex buffer.
 */
void render_upload_index_buffer(struct render_index_buffer *buf, const short *src)
{
	assert(buf != NULL);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * buf->size, src, GL_STATIC_DRAW);
}

/*
 * Destroy an index buffer.
 */
void render_destroy_index_buffer(struct render_index_buffer *buf)
{
	assert(buf != NULL);

	glDeleteBuffers(1, (const GLuint *)&buf->buf);

	buf->is_used = false;
	buf->buf = -1;
}

/*
 * Update a pipeline constant.
 */
bool render_update_constant(struct render_pipeline *pipeline, const char *name, void *src)
{
	int i, index, location;

	/* Search an item by a name. */
	index = -1;
	for (i = 0; i < pipeline->uniform_count; i++) {
		if (strcmp(pipeline->uniform[i].name, name) == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		sys_error("Cannot find a constant \"%s\".", name);
		return false;
	}

	/* Update by the type. */
	location = glGetUniformLocation(pipeline->program, pipeline->uniform[i].name);
	if (strcmp(pipeline->uniform[index].type, "float") == 0)
		glUniform1f(location, 1, GL_FALSE, (float *)src);
	else if (strcmp(pipeline->uniform[index].type, "vec2") == 0)
		glUniform2fv(location, 1, GL_FALSE, (float *)src);
	else if (strcmp(pipeline->uniform[index].type, "vec3") == 0)
		glUniform3fv(location, 1, GL_FALSE, (float *)src);
	else if (strcmp(pipeline->uniform[index].type, "vec4") == 0)
		glUniform4fv(location, 1, GL_FALSE, (float *)src);
	else if (strcmp(pipeline->uniform[index].type, "mat4") == 0)
		glUniformMatrix4fv(location, 1, GL_FALSE, (float *)src);
	else
		assert(NEVER_COME_HERE);

	return true;
}

/*
 * Create a texture.
 */
bool render_create_texture(int width, int height, int miplevel, struct render_texture **tex)
{
	int index, i;

	/* Allocate a struct. */
	index = -1;
	for (i = 0; i < TEXTURE_MAX; i++) {
		if (!render_texture[i].is_used) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		sys_error("Too many textures.");
		return false;
	}

	/* Create a texture. */
	glGenTextures(1, &render_texture[i].tex);

	return true;
}

/*
 * Bind a texture.
 */
void render_bind_texture(int index, struct render_texture *tex)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, tex->tex);
}

/*
 * Upload pixels to a texture.
 */
void render_upload_texture(struct render_texture *tex, int miplevel, struct image *img)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, tex->tex);
#ifdef TARGET_WASM
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,
		     0,
		     GL_RGBA,
		     image_get_width(img),
		     image_get_height(img),
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     image_get_pixels(img));
	glActiveTexture(GL_TEXTURE0);
}

/*
 * Start a frame.
 */
void render_begin_frame(void)
{
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
}

/*
 * Finish a frame.
 */
void render_end_frame(void)
{
	glFlush();
	is_after_reinit = false;
}

/*
 * Draw triangles.
 */
void render_draw_triangle_strip(int offset, int count)
{
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_SHORT, 0);
}
