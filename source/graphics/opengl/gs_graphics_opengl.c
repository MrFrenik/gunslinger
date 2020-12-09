#include "common/gs_containers.h"
#include "serialize/gs_byte_buffer.h"
#include "math/gs_math.h"
#include "common/gs_util.h"
#include "base/gs_engine.h"
#include "graphics/gs_graphics.h"
#include "graphics/gs_material.h"
#include "graphics/gs_quad_batch.h"
#include "graphics/gs_camera.h"
#include "platform/gs_platform.h"

#include <glad/glad.h>

#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

s32 g_int = 3;

/*============================================================
// Graphics Resource Declarations
============================================================*/

/*
	Command buffer architecture: 
		- Function address to be called
		- Size of packet placed into buffer
		- Packet
*/

#define int_2_void_p(i) (void*)(uintptr_t)(i)

gs_inline void gs_mat4_debug_print(gs_mat4* mat)
{
	f32* e = mat->elements;
	gs_println("[%.5f, %.5f, %.5f, %.5f]\n"
			   "[%.5f, %.5f, %.5f, %.5f]\n"
			   "[%.5f, %.5f, %.5f, %.5f]\n"
			   "[%.5f, %.5f, %.5f, %.5f]\n", 
			   e[0], e[1], e[2], e[3], 
			   e[4], e[5], e[6], e[7], 
			   e[8], e[9], e[10], e[11], 
			   e[12], e[13], e[14], e[15]
			 );
}

gs_mat4 __gs_default_view_proj_mat()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_vec2 ws = platform->window_size(platform->main_window());
	gs_vec2 hws = gs_vec2_scale(ws, 0.5f);
	gs_camera_t cam = gs_camera_default();
	cam.transform.position = gs_vec3_add(cam.transform.position, gs_v3(hws.x, hws.y, 1.f));
	f32 l = -ws.x / 2.f; 
	f32 r = ws.x / 2.f;
	f32 b = ws.y / 2.f;
	f32 t = -ws.y / 2.f;
	gs_mat4 ortho = gs_mat4_transpose(gs_mat4_ortho(
		l, r, b, t, 0.01f, 1000.f
	));
	ortho = gs_mat4_mul(ortho, gs_camera_get_view(&cam));
	return ortho;
}

typedef enum gs_opengl_op_code
{
	gs_opengl_op_bind_shader = 0,
	gs_opengl_op_set_view_clear,
	gs_opengl_op_set_viewport,
	gs_opengl_op_set_view_scissor,
	gs_opengl_op_set_depth_enabled,
	gs_opengl_op_set_face_culling,
	gs_opengl_op_set_winding_order,
	gs_opengl_op_set_blend_mode,
	gs_opengl_op_set_blend_equation,
	gs_opengl_op_bind_vertex_buffer,
	gs_opengl_op_bind_index_buffer,
	gs_opengl_op_bind_uniform,
	gs_opengl_op_bind_texture,
	gs_opengl_op_bind_texture_id,
	gs_opengl_op_bind_frame_buffer,
	gs_opengl_op_unbind_frame_buffer,
	gs_opengl_op_update_vertex_data,
	gs_opengl_op_update_index_data,
	gs_opengl_op_set_frame_buffer_attachment,
	gs_opengl_op_draw,
	gs_opengl_op_draw_indexed,

	// Debug/Immediate
	gs_opengl_op_immediate_begin_drawing,
	gs_opengl_op_immediate_end_drawing,
	gs_opengl_op_immediate_push_state,
	gs_opengl_op_immediate_pop_state,
	gs_opengl_op_immediate_flush,
	gs_opengl_op_immediate_push_state_attr,
	gs_opengl_op_immediate_enable_texture_2d,
	gs_opengl_op_immediate_disable_texture_2d,
	gs_opengl_op_immediate_push_matrix,
	gs_opengl_op_immediate_pop_matrix,
	gs_opengl_op_immediate_mat_mul,
	gs_opengl_op_immediate_begin,
	gs_opengl_op_immediate_end,
	gs_opengl_op_immediate_color_ubv,
	gs_opengl_op_immediate_texcoord_2fv,
	gs_opengl_op_immediate_vertex_3fv
} gs_opengl_op_code;

typedef struct immediate_vertex_data_t
{
	gs_vec3 position;
	gs_vec2 texcoord;
	gs_color_t color;
} immediate_vertex_data_t;

// Internal resources for uniform block internally? So that *those* are kept internal, yet materials are just wrappers?
// Could keep data sizes significantly lowered.
// Can I hold an internal resource pool for materials? Don't like it.
// Ease of use for end user? draw_renderable(cb, &renderable);
// Does the pipeline state hold onto a uniform block?
// Lot of potential transferred data moving around

/*
	In the beginning, all stack is cleared.
	For submitting a command buffer, you add data onto the stack.
	Clear all data after final submit of buffer. (so that means all stacks are completely localized within a submit command and state won't "leak")
*/

// Internally
typedef struct immediate_drawing_internal_data_t 
{
	// Bound this all up into a state object, then create stack of states
	gs_resource(gs_render_pipeline_state_t) default_state_2d;
	gs_resource(gs_render_pipeline_state_t) default_state_3d;

	// Current vertex state attributes
	gs_color_t color;
	gs_vec2 texcoord;
	u32 tex_id;
	gs_draw_mode draw_mode;

	// Being able to dynamically push custom vertex data based on a declaration would be nice
	gs_dyn_array(immediate_vertex_data_t) vertex_data;

	// Stacks
	gs_dyn_array(gs_render_pipeline_state_t) 	state_stack;
	gs_dyn_array(gs_mat4) 						vp_matrix_stack;
	gs_dyn_array(gs_mat4) 						model_matrix_stack;
	gs_dyn_array(gs_vec2) 						viewport_stack;			// This can go away (with state stack)
	gs_dyn_array(gs_matrix_mode) 				matrix_modes;
} immediate_drawing_internal_data_t;

typedef gs_command_buffer_t command_buffer_t;
typedef gs_texture_t texture_t;
typedef gs_shader_t shader_t;
typedef gs_uniform_t uniform_t;
typedef gs_index_buffer_t index_buffer_t;
typedef gs_vertex_buffer_t vertex_buffer_t;
typedef gs_vertex_attribute_layout_desc_t vertex_attribute_layout_desc_t;
typedef gs_render_target_t render_target_t;
typedef gs_frame_buffer_t frame_buffer_t;

// Define render resource data structure
typedef struct opengl_render_data_t
{
	immediate_drawing_internal_data_t immediate_data;
} opengl_render_data_t;

gs_global const char* immediate_shader_v_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 a_position;\n"
"layout (location = 1) in vec2 a_texcoord;\n"
"layout (location = 2) in vec4 a_color;\n"
"uniform mat4 u_mvp;\n"
"out vec4 f_color;\n"
"out vec2 f_uv;\n"
"void main() {\n"
" gl_Position = u_mvp * vec4(a_position, 1.0);\n"
" f_color = a_color;\n"
" f_uv = a_texcoord;\n"
"}";

gs_global const char* immediate_shader_f_src = "\n"
"#version 330 core\n"
"in vec4 f_color;\n"
"in vec2 f_uv;\n"
"out vec4 frag_color;\n"
"uniform sampler2D u_tex;\n"
"void main() {\n"
" frag_color = f_color * texture(u_tex, f_uv);\n"
"}";

gs_global gs_shader_t 				g_default_shader 		= gs_default_val();
gs_global gs_vertex_buffer_t 		g_default_vertex_buffer = gs_default_val();
gs_global gs_index_buffer_t 		g_default_index_buffer 	= gs_default_val();
gs_global gs_texture_t 				g_default_texture 		= gs_default_val();
gs_global gs_resource(gs_font_t) 	g_default_font 			= gs_default_val();

gs_resource(gs_font_t) __gs_get_default_font()
{
	return g_default_font;
}

#define __get_opengl_data_internal()\
	(opengl_render_data_t*)(gs_engine_instance()->ctx.graphics->data)

#define __get_opengl_immediate_data()\
	 (&(__get_opengl_data_internal())->immediate_data);

// Forward Decls;
void __reset_command_buffer_internal(command_buffer_t* cb);
immediate_drawing_internal_data_t construct_immediate_drawing_internal_data_t();
gs_texture_t opengl_construct_texture(gs_texture_parameter_desc desc);
void opengl_immediate_submit_vertex_data();

// Gfx Ops
void opengl_set_viewport(gs_command_buffer_t* cb, u32 x, u32 y, u32 width, u32 height);
void opengl_set_view_clear(gs_command_buffer_t* cb, f32* col);
void opengl_set_winding_order(gs_command_buffer_t* cb, gs_winding_order_type type);
void opengl_set_face_culling(gs_command_buffer_t* cb, gs_face_culling_type type);
void opengl_set_blend_equation(gs_command_buffer_t* cb, gs_blend_equation_type eq);
void opengl_set_blend_mode(gs_command_buffer_t* cb, gs_blend_mode_type src, gs_blend_mode_type dst);
void opengl_set_view_scissor(gs_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
void opengl_set_depth_enabled(gs_command_buffer_t* cb, b32 enable);
void opengl_bind_shader(gs_command_buffer_t* cb, gs_shader_t s);

gs_render_pipeline_state_desc_t gs_render_pipeline_state_desc_default()
{
	gs_platform_i* p = gs_engine_instance()->ctx.platform;
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	gs_vec2 fbs = p->frame_buffer_size(p->main_window());

	gs_render_pipeline_state_desc_t state = gs_default_val();

	// Grab description from internal data and copy
	state.blend_func_src = gs_blend_mode_src_alpha;
	state.blend_func_dst = gs_blend_mode_one_minus_src_alpha;
	state.blend_equation = gs_blend_equation_add;
	state.depth_enabled  = false;
	state.winding_order  = gs_winding_order_ccw;
	state.face_culling 	 = gs_face_culling_disabled;
	state.view_scissor 	 = gs_v4_s(0.f);
	state.viewport 	 	 = gs_v4(0.f, 0.f, fbs.x, fbs.y); 
	state.shader 		 = g_default_shader;
	state.vbo  			 = g_default_vertex_buffer;
	state.ibo 	 		 = g_default_index_buffer;
	state.draw_mode 	 = gs_triangles;
	state.clear_color	 = gs_v4(0.f, 0.f, 0.f, 1.f);
	state.clear_bit		 = gs_clear_all;

	return state;
}

gs_resource(gs_render_pipeline_state_t) gs_construct_render_pipeline_state(gs_render_pipeline_state_desc_t desc)
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);

	gs_render_pipeline_state_t state = gs_default_val();
	state.desc 		= desc;
	state.uniforms 	= gfx->uniform_i->construct();

	gs_resource(gs_render_pipeline_state_t) s = gs_default_val();
	s.id = gs_slot_array_insert(gfx->render_pipelines, state);
	return s;
}

/*============================================================
// Graphics Initilization / De-Initialization
============================================================*/

// Won't do this in the future. Will upload a pipeline state instead.
// Also, will just use pipeline states to make transition to other apis easier.
void opengl_init_default_state()
{
	// Need to add blend states
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_SCISSOR_TEST);
}

gs_result opengl_init(struct gs_graphics_i* gfx)
{
	// Construct instance of render data
	struct opengl_render_data_t* data = gs_malloc_init(opengl_render_data_t);

	// Set data
	gfx->data = data;

	// Initialize all graphcis resource caches
	gfx->font_cache 		 = gs_resource_cache_new(gs_font_t);
	gfx->material_cache 	 = gs_resource_cache_new(gs_material_t);
	gfx->uniform_block_cache = gs_resource_cache_new(gs_uniform_block_t);

	gfx->render_pipelines = gs_slot_array_new(gs_render_pipeline_state_t);

	// Initialize data
	data->immediate_data = construct_immediate_drawing_internal_data_t();

	// Init all default opengl state here before frame begins
	opengl_init_default_state();

	// Print some immediate info
	gs_println("OpenGL::Vendor = %s", glGetString(GL_VENDOR)) ;
	gs_println("OpenGL::Renderer = %s", glGetString(GL_RENDERER)) ;
	gs_println("OpenGL::Version = %s", glGetString(GL_VERSION)) ;

	// Init data for utility APIs
	gfx->quad_batch_i->shader = gfx->construct_shader(gs_quad_batch_default_vertex_src, gs_quad_batch_default_frag_src);

	// Initialize all data here
	return gs_result_success;
}

