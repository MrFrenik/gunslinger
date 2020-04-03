#ifndef SS_BLUR_RENDER_PASS
#define SS_BLUR_RENDER_PASS

#include "render_pass/render_pass_i.h"

typedef struct blur_data_t
{
	u32 iterations;
	gs_resource( gs_texture ) blur_render_target_a;
	gs_resource( gs_texture ) blur_render_target_b;
	gs_resource( gs_texture ) small_blur_render_target_a;
	gs_resource( gs_texture ) small_blur_render_target_b;
	gs_resource( gs_texture ) medium_blur_render_target_a;
	gs_resource( gs_texture ) medium_blur_render_target_b;
	gs_resource( gs_texture ) large_blur_render_target_a;
	gs_resource( gs_texture ) large_blur_render_target_b;
	gs_resource( gs_shader ) horizontal_blur_shader;
	gs_resource( gs_shader ) vertical_blur_shader;
	gs_resource ( gs_uniform ) u_input_tex;
	gs_resource ( gs_uniform ) u_tex_size;
	gs_resource( gs_vertex_buffer ) vb;
	gs_resource( gs_index_buffer ) ib;
} blur_data_t;

typedef struct blur_pass_t
{
	_base( render_pass_i );
	blur_data_t data;
} blur_pass_t;

// Use this to pass in parameters for the pass ( will check for this )
typedef struct blur_pass_parameters_t 
{
	gs_resource( gs_texture ) input_texture;
} blur_pass_parameters_t;

// Used to construct new instance of a blur pass
blur_pass_t blur_pass_ctor();

#endif