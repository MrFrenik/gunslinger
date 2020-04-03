
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
"out vec2 v_blur_tex_coods[ 11 ];\n"
"uniform vec2 u_tex_size;"
"void main() {\n"
"	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
"	fs_out.tex_coord = a_uv;\n"
"	fs_out.position = a_position;\n"
"	vec2 center_tex_coords = a_position * 0.5 + 0.5;\n"
"	float pixel_size = 1.0 / u_tex_size.x;\n"
"	for ( int i = -5; i <= 5; ++i ) {\n"
"		v_blur_tex_coods[i + 5] = center_tex_coords + vec2(pixel_size * i, 0.0);\n"
"	}\n"
"}";

const char* v_v_blur_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"out DATA {\n"
"	vec2 position;\n"
"	vec2 tex_coord;\n"
"} fs_out;\n"
"out vec2 v_blur_tex_coods[ 11 ];\n"
"uniform float u_blur_radius;"
"uniform vec2 u_tex_size;"
"void main() {\n"
"	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
"	fs_out.tex_coord = a_uv;\n"
"	fs_out.position = a_position;\n"
"	vec2 center_tex_coords = a_position * 0.5 + 0.5;\n"
"	float pixel_size = 1.0 / u_tex_size.y;\n"
"	for ( int i = -5; i <= 5; ++i ) {\n"
"		v_blur_tex_coods[i + 5] = center_tex_coords + vec2(0.0, pixel_size * i);\n"
"	}\n"
"}";

const char* f_blur_src = "\n"
"#version 330 core\n"
"in DATA {\n"
"	vec2 position;\n"
"	vec2 tex_coord;\n"
"} fs_in;\n"
"in vec2 v_blur_tex_coods[11];\n"
"out vec4 frag_color;\n"
"uniform sampler2D u_tex;\n"
"void main() {\n"
"	frag_color = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[0]) * 0.0093;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[1]) * 0.028002;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[2]) * 0.065984;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[3]) * 0.121703;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[4]) * 0.175713;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[5]) * 0.198596;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[6]) * 0.175713;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[7]) * 0.121703;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[8]) * 0.065984;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[9]) * 0.028002;\n"
"	frag_color += texture(u_tex, v_blur_tex_coods[10]) * 0.0093;\n"
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
	bp.data.iterations = 1;

	// Set pass for base
	bp._base.pass = &_blur_pass;

	// Initialize shaders and uniforms
	bp.data.horizontal_blur_shader = gfx->construct_shader( v_h_blur_src, f_blur_src );
	bp.data.vertical_blur_shader = gfx->construct_shader( v_v_blur_src, f_blur_src );
	bp.data.u_input_tex = gfx->construct_uniform( bp.data.horizontal_blur_shader, "u_tex", gs_uniform_type_sampler2d );
	bp.data.u_tex_size = gfx->construct_uniform( bp.data.horizontal_blur_shader, "u_tex_size", gs_uniform_type_vec2 );

	// Construct render target to render into
	gs_vec2 ws = platform->window_size( platform->main_window() );

	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.texture_wrap_s = gs_clamp_to_border;
	t_desc.texture_wrap_t = gs_clamp_to_border;
	t_desc.mag_filter = gs_linear;
	t_desc.min_filter = gs_linear;
	t_desc.texture_format = gs_texture_format_rgba16f;
	t_desc.generate_mips = false;
	t_desc.num_comps = 4;
	t_desc.data = NULL;
	t_desc.width = (s32)(ws.x / 16);
	t_desc.height = (s32)(ws.y / 16);

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	bp.data.blur_render_target_a = gfx->construct_texture( t_desc );
	bp.data.blur_render_target_b = gfx->construct_texture( t_desc );

	bp.data.vb = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );
	bp.data.ib = gfx->construct_index_buffer( i_data, sizeof(i_data) );

	return bp;
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

	gs_vec2 tex_size = (gs_vec2){ ws.x / 16, ws.y / 16 };

	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.texture_wrap_s = gs_clamp_to_border;
	t_desc.texture_wrap_t = gs_clamp_to_border;
	t_desc.mag_filter = gs_linear;
	t_desc.min_filter = gs_linear;
	t_desc.texture_format = gs_texture_format_rgba16f;
	t_desc.generate_mips = false;
	t_desc.num_comps = 4;
	t_desc.data = NULL;
	t_desc.width = (s32)(ws.x / 16);
	t_desc.height = (s32)(ws.y / 16);

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	gfx->update_texture_data( bp->data.blur_render_target_a, t_desc );
	gfx->update_texture_data( bp->data.blur_render_target_b, t_desc );

	for ( u32 i = 0; i < bp->data.iterations * 2; ++i ) 
	{
		b32 is_even = ( i % 2 == 0 );
		gs_resource( gs_texture )* target = is_even ? &bp->data.blur_render_target_a : &bp->data.blur_render_target_b;
		gs_resource( gs_shader )* shader = is_even ? &bp->data.horizontal_blur_shader : &bp->data.vertical_blur_shader;
		gs_resource( gs_texture )* tex = (i == 0) ? &params->input_texture : is_even ? &bp->data.blur_render_target_b : &bp->data.blur_render_target_a;

		// Set frame buffer attachment for rendering
		gfx->set_frame_buffer_attachment( cb, *target, 0 );

		// Set viewport 
		gfx->set_view_port( cb, (u32)(tex_size.x), (u32)(tex_size.y) );	
		// Clear
		f32 cc[4] = { 0.f, 0.f, 0.f, 1.f };
		gfx->set_view_clear( cb, (f32*)&cc );

		// Use the program
		gfx->bind_shader( cb, *shader );
		{
			gfx->bind_texture( cb, bp->data.u_input_tex, *tex, 0 );
			gfx->bind_uniform( cb, bp->data.u_tex_size, &tex_size );
			gfx->bind_vertex_buffer( cb, bp->data.vb );
			gfx->bind_index_buffer( cb, bp->data.ib );
			gfx->draw_indexed( cb, 6 );
		}
	}
}

