gs_result opengl_update(struct gs_graphics_i* gfx)
{
	return gs_result_in_progress;
}

gs_result opengl_shutdown(struct gs_graphics_i* gfx)
{
	// Release all data here
	return gs_result_success;
}

void gs_reset_default_states()
{
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);

	// 2d state
	gs_render_pipeline_state_desc_t desc_2d = gs_render_pipeline_state_desc_default();
	gs_render_pipeline_state_t* state = gs_slot_array_get_ptr(gfx->render_pipelines, data->default_state_2d.id);
	state->desc = desc_2d;
	gfx->uniform_i->set_uniform_from_shader(state->uniforms, state->desc.shader, gs_uniform_type_mat4, "u_mvp", __gs_default_view_proj_mat());
	gfx->uniform_i->set_uniform_from_shader(state->uniforms, state->desc.shader, gs_uniform_type_sampler2d, "u_tex", g_default_texture, 0);

	// 3d state
	gs_render_pipeline_state_desc_t desc_3d = gs_render_pipeline_state_desc_default();
	desc_3d.depth_enabled = true;
	desc_3d.face_culling = gs_face_culling_back;
	state = gs_slot_array_get_ptr(gfx->render_pipelines, data->default_state_3d.id);
	state->desc = desc_3d;
	gfx->uniform_i->set_uniform_from_shader(state->uniforms, state->desc.shader, gs_uniform_type_mat4, "u_mvp", __gs_default_view_proj_mat());
	gfx->uniform_i->set_uniform_from_shader(state->uniforms, state->desc.shader, gs_uniform_type_sampler2d, "u_tex", g_default_texture, 0);
}

immediate_drawing_internal_data_t construct_immediate_drawing_internal_data_t()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	immediate_drawing_internal_data_t data = gs_default_val();

	// Construct shader
	g_default_shader = gfx->construct_shader(immediate_shader_v_src, immediate_shader_f_src);

	// Construct default font
	g_default_font = __gs_construct_default_font();

	// Construct default white texture
	u8 white[2 * 2 * 4];
	memset(white, 255, 2 * 2 * 4);
	gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();
	desc.data = white;
	desc.width = 2;
	desc.height = 2;
	g_default_texture = gfx->construct_texture(desc);

	// Construct default vertex buffer
	gs_vertex_attribute_type vertex_layout[] = {
		gs_vertex_attribute_float3,	// Position
		gs_vertex_attribute_float2,	// UV
		gs_vertex_attribute_byte4  	// Color
	};
	g_default_vertex_buffer = gfx->construct_vertex_buffer(vertex_layout, sizeof(vertex_layout), NULL, 0);

	// Construct and set default pipeline states
	data.default_state_2d = gs_construct_render_pipeline_state(gs_render_pipeline_state_desc_default());
	data.default_state_3d = gs_construct_render_pipeline_state(gs_render_pipeline_state_desc_default());
	gs_reset_default_states();

	// Construct stacks
	data.vertex_data 		= gs_dyn_array_new(immediate_vertex_data_t);
	data.model_matrix_stack = gs_dyn_array_new(gs_mat4);
	data.vp_matrix_stack 	= gs_dyn_array_new(gs_mat4);
	data.viewport_stack 	= gs_dyn_array_new(gs_vec2);
	data.matrix_modes 		= gs_dyn_array_new(gs_matrix_mode);
	data.state_stack 		= gs_dyn_array_new(gs_render_pipeline_state_t);
	data.draw_mode 			= gs_triangles;

	return data;
}

void __reset_command_buffer_internal(command_buffer_t* cb)
{
	// Clear byte buffer of commands
	gs_byte_buffer_clear(&cb->commands);

	// Set num commands to 0
	cb->num_commands = 0;
}

void opengl_reset_command_buffer(gs_command_buffer_t* cb)
{
	__reset_command_buffer_internal(cb);
}

#define __push_command(cb, op_code, ...)\
do {\
	/* Get data from graphics api */\
	opengl_render_data_t* __data = __get_opengl_data_internal();\
	gs_byte_buffer_write(&cb->commands, u32, op_code);\
	/* Push back all data required */\
	__VA_ARGS__\
	/* Increase command count for command buffer */\
	cb->num_commands++;\
} while (0)

void opengl_set_pipeline_state(gs_command_buffer_t* cb, gs_resource(gs_render_pipeline_state_t) state_h)
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);

	gs_render_pipeline_state_t* state = gs_slot_array_get_ptr(gfx->render_pipelines, state_h.id);

	if (gs_vec4_len(state->desc.view_scissor)) {
		gs_vec4* vs = &state->desc.view_scissor;
		opengl_set_view_scissor(cb, vs->x, vs->y, vs->z, vs->w);
	}

	opengl_set_viewport(cb, state->desc.viewport.x, state->desc.viewport.y, state->desc.viewport.z, state->desc.viewport.w);
	opengl_set_winding_order(cb, state->desc.winding_order);
	opengl_set_face_culling(cb, state->desc.face_culling);
	opengl_set_blend_equation(cb, state->desc.blend_equation);
	opengl_set_blend_mode(cb, state->desc.blend_func_src, state->desc.blend_func_dst);
	opengl_set_depth_enabled(cb, state->desc.depth_enabled);
	opengl_bind_shader(cb, state->desc.shader);

	// Bind uniforms from state as well
}

void opengl_set_frame_buffer_attachment(gs_command_buffer_t* cb, gs_texture_t t, u32 idx)
{
	__push_command(cb, gs_opengl_op_set_frame_buffer_attachment, {
		gs_byte_buffer_write(&cb->commands, u32, t.id);
		gs_byte_buffer_write(&cb->commands, u32, idx);
	});
}

void opengl_bind_frame_buffer(gs_command_buffer_t* cb, gs_frame_buffer_t fb)
{
	__push_command(cb, gs_opengl_op_bind_frame_buffer, {
		gs_byte_buffer_write(&cb->commands, u32, fb.fbo);
	});
}

void opengl_unbind_frame_buffer(gs_command_buffer_t* cb)
{
	__push_command(cb, gs_opengl_op_unbind_frame_buffer, {
		// Nothing...
	});
}

void opengl_bind_shader(gs_command_buffer_t* cb, gs_shader_t s)
{
	__push_command(cb, gs_opengl_op_bind_shader, {
		 gs_byte_buffer_write(&cb->commands, u32, s.program_id);
	});
}

#define __write_uniform_val(bb, type, u_data)\
	gs_byte_buffer_write(&bb, type, *((type*)(u_data)));

void opengl_bind_uniform(gs_command_buffer_t* cb, gs_uniform_t u, void* u_data)
{
	__push_command(cb, gs_opengl_op_bind_uniform, {

		// Write out uniform location
		gs_byte_buffer_write(&cb->commands, u32, (u32)u.location);
		// Write out uniform type
		gs_byte_buffer_write(&cb->commands, u32, (u32)u.type);

		// Write out uniform value
		switch (u.type)
		{
			case gs_uniform_type_float: __write_uniform_val(cb->commands, f32, u_data); 	break;
			case gs_uniform_type_int:   __write_uniform_val(cb->commands, s32, u_data); 	break;
			case gs_uniform_type_vec2:  __write_uniform_val(cb->commands, gs_vec2, u_data); break;
			case gs_uniform_type_vec3:  __write_uniform_val(cb->commands, gs_vec3, u_data); break;
			case gs_uniform_type_vec4:  __write_uniform_val(cb->commands, gs_vec4, u_data); break;
			case gs_uniform_type_mat4:  __write_uniform_val(cb->commands, gs_mat4, u_data); break;
			case gs_uniform_type_sampler2d:  __write_uniform_val(cb->commands, u32, u_data); break;

			default:
			{
				gs_println("Invalid uniform type passed");
				gs_assert(false);
			}
		}
	});
}

void opengl_bind_uniform_mat4(gs_command_buffer_t* cb, gs_uniform_t u, gs_mat4 val)
{
	__push_command(cb, gs_opengl_op_bind_uniform, {

		if (u.type != gs_uniform_type_mat4) {
			return;	
		}

		// Write out uniform location
		gs_byte_buffer_write(&cb->commands, u32, (u32)u.location);
		// Write out uniform type
		gs_byte_buffer_write(&cb->commands, u32, (u32)u.type);

		__write_uniform_val(cb->commands, gs_mat4, &val);
	});
}

void opengl_bind_texture_id(gs_command_buffer_t* cb, gs_uniform_t u, u32 id, u32 slot)
{
	__push_command(cb, gs_opengl_op_bind_texture, {

		// Cannot pass in uniform of wrong type
		if (u.type != gs_uniform_type_sampler2d)
		{
			gs_println("opengl_bind_texture: Must be of uniform type 'gs_uniform_type_sampler2d'");
			gs_assert(false);
		}

		// Write out id
		gs_byte_buffer_write(&cb->commands, u32, id);
		// Write tex unit location
		gs_byte_buffer_write(&cb->commands, u32, slot);
		// Write out uniform location
		gs_byte_buffer_write(&cb->commands, u32, (u32)u.location);
	});
}

void opengl_bind_texture(gs_command_buffer_t* cb, gs_uniform_t u, 
		gs_texture_t tex, u32 tex_unit)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->bind_texture_id(cb, u, tex.id, tex_unit);
}

void opengl_bind_vertex_buffer(gs_command_buffer_t* cb, gs_vertex_buffer_t vb)
{
	__push_command(cb, gs_opengl_op_bind_vertex_buffer, {

		// Write out vao
		gs_byte_buffer_write(&cb->commands, u32, vb.vao);
	});
}

void opengl_bind_index_buffer(gs_command_buffer_t* cb, gs_index_buffer_t ib)
{
	__push_command(cb, gs_opengl_op_bind_index_buffer, {

		// Write out ibo
		gs_byte_buffer_write(&cb->commands, u32, ib.ibo);
	});
}

void opengl_set_view_scissor(gs_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h)
{
	__push_command(cb, gs_opengl_op_set_view_scissor, {

		gs_byte_buffer_write(&cb->commands, u32, x);
		gs_byte_buffer_write(&cb->commands, u32, y);
		gs_byte_buffer_write(&cb->commands, u32, w);
		gs_byte_buffer_write(&cb->commands, u32, h);
	});
}

void opengl_set_viewport(gs_command_buffer_t* cb, u32 x, u32 y, u32 width, u32 height)
{
	__push_command(cb, gs_opengl_op_set_viewport, {

		gs_byte_buffer_write(&cb->commands, u32, x);
		gs_byte_buffer_write(&cb->commands, u32, y);
		gs_byte_buffer_write(&cb->commands, u32, width);
		gs_byte_buffer_write(&cb->commands, u32, height);
	});
}

// Want to set a bitmask for this as well to determine clear types
void opengl_set_view_clear(gs_command_buffer_t* cb, f32* col)
{
	__push_command(cb, gs_opengl_op_set_view_clear, {

		gs_vec4 c = (gs_vec4){col[0], col[1], col[2], col[3]};

		// Write color into buffer (as vec4)
		gs_byte_buffer_write(&cb->commands, gs_vec4, c);
	});
}

