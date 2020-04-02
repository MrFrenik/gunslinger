
#include "render_pass/blur_pass.h"

// Forward Decls.
void calculate_blur_weights( blur_pass_t* pass );
void _blur_pass( gs_resource( gs_command_buffer ) cb, render_pass_i* _pass, void* _params );

/*===============
// Blur Shaders
================*/

const char* v_h_blur_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"out DATA {\n"
"	vec2 position;\n"
"	vec2 tex_coord;\n"
"} fs_out;\n"
"out vec2 v_blur_tex_coods[ 16 ];\n"
"uniform float u_blur_radius;"
"void main() {\n"
"	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
"	fs_out.tex_coord = a_uv;\n"
"	fs_out.position = a_position;\n"
"	vec2 v_tex_coord = fs_out.tex_coord;\n"
"	v_blur_tex_coods[0]  = v_tex_coord + vec2(0.0, -8 * u_blur_radius);\n"
"	v_blur_tex_coods[1]  = v_tex_coord + vec2(0.0, -7 * u_blur_radius);\n"
"	v_blur_tex_coods[2]  = v_tex_coord + vec2(0.0, -6 * u_blur_radius);\n"
"	v_blur_tex_coods[3]  = v_tex_coord + vec2(0.0, -5 * u_blur_radius);\n"
"	v_blur_tex_coods[4]  = v_tex_coord + vec2(0.0, -4 * u_blur_radius);\n"
"	v_blur_tex_coods[5]  = v_tex_coord + vec2(0.0, -3 * u_blur_radius);\n"
"	v_blur_tex_coods[6]  = v_tex_coord + vec2(0.0, -2 * u_blur_radius);\n"
"	v_blur_tex_coods[7]  = v_tex_coord + vec2(0.0, -1 * u_blur_radius);\n"
"	v_blur_tex_coods[8]  = v_tex_coord + vec2(0.0,  1 * u_blur_radius);\n"
"	v_blur_tex_coods[9]  = v_tex_coord + vec2(0.0,  2 * u_blur_radius);\n"
"	v_blur_tex_coods[10] = v_tex_coord + vec2(0.0,  3 * u_blur_radius);\n"
"	v_blur_tex_coods[11] = v_tex_coord + vec2(0.0,  4 * u_blur_radius);\n"
"	v_blur_tex_coods[13] = v_tex_coord + vec2(0.0,  5 * u_blur_radius);\n"
"	v_blur_tex_coods[14] = v_tex_coord + vec2(0.0,  6 * u_blur_radius);\n"
"	v_blur_tex_coods[15] = v_tex_coord + vec2(0.0,  7 * u_blur_radius);\n"
"	v_blur_tex_coods[15] = v_tex_coord + vec2(0.0,  8 * u_blur_radius);\n"
"}";

const char* v_v_blur_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"out DATA {\n"
"	vec2 position;\n"
"	vec2 tex_coord;\n"
"} fs_out;\n"
"out vec2 v_blur_tex_coods[ 16 ];\n"
"uniform float u_blur_radius;"
"void main() {\n"
"	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
"	fs_out.tex_coord = a_uv;\n"
"	fs_out.position = a_position;\n"
"	vec2 v_tex_coord = fs_out.tex_coord;\n"
"	v_blur_tex_coods[0]  = v_tex_coord + vec2(-8 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[1]  = v_tex_coord + vec2(-7 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[2]  = v_tex_coord + vec2(-6 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[3]  = v_tex_coord + vec2(-5 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[4]  = v_tex_coord + vec2(-4 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[5]  = v_tex_coord + vec2(-3 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[6]  = v_tex_coord + vec2(-2 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[7]  = v_tex_coord + vec2(-1 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[8]  = v_tex_coord + vec2( 1 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[9]  = v_tex_coord + vec2( 2 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[10] = v_tex_coord + vec2( 3 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[11] = v_tex_coord + vec2( 4 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[13] = v_tex_coord + vec2( 5 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[14] = v_tex_coord + vec2( 6 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[15] = v_tex_coord + vec2( 7 * u_blur_radius, 0.0);\n"
"	v_blur_tex_coods[15] = v_tex_coord + vec2( 8 * u_blur_radius, 0.0);\n"
"}";

