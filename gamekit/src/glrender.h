/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2024, 2025, The Horizon Authors. All rights reserved.
 */

/*
 * glrender.h: The OpenGL implementation for "render_" component.
 */

#ifndef GAMEKIT_GLRENDER_H
#define GAMEKIT_GLRENDER_H

#include "compat.h"

/* Initialize the glrender module. */
bool glrender_init(int x, int y, int w, int h);

/* Cleanup the glrender module. */
void glrender_cleanup(void);

/* Reisze the viewport. */
void glrender_resize(int x, int y, int w, int h);

/*
 * Appendix
 */

#include <GL/gl.h>

/*
 * Define some macros for OpenGL 2+ and OpenGL ES 2+ if missing.
 */

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE	0x812F
#endif

#ifndef GL_TEXTURE0
#define GL_TEXTURE0		0x84C0
#endif

#ifndef GL_TEXTURE1
#define GL_TEXTURE1		0x84C1
#endif

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER		0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER		0x8893
#endif

#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW		0x88E4
#endif

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER	0x8B30
#endif

#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS		0x8B82
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER	0x8B31
#endif

#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS	0x8B81
#endif

/*
 * Define missing typedefs if glext.h is not included.
 */

#ifndef __gl_glext_h_
typedef char GLchar;
typedef ssize_t GLsizeiptr;
#endif

/*
 * OpenGL 2+ API symbols.
 */
#if defined(TARGET_LINUX)
extern GLuint (APIENTRY *glCreateShader)(GLenum type);
extern void (APIENTRY *glShaderSource)(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
extern void (APIENTRY *glCompileShader)(GLuint shader);
extern void (APIENTRY *glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
extern void (APIENTRY *glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (APIENTRY *glAttachShader)(GLuint program, GLuint shader);
extern void (APIENTRY *glLinkProgram)(GLuint program);
extern void (APIENTRY *glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
extern void (APIENTRY *glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern GLuint (APIENTRY *glCreateProgram)(void);
extern void (APIENTRY *glUseProgram)(GLuint program);
extern void (APIENTRY *glGenVertexArrays)(GLsizei n, GLuint *arrays);
extern void (APIENTRY *glBindVertexArray)(GLuint array);
extern void (APIENTRY *glGenBuffers)(GLsizei n, GLuint *buffers);
extern void (APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
extern GLint (APIENTRY *glGetAttribLocation)(GLuint program, const GLchar *name);
extern void (APIENTRY *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (APIENTRY *glEnableVertexAttribArray)(GLuint index);
extern GLint (APIENTRY *glGetUniformLocation)(GLuint program, const GLchar *name);
extern void (APIENTRY *glUniform1i)(GLint location, GLint v0);
extern void (APIENTRY *glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void (APIENTRY *glDeleteShader)(GLuint shader);
extern void (APIENTRY *glDeleteProgram)(GLuint program);
extern void (APIENTRY *glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
extern void (APIENTRY *glDeleteBuffers)(GLsizei n, const GLuint *buffers);
#endif

#endif