void opengl_set_winding_order(gs_command_buffer_t* cb, gs_winding_order_type type)
{
	__push_command(cb, gs_opengl_op_set_winding_order, 
	{
		gs_byte_buffer_write(&cb->commands, u32, (u32)type);
	});
}

void opengl_set_face_culling(gs_command_buffer_t* cb, gs_face_culling_type type)
{	
	__push_command(cb, gs_opengl_op_set_face_culling, 
	{
		gs_byte_buffer_write(&cb->commands, u32, (u32)type);
	});
}

void opengl_set_blend_equation(gs_command_buffer_t* cb, gs_blend_equation_type eq)
{
	__push_command(cb, gs_opengl_op_set_blend_equation, {

		// Write blend mode for blend equation
		gs_byte_buffer_write(&cb->commands, u32, (u32)eq);
	});

}

void opengl_set_blend_mode(gs_command_buffer_t* cb, gs_blend_mode_type src, gs_blend_mode_type dst)
{
	__push_command(cb, gs_opengl_op_set_blend_mode, {

		// Write blend mode for source
		gs_byte_buffer_write(&cb->commands, u32, (u32)src);
		// Write blend mode for destination
		gs_byte_buffer_write(&cb->commands, u32, (u32)dst);
	});
}

void opengl_set_depth_enabled(gs_command_buffer_t* cb, b32 enable)
{
	__push_command(cb, gs_opengl_op_set_depth_enabled, {

		// Write color into buffer (as vec4)
		gs_byte_buffer_write(&cb->commands, b32, enable);
	});
}

void opengl_draw_indexed(gs_command_buffer_t* cb, u32 count, u32 offset)
{
	__push_command(cb, gs_opengl_op_draw_indexed, {

		// Write count and offset
		gs_byte_buffer_write(&cb->commands, u32, count);
		gs_byte_buffer_write(&cb->commands, u32, offset);
	});
}

void opengl_draw(gs_command_buffer_t* cb, u32 start, u32 count)
{
	__push_command(cb, gs_opengl_op_draw, {

		// Write start
		gs_byte_buffer_write(&cb->commands, u32, start);
		// Write count
		gs_byte_buffer_write(&cb->commands, u32, count);
	});
}

void opengl_update_index_data_command(gs_command_buffer_t* cb, 
	gs_index_buffer_t ib, void* i_data, usize i_size)
{
	// Need to memcpy the data over to the command buffer	
	__push_command(cb, gs_opengl_op_update_index_data, {

		// Write out vao/vbo
		gs_byte_buffer_write(&cb->commands, u32, ib.ibo);
		gs_byte_buffer_write(&cb->commands, u32, i_size);
		gs_byte_buffer_bulk_write(&cb->commands, i_data, i_size);
	});
}

void opengl_update_vertex_data_command(gs_command_buffer_t* cb, gs_vertex_buffer_t vb, 
	void* v_data, usize v_size)
{
	// Need to memcpy the data over to the command buffer	
	__push_command(cb, gs_opengl_op_update_vertex_data, {

		// Write out vao/vbo
		gs_byte_buffer_write(&cb->commands, u32, vb.vao);
		gs_byte_buffer_write(&cb->commands, u32, vb.vbo);
		gs_byte_buffer_write(&cb->commands, u32, v_size);
		gs_byte_buffer_bulk_write(&cb->commands, v_data, v_size);
	});
}

GLenum __get_opengl_blend_mode(gs_blend_mode_type mode)
{
	switch (mode)
	{
		case gs_blend_mode_zero:
		case gs_blend_mode_disabled:
		default: {

			return GL_ZERO;
		} break;

		case gs_blend_mode_one:							return GL_ONE; break;
		case gs_blend_mode_src_color: 					return GL_SRC_COLOR; break;
		case gs_blend_mode_one_minus_src_color: 		return GL_ONE_MINUS_SRC_COLOR; break;
		case gs_blend_mode_dst_color: 					return GL_DST_COLOR; break;
		case gs_blend_mode_one_minus_dst_color: 		return GL_ONE_MINUS_DST_COLOR; break;
		case gs_blend_mode_src_alpha: 					return GL_SRC_ALPHA; break;
		case gs_blend_mode_one_minus_src_alpha: 		return GL_ONE_MINUS_SRC_ALPHA; break;
		case gs_blend_mode_dst_alpha: 					return GL_DST_ALPHA; break;
		case gs_blend_mode_one_minus_dst_alpha: 		return GL_ONE_MINUS_DST_ALPHA; break;
		case gs_blend_mode_constant_color: 				return GL_CONSTANT_COLOR; break;
		case gs_blend_mode_one_minus_constant_color: 	return GL_ONE_MINUS_CONSTANT_COLOR; break;
		case gs_blend_mode_constant_alpha: 				return GL_CONSTANT_ALPHA; break;
		case gs_blend_mode_one_minus_constant_alpha: 	return GL_ONE_MINUS_CONSTANT_ALPHA; break;
	};

	// Shouldn't ever hit here
	return GL_ZERO;
}

GLenum __get_opengl_blend_equation(gs_blend_equation_type eq)
{
	switch (eq)
	{
		case gs_blend_equation_add:					return GL_FUNC_ADD; break; 
		case gs_blend_equation_subtract:			return GL_FUNC_SUBTRACT; break;
		case gs_blend_equation_reverse_subtract:	return GL_FUNC_REVERSE_SUBTRACT; break;
		case gs_blend_equation_min:					return GL_MAX; break;
		case gs_blend_equation_max:					return GL_MIN; break;
		default: 									return GL_ZERO; break;
	}

	// Shouldn't ever hit here
	return GL_ZERO;
}

GLenum __get_opengl_cull_face(gs_face_culling_type type)
{
	switch (type)
	{
		case gs_face_culling_front:					return GL_FRONT; break;
		case gs_face_culling_back:					return GL_BACK; break;
		case gs_face_culling_front_and_back:		return GL_FRONT_AND_BACK; break;

		default:
		case gs_face_culling_disabled: 				return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum __get_opengl_winding_order(gs_winding_order_type type)
{
	switch (type)
	{
		case gs_winding_order_cw:			return GL_CW; break;
		case gs_winding_order_ccw:			return GL_CCW; break;
	}

	return GL_ZERO;
}

typedef struct color_t
{
	f32 r, g, b, a;
} color_t;

typedef struct vert_t
{
	gs_vec2 position;
	gs_vec2 uv;
	color_t color;
} vert_t;

/*====================
// Immediate Utilties
==================-=*/

gs_resource(gs_render_pipeline_state_t) opengl_default_pipeline_state(gs_graphics_immediate_mode mode)
{
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	switch (mode)
	{
		default:
		case gs_immediate_mode_2d: return data->default_state_2d; break;
		case gs_immediate_mode_3d: return data->default_state_3d; break;
	}
}

// What does this do? Binds debug shader and sets default uniform values.
void opengl_immediate_begin_drawing(gs_command_buffer_t* cb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();

	gs_resource(gs_render_pipeline_state_t) state = data->default_state_2d;
	gfx->immediate.push_state(cb, state);

	// Bind shader command
	// gfx->bind_uniform_mat4(cb, data->u_mvp, __gs_default_view_proj_mat());	// Bind mvp matrix uniform
	// gfx->bind_texture(cb, data->u_tex, data->default_texture, 0);

	__push_command(cb, gs_opengl_op_immediate_begin_drawing, {
		// Nothing...
	});
}

void opengl_immediate_end_drawing(gs_command_buffer_t* cb)
{
	__push_command(cb, gs_opengl_op_immediate_end_drawing, {
		// Nothing...
	});
}

void opengl_immediate_flush(gs_command_buffer_t* cb)
{
	__push_command(cb, gs_opengl_op_immediate_flush, {
		// Nothing...
	});
}

void opengl_immediate_push_state(gs_command_buffer_t* cb, gs_resource(gs_render_pipeline_state_t) state)
{
	__push_command(cb, gs_opengl_op_immediate_push_state, {
		gs_byte_buffer_write(&cb->commands, gs_resource(gs_render_pipeline_state_t), state);
	});
}

void opengl_immediate_pop_state(gs_command_buffer_t* cb)
{
	__push_command(cb, gs_opengl_op_immediate_pop_state, {
	});
}

void opengl_immediate_clear(gs_command_buffer_t* cb, f32 r, f32 g, f32 b, f32 a)
{
	__push_command(cb, gs_opengl_op_set_view_clear, 
	{
		gs_vec4 c = (gs_vec4){r, g, b, a};
		gs_byte_buffer_write(&cb->commands, gs_vec4, c);
	});
}

// Will think about a way to handle this better for setting multiple items at once
void opengl_immediate_push_state_attr(gs_command_buffer_t* cb, gs_pipeline_state_attr_type type, ...)
{
	// This isn't correct, since it's looking too early in the frame...
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();

	__push_command(cb, gs_opengl_op_immediate_push_state_attr, 
	{
		gs_byte_buffer_write(&cb->commands, gs_pipeline_state_attr_type, type);

		va_list ap;
		const s32 count = 1;
		va_start(ap, type);
		switch (type)
		{
			case gs_blend_func_src: gs_byte_buffer_write(&cb->commands, gs_blend_mode_type, va_arg(ap, gs_blend_mode_type)); break;
			case gs_blend_func_dst: gs_byte_buffer_write(&cb->commands, gs_blend_mode_type, va_arg(ap, gs_blend_mode_type)); break;
			case gs_blend_equation: gs_byte_buffer_write(&cb->commands, gs_blend_equation_type, va_arg(ap, gs_blend_equation_type)); break;
			case gs_depth_enabled: 	gs_byte_buffer_write(&cb->commands, b32, va_arg(ap, b32)); break;
			case gs_winding_order: 	gs_byte_buffer_write(&cb->commands, gs_winding_order_type, va_arg(ap, gs_winding_order_type)); break;
			case gs_face_culling: 	gs_byte_buffer_write(&cb->commands, gs_face_culling_type, va_arg(ap, gs_face_culling_type)); break;
			case gs_viewport: 		gs_byte_buffer_write(&cb->commands, gs_vec2, va_arg(ap, gs_vec2)); break;
			case gs_view_scissor: 	gs_byte_buffer_write(&cb->commands, gs_vec4, va_arg(ap, gs_vec4)); break;
			case gs_shader: 		gs_byte_buffer_write(&cb->commands, gs_shader_t, va_arg(ap, gs_shader_t)); break;
		}
		va_end(ap);
	});
}

// Pop previous state
void opengl_pop_start_attr(gs_command_buffer_t* cb)
{
	opengl_immediate_pop_state(cb);
}

void opengl_immediate_enable_texture_2d(gs_command_buffer_t* cb, u32 id)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	__push_command(cb, gs_opengl_op_immediate_enable_texture_2d, {
		gs_byte_buffer_write(&cb->commands, u32, id);
	});
	// gfx->bind_texture_id(cb, data->u_tex, id, 0);

	// Set the uniform for the top state active.
	// How do I handle this? Would have to pass the uniform
}

void opengl_immediate_disable_texture_2d(gs_command_buffer_t* cb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	__push_command(cb, gs_opengl_op_immediate_disable_texture_2d, {
		// Nothing...
	});
	// gfx->bind_texture_id(cb, data->u_tex, data->default_texture.id, 0);
}

void opengl_immediate_push_matrix(gs_command_buffer_t* cb, gs_matrix_mode mode)
{
	__push_command(cb, gs_opengl_op_immediate_push_matrix, {
		gs_byte_buffer_write(&cb->commands, u32, (u32)mode);
	});
}

void opengl_immediate_pop_matrix(gs_command_buffer_t* cb)
{
	__push_command(cb, gs_opengl_op_immediate_pop_matrix, {
	});
}

void opengl_immediate_mat_mul(gs_command_buffer_t* cb, gs_mat4 m)
{
	__push_command(cb, gs_opengl_op_immediate_mat_mul, {
		gs_byte_buffer_write(&cb->commands, gs_mat4, m);
	});
}

void opengl_immediate_begin(gs_command_buffer_t* cb, gs_draw_mode mode)
{
	__push_command(cb, gs_opengl_op_immediate_begin, {
		gs_byte_buffer_write(&cb->commands, u32, (u32)mode);
	});
}

void opengl_immediate_end(gs_command_buffer_t* cb)
{
	__push_command(cb, gs_opengl_op_immediate_end, {
	});
}

void opengl_immediate_color_ubv(gs_command_buffer_t* cb, gs_color_t color)
{
	// This should push a vertex color onto the op stack
	__push_command(cb, gs_opengl_op_immediate_color_ubv, {
		gs_byte_buffer_write(&cb->commands, gs_color_t, color);
	});
}

void opengl_immediate_vertex_3fv(gs_command_buffer_t* cb, gs_vec3 v)
{
	__push_command(cb, gs_opengl_op_immediate_vertex_3fv, {
		gs_byte_buffer_write(&cb->commands, gs_vec3, v);
	});
}

void opengl_immediate_texcoord_2fv(gs_command_buffer_t* cb, gs_vec2 v)
{
	__push_command(cb, gs_opengl_op_immediate_texcoord_2fv, {
		gs_byte_buffer_write(&cb->commands, gs_vec2, v);
	});
}

void opengl_immediate_texcoord_2f(gs_command_buffer_t* cb, f32 s, f32 t)
{
	opengl_immediate_texcoord_2fv(cb, gs_v2(s, t));
}

void opengl_immediate_color_ub(gs_command_buffer_t* cb, u8 r, u8 g, u8 b, u8 a)
{
	opengl_immediate_color_ubv(cb, gs_color(r, g, b, a));
}

void opengl_immediate_color_4f(gs_command_buffer_t* cb, f32 r, f32 g, f32 b, f32 a)
{
	opengl_immediate_color_ub(cb, (u8)(r * 255.f), (u8)(g * 255.f), (u8)(b * 255.f), (u8)(a * 255.f));
}

void opengl_immediate_color_4v(gs_command_buffer_t* cb, gs_vec4 v)
{
	opengl_immediate_color_ub(cb, (u8)(v.x * 255.f), (u8)(v.y * 255.f), (u8)(v.z * 255.f), (u8)(v.w * 255.f));
}

void opengl_immediate_vertex_2fv(gs_command_buffer_t* cb, gs_vec2 v)
{
	opengl_immediate_vertex_3fv(cb, gs_v3(v.x, v.y, 0.f));
}

void opengl_immediate_vertex_3f(gs_command_buffer_t* cb, f32 x, f32 y, f32 z)
{
	opengl_immediate_vertex_3fv(cb, gs_v3(x, y, z));
}

void opengl_immediate_vertex_2f(gs_command_buffer_t* cb, f32 x, f32 y)
{
	opengl_immediate_vertex_3f(cb, x, y, 0.f);
}

// Immediate Ops
void gs_begin(gs_draw_mode mode) 
{
	// Need to pop current state and restore preivous state
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	if (data->draw_mode != mode) {
		opengl_immediate_submit_vertex_data();
	}
	data->draw_mode = mode;
	data->color = gs_color_white;
	data->texcoord = gs_v2(0.f, 0.f);
}

void gs_end()
{
}

void gs_push_matrix_uniform()
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	immediate_drawing_internal_data_t* _data = __get_opengl_immediate_data();
	gs_mat4 mvp = gs_dyn_array_back(_data->vp_matrix_stack);

	// Just set the uniform value for top state
	gs_render_pipeline_state_t state = gs_dyn_array_back(_data->state_stack);
	gfx->uniform_i->set_uniform_from_shader(state.uniforms, state.desc.shader, gs_uniform_type_mat4, "u_mvp", mvp);
}

