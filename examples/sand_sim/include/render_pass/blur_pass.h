#ifndef SS_BLUR_RENDER_PASS
#define SS_BLUR_RENDER_PASS

#include <gs.h>
#include "render_pass/render_pass_i.h"

typedef struct blur_data_t
{
	gs_vec3 weights;
	gs_vec3 iterations;
	gs_vec3 radius;
	f64 small_gaussian_curve[16];
	f64 medium_gaussian_curve[16];
	f64 large_gaussian_curve[16];
	gs_resource( gs_texture ) small_blur_render_target_a;
	gs_resource( gs_texture ) small_blur_render_target_b;
	gs_resource( gs_texture ) medium_blur_render_target_a;
	gs_resource( gs_texture ) medium_blur_render_target_b;
	gs_resource( gs_texture ) large_blur_render_target_a;
	gs_resource( gs_texture ) large_blur_render_target_b;
	gs_resource( gs_shader ) horizontal_blur_shader;
	gs_resource( gs_shader ) vertical_blur_shader;
	gs_resource( gs_uniform ) u_curve[ 16];
	gs_resource( gs_uniform ) u_weight;
	gs_resource( gs_uniform ) u_blur_radius;
	gs_resource ( gs_uniform ) u_input_tex;
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

/*
	typedef struct render_pass_i {
		void ( * pass )( void* data );	
	} render_pass_i;

	typedef struct blur_pass_t {

		_base( render_pass_i )	

		// ...
	} blur_pass_t;

	blur_pass_ctor() {
		// Construct pass (just set render pass function pointer)	
	}

	// Use
	{
				
	}
*/

#endif