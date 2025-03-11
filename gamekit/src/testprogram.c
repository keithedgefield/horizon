/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * testmain.c: Test program.
 */

#include "gamekit/gamekit.h"

static struct render_pipeline *pipeline;
static struct render_vertex_buffer *vertex_buffer;
static struct render_index_buffer *index_buffer;

/*
 * Called after the "file" initializain and before the "render" initialization.
 */
bool on_hal_init_render(char **title, int *width, int *height)
{
	*title = strdup("Example");
	*width = 640;
	*height = 480;

	return true;
}

/*
 * Called after the whole HAL initialization and before the game loop.
 */
bool on_hal_ready(void)
{
	render_begin_pipeline();

	render_add_vertex_shader_input("vec3", "a_pos", RENDER_POSITION0);
	render_add_pixel_shader_input("vec4", "v_pos", RENDER_SVPOSITION);

	render_begin_vertex_shader();
	render_vertex_shader_assign_input("vec3", "pos", "a_pos");
	render_vertex_shader_assign_output("v_pos", "vec4(pos.x, pos.y, pos.z, 1.0)");
	render_end_vertex_shader();

	render_begin_pixel_shader();
	render_pixel_shader_return("vec4(1.0, 1.0, 1.0, 1.0)");
	render_end_pixel_shader();

	if (!render_end_pipeline(&pipeline))
		return false;

	render_bind_pipeline(pipeline);

	float vertices[12] = {0, 0, 0,   0.5, 0, 0,   0.5, 0.5, 0,   0, 0.5, 0};
	render_create_vertex_buffer(12, &vertex_buffer);
	render_bind_vertex_buffer(vertex_buffer);
	render_upload_vertex_buffer(vertex_buffer, vertices);

	render_create_index_buffer(4, &index_buffer);
	render_bind_index_buffer(index_buffer);

	short indices[4] = {0, 1, 2, 3};
	render_upload_index_buffer(index_buffer, indices);

	return true;
}

/*
 * Called every frame.
 */
bool on_hal_frame(void)
{
	render_begin_frame();
	render_bind_pipeline(pipeline);
	render_bind_vertex_buffer(vertex_buffer);
	render_bind_index_buffer(index_buffer);
	render_draw_triangle_strip(0, 4);
	render_end_frame();

	return true;
}