#define gs_push_matrix(mode)\
do {\
	immediate_drawing_internal_data_t* _data = __get_opengl_immediate_data();\
	gs_dyn_array_push(_data->matrix_modes, mode);\
	switch(mode) {\
		case gs_matrix_model:\
		{\
			gs_dyn_array_push(_data->model_matrix_stack, gs_mat4_identity());\
		} break;\
		case gs_matrix_vp:\
		{\
			gs_dyn_array_push(_data->vp_matrix_stack, gs_mat4_identity());\
			gs_push_matrix_uniform();\
		} break;\
	}\
} while(0)

#define gs_pop_matrix()\
do {\
	immediate_drawing_internal_data_t* _data = __get_opengl_immediate_data();\
	if (!gs_dyn_array_empty(_data->matrix_modes)) {\
		gs_matrix_mode mode = gs_dyn_array_back(_data->matrix_modes);\
		/* Flush data if necessary */\
		if (mode == gs_matrix_vp) {\
			opengl_immediate_submit_vertex_data();\
		}\
		gs_dyn_array_pop(_data->matrix_modes);\
		switch(mode) {\
			case gs_matrix_model:\
			{\
				gs_dyn_array_pop(_data->model_matrix_stack);\
			} break;\
			case gs_matrix_vp:\
			{\
				gs_dyn_array_pop(_data->vp_matrix_stack);\
				gs_push_matrix_uniform();\
			} break;\
		}\
	}\
} while(0)

void gs_vert3fv(gs_vec3 v)
{
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	immediate_vertex_data_t vert;
	vert.color = data->color;
	vert.texcoord = data->texcoord;
	vert.position = v;
	/*Check if it's necessary to transform vert by model matrix stack*/
	gs_for_range_i(gs_dyn_array_size(data->model_matrix_stack))
	{
		gs_vec4 v = gs_mat4_mul_vec4(
			data->model_matrix_stack[i],
			gs_v4(vert.position.x, vert.position.y, vert.position.z, 1.f)
		);
		vert.position = gs_v4_to_v3(v);
	}
	gs_dyn_array_push(data->vertex_data, vert);
}

void gs_texcoord2fv(gs_vec2 v)
{
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	data->texcoord = v;
}

void gs_color4ubv(gs_color_t c)
{
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	data->color = c;
}

void gs_set_view_clear(gs_vec4 col)
{
	// Set clear color
	glClearColor(col.x, col.y, col.z, col.w);
	// Clear screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gs_set_view_scissor(s32 x, s32 y, s32 w, s32 h)
{
	if (x == 0 && y == 0 && w == 0 && h == 0)
	{
		glDisable(GL_SCISSOR_TEST);	
	}
	else 
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor((s32)x, (s32)y, (usize)w, (usize)h);	
	}
}

void gs_set_viewport(s32 x, s32 y, s32 width, s32 height)
{
	// Set viewport
	glViewport(x, y, (s32)width, (s32)height);
}

void gs_set_depth_enabled(b32 enabled)
{
	// Clear screen
	switch (enabled)
	{
		case true: glEnable(GL_DEPTH_TEST); break;
		case false: glDisable(GL_DEPTH_TEST); break;
	}
}

void gs_set_winding_order(gs_winding_order_type type)
{
	glFrontFace(__get_opengl_winding_order(type));
}

void gs_set_face_culling(gs_face_culling_type type)
{
	switch(type)
	{
		case gs_face_culling_disabled: glDisable(GL_CULL_FACE); break;
		default:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(__get_opengl_cull_face(type));
		} break;
	}
}

void gs_set_blend_equation(gs_blend_equation_type type)
{
	glBlendEquation(__get_opengl_blend_equation(type));
}

void gs_set_blend_mode(gs_blend_mode_type src, gs_blend_mode_type dst)
{
	if (src == gs_blend_mode_disabled || dst == gs_blend_mode_disabled) 
	{
		glDisable(GL_BLEND);
	} 
	else 
	{
		glEnable(GL_BLEND);
		glBlendFunc(__get_opengl_blend_mode(src), __get_opengl_blend_mode(dst));	
	}
}

#define gs_bind_texture(tex_unit, tex_id, location)\
do {\
	glActiveTexture(GL_TEXTURE0 + tex_unit);\
	glBindTexture(GL_TEXTURE_2D, tex_id);\
	glUniform1i(location, tex_unit);\
} while(0)

#define gs_bind_uniform_float(location, val)\
	glUniform1f((location), (val));

#define gs_bind_uniform_int(location, val)\
	glUniform1i((location), (val));

#define gs_bind_uniform_vec2(location, val)\
	glUniform2f((location), (val).x, (val).y);

#define gs_bind_uniform_vec3(location, val)\
	glUniform3f((location), (val).x, (val).y, (val).z);

#define gs_bind_uniform_vec4(location, val)\
	glUniform4f((location), (val).x, (val).y, (val).z, (val).w);

#define gs_bind_uniform_mat4(location, val)\
	glUniformMatrix4fv((location), 1, false, (f32*)((val).elements));

#define gs_bind_index_buffer(ibo)\
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

#define gs_bind_vertex_buffer(vao)\
	glBindVertexArray(vao);

#define gs_bind_shader(program_id)\
	glUseProgram(program_id);

void gs_set_state(gs_render_pipeline_state_t state)
{
	gs_set_blend_mode(state.desc.blend_func_src, state.desc.blend_func_dst);
	gs_set_blend_equation(state.desc.blend_equation);
	gs_set_depth_enabled(state.desc.depth_enabled);
	gs_set_winding_order(state.desc.winding_order);
	gs_set_face_culling(state.desc.face_culling);
	gs_set_viewport(state.desc.viewport.x, state.desc.viewport.y, state.desc.viewport.z, state.desc.viewport.w);
	gs_set_view_scissor(state.desc.view_scissor.x, state.desc.view_scissor.y, state.desc.view_scissor.z, state.desc.view_scissor.w);
	gs_bind_shader(state.desc.shader.program_id);
}

void opengl_immediate_push_top_state(gs_render_pipeline_state_t state)
{
	// Submit previous vertex data
	opengl_immediate_submit_vertex_data();

	// Push new state and set
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	gs_dyn_array_push(data->state_stack, state);
	gs_set_state(state);
}

// This all needs to change.
void opengl_immediate_pop_top_state()
{
	// Submit previous vertex data
	opengl_immediate_submit_vertex_data();

	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	if (!gs_dyn_array_empty(data->state_stack))
	{
		gs_dyn_array_pop(data->state_stack);
	}
	if (gs_dyn_array_empty(data->state_stack))
	{
		gs_render_pipeline_state_t state = gs_slot_array_get(gfx->render_pipelines, 
			gfx->immediate.default_pipeline_state(gs_immediate_mode_2d).id);
		gs_dyn_array_push(data->state_stack, state);
	}
	gs_set_state(gs_dyn_array_back(data->state_stack));
}

// This is temporary to set manually from state
void gs_bind_uniform_block(gs_resource(gs_uniform_block_t) uniforms)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_uniform_block_t* u_block = gs_slot_array_get_ptr(gfx->uniform_i->uniform_blocks, uniforms.id);

	// Set data to beginning
	gs_byte_buffer_seek_to_beg(&u_block->data);

	// Rip through uniforms, determine size, then bind accordingly
	gs_for_range_i(u_block->count)
	{
		// Grab uniform
		gs_uniform_t uniform;
		gs_byte_buffer_bulk_read(&u_block->data, &uniform, sizeof(gs_uniform_t));
		// Grab data size
		usize sz; gs_byte_buffer_read(&u_block->data, usize, &sz);
		// Grab type
		gs_uniform_type type = uniform.type;

		// Grab data based on type and bind
		switch (type)
		{
			case gs_uniform_type_sampler2d:
			{
				gs_uniform_block_type(texture_sampler) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_texture(value.slot, value.data, uniform.location);
			} break;

			case gs_uniform_type_mat4:
			{
				gs_uniform_block_type(mat4) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_uniform_mat4(uniform.location, value.data);
			} break;

			case gs_uniform_type_vec4:
			{
				gs_uniform_block_type(vec4) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_uniform_vec4(uniform.location, value.data);
			} break;

			case gs_uniform_type_vec3:
			{
				gs_uniform_block_type(vec3) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_uniform_vec3(uniform.location, value.data);
			} break;

			case gs_uniform_type_vec2:
			{
				gs_uniform_block_type(vec2) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_uniform_vec2(uniform.location, value.data);
			} break;

			case gs_uniform_type_float:
			{
				gs_uniform_block_type(float) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_uniform_float(uniform.location, value.data);
			} break;

			case gs_uniform_type_int:
			{
				gs_uniform_block_type(int) value = gs_default_val();
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gs_bind_uniform_int(uniform.location, value.data);
			} break;
 		}
	}
}

