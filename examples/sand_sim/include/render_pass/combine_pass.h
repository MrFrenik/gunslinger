#ifndef SS_COMBINE_PASS_H
#define SS_COMBINE_PASS_H

#include "render_pass/render_pass_i.h"

typedef struct combine_pass_data_t
{
	gs_resource( gs_shader ) shader;
	gs_resource ( gs_uniform ) u_input_tex;
	gs_resource ( gs_uniform ) u_blur_tex;
	gs_resource ( gs_uniform ) u_exposure;
	gs_resource ( gs_uniform ) u_gamma;
	gs_resource ( gs_uniform ) u_bloom_scalar;
	gs_resource ( gs_uniform ) u_saturation;
	gs_resource( gs_texture ) render_target;
	gs_resource( gs_vertex_buffer ) vb;
	gs_resource( gs_index_buffer ) ib;
} combine_pass_data_t;

typedef struct combine_pass_t
{
	_base( render_pass_i );
	combine_pass_data_t data;
} combine_pass_t;

// Use this to pass in parameters for the pass ( will check for this )
typedef struct combine_pass_parameters_t 
{
	gs_resource( gs_texture ) input_texture;
	gs_resource( gs_texture ) blur_texture;
} combine_pass_parameters_t;

combine_pass_t combine_pass_ctor();

#endif#endif