const char* f_blur_src = "\n"
"#version 330 core\n"
"in DATA {\n"
"	vec2 position;\n"
"	vec2 tex_coord;\n"
"} fs_in;\n"
"in vec2 v_blur_tex_coods[16];\n"
"out vec4 frag_color;\n"
"uniform sampler2D u_tex;\n"
"uniform float u_weight;\n"
"uniform float u_curve[16];\n"
"void main() {\n"
"	frag_color = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	for ( int i = 0; i < 16; ++i ) {\n"
"		frag_color.rgb += texture( u_tex, v_blur_tex_coods[i] ).rgb * u_curve[i] * u_weight;\n"
"	}\n"
// "	frag_color = vec4(v_blur_tex_coods[15], 0.0, 1.0);\n"
// "	frag_color = texture(u_tex, fs_in.tex_coord);\n"
"}\n";

// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
_global gs_vertex_attribute_type layout[] = {
	gs_vertex_attribute_float2,
	gs_vertex_attribute_float2
};
// Count of our vertex attribute array
_global u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

// Vertex data for triangle
_global f32 v_data[] = {
	// Positions  UVs
	-1.0f, -1.0f,  0.0f, 0.0f,	// Top Left
	 1.0f, -1.0f,  1.0f, 0.0f,	// Top Right 
	-1.0f,  1.0f,  0.0f, 1.0f,  // Bottom Left
	 1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
};

_global u32 i_data[] = {
	0, 2, 3,
	3, 1, 0
};

// All of these settings are being directly copied from my previous implementation in Enjon to save time
// GraphicsSubsystem: https://github.com/MrFrenik/Enjon/blob/master/Include/Graphics/GraphicsSubsystem.h
blur_pass_t blur_pass_ctor()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	blur_pass_t bp = {0};
	bp.data.weights = (gs_vec3){ 0.384f, 0.366f, 0.500f };
	bp.data.iterations = (gs_vec3){ 3, 3, 2 };
	bp.data.radius = (gs_vec3){ 0.0001f, 0.0006f, 0.0015f };

	// Set pass for base
	bp._base.pass = &_blur_pass;

	// Calculate blur weights for pass using parameters
	calculate_blur_weights( &bp );

	// Initialize shaders and uniforms
	bp.data.horizontal_blur_shader = gfx->construct_shader( v_h_blur_src, f_blur_src );
	bp.data.vertical_blur_shader = gfx->construct_shader( v_v_blur_src, f_blur_src );
	bp.data.u_weight = gfx->construct_uniform( bp.data.horizontal_blur_shader, "u_weight", gs_uniform_type_float );
	bp.data.u_blur_radius = gfx->construct_uniform( bp.data.horizontal_blur_shader, "u_blur_radius", gs_uniform_type_float );
	gs_for_range_i( 16 ) {
		char tmp[ 256 ];
		gs_snprintf (tmp, sizeof(tmp), "u_curve[%d]", i );
		bp.data.u_curve[ i ] = gfx->construct_uniform( bp.data.horizontal_blur_shader, tmp, gs_uniform_type_float );		
	}
	bp.data.u_input_tex = gfx->construct_uniform( bp.data.horizontal_blur_shader, "u_tex", gs_uniform_type_sampler2d );

	// Construct render target to render into
	gs_vec2 ws = platform->window_size( platform->main_window() );

	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.mag_filter = gs_linear;
	t_desc.min_filter = gs_linear;
	t_desc.generate_mips = false;
	t_desc.num_comps = 4;
	t_desc.data = NULL;

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	t_desc.width = (s32)(ws.x / 4);
	t_desc.height = (s32)(ws.y / 4);
	bp.data.small_blur_render_target_a = gfx->construct_texture( t_desc );
	bp.data.small_blur_render_target_b = gfx->construct_texture( t_desc );

	t_desc.width = (s32)(ws.x / 8);
	t_desc.height = (s32)(ws.y / 8);

	bp.data.medium_blur_render_target_a = gfx->construct_texture( t_desc );
	bp.data.medium_blur_render_target_b = gfx->construct_texture( t_desc );

	t_desc.width = (s32)(ws.x / 16);
	t_desc.height = (s32)(ws.y / 16);

	bp.data.large_blur_render_target_a = gfx->construct_texture( t_desc );
	bp.data.large_blur_render_target_b = gfx->construct_texture( t_desc );

	bp.data.vb = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );
	bp.data.ib = gfx->construct_index_buffer( i_data, sizeof(i_data) );

	return bp;
}