// Push vertex data util for immediate mode data
void opengl_immediate_submit_vertex_data()
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
	u32 count = gs_dyn_array_size(data->vertex_data);

	if (!count) {
		return;
	}

	// Bind current state uniforms
	gs_assert(!gs_dyn_array_empty(data->state_stack));
	gs_render_pipeline_state_t* state = &gs_dyn_array_back(data->state_stack);
	gs_bind_uniform_block(state->uniforms);

	u32 vao = g_default_vertex_buffer.vao;
	u32 vbo = g_default_vertex_buffer.vbo;
	usize sz = count * sizeof(immediate_vertex_data_t);

	u32 mode;
	switch (data->draw_mode) {
		case gs_lines: mode = GL_LINES; break;
		case gs_quads: mode = GL_QUADS; break;
		case gs_triangles: mode = GL_TRIANGLES; break;
		default: mode = GL_TRIANGLES;
	} 

	// Final submit
	glBindVertexArray(vao);	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sz, data->vertex_data, GL_STATIC_DRAW);
	glDrawArrays(mode, 0, count);

	// Unbind buffer and array
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Clear buffer
	gs_dyn_array_clear(data->vertex_data);
}

/*================
// Submit Function
=================*/

// For now, just rip through command buffer. Later on, will add all command buffers into a queue to be processed on rendering thread.
void opengl_submit_command_buffer(gs_command_buffer_t* cb)
{
	/*
		// Structure of command: 
			- Op code
			- Data packet
	*/

	// Set read position of buffer to beginning
	gs_byte_buffer_seek_to_beg(&cb->commands);

	u32 num_commands = cb->num_commands;

	// For each command in buffer
	gs_for_range_i(num_commands)
	{
		// Read in op code of command
		gs_byte_buffer_readc(&cb->commands, gs_opengl_op_code, op_code);

		switch (op_code)
		{
			case gs_opengl_op_set_frame_buffer_attachment: 
			{
				gs_byte_buffer_readc(&cb->commands, u32, t_id);
				gs_byte_buffer_readc(&cb->commands, u32, idx);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, t_id, 0);
			} break;

			case gs_opengl_op_bind_frame_buffer: 
			{
				gs_byte_buffer_readc(&cb->commands, u32, fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			} break;

			case gs_opengl_op_unbind_frame_buffer: 
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			} break;

			case gs_opengl_op_draw:
			{
				gs_byte_buffer_readc(&cb->commands, u32, start);
				gs_byte_buffer_readc(&cb->commands, u32, count);
				glDrawArrays(GL_TRIANGLES, start, count);
			} break;

			case gs_opengl_op_draw_indexed:
			{
				// Read count and offest
				gs_byte_buffer_readc(&cb->commands, u32, count);
				gs_byte_buffer_readc(&cb->commands, u32, offset);
				glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, int_2_void_p(offset));
			} break;

			case gs_opengl_op_set_view_clear: 
			{
				gs_byte_buffer_readc(&cb->commands, gs_vec4, col);
				gs_set_view_clear(col);
			} break;

			case gs_opengl_op_set_view_scissor: 
			{
				gs_byte_buffer_readc(&cb->commands, u32, x);
				gs_byte_buffer_readc(&cb->commands, u32, y);
				gs_byte_buffer_readc(&cb->commands, u32, w);
				gs_byte_buffer_readc(&cb->commands, u32, h);
				gs_set_view_scissor(x, y, w, h);
			} break;

			case gs_opengl_op_set_viewport: 
			{
				gs_byte_buffer_readc(&cb->commands, u32, x);
				gs_byte_buffer_readc(&cb->commands, u32, y);
				gs_byte_buffer_readc(&cb->commands, u32, width);
				gs_byte_buffer_readc(&cb->commands, u32, height);
				gs_set_viewport(x, y, width, height);
			} break;

			case gs_opengl_op_set_depth_enabled: 
			{
				gs_byte_buffer_readc(&cb->commands, b32, enabled);
				gs_set_depth_enabled(enabled);
			} break;

			case gs_opengl_op_set_winding_order: 
			{
				gs_byte_buffer_readc(&cb->commands, gs_winding_order_type, type);
				gs_set_winding_order(type);
			} break;			

			case gs_opengl_op_set_face_culling: 
			{
				gs_byte_buffer_readc(&cb->commands, gs_face_culling_type, type);
				gs_set_face_culling(type);
			} break;

			case gs_opengl_op_set_blend_equation:
			{
				gs_byte_buffer_readc(&cb->commands, gs_blend_equation_type, type);
				gs_set_blend_equation(type);
			} break;

			case gs_opengl_op_set_blend_mode: 
			{
				gs_byte_buffer_readc(&cb->commands, gs_blend_mode_type, src);
				gs_byte_buffer_readc(&cb->commands, gs_blend_mode_type, dst);
				gs_set_blend_mode(src, dst);
			} break;

			case gs_opengl_op_bind_texture:
			{
				gs_byte_buffer_readc(&cb->commands, u32, tex_id);
				gs_byte_buffer_readc(&cb->commands, u32, tex_unit);
				gs_byte_buffer_readc(&cb->commands, u32, location);
				gs_bind_texture(tex_unit, tex_id, location);
			} break;

			case gs_opengl_op_bind_vertex_buffer:
			{
				gs_byte_buffer_readc(&cb->commands, u32, vao);
				gs_bind_vertex_buffer(vao);
			} break;

			case gs_opengl_op_bind_index_buffer:
			{
				// Read out vao
				gs_byte_buffer_readc(&cb->commands, u32, ibo);
				gs_bind_index_buffer(ibo);
			} break;

			case gs_opengl_op_bind_shader: 
			{
				gs_byte_buffer_readc(&cb->commands, u32, program_id);
				gs_bind_shader(program_id);
			} break;

			case gs_opengl_op_bind_uniform:
			{
				gs_byte_buffer_readc(&cb->commands, u32, location);
				gs_byte_buffer_readc(&cb->commands, gs_uniform_type, type);

				// Read and bind val
				switch (type)
				{
					case gs_uniform_type_float:
					{
						gs_byte_buffer_readc(&cb->commands, f32, val);
						glUniform1f(location, val);
					} break;

					case gs_uniform_type_int: 
					{
						gs_byte_buffer_readc(&cb->commands, s32, val);
						glUniform1i(location, val);
					} break;

					case gs_uniform_type_vec2:
					{
						gs_byte_buffer_readc(&cb->commands, gs_vec2, val);
						glUniform2f(location, val.x, val.y);
					} break;

					case gs_uniform_type_vec3:
					{
						gs_byte_buffer_readc(&cb->commands, gs_vec3, val);
						glUniform3f(location, val.x, val.y, val.z);
					} break;

					case gs_uniform_type_vec4:
					{
						gs_byte_buffer_readc(&cb->commands, gs_vec4, val);
						glUniform4f(location, val.x, val.y, val.z, val.w);
					} break;

					case gs_uniform_type_mat4:
					{
						gs_byte_buffer_readc(&cb->commands, gs_mat4, val);
						glUniformMatrix4fv(location, 1, false, (f32*)(val.elements));
					} break;

					case gs_uniform_type_sampler2d:
					{
						gs_byte_buffer_readc(&cb->commands, u32, val);
						glUniform1i(location, val);
					} break;

					default: 
					{
						gs_println("Invalid uniform type read.");
						gs_assert(false);
					}
				}

			} break;

			case gs_opengl_op_update_index_data:
			{
				// Write out vao/vbo
				gs_byte_buffer_readc(&cb->commands, u32, ibo);
				gs_byte_buffer_readc(&cb->commands, u32, data_size);

				void* tmp_data = gs_malloc(data_size);
				memset(tmp_data, 0, data_size);
				gs_byte_buffer_bulk_read(&cb->commands, tmp_data, data_size);

				// Update index data
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (u32)ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size, tmp_data, GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				gs_free(tmp_data);
				tmp_data = NULL;
			} break;

			case gs_opengl_op_update_vertex_data: 
			{
				// Read in vao
				gs_byte_buffer_readc(&cb->commands, u32, vao);
				gs_byte_buffer_readc(&cb->commands, u32, vbo);
				u32 data_size; gs_byte_buffer_read(&cb->commands, u32, &data_size);

				// Read in data for vertices
				void* tmp_data = gs_malloc(data_size);
				memset(tmp_data, 0, data_size);
				gs_byte_buffer_bulk_read(&cb->commands, tmp_data, data_size);

				// Update vertex data
				// Bind vao/vbo
				glBindVertexArray((u32)vao);
				glBindBuffer(GL_ARRAY_BUFFER, (u32)vbo);
				glBufferData(GL_ARRAY_BUFFER, data_size, tmp_data, GL_STATIC_DRAW);

				// Unbind buffer and array
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				gs_free(tmp_data);
				tmp_data = NULL;

			} break;

			/*===============
			// Immediate Mode
			===============*/

			case gs_opengl_op_immediate_begin_drawing:
			{
				// Push any necessary state here
				// Clear previous vertex data

				// Default stacks
				immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
				gs_mat4 ortho = __gs_default_view_proj_mat();
				gs_dyn_array_push(data->vp_matrix_stack, ortho);

				data->tex_id = g_default_texture.id;
				data->draw_mode = gs_triangles;
			} break;

			case gs_opengl_op_immediate_end_drawing:
			{
				// Time to draw da data
				opengl_immediate_submit_vertex_data();

				immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
				gs_dyn_array_clear(data->vertex_data);

				// Clear stacks
				gs_dyn_array_clear(data->model_matrix_stack);
				gs_dyn_array_clear(data->vp_matrix_stack);
				gs_dyn_array_clear(data->viewport_stack);
				gs_dyn_array_clear(data->matrix_modes);
				gs_dyn_array_clear(data->state_stack);

				// Reset default states
				gs_reset_default_states();
			} break;

			case gs_opengl_op_immediate_flush:
			{
				opengl_immediate_submit_vertex_data();
			} break;

			case gs_opengl_op_immediate_push_state:
			{
				gs_byte_buffer_readc(&cb->commands, gs_resource(gs_render_pipeline_state_t), state_h);
				gs_render_pipeline_state_t state = gs_slot_array_get(gs_engine_subsystem(graphics)->render_pipelines, state_h.id);
				opengl_immediate_push_top_state(state);
			} break;

			case gs_opengl_op_immediate_pop_state:
			{
				// Push back new state, then submit vertex data
				opengl_immediate_pop_top_state();
			} break;

			case gs_opengl_op_immediate_push_state_attr:
			{
				immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
				gs_byte_buffer_readc(&cb->commands, gs_pipeline_state_attr_type, type);

				// Update last state
				gs_render_pipeline_state_t state = gs_dyn_array_back(data->state_stack);
				switch (type)
				{
					case gs_blend_func_src: gs_byte_buffer_read(&cb->commands, gs_blend_mode_type, &state.desc.blend_func_src); break;
					case gs_blend_func_dst: gs_byte_buffer_read(&cb->commands, gs_blend_mode_type, &state.desc.blend_func_dst); break;
					case gs_blend_equation: gs_byte_buffer_read(&cb->commands, gs_blend_equation_type, &state.desc.blend_equation); break;
					case gs_depth_enabled: 	gs_byte_buffer_read(&cb->commands, b32, &state.desc.depth_enabled); break;
					case gs_winding_order: 	gs_byte_buffer_read(&cb->commands, gs_winding_order_type, &state.desc.winding_order); break;
					case gs_face_culling: 	gs_byte_buffer_read(&cb->commands, gs_face_culling_type, &state.desc.face_culling); break;
					case gs_viewport: 		gs_byte_buffer_read(&cb->commands, gs_vec2, &state.desc.viewport); break;
					case gs_view_scissor: 	gs_byte_buffer_read(&cb->commands, gs_vec4, &state.desc.view_scissor); break;
					case gs_shader: 		gs_byte_buffer_read(&cb->commands, gs_shader_t, &state.desc.shader); break;
				}
				opengl_immediate_push_top_state(state);
			} break;

			case gs_opengl_op_immediate_enable_texture_2d:
			{
				// If current texture doesn't equal new texture, then submit vertex data
				gs_graphics_i* gfx = gs_engine_subsystem(graphics);
				immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
				u32 tex_id; gs_byte_buffer_read(&cb->commands, u32, &tex_id);
				if (data->tex_id != tex_id) {
					opengl_immediate_submit_vertex_data();
				}
				data->tex_id = tex_id;

				// Bind new uniform for top state (u_tex)
				gs_render_pipeline_state_t state = gs_dyn_array_back(data->state_stack);
				gfx->uniform_i->set_uniform_from_shader(state.uniforms, state.desc.shader, gs_uniform_type_sampler2d, "u_tex", tex_id, 0);
			} break;

			case gs_opengl_op_immediate_disable_texture_2d:
			{
				gs_graphics_i* gfx = gs_engine_subsystem(graphics);
				immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
				if (data->tex_id != g_default_texture.id) {
					opengl_immediate_submit_vertex_data();
				}
				gs_render_pipeline_state_t state = gs_dyn_array_back(data->state_stack);
				gfx->uniform_i->set_uniform_from_shader(state.uniforms, state.desc.shader, gs_uniform_type_sampler2d, "u_tex", g_default_texture.id, 0);
			} break;

			case gs_opengl_op_immediate_begin:
			{
				gs_draw_mode mode; gs_byte_buffer_read(&cb->commands, gs_draw_mode, &mode);
				gs_begin(mode);
			} break;

			case gs_opengl_op_immediate_end:
			{
				gs_end();
			} break;

			case gs_opengl_op_immediate_push_matrix:
			{
				gs_matrix_mode mode; gs_byte_buffer_read(&cb->commands, gs_matrix_mode, &mode);

				if (mode == gs_matrix_vp) {
					opengl_immediate_submit_vertex_data();
				}

				gs_push_matrix(mode);
			} break;

			case gs_opengl_op_immediate_pop_matrix:
			{
				// Then pop matrix
				gs_pop_matrix();
			} break;

			case gs_opengl_op_immediate_mat_mul:
			{
				// If we're applying this to the vp matrix, need to update the uniform
				immediate_drawing_internal_data_t* data = __get_opengl_immediate_data();
				if (!gs_dyn_array_empty(data->matrix_modes)) {
					gs_mat4 m; gs_byte_buffer_read(&cb->commands, gs_mat4, &m);
					switch (gs_dyn_array_back(data->matrix_modes)) {
						case gs_matrix_vp:
						{
							gs_mat4* mm = &gs_dyn_array_back(data->vp_matrix_stack);
							*mm = gs_mat4_mul(*mm, m);
							gs_push_matrix_uniform();
						} break;
						case gs_matrix_model:
						{
							gs_mat4* mm = &gs_dyn_array_back(data->model_matrix_stack);
							*mm = gs_mat4_mul(*mm, m);
						} break;
					}
				}
			} break;

			case gs_opengl_op_immediate_vertex_3fv:
			{
				gs_vec3 v; gs_byte_buffer_read(&cb->commands, gs_vec3, &v);
				gs_vert3fv(v);
			} break;

			case gs_opengl_op_immediate_texcoord_2fv:
			{
				gs_vec2 v; gs_byte_buffer_read(&cb->commands, gs_vec2, &v);
				gs_texcoord2fv(v);
			} break;

			case gs_opengl_op_immediate_color_ubv:
			{
				gs_color_t v; gs_byte_buffer_read(&cb->commands, gs_color_t, &v);
				gs_color4ubv(v);
			} break;

			default:
			{
				// Op code not supported yet!
				gs_println("Op code not supported yet: %zu", (u32)op_code);
				gs_assert(false);
			}
		}
	}

	// Reset command buffer
	__reset_command_buffer_internal(cb);

	// Reset default opengl state
	opengl_init_default_state();
}

