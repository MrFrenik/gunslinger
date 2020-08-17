#ifndef SS_BLUR_RENDER_PASS
#define SS_BLUR_RENDER_PASS

#include "render_pass/render_pass_i.h"

typedef struct blur_data_t
{
	u32 iterations;
	gs_texture_t blur_render_target_a;
	gs_texture_t blur_render_target_b;
	gs_texture_t small_blur_render_target_a;
	gs_texture_t small_blur_render_target_b;
	gs_texture_t medium_blur_render_target_a;
	gs_texture_t medium_blur_render_target_b;
	gs_texture_t large_blur_render_target_a;
	gs_texture_t large_blur_render_target_b;
	gs_shader_t horizontal_blur_shader;
	gs_shader_t vertical_blur_shader;
	gs_uniform_t u_input_tex;
	gs_uniform_t u_tex_size;
	gs_vertex_buffer_t vb;
	gs_index_buffer_t ib;
} blur_data_t;

typedef struct blur_pass_t
{
	_base( render_pass_i );
	blur_data_t data;
} blur_pass_t;

// Use this to pass in parameters for the pass ( will check for this )
typedef struct blur_pass_parameters_t 
{
	gs_texture_t input_texture;
} blur_pass_parameters_t;

// Used to construct new instance of a blur pass
blur_pass_t blur_pass_ctor();

#endif