f64 normal_pdf( f64 x, f64 s, f64 m )
{
	static const f64 inv_sqrt_2pi = 0.3989422804014327;
	f64 a = (x - m) / s;
	return (inv_sqrt_2pi) / s * exp( -0.5 * a * a );
}

// These have been pre-calculated using a simple graph visualizer
// blur Weights Graph: https://www.desmos.com/calculator/3zcgciuf5i
void calculate_blur_weights( blur_pass_t* pass )
{
	f64 start = -3.0;
	f64 end = 3.0;
	f64 denom = 2.0 * end + 1.0;
	f64 num_samples = 15.0;
	f64 range = end * 2.0;
	f64 step = range / num_samples;
	u32 i = 0;

	for ( f64 x = start; x <= end; x += step ) {
		f64 pdf = normal_pdf( x, 0.23, 0.0 );
		pass->data.small_gaussian_curve[ i++ ] = pdf;
	}

	i = 0;
	for ( f64 x = start; x <= end; x += step ) {
		f64 pdf = normal_pdf( x, 0.775, 0.0 );
		pass->data.medium_gaussian_curve[ i++ ] = pdf;
	}
	
	i = 0;	
	for ( f64 x = start; x <= end; x += step ) {
		f64 pdf = normal_pdf( x, 1.0, 0.0 );
		pass->data.large_gaussian_curve[ i++ ] = pdf;
	}
}