u32 get_byte_size_of_vertex_attribute(gs_vertex_attribute_type type)
{
	u32 byte_size = 0; 
	switch (type)
	{
		case gs_vertex_attribute_float4:	{ byte_size = sizeof(f32) * 4; } break;
		case gs_vertex_attribute_float3:	{ byte_size = sizeof(f32) * 3; } break;
		case gs_vertex_attribute_float2:	{ byte_size = sizeof(f32) * 2; } break;
		case gs_vertex_attribute_float:		{ byte_size = sizeof(f32) * 1; } break;
		case gs_vertex_attribute_uint4:		{ byte_size = sizeof(u32) * 4; } break;
		case gs_vertex_attribute_uint3:		{ byte_size = sizeof(u32) * 3; } break;
		case gs_vertex_attribute_uint2:		{ byte_size = sizeof(u32) * 2; } break;
		case gs_vertex_attribute_uint:		{ byte_size = sizeof(u32) * 1; } break;
		case gs_vertex_attribute_byte4:		{ byte_size = sizeof(u8) * 4; } break;
		case gs_vertex_attribute_byte3:		{ byte_size = sizeof(u8) * 3; } break;
		case gs_vertex_attribute_byte2:		{ byte_size = sizeof(u8) * 2; } break;
		case gs_vertex_attribute_byte:		{ byte_size = sizeof(u8) * 1; } break;
	} 

	return byte_size;
}

u32 calculate_vertex_size_in_bytes(gs_vertex_attribute_type* layout_data, u32 count)
{
	// Iterate through all formats in delcarations and calculate total size
	u32 sz = 0;

	gs_for_range_i(count)
	{
		gs_vertex_attribute_type type = layout_data[i];
		sz += get_byte_size_of_vertex_attribute(type);
	}

	return sz;
}

s32 get_byte_offest(gs_vertex_attribute_type* layout_data, u32 index)
{
	gs_assert(layout_data);

	// Recursively calculate offset
	s32 total_offset = 0;

	// Base case
	if (index == 0)
	{
		return total_offset;
	} 

	// Calculate total offset up to this point
	for (u32 i = 0; i < index; ++i)
	{ 
		total_offset += get_byte_size_of_vertex_attribute(layout_data[i]);
	} 

	return total_offset;
}

gs_vertex_buffer_t opengl_construct_vertex_buffer(gs_vertex_attribute_type* layout_data, usize layout_sz, 
	void* v_data, usize v_data_size)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	gs_vertex_buffer_t vb = gs_default_val();

	// Create and bind vertex array
	glGenVertexArrays(1, (u32*)&vb.vao);
	glBindVertexArray((u32)vb.vao);

	// Create and upload mesh data
	glGenBuffers(1, (u32*)&vb.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, (u32)vb.vbo);
	glBufferData(GL_ARRAY_BUFFER, v_data_size, v_data, GL_STATIC_DRAW);

	u32 layout_count = layout_sz / sizeof(gs_vertex_attribute_type);
	u32 total_size = calculate_vertex_size_in_bytes(layout_data, layout_count);

	// Bind vertex attrib pointers for vao using layout descriptor
	gs_for_range_i(layout_count)
	{
		gs_vertex_attribute_type type = layout_data[i];
		u32 byte_offset = get_byte_offest(layout_data, i);

		switch (type)
		{
			case gs_vertex_attribute_float4: 
			{
				glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_float3: 
			{
				glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_float2: 
			{
				glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_float: 
			{
				glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_uint4: {
				glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_uint3: 
			{
				glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_uint2: 
			{
				glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_uint: 
			{
				glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_byte: 
			{
				glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_byte2: 
			{
				glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_byte3: 
			{
				glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, total_size, int_2_void_p(byte_offset));
			} break;	
			case gs_vertex_attribute_byte4: 
			{
				glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, total_size, int_2_void_p(byte_offset));
			} break;	

			default: 
			{
				// Shouldn't get here
				gs_assert(false);
			} break;
		}

		// Enable the vertex attribute pointer
		glEnableVertexAttribArray(i);
	}

	// Unbind buffer and array
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return vb;
}

void opengl_update_vertex_buffer_data(gs_vertex_buffer_t vb, void* v_data, usize v_sz)
{
	gs_assert(v_data);

	opengl_render_data_t* data = __get_opengl_data_internal();

	// Bind vao/vbo
	glBindVertexArray((u32)vb.vao);
	glBindBuffer(GL_ARRAY_BUFFER, (u32)vb.vbo);
	glBufferData(GL_ARRAY_BUFFER, v_sz, v_data, GL_STATIC_DRAW);

	// Unbind buffer and array
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

gs_index_buffer_t opengl_construct_index_buffer(void* indices, usize sz)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	gs_index_buffer_t ib = gs_default_val();

	glGenBuffers(1, &ib.ibo);
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, indices, GL_STATIC_DRAW);

    // Unbind buffer after setting data
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return ib;
}

// Compiles single shader
void opengl_compile_shader(const char* src, u32 id) 
{
	//Tell opengl that we want to use fileContents as the contents of the shader file
	glShaderSource(id, 1, &src, NULL);

	//Compile the shader
	glCompileShader(id);

	//Check for errors
	GLint success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		GLint max_len = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &max_len);

		char* log = gs_malloc(max_len);
		memset(log, 0, max_len);

		//The max_len includes the NULL character
		glGetShaderInfoLog(id, max_len, &max_len, log);

		//Provide the infolog in whatever manor you deem best.
		//Exit with failure.
		glDeleteShader(id); //Don't leak the shader.

		gs_println("Opengl::opengl_compile_shader::FAILED_TO_COMPILE: %s\n %s", log, src);

		free(log);
		log = NULL;

		gs_assert(false);
	}
}

void opengl_link_shaders(u32 program_id, u32 vert_id, u32 frag_id)
{
	//Attach our shaders to our program
	glAttachShader(program_id, vert_id);
	glAttachShader(program_id, frag_id);

	//Link our program
	glLinkProgram(program_id);

	//Create info log
	// Error shit...
	s32 is_linked = 0;
	glGetProgramiv(program_id, GL_LINK_STATUS, (s32*)&is_linked);
	if (is_linked == GL_FALSE)
	{
		GLint max_len = 0;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &max_len);

		char* log = gs_malloc(max_len);
		memset(log, 0, max_len);
		glGetProgramInfoLog(program_id, max_len, &max_len, log); 

		// Print error
		gs_println("Fail To Link::opengl_link_shaders::%s");

		// //We don't need the program anymore.
		glDeleteProgram(program_id);

		// //Don't leak shaders either.
		glDeleteShader(vert_id);
		glDeleteShader(frag_id);

		free(log);
		log = NULL;

		// Just assert for now
		gs_assert(false);
	}
}

void opengl_quad_batch_begin(gs_quad_batch_t* qb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->quad_batch_i->begin(qb);
}

void opengl_quad_batch_add(gs_quad_batch_t* qb, void* qb_data) 
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->quad_batch_i->add(qb, qb_data);
}

void opengl_quad_batch_end(gs_quad_batch_t* qb) 
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->quad_batch_i->end(qb);
}

void opengl_quad_batch_submit(gs_command_buffer_t* cb, gs_quad_batch_t* qb) 
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->quad_batch_i->submit(cb, qb);
}

