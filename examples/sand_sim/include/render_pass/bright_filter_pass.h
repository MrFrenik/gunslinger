#ifndef SS_BRIGHT_FILTER_PASS_H
#define SS_BRIGHT_FILTER_PASS_H

#include "render_pass/render_pass_i.h"

typedef struct bright_filter_data_t
{
	gs_shader_t shader;
	gs_uniform_t u_input_tex;
	gs_texture_t render_target;
	gs_vertex_buffer_t vb;
	gs_index_buffer_t ib;
} bright_filter_data_t;

typedef struct bright_filter_pass_t
{
	_base(render_pass_i);
	bright_filter_data_t data;
} bright_filter_pass_t;

// Use this to pass in parameters for the pass (will check for this)
typedef struct bright_filter_pass_parameters_t 
{
	gs_texture_t input_texture;
} bright_filter_pass_parameters_t;

bright_filter_pass_t bright_filter_pass_ctor();

#endif