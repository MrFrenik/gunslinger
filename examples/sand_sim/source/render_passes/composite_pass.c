
#include "render_pass/composite_pass.h"

// typedef struct composite_pass_data_t
// {
// 	gs_resource( gs_shader ) shader;
// 	gs_resource ( gs_uniform ) u_input_tex;
// 	gs_resource( gs_texture ) render_target;
// 	gs_resource( gs_vertex_buffer ) vb;
// 	gs_resource( gs_index_buffer ) ib;
// } composite_pass_data_t;

// typedef struct composite_pass_t
// {
// 	_base( render_pass_i );
// 	composite_pass_data_t data;
// } composite_pass_t;

// // Use this to pass in parameters for the pass ( will check for this )
// typedef struct composite_pass_parameters_t 
// {
// 	gs_resource( gs_texture ) input_texture;
// } composite_pass_parameters_t;

// Forward Decl.
void cp_pass( gs_resource( gs_command_buffer ) cb, struct render_pass_i* pass, void* paramters );

const char* cp_v_src = "\n"
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

const char* cp_f_src = "\n"
"#version 330 core\n"
"in DATA {\n"
"	vec2 tex_coord;\n"
"} fs_in;\n"
"out vec4 frag_color;\n"
"uniform sampler2D u_tex;\n"
"uniform sampler2D u_blur_tex;\n"
"uniform float u_exposure;\n"
"uniform float u_gamma;\n"
"uniform float u_bloom_scalar;\n"
"uniform float u_saturation;\n"
"float A = 0.15;\n"
"float B = 0.50;\n"
"float C = 0.10;\n"
"float D = 0.20;\n"
"float E = 0.02;\n"
"float F = 0.30;\n"
"float W = 11.20;\n"
"vec3 uncharted_tone_map(vec3 x) {\n"
"	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;\n"
"}\n"
"void main() {\n"
"	vec3 hdr = max( vec3(0.0), texture(u_tex, fs_in.tex_coord).rgb );\n"
"	vec3 bloom = texture(u_blur_tex, fs_in.tex_coord).rgb;\n"
"	hdr += bloom * u_bloom_scalar;\n"
"	vec3 result = vec3(1.0) - exp(-hdr * u_exposure);\n"
"	result = pow(result, vec3(1.0 / u_gamma));\n"
"	float lum = result.r * 0.2 + result.g * 0.7 + result.b * 0.1;\n"
"	vec3 diff = result.rgb - vec3(lum);\n"
"	frag_color = vec4(vec3(diff) * u_saturation + lum, 1.0);\n"
"	frag_color = vec4(hdr, 1.0);\n"
"}\n";

// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
_global gs_vertex_attribute_type layout[] = {
	gs_vertex_attribute_float2,
	gs_vertex_attribute_float2
};
// Count of our vertex attribute array
_global u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

// Vertex data for triangle
_global f32 cp_v_data[] = {
	// Positions  UVs
	-1.0f, -1.0f,  0.0f, 0.0f,	// Top Left
	 1.0f, -1.0f,  1.0f, 0.0f,	// Top Right 
	-1.0f,  1.0f,  0.0f, 1.0f,  // Bottom Left
	 1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
};

_global u32 cp_i_data[] = {
	0, 2, 3,
	3, 1, 0
};

composite_pass_t composite_pass_ctor()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	composite_pass_t p = {0};

	// Construct shaders resources
	p.data.vb = gfx->construct_vertex_buffer( layout, layout_count, cp_v_data, sizeof(cp_v_data) );
	p.data.ib = gfx->construct_index_buffer( cp_i_data, sizeof(cp_i_data) );
	p.data.shader = gfx->construct_shader( cp_v_src, cp_f_src );
	p.data.u_input_tex = gfx->construct_uniform( p.data.shader, "u_tex", gs_uniform_type_sampler2d );
	p.data.u_blur_tex = gfx->construct_uniform( p.data.shader, "u_blur_tex", gs_uniform_type_sampler2d );
	// p.data.u_exposure = gfx->construct_uniform( p.data.shader, "u_exposure", gs_uniform_type_float );
	// p.data.u_gamma = gfx->construct_uniform( p.data.shader, "u_gamma", gs_uniform_type_float );
	p.data.u_bloom_scalar = gfx->construct_uniform( p.data.shader, "u_bloom_scalar", gs_uniform_type_float );
	// p.data.u_saturation = gfx->construct_uniform( p.data.shader, "u_saturation", gs_uniform_type_float );

	p._base.pass = &cp_pass;

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
	t_desc.width = (s32)(ws.x);
	t_desc.height = (s32)(ws.y);

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	p.data.render_target = gfx->construct_texture( t_desc );

	return p;
}

void cp_pass( gs_resource( gs_command_buffer ) cb, struct render_pass_i* _pass, void* _params )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_vec2 ws = platform->window_size(platform->main_window());

	composite_pass_t* p = (composite_pass_t*)_pass;
	if ( !p ) {
		return;
	} 

	// Can only use valid params
	composite_pass_parameters_t* params = (composite_pass_parameters_t*)_params;
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
	t_desc.width = (s32)(ws.x);
	t_desc.height = (s32)(ws.y);

	// Two render targets for double buffered separable blur ( For now, just set to window's viewport )
	gfx->update_texture_data( p->data.render_target, t_desc );

	// Set frame buffer attachment for rendering
	gfx->set_frame_buffer_attachment( cb, p->data.render_target, 0 );

	// Set viewport 
	gfx->set_view_port( cb, (u32)(ws.x), (u32)(ws.y) );	

	// Clear
	f32 cc[4] = { 0.f, 0.f, 0.f, 1.f };
	gfx->set_view_clear( cb, (f32*)&cc );

	// Use the program
	gfx->bind_shader( cb, p->data.shader );
	{
		f32 saturation = 2.0f;
		f32 gamma = 2.2f;
		f32 exposure = 0.5f;
		f32 bloom_scalar = 1.0f;

		gfx->bind_texture( cb, p->data.u_input_tex, params->input_texture, 0 );
		gfx->bind_texture( cb, p->data.u_blur_tex, params->blur_texture, 1 );
		gfx->bind_uniform( cb, p->data.u_saturation, &saturation );
		gfx->bind_uniform( cb, p->data.u_gamma, &gamma );
		gfx->bind_uniform( cb, p->data.u_exposure, &exposure );
		gfx->bind_uniform( cb, p->data.u_bloom_scalar, &bloom_scalar );
		gfx->bind_vertex_buffer( cb, p->data.vb );
		gfx->bind_index_buffer( cb, p->data.ib );
		gfx->draw_indexed( cb, 6 );
	}
}