void opengl_set_material_uniform(gs_resource(gs_material_t) mat, gs_uniform_type type, const char* name, void* data)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	opengl_render_data_t* __data = __get_opengl_data_internal();
	gs_engine_instance()->ctx.graphics->material_i->set_uniform(mat, type, name, data);
}

void opengl_set_material_uniform_mat4(gs_resource(gs_material_t) handle, const char* name, gs_mat4 val)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_mat4, name, val);
}

void opengl_set_material_uniform_vec4(gs_resource(gs_material_t) handle, const char* name, gs_vec4 val)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_vec4, name, val);
}

void opengl_set_material_uniform_vec3(gs_resource(gs_material_t) handle, const char* name, gs_vec3 val)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_vec3, name, val);
}

void opengl_set_material_uniform_vec2(gs_resource(gs_material_t) handle, const char* name, gs_vec2 val)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_vec2, name, val);
}

void opengl_set_material_uniform_float(gs_resource(gs_material_t) handle, const char* name, f32 val)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_float, name, val);
}

void opengl_set_material_uniform_int(gs_resource(gs_material_t) handle, const char* name, s32 val)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_int, name, val);
}

void opengl_set_material_uniform_sampler2d(gs_resource(gs_material_t) handle, const char* name, gs_texture_t val, u32 slot)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->uniform_i->set_uniform_from_shader(mat->uniforms, mat->shader, gs_uniform_type_sampler2d, name, val.id, slot);
}

void opengl_bind_material_shader(gs_command_buffer_t* cb, gs_resource(gs_material_t) handle)
{
	gs_material_t* mat = gs_resource_cache_get_ptr(gs_engine_subsystem(graphics)->material_cache, handle);
	gs_engine_instance()->ctx.graphics->bind_shader(cb, mat->shader);
}

void opengl_bind_material_uniforms(gs_command_buffer_t* cb, gs_resource(gs_material_t) handle)
{
	gs_engine_instance()->ctx.graphics->material_i->bind_uniforms(cb, handle);
}

gs_shader_t opengl_construct_shader(const char* vert_src, const char* frag_src)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Shader to fill out
	gs_shader_t s = gs_default_val();

	// Construct vertex program
	s.program_id = glCreateProgram();

	// Construct vertex shader
	u32 vert_id = glCreateShader(GL_VERTEX_SHADER);
	gs_assert(vert_id != 0);

	// Construct fragment shader
	u32 frag_id = glCreateShader(GL_FRAGMENT_SHADER);
	gs_assert(frag_id != 0);

	// Compile each shader separately
	opengl_compile_shader(vert_src, vert_id);
	opengl_compile_shader(frag_src, frag_id);

	// Link shaders once compiled
	opengl_link_shaders(s.program_id, vert_id, frag_id);

	return s;
}

gs_uniform_t opengl_construct_uniform(gs_shader_t s, const char* uniform_name, gs_uniform_type type)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Uniform to fill out
	uniform_t u = gs_default_val();

	// Grab location of uniform
	u32 location = glGetUniformLocation(s.program_id, uniform_name);

	if (location >= u32_max) {
		gs_println("Warning: uniform not found: \"%s\"", uniform_name);
	}

	// Won't know uniform's type, unfortunately...
	// Set uniform location
	u.location = location;
	u.type = type;

	return u;
}

gs_frame_buffer_t opengl_construct_frame_buffer(gs_texture_t tex)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Render target to create
	frame_buffer_t fb = gs_default_val();

	// Construct and bind frame buffer
	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

	// Set list of buffers to draw for this framebuffer 
	// This will need to be changed, I think, but when and where?
	GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	// Idx is 0, for now
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, GL_TEXTURE_2D, tex.id , 0);

	// Error checking
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		gs_println("Error: Frame buffer could not be created.");
		gs_assert(false);
	}

	// Set frame buffer to back buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fb;
}

gs_render_target_t opengl_construct_render_target(gs_texture_parameter_desc t_desc)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Render target to create
	render_target_t target = gs_default_val();

	gs_texture_t tex = opengl_construct_texture(t_desc);
	target.tex_id = tex.id;

	return target;
}

gs_texture_t opengl_construct_texture(gs_texture_parameter_desc desc)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Texture to fill out
	texture_t tex = gs_default_val();

	gs_texture_format texture_format = desc.texture_format;
	u32 width = desc.width;
	u32 height = desc.height;
	u32 num_comps = desc.num_comps;

	glGenTextures(1, &(tex.id));
	glBindTexture(GL_TEXTURE_2D, tex.id);

	// Construct texture based on appropriate format
	switch(texture_format) 
	{
		case gs_texture_format_a8: 		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, desc.data); break;
		case gs_texture_format_r8: 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, desc.data); break;
		case gs_texture_format_rgb8: 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, desc.data); break;
		case gs_texture_format_rgba8: 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, desc.data); break;
		case gs_texture_format_rgba16f: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, desc.data); break;
	}

	s32 mag_filter = desc.mag_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;
	s32 min_filter = desc.min_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;

	if (desc.generate_mips) 
	{
		if (desc.min_filter == gs_nearest) 
		{
			min_filter = desc.mipmap_filter == gs_nearest ? GL_NEAREST_MIPMAP_NEAREST : 
				GL_NEAREST_MIPMAP_LINEAR;
		} 
		else 
		{
			min_filter = desc.mipmap_filter == gs_nearest ? GL_LINEAR_MIPMAP_NEAREST : 
				GL_NEAREST_MIPMAP_LINEAR;
		}
	}

	s32 texture_wrap_s = desc.texture_wrap_s == gs_repeat ? GL_REPEAT : 
						 desc.texture_wrap_s == gs_mirrored_repeat ? GL_MIRRORED_REPEAT : 
						 desc.texture_wrap_s == gs_clamp_to_edge ? GL_CLAMP_TO_EDGE : 
						 GL_CLAMP_TO_BORDER;
	s32 texture_wrap_t = desc.texture_wrap_t == gs_repeat ? GL_REPEAT : 
						 desc.texture_wrap_t == gs_mirrored_repeat ? GL_MIRRORED_REPEAT : 
						 desc.texture_wrap_t == gs_clamp_to_edge ? GL_CLAMP_TO_EDGE : 
						 GL_CLAMP_TO_BORDER;

	// Anisotropic filtering (not supported by default, need to figure this out)
	// f32 aniso = 0.0f; 
	// glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

	if (desc.generate_mips)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	tex.width 			= width;
	tex.height 			= height;
	tex.num_comps 		= num_comps;
	tex.texture_format 	= texture_format;

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

void* opengl_load_texture_data_from_file(const char* file_path, b32 flip_vertically_on_load, 
				gs_texture_format texture_format, s32* width, s32* height, s32* num_comps)
	{
	// Load texture data
	stbi_set_flip_vertically_on_load(flip_vertically_on_load);

	// Load in texture data using stb for now
	char temp_file_extension_buffer[16] = gs_default_val(); 
	gs_util_get_file_extension(temp_file_extension_buffer, sizeof(temp_file_extension_buffer), file_path);

	// Load texture data
	stbi_set_flip_vertically_on_load(flip_vertically_on_load);

	// Texture data to fill out
	void* texture_data = NULL;

	switch (texture_format)
	{
		case gs_texture_format_rgba8: texture_data = (u8*)stbi_load(file_path, width, height, num_comps, STBI_rgb_alpha); break;
		case gs_texture_format_rgb8: texture_data = (u8*)stbi_load(file_path, width, height, num_comps, STBI_rgb); break;

		// TODO(john): support this format
		case gs_texture_format_rgba16f: 
		default:
		{
			gs_assert(false);
		} break;
	}

	if (!texture_data) {
		gs_println("Warning: could not load texture: %s", file_path);
	}

	return texture_data;
}


gs_texture_t opengl_construct_texture_from_file(const char* file_path, gs_texture_parameter_desc* t_desc)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	opengl_render_data_t* data = __get_opengl_data_internal();
	gs_texture_t tex = gs_default_val();

	if (t_desc) 
	{
		// Load texture data and fill out parameters for descriptor
		t_desc->data = gfx->load_texture_data_from_file(file_path, true, t_desc->texture_format, 
			(s32*)&t_desc->width, (s32*)&t_desc->height, (s32*)&t_desc->num_comps);
		
		// Finish constructing texture resource from descriptor and return handle
		tex = opengl_construct_texture(*t_desc);

		// gs_free(t_desc->data);
	}
	else
	{
		gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();
		// Load texture data and fill out parameters for descriptor
		desc.data = gfx->load_texture_data_from_file(file_path, true, desc.texture_format, 
			(s32*)&desc.width, (s32*)&desc.height, (s32*)&desc.num_comps);

		// Finish constructing texture resource from descriptor and return handle
		tex = opengl_construct_texture(desc);

		// gs_free(desc.data);
	}

	return tex;
}

void opengl_update_texture_data(gs_texture_t* tex, gs_texture_parameter_desc t_desc)
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	gs_texture_format texture_format = t_desc.texture_format;
	u32 width = t_desc.width;
	u32 height = t_desc.height;
	u32 num_comps = t_desc.num_comps;

	glBindTexture(GL_TEXTURE_2D, tex->id);

	// Construct texture based on appropriate format
	switch(texture_format) 
	{
		case gs_texture_format_r8: 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, t_desc.data); break;
		case gs_texture_format_a8: 		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, t_desc.data); break;
		case gs_texture_format_rgb8: 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, t_desc.data); break;
		case gs_texture_format_rgba8: 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t_desc.data); break;
		case gs_texture_format_rgba16f: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, t_desc.data); break;
	}

	s32 mag_filter = t_desc.mag_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;
	s32 min_filter = t_desc.min_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;

	if (t_desc.generate_mips) 
	{
		if (t_desc.min_filter == gs_nearest) 
		{
			min_filter = t_desc.mipmap_filter == gs_nearest ? GL_NEAREST_MIPMAP_NEAREST : 
				GL_NEAREST_MIPMAP_LINEAR;
		} 
		else 
		{
			min_filter = t_desc.mipmap_filter == gs_nearest ? GL_LINEAR_MIPMAP_NEAREST : 
				GL_NEAREST_MIPMAP_LINEAR;
		}
	}

	s32 texture_wrap_s = t_desc.texture_wrap_s == gs_repeat ? GL_REPEAT : 
						 t_desc.texture_wrap_s == gs_mirrored_repeat ? GL_MIRRORED_REPEAT : 
						 t_desc.texture_wrap_s == gs_clamp_to_edge ? GL_CLAMP_TO_EDGE : 
						 GL_CLAMP_TO_BORDER;
	s32 texture_wrap_t = t_desc.texture_wrap_t == gs_repeat ? GL_REPEAT : 
						 t_desc.texture_wrap_t == gs_mirrored_repeat ? GL_MIRRORED_REPEAT : 
						 t_desc.texture_wrap_t == gs_clamp_to_edge ? GL_CLAMP_TO_EDGE : 
						 GL_CLAMP_TO_BORDER;

	// Anisotropic filtering (not supported by default, need to figure this out)
	// f32 aniso = 0.0f; 
	// glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

	if (t_desc.generate_mips)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	tex->width 			= width;
	tex->height 		= height;
	tex->num_comps 		= num_comps;
	tex->texture_format = texture_format;
}

void shader_t_free(shader_t* shader)
{
	glDeleteProgram(shader->program_id);
}

void opengl_free_shader(gs_shader_t shader)
{
	opengl_render_data_t* __data = __get_opengl_data_internal();
	
	shader_t_free(&shader);
}