void _blur_pass( gs_resource( gs_command_buffer ) cb, render_pass_i* _pass, void* _params )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_vec2 ws = platform->window_size(platform->main_window());

	blur_pass_t* bp = (blur_pass_t*)_pass;
	if ( !bp ) {
		return;
	} 

	// Can only use valid params
	blur_pass_parameters_t* params = (blur_pass_parameters_t*)_params;
	if ( !params ) {
		return;
	}

	// Small blur
	for ( u32 i = 0; i < bp->data.iterations.x * 2; ++i ) 
	{
		b32 is_even = ( i % 2 == 0 );
		gs_resource( gs_texture )* target = is_even ? &bp->data.small_blur_render_target_a : &bp->data.small_blur_render_target_b;
		gs_resource( gs_shader )* shader = is_even ? &bp->data.horizontal_blur_shader : &bp->data.vertical_blur_shader;
		gs_resource( gs_texture )* tex = (i == 0) ? &params->input_texture : is_even ? &bp->data.small_blur_render_target_b : &bp->data.small_blur_render_target_a;

		// Set frame buffer attachment for rendering
		gfx->set_frame_buffer_attachment( cb, *target, 0 );

		// Set viewport 
		gfx->set_view_port( cb, (u32)(ws.x / 4), (u32)(ws.y / 4) );	
		// Clear
		f32 cc[4] = { 0.f, 0.f, 0.f, 1.f };
		gfx->set_view_clear( cb, (f32*)&cc );

		// Use the program
		gfx->bind_shader( cb, *shader );
		{
			// Bind uniforms
			for ( u32 j = 0; j < 16; ++j ) {
				gfx->bind_uniform( cb, bp->data.u_curve[j], &bp->data.small_gaussian_curve[j] );
			}

			gfx->bind_uniform( cb, bp->data.u_weight, &bp->data.weights.x );
			gfx->bind_uniform( cb, bp->data.u_blur_radius, &bp->data.radius.x );
			gfx->bind_texture( cb, bp->data.u_input_tex, *tex, 0 );
			gfx->bind_vertex_buffer( cb, bp->data.vb );
			gfx->bind_index_buffer( cb, bp->data.ib );
			gfx->draw_indexed( cb, 6 );
		}
	}

	// Medium blur
	for ( u32 i = 0; i < bp->data.iterations.y * 2; ++i ) 
	{
		b32 is_even = ( i % 2 == 0 );
		gs_resource( gs_texture )* target = is_even ? &bp->data.medium_blur_render_target_a : &bp->data.medium_blur_render_target_b;
		gs_resource( gs_shader )* shader = is_even ? &bp->data.horizontal_blur_shader : &bp->data.vertical_blur_shader;
		gs_resource( gs_texture )* tex = i == 0 ? &params->input_texture : is_even ? &bp->data.medium_blur_render_target_b : &bp->data.medium_blur_render_target_a;

		// Set frame buffer attachment for rendering
		gfx->set_frame_buffer_attachment( cb, *target, 0 );

		// Set viewport 
		gfx->set_view_port( cb, (u32)ws.x, (u32)ws.y );	
		// Clear
		f32 cc[4] = { 0.f, 0.f, 0.f, 1.f };
		gfx->set_view_clear( cb, (f32*)&cc );

		// Use the program
		gfx->bind_shader( cb, *shader );
		{
			// Bind uniforms
			for ( u32 j = 0; j < 16; ++j ) {
				gfx->bind_uniform( cb, bp->data.u_curve[j], &bp->data.medium_gaussian_curve[j] );
			}

			gfx->bind_uniform( cb, bp->data.u_weight, &bp->data.weights.y );
			gfx->bind_uniform( cb, bp->data.u_blur_radius, &bp->data.radius.y );
			gfx->bind_texture( cb, bp->data.u_input_tex, *tex, 0 );
			gfx->bind_vertex_buffer( cb, bp->data.vb );
			gfx->bind_index_buffer( cb, bp->data.ib );
			gfx->draw_indexed( cb, 6 );
		}
	}

	// Large blur
	for ( u32 i = 0; i < bp->data.iterations.z * 2; ++i ) 
	{
		b32 is_even = ( i % 2 == 0 );
		gs_resource( gs_texture )* target = is_even ? &bp->data.large_blur_render_target_a : &bp->data.large_blur_render_target_b;
		gs_resource( gs_shader )* shader = is_even ? &bp->data.horizontal_blur_shader : &bp->data.vertical_blur_shader;
		gs_resource( gs_texture )* tex = i == 0 ? &params->input_texture : is_even ? &bp->data.large_blur_render_target_b : &bp->data.large_blur_render_target_a;
		
		// Set frame buffer attachment for rendering
		gfx->set_frame_buffer_attachment( cb, *target, 0 );

		// Set viewport 
		gfx->set_view_port( cb, (u32)ws.x, (u32)ws.y );	
		// Clear
		f32 cc[4] = { 0.f, 0.f, 0.f, 1.f };
		gfx->set_view_clear( cb, (f32*)&cc );

		// Use the program
		gfx->bind_shader( cb, *shader );
		{
			// Bind uniforms
			for ( u32 j = 0; j < 16; ++j ) {
				gfx->bind_uniform( cb, bp->data.u_curve[j], &bp->data.large_gaussian_curve[j] );
			}

			gfx->bind_uniform( cb, bp->data.u_weight, &bp->data.weights.z );
			gfx->bind_uniform( cb, bp->data.u_blur_radius, &bp->data.radius.z );
			gfx->bind_texture( cb, bp->data.u_input_tex, *tex, 0 );
			gfx->bind_vertex_buffer( cb, bp->data.vb );
			gfx->bind_index_buffer( cb, bp->data.ib );
			gfx->draw_indexed( cb, 6 );
		}
	}
}

















