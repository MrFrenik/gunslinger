#include "render_pass/bright_filter_pass.h"

// Foward Decls
void bp_pass( gs_resource( gs_command_buffer ) cb, struct render_pass_i* pass, void* paramters );

const char* bp_v_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"out DATA {\n"
"	vec2 tex_coord;\n"
"} fs_out;\n"
"void main() {\n"
"	gl_Position = vec4(a_position, 0.0, 1.0);\n"
"	fs_out.tex_coord = a_uv;\n"
"}\n";

const char* bp_f_src = "\n"
"#version 330 core\n"
"in DATA {\n"
"	vec2 tex_coord;\n"
"} fs_in;\n"
"out vec4 frag_color;\n"
"uniform sampler2D u_tex;\n"
"void main() {\n"
"	frag_color = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	vec3 tex_color = texture(u_tex, fs_in.tex_coord).rgb;\n"
"	float brightness = dot(tex_color, vec3(0.2126, 0.7152, 0.0722));\n"
"	if (tex_color.b < 0.2 && brightness > 0.4 ) {\n"
"		vec3 op = clamp(tex_color, vec3(0), vec3(255));\n"
"		frag_color = vec4(op * 0.1, 1.0);\n"
"	}\n"
"}\n";

// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
_global gs_vertex_attribute_type layout[] = {
	gs_vertex_attribute_float2,
	gs_vertex_attribute_float2
};
// Count of our vertex attribute array
_global u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

// Vertex data for triangle
_global f32 bp_v_data[] = {
	// Positions  UVs
	-1.0f, -1.0f,  0.0f, 0.0f,	// Top Left
	 1.0f, -1.0f,  1.0f, 0.0f,	// Top Right 
	-1.0f,  1.0f,  0.0f, 1.0f,  // Bottom Left
	 1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
};

_global u32 bp_i_data[] = {
	0, 2, 3,
	3, 1, 0
};

bright_filter_pass_t bright_filter_pass_ctor()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	bright_filter_pass_t bp = {0};

	// Construct shaders resources
	bp.data.vb = gfx->construct_vertex_buffer( layout, layout_count, bp_v_data, sizeof(bp_v_data) );
	bp.data.ib = gfx->construct_index_buffer( bp_i_data, sizeof(bp_i_data) );
	bp.data.shader = gfx->construct_shader( bp_v_src, bp_f_src );
	bp.data.u_input_tex = gfx->construct_uniform( bp.data.shader, "u_tex", gs_uniform_type_sampler2d );
	bp._base.pass = &bp_pass;

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
	t_desc.width = (s32)(ws.x / 8);
	t_desc.height = (s32)(ws.y / 8);

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	bp.data.render_target = gfx->construct_texture( t_desc );

	return bp;
}

 void bp_pass( gs_resource( gs_command_buffer ) cb, struct render_pass_i* _pass, void* _params )
 {
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_vec2 ws = platform->window_size(platform->main_window());

	bright_filter_pass_t* bp = (bright_filter_pass_t*)_pass;
	if ( !bp ) {
		return;
	} 

	// Can only use valid params
	bright_filter_pass_parameters_t* params = (bright_filter_pass_parameters_t*)_params;
	if ( !params ) {
		return;
	}

	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.texture_wrap_s = gs_clamp_to_border;
	t_desc.texture_wrap_t = gs_clamp_to_border;
	t_desc.mag_filter = gs_linear;
	t_desc.min_filter = gs_linear;
	t_desc.texture_format = gs_texture_format_rgba16f;
	t_desc.generate_mips = false;
	t_desc.num_comps = 4;
	t_desc.data = NULL;
	t_desc.width = (s32)(ws.x / 8);
	t_desc.height = (s32)(ws.y / 8);

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	gfx->update_texture_data( bp->data.render_target, t_desc );

	// Set frame buffer attachment for rendering
	gfx->set_frame_buffer_attachment( cb, bp->data.render_target, 0 );

	// Set viewport 
	gfx->set_view_port( cb, (u32)(ws.x / 8), (u32)(ws.y / 8) );	

	// Clear
	f32 cc[4] = { 0.f, 0.f, 0.f, 1.f };
	gfx->set_view_clear( cb, (f32*)&cc );

	// Use the program
	gfx->bind_shader( cb, bp->data.shader );
	{
		gfx->bind_texture( cb, bp->data.u_input_tex, params->input_texture, 0 );
		gfx->bind_vertex_buffer( cb, bp->data.vb );
		gfx->bind_index_buffer( cb, bp->data.ib );
		gfx->draw_indexed( cb, 6 );
	}
 }