// Method for creating graphics layer for opengl
struct gs_graphics_i* __gs_graphics_construct()
{
	// Construct new graphics interface instance
	struct gs_graphics_i* gfx = gs_malloc_init(gs_graphics_i);

	// Null out data
	gfx->data = NULL;

	/*============================================================
	// Graphics Initilization / De-Initialization
	============================================================*/
	gfx->init 		= &opengl_init;
	gfx->shutdown 	= &opengl_shutdown;
	gfx->update 	= &opengl_update;

	/*============================================================
	// Graphics Command Buffer Ops
	============================================================*/
	gfx->reset_command_buffer 			= &opengl_reset_command_buffer;
	gfx->set_pipeline_state 			= &opengl_set_pipeline_state;
	gfx->bind_shader	      			= &opengl_bind_shader;
	gfx->bind_vertex_buffer 			= &opengl_bind_vertex_buffer;
	gfx->bind_index_buffer 				= &opengl_bind_index_buffer;
	gfx->bind_texture 					= &opengl_bind_texture;
	gfx->bind_texture_id 				= &opengl_bind_texture_id;
	gfx->bind_uniform 					= &opengl_bind_uniform;
	gfx->bind_uniform_mat4 				= &opengl_bind_uniform_mat4;
	gfx->bind_frame_buffer  			= &opengl_bind_frame_buffer;
	gfx->unbind_frame_buffer 			= &opengl_unbind_frame_buffer;
	gfx->set_frame_buffer_attachment 	= &opengl_set_frame_buffer_attachment;
	gfx->set_view_clear 				= &opengl_set_view_clear;
	gfx->set_viewport 					= &opengl_set_viewport;
	gfx->set_view_scissor 				= &opengl_set_view_scissor;
	gfx->set_depth_enabled 				= &opengl_set_depth_enabled;
	gfx->set_blend_mode 				= &opengl_set_blend_mode;
	gfx->set_blend_equation  			= &opengl_set_blend_equation;
	gfx->set_winding_order 				= &opengl_set_winding_order;
	gfx->set_face_culling 				= &opengl_set_face_culling;
	gfx->draw 							= &opengl_draw;
	gfx->draw_indexed 					= &opengl_draw_indexed;
	gfx->submit_command_buffer 			= &opengl_submit_command_buffer;
	gfx->update_vertex_data 			= &opengl_update_vertex_data_command;
	gfx->update_index_data 				= &opengl_update_index_data_command;
	gfx->bind_material_uniforms 		= &opengl_bind_material_uniforms;
	gfx->bind_material_shader 			= &opengl_bind_material_shader;

	// void (* set_uniform_buffer_sub_data)(gs_command_buffer_t*, gs_resource(gs_uniform_buffer), void*, usize);
	// void (* set_index_buffer)(gs_command_buffer_t*, gs_resource(gs_index_buffer));

	/*============================================================
	// Graphics Resource Construction
	============================================================*/
	gfx->construct_shader 						= &opengl_construct_shader;
	gfx->construct_uniform 						= &opengl_construct_uniform;
	// gfx->construct_command_buffer 				= &opengl_construct_command_buffer;
	gfx->construct_frame_buffer 				= &opengl_construct_frame_buffer;
	gfx->construct_render_target 				= &opengl_construct_render_target;
	gfx->construct_vertex_buffer 				= &opengl_construct_vertex_buffer;
	gfx->update_vertex_buffer_data 				= &opengl_update_vertex_buffer_data;
	gfx->construct_index_buffer 				= &opengl_construct_index_buffer;
	gfx->construct_texture 						= &opengl_construct_texture;
	gfx->construct_texture_from_file 			= &opengl_construct_texture_from_file;
	gfx->update_texture_data 					= &opengl_update_texture_data;
	gfx->load_texture_data_from_file 			= &opengl_load_texture_data_from_file;
	// gs_resource(gs_uniform_buffer)(* construct_uniform_buffer)(gs_resource(gs_shader), const char* uniform_name);
	// gfx->construct_material 					= &opengl_construct_material;
	// gfx->construct_quad_batch 					= &opengl_construct_quad_batch;
	gfx->construct_font_from_file 				= &__gs_construct_font_from_file;
	/*============================================================
	// Graphics Ops
	============================================================*/

	/*============================================================
	// Graphics Resource Free Ops
	============================================================*/
	// void (* free_vertex_attribute_layout_t_desc)(gs_resource(gs_vertex_attribute_layout_desc));
	// void (* free_vertex_buffer)(gs_resource(gs_vertex_buffer));
	// void (* free_index_buffer)(gs_resource(gs_index_buffer));
	// void (* free_shader)(gs_resource(gs_shader));
	// void (* free_uniform_buffer)(gs_resource(gs_uniform_buffer));
	gfx->free_shader = &opengl_free_shader;

	/*============================================================
	// Graphics Update Ops
	============================================================*/
	gfx->set_material_uniform 			= &opengl_set_material_uniform;
	gfx->set_material_uniform_mat4 		= &opengl_set_material_uniform_mat4;
	gfx->set_material_uniform_vec4 		= &opengl_set_material_uniform_vec4;
	gfx->set_material_uniform_vec3 		= &opengl_set_material_uniform_vec3;
	gfx->set_material_uniform_vec2 		= &opengl_set_material_uniform_vec2;
	gfx->set_material_uniform_float 	= &opengl_set_material_uniform_float;
	gfx->set_material_uniform_int 		= &opengl_set_material_uniform_int;
	gfx->set_material_uniform_sampler2d = &opengl_set_material_uniform_sampler2d;
	gfx->quad_batch_begin 				= &opengl_quad_batch_begin;
	gfx->quad_batch_add 				= &opengl_quad_batch_add;
	gfx->quad_batch_end 				= &opengl_quad_batch_end;
	gfx->quad_batch_submit 				= &opengl_quad_batch_submit;

	/*============================================================
	// Graphics immediate Drawing Utilities
	============================================================*/
	gfx->immediate.begin_drawing 		= &opengl_immediate_begin_drawing;
	gfx->immediate.end_drawing 			= &opengl_immediate_end_drawing;
	gfx->immediate.default_pipeline_state = &opengl_default_pipeline_state;
	gfx->immediate.begin 				= &opengl_immediate_begin;
	gfx->immediate.end 					= &opengl_immediate_end;
	gfx->immediate.clear 				= &opengl_immediate_clear;
	gfx->immediate.flush 				= &opengl_immediate_flush;
	gfx->immediate.push_state 			= &opengl_immediate_push_state;
	gfx->immediate.pop_state 			= &opengl_immediate_pop_state;
	gfx->immediate.push_state_attr 		= &opengl_immediate_push_state_attr;
	gfx->immediate.pop_state_attr		= &opengl_pop_start_attr;
	gfx->immediate.push_matrix 			= &opengl_immediate_push_matrix;
	gfx->immediate.pop_matrix 			= &opengl_immediate_pop_matrix;
	gfx->immediate.mat_mul 				= &opengl_immediate_mat_mul;
	gfx->immediate.enable_texture_2d 	= &opengl_immediate_enable_texture_2d;
	gfx->immediate.disable_texture_2d 	= &opengl_immediate_disable_texture_2d;

	gfx->immediate.begin_3d				= &__gs_begin_3d;
	gfx->immediate.end_3d 				= &__gs_end_3d;
	gfx->immediate.begin_2d 			= &__gs_begin_2d;
	gfx->immediate.end_2d 				= &__gs_end_2d;

	gfx->immediate.push_camera 			= &__gs_push_camera;
	gfx->immediate.pop_camera 			= &__gs_pop_camera;
	gfx->immediate.mat_rotatef 			= &__gs_mat_rotatef;
	gfx->immediate.mat_rotatev 			= &__gs_mat_rotatev;
	gfx->immediate.mat_rotateq 			= &__gs_mat_rotateq;
	gfx->immediate.mat_transf 			= &__gs_mat_transf;
	gfx->immediate.mat_transv 			= &__gs_mat_transv;
	gfx->immediate.mat_scalef 			= &__gs_mat_scalef;
	gfx->immediate.mat_scalev 			= &__gs_mat_scalev;
	gfx->immediate.mat_mul_vqs 			= &__gs_mat_mul_vqs;

	gfx->immediate.draw_line 			= &__gs_draw_line_2d;
	gfx->immediate.draw_line_ext 		= &__gs_draw_line_2d_ext;
	gfx->immediate.draw_line_3d 		= &__gs_draw_line_3d;
	gfx->immediate.draw_triangle 		= &__gs_draw_triangle_2d;
	gfx->immediate.draw_triangle_ext 	= &__gs_draw_triangle_3d_ext;
	gfx->immediate.draw_rect 			= &__gs_draw_rect_2d;
	gfx->immediate.draw_rectv 			= &__gs_draw_rect_2dv;
	gfx->immediate.draw_rect_textured 	= &__gs_draw_rect_2d_textured;
	gfx->immediate.draw_rect_textured_ext = &__gs_draw_rect_2d_textured_ext;
	gfx->immediate.draw_rect_lines 		= &__gs_draw_rect_2d_lines;
	gfx->immediate.draw_box 			= &__gs_draw_box;
	gfx->immediate.draw_box_textured 	= &__gs_draw_box_textured;
	gfx->immediate.draw_box_vqs 		= &__gs_draw_box_vqs;
	gfx->immediate.draw_box_textured_vqs = &__gs_draw_box_textured_vqs;
	gfx->immediate.draw_box_lines 		= &__gs_draw_box_lines;
	gfx->immediate.draw_box_lines_vqs 	= &__gs_draw_box_lines_vqs;
	gfx->immediate.draw_sphere 			= &__gs_draw_sphere;
	gfx->immediate.draw_sphere_lines 	= &__gs_draw_sphere_lines;
	gfx->immediate.draw_sphere_lines_vqs = &__gs_draw_sphere_lines_vqs;
	gfx->immediate.draw_text 			= &__gs_draw_text;
	gfx->immediate.draw_text_ext 		= &__gs_draw_text_ext;
	gfx->immediate.draw_circle_sector 	= &__gs_draw_circle_sector;
	gfx->immediate.draw_circle 			= &__gs_draw_circle;

	gfx->immediate.color_ub 			= &opengl_immediate_color_ub;
	gfx->immediate.color_ubv 			= &opengl_immediate_color_ubv;
	gfx->immediate.color_4f 			= &opengl_immediate_color_4f;
	gfx->immediate.vertex_3fv 			= &opengl_immediate_vertex_3fv;
	gfx->immediate.vertex_3f 			= &opengl_immediate_vertex_3f;
	gfx->immediate.vertex_2fv 			= &opengl_immediate_vertex_2fv;
	gfx->immediate.vertex_2f 			= &opengl_immediate_vertex_2f;
	gfx->immediate.texcoord_2f 			= &opengl_immediate_texcoord_2f;
	gfx->immediate.texcoord_2fv 		= &opengl_immediate_texcoord_2fv;

	/*============================================================
	// Graphics Utility Function
	============================================================*/
	gfx->get_byte_size_of_vertex_attribute	= &get_byte_size_of_vertex_attribute;
	gfx->calculate_vertex_size_in_bytes 	= &calculate_vertex_size_in_bytes;
	gfx->text_dimensions 					= &__gs_text_dimensions;

	gfx->default_font 						= &__gs_get_default_font;

	/*============================================================
	// Graphics Utility APIs
	============================================================*/
	gfx->material_i = gs_malloc_init(gs_material_i); 
	gfx->uniform_i = gs_malloc_init(gs_uniform_block_i);
	gfx->quad_batch_i = gs_malloc_init(gs_quad_batch_i);

	*(gfx->material_i) = __gs_material_i_new();
	*(gfx->uniform_i) = __gs_uniform_block_i_new();
	*(gfx->quad_batch_i) = __gs_quad_batch_i_new();

	return gfx;
}





