#include "graphics/gs_graphics.h"
#include "common/gs_containers.h"
#include "serialize/gs_byte_buffer.h"
#include "math/gs_math.h"
#include "common/gs_util.h"
#include "base/gs_engine.h"

#include <glad/glad.h>

#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

/*============================================================
// Graphics Resource Declarations
============================================================*/

/*
	Command buffer architecture: 
		- Function address to be called
		- Size of packet placed into buffer
		- Packet
*/

_inline void gs_mat4_debug_print( gs_mat4* mat )
{
	f32* e = mat->elements;
	gs_println( "[%.5f, %.5f, %.5f, %.5f]\n"
			   "[%.5f, %.5f, %.5f, %.5f]\n"
			   "[%.5f, %.5f, %.5f, %.5f]\n"
			   "[%.5f, %.5f, %.5f, %.5f]\n", 
			   e[0], e[1], e[2], e[3], 
			   e[4], e[5], e[6], e[7], 
			   e[8], e[9], e[10], e[11], 
			   e[12], e[13], e[14], e[15]
			   );
}

typedef enum gs_opengl_op_code
{
	gs_opengl_op_bind_shader = 0,
	gs_opengl_op_set_view_clear,
	gs_opengl_op_set_view_port,
	gs_opengl_op_set_depth_enabled,
	gs_opengl_op_bind_vertex_buffer,
	gs_opengl_op_bind_index_buffer,
	gs_opengl_op_bind_uniform,
	gs_opengl_op_bind_texture,
	gs_opengl_op_bind_frame_buffer,
	gs_opengl_op_unbind_frame_buffer,
	gs_opengl_op_set_frame_buffer_attachment,
	gs_opengl_op_draw,
	gs_opengl_op_debug_draw_line,
	gs_opengl_op_debug_draw_square,
	gs_opengl_op_debug_draw_submit,
	gs_opengl_op_debug_set_properties,
	gs_opengl_draw_indexed
} gs_opengl_op_code;

// Could make this a simple byte buffer, then read from that buffer for commands?
typedef struct command_buffer_t
{
	u32 num_commands;
	gs_byte_buffer commands;
} command_buffer_t;

typedef struct texture_t
{
	u16 width;
	u16 height;
	u32 id;	
	u32 num_comps;
	gs_texture_format texture_format;
} texture_t;

typedef struct shader_t
{
	u32 program_id;
} shader_t;

typedef struct uniform_t
{
	gs_uniform_type type;
	u32 location;
} uniform_t;

typedef struct index_buffer_t
{
	u32 ibo;
} index_buffer_t;

typedef struct vertex_buffer_t
{
	u32 vbo;
	u32 vao;		// Not sure if I need to do this as well, but we will for now...
} vertex_buffer_t;

typedef struct render_target_t 
{
	gs_resource( gs_texture ) tex_handle;
} render_target_t;

typedef struct frame_buffer_t
{
	u32 fbo;
} frame_buffer_t;

typedef struct vertex_attribute_layout_desc_t
{
	gs_dyn_array( gs_vertex_attribute_type ) attributes;	
} vertex_attribute_layout_desc_t;

// Internally
typedef struct debug_drawing_internal_data 
{
	gs_mat4 view_mat;
	gs_mat4 proj_mat;
	gs_resource( gs_shader ) shader;
	gs_resource( gs_uniform ) u_proj;
	gs_resource( gs_uniform ) u_view;
	gs_dyn_array( f32 ) line_vertex_data; 
	gs_dyn_array( f32 ) quad_vertex_data;
	gs_resource( gs_vertex_buffer) quad_vbo;
	gs_resource( gs_vertex_buffer ) line_vbo;
} debug_drawing_internal_data;

// Slot array declarations
gs_slot_array_decl( command_buffer_t );
gs_slot_array_decl( texture_t );
gs_slot_array_decl( shader_t );
gs_slot_array_decl( uniform_t );
gs_slot_array_decl( index_buffer_t );
gs_slot_array_decl( vertex_buffer_t );
gs_slot_array_decl( vertex_attribute_layout_desc_t );
gs_slot_array_decl( render_target_t );
gs_slot_array_decl( frame_buffer_t );

// Define render resource data structure
typedef struct opengl_render_data_t
{
	gs_slot_array( command_buffer_t ) 				command_buffers;
	gs_slot_array( texture_t ) 						textures;
	gs_slot_array( shader_t ) 						shaders;
	gs_slot_array( uniform_t ) 						uniforms;
	gs_slot_array( index_buffer_t )   				index_buffers;
	gs_slot_array( vertex_buffer_t )  				vertex_buffers;
	gs_slot_array( vertex_attribute_layout_desc_t ) vertex_layout_descs;
	gs_slot_array( render_target_t ) 				render_targets;
	gs_slot_array( frame_buffer_t ) 				frame_buffers;
	debug_drawing_internal_data 					debug_data;
} opengl_render_data_t;

// _global const char* debug_shader_v_src = "\n"
// "#version 330 core\n"
// "layout (location = 0) in vec3 a_position;\n"
// "layout (location = 1) in vec3 a_color;\n"
// "uniform mat4 u_proj;\n"
// "uniform mat4 u_view;\n"
// "out vec3 f_color;\n"
// "void main() {\n"
// " gl_Position = u_proj * u_view * vec4(a_position, 1.0);\n"
// " f_color = a_color;\n"
// "}";

// _global const char* debug_shader_f_src = "\n"
// "#version 330 core\n"
// "in vec3 f_color;\n"
// "out vec4 frag_color;\n"
// "void main() {\n"
// " frag_color = vec4(f_color, 1.0);\n"
// "}";

#define __get_opengl_data_internal()\
	(opengl_render_data_t*)(gs_engine_instance()->ctx.graphics->data)

#define __get_command_buffer_internal( data, cb )\
	(gs_slot_array_get_ptr(data->command_buffers, cb.id))

// Forward Decls;
void __reset_command_buffer_internal( command_buffer_t* cb );
debug_drawing_internal_data construct_debug_drawing_internal_data();
gs_resource( gs_texture ) opengl_construct_texture( gs_texture_parameter_desc desc );

/*============================================================
// Graphics Initilization / De-Initialization
============================================================*/

void opengl_init_default_state()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glDisable(GL_DEPTH_TEST);
	// glBlendFunc(GL_ONE, GL_ONE);
	// Init things...
}

gs_result opengl_init( struct gs_graphics_i* gfx )
{
	// Construct instance of render data
	struct opengl_render_data_t* data = gs_malloc_init( opengl_render_data_t );

	// Set data
	gfx->data = data;

	// Initialize data
	data->command_buffers 		= gs_slot_array_new( command_buffer_t );
	data->textures				= gs_slot_array_new( texture_t );
	data->shaders 				= gs_slot_array_new( shader_t );
	data->uniforms 				= gs_slot_array_new( uniform_t );
	data->index_buffers 		= gs_slot_array_new( index_buffer_t );
	data->vertex_buffers 		= gs_slot_array_new( vertex_buffer_t );
	data->vertex_layout_descs 	= gs_slot_array_new( vertex_attribute_layout_desc_t );
	data->render_targets 		= gs_slot_array_new( render_target_t );
	data->frame_buffers 		= gs_slot_array_new( frame_buffer_t );
	data->debug_data 			= construct_debug_drawing_internal_data();

	// Init all default opengl state here before frame begins
	opengl_init_default_state();

	// Print some debug info
	gs_println( "OpenGL::Vendor = %s", glGetString( GL_VENDOR ) ) ;
	gs_println( "OpenGL::Renderer = %s", glGetString( GL_RENDERER ) ) ;
	gs_println( "OpenGL::Version = %s", glGetString( GL_VERSION ) ) ;

	// Initialize all data here
	return gs_result_success;
}

gs_result opengl_update()
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// For now, just go through all command buffers and reset them
	gs_for_range_i( gs_dyn_array_size( data->command_buffers.data ) )
	{
		__reset_command_buffer_internal( &data->command_buffers.data[ i ] );
	}

	return gs_result_in_progress;
}

gs_result opengl_shutdown( struct gs_graphics_i* gfx )
{
	// Release all data here
	return gs_result_success;
}

debug_drawing_internal_data construct_debug_drawing_internal_data()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	debug_drawing_internal_data data = {0};
	data.line_vertex_data = gs_dyn_array_new( f32 );
	data.quad_vertex_data = gs_dyn_array_new( f32 );

	// Construct shader
	// data.shader = gfx->construct_shader( debug_shader_v_src, debug_shader_f_src );

	// Construct uniforms
	data.u_proj = gfx->construct_uniform( data.shader, "u_proj", gs_uniform_type_mat4 );
	data.u_view = gfx->construct_uniform( data.shader, "u_view", gs_uniform_type_mat4 );

	// Vertex data layout
	gs_vertex_attribute_type vertex_layout[] = {
		gs_vertex_attribute_float3,	// Position
		gs_vertex_attribute_float3  // Color
	};
	u32 layout_count = sizeof(vertex_layout) / sizeof(gs_vertex_attribute_type);

	// Construct vertex buffer objects
	data.line_vbo = gfx->construct_vertex_buffer( vertex_layout, layout_count, NULL, 0 );

	// Construct default matrix data to be used
	data.view_mat = gs_mat4_identity();
	data.proj_mat = gs_mat4_identity();

	return data;
}

void __reset_command_buffer_internal( command_buffer_t* cb )
{
	// Clear byte buffer of commands
	gs_byte_buffer_clear( &cb->commands );

	// Set num commands to 0
	cb->num_commands = 0;
}

void opengl_reset_command_buffer( gs_resource( gs_command_buffer ) cb_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	__reset_command_buffer_internal( cb );
}

void opengl_set_frame_buffer_attachment( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_texture ) t_handle, u32 idx )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab texture from handle
	texture_t t = gs_slot_array_get( data->textures, t_handle.id );

	// Push back commands
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_set_frame_buffer_attachment );
	gs_byte_buffer_write( &cb->commands, u32, t.id );
	gs_byte_buffer_write( &cb->commands, u32, idx );

	// Increase num commands
	cb->num_commands++;
}

void opengl_bind_frame_buffer( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_frame_buffer ) fb_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab shader from handle
	frame_buffer_t fb = gs_slot_array_get( data->frame_buffers, fb_handle.id );

	// Push back commands
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_frame_buffer );
	gs_byte_buffer_write( &cb->commands, u32, fb.fbo );

	cb->num_commands++;
}

void opengl_unbind_frame_buffer( gs_resource( gs_command_buffer ) cb_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Push back commands
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_unbind_frame_buffer );

	cb->num_commands++;
}

void opengl_bind_shader( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_shader ) s_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab shader from handle
	shader_t s = gs_slot_array_get( data->shaders, s_handle.id );

	// Construct command packet for binding shader
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_shader );
	gs_byte_buffer_write( &cb->commands, u32, s.program_id );

	// Increase command amount
	cb->num_commands++;
}

#define __write_uniform_val(bb, type, u_data)\
	gs_byte_buffer_write(&bb, type, *((type*)(u_data)));

void opengl_bind_uniform( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_uniform ) u_handle, void* u_data )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab uniform from handle
	uniform_t u = gs_slot_array_get( data->uniforms, u_handle.id );

	// Write out op code
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_uniform );
	// Write out uniform location
	gs_byte_buffer_write( &cb->commands, u32, (u32)u.location );
	// Write out uniform type
	gs_byte_buffer_write( &cb->commands, u32, (u32)u.type );

	// Write out uniform value
	switch ( u.type )
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
			gs_println( "Invalid uniform type passed" );
			gs_assert( false );
		}
	}

	// Increase command amount
	cb->num_commands++;
}

void opengl_bind_texture( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_uniform ) u_handle, 
		gs_resource( gs_texture ) tex_handle, u32 tex_unit )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Get texture
	texture_t tex = gs_slot_array_get( data->textures, tex_handle.id );

	// Grab uniform from handle
	uniform_t u = gs_slot_array_get( data->uniforms, u_handle.id );

	// Cannot pass in uniform of wrong type
	if ( u.type != gs_uniform_type_sampler2d )
	{
		gs_println( "opengl_bind_texture: Must be of uniform type 'gs_uniform_type_sampler2d'" );
		gs_assert( false );
	}

	// Write op code
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_texture );
	// Write out id
	gs_byte_buffer_write( &cb->commands, u32, tex.id );
	// Write tex unit location
	gs_byte_buffer_write( &cb->commands, u32, tex_unit );
	// Write out uniform location
	gs_byte_buffer_write( &cb->commands, u32, (u32)u.location );

	// Increase command amount
	cb->num_commands++;
}

void opengl_bind_vertex_buffer( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_vertex_buffer ) vb_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Get vertex buffer data
	vertex_buffer_t vb = gs_slot_array_get( data->vertex_buffers, vb_handle.id );

	// Write op code
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_vertex_buffer );
	// Write out vao
	gs_byte_buffer_write( &cb->commands, u32, vb.vao );

	// Increase command amount
	cb->num_commands++;
}

void opengl_bind_index_buffer( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_index_buffer ) ib_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Get index buffer data
	index_buffer_t ib = gs_slot_array_get( data->index_buffers, ib_handle.id );

	// Write op code
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_index_buffer );
	// Write out ibo
	gs_byte_buffer_write( &cb->commands, u32, ib.ibo );

	// Increase command amount
	cb->num_commands++;
}

void opengl_set_view_port( gs_resource( gs_command_buffer ) cb_handle, u32 width, u32 height )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Write op into buffer
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_set_view_port );
	// Write width into buffer
	gs_byte_buffer_write( &cb->commands, u32, width );
	// Write height into buffer
	gs_byte_buffer_write( &cb->commands, u32, height );

	// Increase command amount
	cb->num_commands++;
}

// Want to set a bitmask for this as well to determine clear types
void opengl_set_view_clear( gs_resource( gs_command_buffer ) cb_handle, f32* col )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	gs_vec4 c = (gs_vec4){col[0], col[1], col[2], col[3]};

	// Write op into buffer
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_set_view_clear );
	// Write color into buffer (as vec4)
	gs_byte_buffer_write( &cb->commands, gs_vec4, c );

	// Increase command amount
	cb->num_commands++;
}

void opengl_set_depth_enabled( gs_resource( gs_command_buffer ) cb_handle, b32 enable )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Write op into buffer
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_set_depth_enabled );
	// Write color into buffer (as vec4)
	gs_byte_buffer_write( &cb->commands, b32, enable );

	// Increase command amount
	cb->num_commands++;
}

void opengl_draw_indexed( gs_resource( gs_command_buffer ) cb_handle, u32 count )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Write draw command
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_draw_indexed );	
	// Write count
	gs_byte_buffer_write( &cb->commands, u32, count );

	// Increase command amount
	cb->num_commands++;

}

void opengl_draw( gs_resource( gs_command_buffer ) cb_handle, u32 start, u32 count )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Write draw command
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_draw );	
	// Write start
	gs_byte_buffer_write( &cb->commands, u32, start );
	// Write count
	gs_byte_buffer_write( &cb->commands, u32, count );

	// Increase command amount
	cb->num_commands++;
}

void opengl_set_debug_draw_properties( gs_resource( gs_command_buffer ) cb_handle, gs_debug_draw_properties props )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Write draw command
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_debug_set_properties );	
	// Write view
	gs_byte_buffer_write( &cb->commands, gs_mat4, props.view_mat );
	// Write proj
	gs_byte_buffer_write( &cb->commands, gs_mat4, props.proj_mat );

	// Increase command amount
	cb->num_commands++;
}

/*
#define push_command( command_buffer, command, sz, type, op )\
do {\
	gs_byte_buffer_write( &cb->commands, u32, op );\
	gs_byte_buffer_write_bytes( &cb->commands, data, sz );\
} while( 0 )
	void opengl_add_command( gs_resource( gs_command_buffer ) cb_handle, void* command, opengl_op_code_type op )
	{
		// Get command buffer
		command_buffer_t* cb = ...

		switch ( op )
		{
			case gs_opengl_op_debug_draw_line: 
			{
				opengl_draw_debug_line_command* cmd = (opengl_draw_debug_line_command*)command;
				gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_debug_draw_line)
				gs_byte_buffer_write_bytes( &cb->commands, cmd, sizeof(opengl_draw_debug_line_command));
			} break;
		}
	}
*/

// Yeah, don't like this...Because it takes control away from the user completely.
// Maybe user has ability to add as much as they want into the debug buffer and then can control when to submit it? Hmm...
void opengl_draw_line( gs_resource( gs_command_buffer ) cb_handle, gs_vec3 start, gs_vec3 end, gs_vec3 color )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Write op
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_debug_draw_line );
	// Write start position
	gs_byte_buffer_write( &cb->commands, gs_vec3, start );
	// Write end position
	gs_byte_buffer_write( &cb->commands, gs_vec3, end );
	// Write color
	gs_byte_buffer_write( &cb->commands, gs_vec3, color );

	// Increase command amount		// This is incredibly error prone...
	// Should have a structure that I pass in instead...
	cb->num_commands++;
}

void opengl_draw_square( gs_resource( gs_command_buffer ) cb_handle, gs_vec3 origin, f32 width, f32 height, gs_vec3 color, gs_mat4 model )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_debug_draw_square );
	gs_byte_buffer_write( &cb->commands, gs_vec3, origin );
	gs_byte_buffer_write( &cb->commands, f32, width );
	gs_byte_buffer_write( &cb->commands, f32, height );
	gs_byte_buffer_write( &cb->commands, gs_vec3, color );
	gs_byte_buffer_write( &cb->commands, gs_mat4, model );

	// Increase command amount		// This is incredibly error prone...
	// Should have a structure that I pass in instead...
	cb->num_commands++;

}

void opengl_submit_debug_drawing( gs_resource( gs_command_buffer ) cb_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Simply write back command buffer stuff and things...
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_debug_draw_submit );

	cb->num_commands++;
}

#define __gfx_add_debug_line_internal( data, start, end, color )\
do {\
gs_dyn_array_push( data, start.x );\
gs_dyn_array_push( data, start.y );\
gs_dyn_array_push( data, start.z );\
gs_dyn_array_push( data, color.x );\
gs_dyn_array_push( data, color.y );\
gs_dyn_array_push( data, color.z );\
gs_dyn_array_push( data, end.x );\
gs_dyn_array_push( data, end.y );\
gs_dyn_array_push( data, end.z );\
gs_dyn_array_push( data, color.x );\
gs_dyn_array_push( data, color.y );\
gs_dyn_array_push( data, color.z );\
} while ( 0 )	


// For now, just rip through command buffer. Later on, will add all command buffers into a queue to be processed on rendering thread.
void opengl_submit_command_buffer( gs_resource( gs_command_buffer ) cb_handle )
{
	// Get data from graphics api
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer_t* cb = __get_command_buffer_internal( data, cb_handle );

	// Read through all commands
	/*
		// Structure of command: 
			- Op code
			- Data packet
	*/

	// Set read position of buffer to beginning
	gs_byte_buffer_seek_to_beg( &cb->commands );

	u32 num_commands = cb->num_commands;

	// For each command in buffer
	gs_for_range_i( num_commands )
	{
		// Read in op code of command
		gs_opengl_op_code op_code = ( gs_opengl_op_code )gs_byte_buffer_read( &cb->commands, u32 );

		switch ( op_code )
		{
			case gs_opengl_op_set_frame_buffer_attachment: 
			{
				u32 t_id = gs_byte_buffer_read( &cb->commands, u32 );
				u32 idx  = gs_byte_buffer_read( &cb->commands, u32 );
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, t_id, 0);
			} break;

			case gs_opengl_op_bind_frame_buffer: 
			{
				u32 fbo = gs_byte_buffer_read( &cb->commands, u32 );
				glBindFramebuffer( GL_FRAMEBUFFER, fbo );
			} break;

			case gs_opengl_op_unbind_frame_buffer: 
			{
				glBindFramebuffer( GL_FRAMEBUFFER, 0 );
			} break;

			case gs_opengl_op_draw:
			{
				// Read start
				u32 start = gs_byte_buffer_read( &cb->commands, u32 );
				// Read count
				u32 count = gs_byte_buffer_read( &cb->commands, u32 );

				// Draw ( this assumes a vao is set, which is not correct )...for now, will assume
				// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawArrays( GL_TRIANGLES, start, count );
			} break;

			case gs_opengl_draw_indexed:
			{
				// Read count
				u32 count = gs_byte_buffer_read( &cb->commands, u32 );

				// Draw ( this assumes a ibo is set, which is not correct )...for now, will assume
				glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_INT, 0 );
			} break;

			case gs_opengl_op_set_view_clear: 
			{
				// Read color from buffer (as vec4)
				gs_vec4 col = gs_byte_buffer_read( &cb->commands, gs_vec4 );
				// Set clear color
				glClearColor( col.x, col.y, col.z, col.w );
				// Clear screen
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			} break;

			case gs_opengl_op_set_view_port: 
			{
				// Read width from buffer
				u32 width = gs_byte_buffer_read( &cb->commands, u32 );
				// Read height from buffer
				u32 height = gs_byte_buffer_read( &cb->commands, u32 );
				// Set viewport
				glViewport( 0, 0, (s32)width, (s32)height );

			} break;

			case gs_opengl_op_set_depth_enabled: 
			{
				// Read color from buffer (as vec4)
				b32 enabled = gs_byte_buffer_read( &cb->commands, b32 );
				// Clear screen
				if ( enabled ) {
					glEnable( GL_DEPTH_TEST );
				} else {
					glDisable( GL_DEPTH_TEST );
				}
			} break;

			case gs_opengl_op_bind_texture:
			{
				// Write out id
				u32 tex_id = gs_byte_buffer_read( &cb->commands, u32 );
				// Write tex unit location
				u32 tex_unit = gs_byte_buffer_read( &cb->commands, u32 );
				// Write out uniform location
				u32 location = gs_byte_buffer_read( &cb->commands, u32 );

				// Activate texture unit
				glActiveTexture( GL_TEXTURE0 + tex_unit );
				// Bind texture
				glBindTexture( GL_TEXTURE_2D, tex_id );
				// Bind uniform
				glUniform1i( location, tex_unit );
			} break;

			case gs_opengl_op_bind_vertex_buffer:
			{
				// Read out vao
				u32 vao = gs_byte_buffer_read( &cb->commands, u32 );

				// Bind vao
				glBindVertexArray( vao );
			} break;

			case gs_opengl_op_bind_index_buffer:
			{
				// Read out vao
				u32 ibo = gs_byte_buffer_read( &cb->commands, u32 );

				// Bind vao
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
			} break;

			case gs_opengl_op_bind_shader: 
			{
				// Read in shader id
				u32 program_id = gs_byte_buffer_read( &cb->commands, u32 );
				// Bind program
				glUseProgram( program_id );
			} break;

			case gs_opengl_op_bind_uniform:
			{
				// Read in uniform location
				u32 location = gs_byte_buffer_read( &cb->commands, u32 );
				// Read in uniform type
				gs_uniform_type type = (gs_uniform_type)gs_byte_buffer_read( &cb->commands, u32 );

				// Read and bind val
				switch ( type )
				{
					case gs_uniform_type_float:
					{
						f32 val = gs_byte_buffer_read( &cb->commands, f32 );
						glUniform1f( location, val );
					} break;

					case gs_uniform_type_int: 
					{
						s32 val = gs_byte_buffer_read( &cb->commands, s32 );
						glUniform1i( location, val );
					} break;

					case gs_uniform_type_vec2:
					{
						gs_vec2 val = gs_byte_buffer_read( &cb->commands, gs_vec2 );
						glUniform2f( location, val.x, val.y );
					} break;

					case gs_uniform_type_vec3:
					{
						gs_vec3 val = gs_byte_buffer_read( &cb->commands, gs_vec3 );
						glUniform3f( location, val.x, val.y, val.z );
					} break;

					case gs_uniform_type_vec4:
					{
						gs_vec4 val = gs_byte_buffer_read( &cb->commands, gs_vec4 );
						glUniform4f( location, val.x, val.y, val.z, val.w );
					} break;

					case gs_uniform_type_mat4:
					{
						gs_mat4 val = gs_byte_buffer_read( &cb->commands, gs_mat4 );
						glUniformMatrix4fv( location, 1, false, (f32*)(val.elements) );
					} break;

					case gs_uniform_type_sampler2d:
					{
						u32 val = gs_byte_buffer_read( &cb->commands, u32 );
						glUniform1i( location, val );
					} break;

					default: 
					{
						gs_println( "Invalid uniform type read." );
						gs_assert( false );
					}
				}

			} break;

			case gs_opengl_op_debug_draw_line: 
			{
				// Push back vertices into local debug line drawing data buffer

				// Read start position
				gs_vec3 start = gs_byte_buffer_read( &cb->commands, gs_vec3 );
				// Read end position
				gs_vec3 end = gs_byte_buffer_read( &cb->commands, gs_vec3 );
				// Read color
				gs_vec3 color = gs_byte_buffer_read( &cb->commands, gs_vec3 );

				// Add line data
				__gfx_add_debug_line_internal( data->debug_data.line_vertex_data, start, end, color );
			} break;

			case gs_opengl_op_debug_draw_square: 
			{
				gs_vec3 origin = gs_byte_buffer_read( &cb->commands, gs_vec3 );
				f32 w = gs_byte_buffer_read( &cb->commands, f32 );
				f32 h = gs_byte_buffer_read( &cb->commands, f32 );
				gs_vec3 color = gs_byte_buffer_read( &cb->commands, gs_vec3 );
				gs_mat4 model = gs_byte_buffer_read( &cb->commands, gs_mat4 );

				gs_vec3 tl = gs_mat4_mul_vec3( model, origin );
				gs_vec3 tr = gs_mat4_mul_vec3( model, (gs_vec3){ origin.x + w, origin.y } );
				gs_vec3 bl = gs_mat4_mul_vec3( model, (gs_vec3){ origin.x, origin.y + h } );
				gs_vec3 br = gs_mat4_mul_vec3( model, (gs_vec3){ origin.x + w, origin.y + h } );

				// Four lines, don'tcha know?
				__gfx_add_debug_line_internal( data->debug_data.line_vertex_data, tl, tr, color );	
				__gfx_add_debug_line_internal( data->debug_data.line_vertex_data, tr, br, color );	
				__gfx_add_debug_line_internal( data->debug_data.line_vertex_data, br, bl, color );	
				__gfx_add_debug_line_internal( data->debug_data.line_vertex_data, bl, tl, color );
			} break;

			case gs_opengl_op_debug_set_properties: 
			{
				// Read view
				gs_mat4 view = gs_byte_buffer_read( &cb->commands, gs_mat4 );
				// Read proj
				gs_mat4 proj = gs_byte_buffer_read( &cb->commands, gs_mat4 );

				data->debug_data.view_mat = view;  
				data->debug_data.proj_mat = proj;

				// gs_timed_action( 10, 
				// {
				// 	gs_mat4_debug_print( &data->debug_data.proj_mat );
				// });
			}

			case gs_opengl_op_debug_draw_submit: 
			{
				// Construct all line vertex data from debug data buffer
				// Vertex count is line_vertex_data_array_size / size of single vertex = array_size / (sizeof(f32) * 6)
				/*
				usize vertex_data_size = gs_dyn_array_size(data->debug_data.line_vertex_data) * sizeof(f32);
				u32 vert_count = (vertex_data_size) / (sizeof(f32) * 6);

				// Bind shader program
				shader_t s = gs_slot_array_get( data->shaders, data->debug_data.shader.id );
				glUseProgram( s.program_id );

				// Bind uniforms
				// Let's just create an ortho projection real quirk...
			    s32 width = 800, height = 600;
				// gs_platform_window_width_height( gs_engine_instance()->ctx.window, &width, &height );
			    // glViewport(0, 0, width, height);
			    f32 L = 0.0f;
			    f32 R = 0.0f + width;
			    f32 T = 0.0f;
			    f32 B = 0.0f + height;
			    const f32 ortho_mat_2[4][4] =
			    {
			        { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
			        { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
			        { 0.0f,         0.0f,        -1.0f,   0.0f },
			        { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
			    };

			    gs_mat4 vm = gs_mat4_identity();

				uniform_t proj = gs_slot_array_get( data->uniforms, data->debug_data.u_proj.id );
				uniform_t view = gs_slot_array_get( data->uniforms, data->debug_data.u_view.id );

				// gs_timed_action( 10, 
				// {
				// 	gs_println( "after" );
				// 	gs_mat4_debug_print( &data->debug_data.proj_mat );
				// });

				glUniformMatrix4fv( proj.location, 1, false, (f32*)(data->debug_data.proj_mat.elements) );
				glUniformMatrix4fv( view.location, 1, false, (f32*)(data->debug_data.view_mat.elements) );

				// glUniformMatrix4fv( proj.location, 1, false, (f32*)(ortho_mat_2) );
				// glUniformMatrix4fv( view.location, 1, false, (f32*)(vm.elements) );

				// Bind vertex data
				vertex_buffer_t vb = gs_slot_array_get( data->vertex_buffers, data->debug_data.line_vbo.id );
				glBindVertexArray( vb.vao );
				// Upload data
				glBindBuffer( GL_ARRAY_BUFFER, vb.vbo );
				glBufferData( GL_ARRAY_BUFFER, vertex_data_size, data->debug_data.line_vertex_data, GL_STATIC_DRAW );
				// glBufferSubData( GL_ARRAY_BUFFER, 0, vertex_data_size, (f32*)data->debug_data.line_vertex_data );

				// Draw data
				glDrawArrays( GL_LINES, 0, vert_count );

				// Unbind data
				glBindVertexArray(0);
				glUseProgram(0);
				*/

				// Reset all debug data
				// data->debug_data.view_mat = gs_mat4_identity();
				// data->debug_data.proj_mat = gs_mat4_identity();

				// Flush vertex data
				gs_dyn_array_clear( data->debug_data.line_vertex_data );
			} break;

			default:
			{
				// Op code not supported yet!
				gs_println( "Op code not supported yet: %zu", (u32)op_code );
				gs_assert( false );
			}
		}
	}

	// Reset command buffer
	__reset_command_buffer_internal( cb );
}

gs_resource( gs_command_buffer ) opengl_construct_command_buffer()
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Construct new command buffer, then insert into slot array
	u32 cb_handle = gs_slot_array_insert( data->command_buffers, (command_buffer_t){0} );
	command_buffer_t* cb = gs_slot_array_get_ptr( data->command_buffers, cb_handle );

	// Initialize command buffer 
	cb->num_commands = 0;
	cb->commands = gs_byte_buffer_new();

	// Set resource handle
	gs_resource( gs_command_buffer ) handle = {0};
	handle.id = cb_handle;

	return handle;
}

u32 get_byte_size_of_vertex_attribute( gs_vertex_attribute_type type )
{
	u32 byte_size = 0; 
	switch ( type )
	{
		case gs_vertex_attribute_float4:	{ byte_size = sizeof(f32) * 4; } break;
		case gs_vertex_attribute_float3:	{ byte_size = sizeof(f32) * 3; } break;
		case gs_vertex_attribute_float2:	{ byte_size = sizeof(f32) * 2; } break;
		case gs_vertex_attribute_float:		{ byte_size = sizeof(f32) * 1; } break;
		case gs_vertex_attribute_uint4:		{ byte_size = sizeof(u32) * 4; } break;
		case gs_vertex_attribute_uint3:		{ byte_size = sizeof(u32) * 3; } break;
		case gs_vertex_attribute_uint2:		{ byte_size = sizeof(u32) * 2; } break;
		case gs_vertex_attribute_uint:		{ byte_size = sizeof(u32) * 1; } break;
	} 

	return byte_size;
}

u32 calculate_vertex_size_in_bytes( gs_vertex_attribute_type* layout_data, u32 count )
{
	// Iterate through all formats in delcarations and calculate total size
	u32 sz = 0;

	gs_for_range_i( count )
	{
		gs_vertex_attribute_type type = layout_data[ i ];
		sz += get_byte_size_of_vertex_attribute( type );
	}

	return sz;
}

s32 get_byte_offest( gs_vertex_attribute_type* layout_data, u32 index )
{
	gs_assert( layout_data );

	// Recursively calculate offset
	s32 total_offset = 0;

	// Base case
	if ( index == 0 )
	{
		return total_offset;
	} 

	// Calculate total offset up to this point
	for ( u32 i = 0; i < index; ++i )
	{ 
		total_offset += get_byte_size_of_vertex_attribute( layout_data[ i ] );
	} 

	return total_offset;
}

#define int_2_void_p(i) (void*)(uintptr_t)(i)

gs_resource( gs_vertex_buffer ) opengl_construct_vertex_buffer( gs_vertex_attribute_type* layout_data, u32 layout_size, 
	void* v_data, usize v_data_size )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Construct new vertex buffer, then insert into slot array
	u32 vb_handle = gs_slot_array_insert( data->vertex_buffers, (vertex_buffer_t){0} );
	vertex_buffer_t* vb = gs_slot_array_get_ptr( data->vertex_buffers, vb_handle );

	// Create and bind vertex array
	glGenVertexArrays( 1, (u32*)&vb->vao );
	glBindVertexArray( (u32)vb->vao );

	// Create and upload mesh data
	glGenBuffers( 1, (u32*)&vb->vbo );
	glBindBuffer( GL_ARRAY_BUFFER, (u32)vb->vbo );
	glBufferData( GL_ARRAY_BUFFER, v_data_size, v_data, GL_STATIC_DRAW );

	u32 total_size = calculate_vertex_size_in_bytes( layout_data, layout_size );

	// Bind vertex attrib pointers for vao using layout descriptor
	gs_for_range_i( layout_size )
	{
		gs_vertex_attribute_type type = layout_data[ i ];
		u32 byte_offset = get_byte_offest( layout_data, i );

		switch ( type )
		{
			case gs_vertex_attribute_float4: 
			{
				glVertexAttribPointer( i, 4, GL_FLOAT, GL_FALSE, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_float3: 
			{
				glVertexAttribPointer( i, 3, GL_FLOAT, GL_FALSE, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_float2: 
			{
				glVertexAttribPointer( i, 2, GL_FLOAT, GL_FALSE, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_float: 
			{
				glVertexAttribPointer( i, 1, GL_FLOAT, GL_FALSE, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_uint4: {
				glVertexAttribIPointer( i, 4, GL_UNSIGNED_INT, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_uint3: 
			{
				glVertexAttribIPointer( i, 3, GL_UNSIGNED_INT, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_uint2: 
			{
				glVertexAttribIPointer( i, 2, GL_UNSIGNED_INT, total_size, int_2_void_p( byte_offset ) );
			} break;	
			case gs_vertex_attribute_uint: 
			{
				glVertexAttribIPointer( i, 1, GL_UNSIGNED_INT, total_size, int_2_void_p( byte_offset ) );
			} break;	

			default: 
			{
				// Shouldn't get here
				gs_assert( false );
			} break;
		}

		// Enable the vertex attribute pointer
		glEnableVertexAttribArray( i );
	}

	// Unbind buffer and array
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	// Set resource handle
	gs_resource( gs_vertex_buffer ) handle = {0};
	handle.id = vb_handle;
	return handle;
}

void opengl_update_vertex_buffer_data( gs_resource( gs_vertex_buffer ) v_handle, void* v_data, usize v_sz )
{
	gs_assert( v_data );

	opengl_render_data_t* data = __get_opengl_data_internal();

	// Construct new vertex buffer, then insert into slot array
	vertex_buffer_t* vb = gs_slot_array_get_ptr( data->vertex_buffers, v_handle.id );

	// Bind vao/vbo
	glBindVertexArray( (u32)vb->vao );
	glBindBuffer( GL_ARRAY_BUFFER, (u32)vb->vbo );
	glBufferData( GL_ARRAY_BUFFER, v_sz, v_data, GL_STATIC_DRAW );

	// Unbind buffer and array
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
}

gs_resource( gs_index_buffer ) opengl_construct_index_buffer( void* indices, usize sz )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Construct new vertex buffer, then insert into slot array
	u32 ib_handle = gs_slot_array_insert( data->index_buffers, (index_buffer_t){0} );
	index_buffer_t* ib = gs_slot_array_get_ptr( data->index_buffers, ib_handle );

	glGenBuffers( 1, &ib->ibo );
  	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sz, indices, GL_STATIC_DRAW );

    // Unbind buffer after setting data
  	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  	// Create resource
	gs_resource( gs_index_buffer ) handle = {0};
	handle.id = ib_handle;
	return handle;
}

// Compiles single shader
void opengl_compile_shader( const char* src, u32 id ) 
{
	//Tell opengl that we want to use fileContents as the contents of the shader file
	glShaderSource( id, 1, &src, NULL );

	//Compile the shader
	glCompileShader( id );

	//Check for errors
	GLint success = 0;
	glGetShaderiv( id, GL_COMPILE_STATUS, &success );

	if (success == GL_FALSE)
	{
		GLint max_len = 0;
		glGetShaderiv( id, GL_INFO_LOG_LENGTH, &max_len );

		char* log = gs_malloc( max_len );
		memset( log, 0, max_len );

		//The max_len includes the NULL character
		glGetShaderInfoLog( id, max_len, &max_len, log );

		//Provide the infolog in whatever manor you deem best.
		//Exit with failure.
		glDeleteShader( id ); //Don't leak the shader.

		gs_println( "Opengl::opengl_compile_shader::FAILED_TO_COMPILE: %s", log );

		free( log );
		log = NULL;

		gs_assert( false );
	}
}

void opengl_link_shaders( u32 program_id, u32 vert_id, u32 frag_id )
{
	//Attach our shaders to our program
	glAttachShader( program_id, vert_id );
	glAttachShader( program_id, frag_id );

	//Link our program
	glLinkProgram( program_id );

	//Create info log
	// Error shit...
	s32 is_linked = 0;
	glGetProgramiv( program_id, GL_LINK_STATUS, (s32*)&is_linked );
	if ( is_linked == GL_FALSE )
	{
		GLint max_len = 0;
		glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &max_len );

		char* log = gs_malloc( max_len );
		memset( log, 0, max_len );
		glGetProgramInfoLog( program_id, max_len, &max_len, log ); 

		// Print error
		gs_println( "Fail To Link::opengl_link_shaders::%s" );

		// //We don't need the program anymore.
		glDeleteProgram( program_id );

		// //Don't leak shaders either.
		glDeleteShader( vert_id );
		glDeleteShader( frag_id );

		free( log );
		log = NULL;

		// Just assert for now
		gs_assert( false );
	}
}

gs_resource( gs_shader ) opengl_construct_shader( const char* vert_src, const char* frag_src )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Shader to fill out
	shader_t s = {0};

	// Construct vertex program
	s.program_id = glCreateProgram();

	// Construct vertex shader
	u32 vert_id = glCreateShader( GL_VERTEX_SHADER );
	gs_assert( vert_id != 0 );

	// Construct fragment shader
	u32 frag_id = glCreateShader( GL_FRAGMENT_SHADER );
	gs_assert( frag_id != 0 );

	// Compile each shader separately
	opengl_compile_shader( vert_src, vert_id );
	opengl_compile_shader( frag_src, frag_id );

	// Link shaders once compiled
	opengl_link_shaders( s.program_id, vert_id, frag_id );

	// Push shader into slot array
	u32 s_handle = gs_slot_array_insert( data->shaders, s );

	// Set resource handle
	gs_resource( gs_shader ) handle = {0};
	handle.id = s_handle;

	return handle;
}

gs_resource( gs_uniform ) opengl_construct_uniform( gs_resource( gs_shader ) s_handle, const char* uniform_name, gs_uniform_type type )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Uniform to fill out
	uniform_t u = {0};

	// Grab shader from data
	shader_t s = gs_slot_array_get( data->shaders, s_handle.id );

	// Grab location of uniform
	u32 location = glGetUniformLocation( s.program_id, uniform_name );

	if ( location >= u32_max ) {
		gs_println( "Warning: uniform not found: \"%s\"", uniform_name );
	}

	// Won't know uniform's type, unfortunately...
	// Set uniform location
	u.location = location;
	u.type = type;

	// Push back uniform into slot array
	u32 u_handle = gs_slot_array_insert( data->uniforms, u );

	// Set resource handle
	gs_resource( gs_uniform ) handle = {0};
	handle.id = u_handle;
	return handle;
}

gs_resource( gs_frame_buffer ) opengl_construct_frame_buffer( gs_resource( gs_texture ) t_handle )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Render target to create
	frame_buffer_t fb = {0};

	// render_target_t rt = gs_slot_array_get( data->render_targets, rt_handle.id );
	texture_t tex = gs_slot_array_get( data->textures, t_handle.id );

	// Construct and bind frame buffer
	glGenFramebuffers( 1, &fb.fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, fb.fbo );

	// Set list of buffers to draw for this framebuffer 
	// This will need to be changed, I think, but when and where?
	GLenum draw_buffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, draw_buffers );

	// Idx is 0, for now
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, tex.id , 0);

	// Error checking
	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		gs_println( "Error: Frame buffer could not be created." );
		gs_assert( false );
	}

	// Set frame buffer to back buffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Push back uniform into slot array
	u32 _handle = gs_slot_array_insert( data->frame_buffers, fb );

	// Set resource handle
	gs_resource( gs_frame_buffer ) handle = {0};
	handle.id = _handle;
	return handle;
}

gs_resource( gs_render_target ) opengl_construct_render_target( gs_texture_parameter_desc t_desc )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Render target to create
	render_target_t target = {0};

	// Construct color texture
	target.tex_handle = opengl_construct_texture( t_desc );

	// Push back uniform into slot array
	u32 _handle = gs_slot_array_insert( data->render_targets, target );

	// Set resource handle
	gs_resource( gs_render_target ) handle = {0};
	handle.id = _handle;
	return handle;
}

gs_resource( gs_texture ) opengl_construct_texture( gs_texture_parameter_desc desc )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Texture to fill out
	texture_t tex = {0};

	gs_texture_format texture_format = desc.texture_format;
	u32 width = desc.width;
	u32 height = desc.height;
	u32 num_comps = desc.num_comps;

	glGenTextures( 1, &( tex.id ) );
	glBindTexture( GL_TEXTURE_2D, tex.id );

	// Construct texture based on appropriate format
	switch( texture_format ) 
	{
		case gs_texture_format_rgb8: 	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, desc.data ); break;
		case gs_texture_format_rgba8: 	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, desc.data ); break;
		case gs_texture_format_rgba16f: glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, desc.data ); break;
	}

	s32 mag_filter = desc.mag_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;
	s32 min_filter = desc.min_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;

	if ( desc.generate_mips ) 
	{
		if ( desc.min_filter == gs_nearest ) 
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

	// Anisotropic filtering ( not supported by default, need to figure this out )
	// f32 aniso = 0.0f; 
	// glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
	// glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );

	if ( desc.generate_mips )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	tex.width 			= width;
	tex.height 			= height;
	tex.num_comps 		= num_comps;
	tex.texture_format 	= texture_format;

	glBindTexture( GL_TEXTURE_2D, 0 );

	// Push back texture for handle
	u32 t_handle = gs_slot_array_insert( data->textures, tex );

	gs_resource( gs_texture ) handle = {0};
	handle.id = t_handle;
	return handle;
}

void* opengl_load_texture_data_from_file( const char* file_path, b32 flip_vertically_on_load, 
				gs_texture_format texture_format, s32* width, s32* height, s32* num_comps )
{
	// Load texture data
	stbi_set_flip_vertically_on_load( flip_vertically_on_load );

	// Load in texture data using stb for now
	char temp_file_extension_buffer[ 16 ] = {0}; 
	gs_util_get_file_extension( temp_file_extension_buffer, sizeof( temp_file_extension_buffer ), file_path );

	// Load texture data
	stbi_set_flip_vertically_on_load( flip_vertically_on_load );

	// Texture data to fill out
	void* texture_data = NULL;

	switch ( texture_format )
	{
		case gs_texture_format_rgba8: texture_data = (u8*)stbi_load( file_path, width, height, num_comps, STBI_rgb_alpha ); break;
		case gs_texture_format_rgb8: texture_data = (u8*)stbi_load( file_path, width, height, num_comps, STBI_rgb ); break;

		// TODO(john): support this format
		case gs_texture_format_rgba16f: 
		default:
		{
			gs_assert( false );
		} break;
	}

	if ( !texture_data ) {
		gs_println( "Warning: could not load texture: %s", file_path );
	}

	return texture_data;
}

gs_resource( gs_texture ) opengl_construct_texture_from_file( const char* file_path, b32 flip_vertically_on_load, gs_texture_parameter_desc t_desc )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Load texture data and fill out parameters for descriptor
	t_desc.data = opengl_load_texture_data_from_file( file_path, t_desc.texture_format, 
		flip_vertically_on_load, (s32*)&t_desc.width, (s32*)&t_desc.height, (s32*)&t_desc.num_comps );

	// Finish constructing texture resource from descriptor and return handle
	return ( opengl_construct_texture( t_desc ) );
}

void opengl_update_texture_data( gs_resource( gs_texture ) t_handle, gs_texture_parameter_desc t_desc )
{
	opengl_render_data_t* data = __get_opengl_data_internal();

	// Get texture from slot id
	texture_t* tex = gs_slot_array_get_ptr( data->textures, t_handle.id );

	gs_texture_format texture_format = t_desc.texture_format;
	u32 width = t_desc.width;
	u32 height = t_desc.height;
	u32 num_comps = t_desc.num_comps;

	glBindTexture( GL_TEXTURE_2D, tex->id );

	// Construct texture based on appropriate format
	switch( texture_format ) 
	{
		case gs_texture_format_rgb8: 	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, t_desc.data ); break;
		case gs_texture_format_rgba8: 	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, t_desc.data ); break;
		case gs_texture_format_rgba16f: glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, t_desc.data ); break;
	}

	s32 mag_filter = t_desc.mag_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;
	s32 min_filter = t_desc.min_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;

	if ( t_desc.generate_mips ) 
	{
		if ( t_desc.min_filter == gs_nearest ) 
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

	// Anisotropic filtering ( not supported by default, need to figure this out )
	// f32 aniso = 0.0f; 
	// glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
	// glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );

	if ( t_desc.generate_mips )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	glBindTexture( GL_TEXTURE_2D, 0 );

	tex->width 			= width;
	tex->height 		= height;
	tex->num_comps 		= num_comps;
	tex->texture_format = texture_format;
}

// Method for creating platform layer for SDL
struct gs_graphics_i* gs_graphics_construct()
{
	// Construct new graphics interface instance
	struct gs_graphics_i* gfx = gs_malloc_init( gs_graphics_i );

	// Null out data
	gfx->data = NULL;

	// /*============================================================
	// // Graphics Initilization / De-Initialization
	// ============================================================*/
	gfx->init 		= &opengl_init;
	gfx->shutdown 	= &opengl_shutdown;
	gfx->update 	= &opengl_update;

	// /*============================================================
	// // Graphics Command Buffer Ops
	// ============================================================*/
	gfx->reset_command_buffer 			= &opengl_reset_command_buffer;
	gfx->bind_shader	      			= &opengl_bind_shader;
	gfx->bind_vertex_buffer 			= &opengl_bind_vertex_buffer;
	gfx->bind_index_buffer 				= &opengl_bind_index_buffer;
	gfx->bind_texture 					= &opengl_bind_texture;
	gfx->bind_uniform 					= &opengl_bind_uniform;
	gfx->bind_frame_buffer  			= &opengl_bind_frame_buffer;
	gfx->unbind_frame_buffer 			= &opengl_unbind_frame_buffer;
	gfx->set_frame_buffer_attachment 	= &opengl_set_frame_buffer_attachment;
	gfx->set_view_clear 				= &opengl_set_view_clear;
	gfx->set_view_port 					= &opengl_set_view_port;
	gfx->set_depth_enabled 				= &opengl_set_depth_enabled;
	gfx->draw 							= &opengl_draw;
	gfx->draw_indexed 					= &opengl_draw_indexed;
	gfx->submit_command_buffer 			= &opengl_submit_command_buffer;


	// void ( * set_uniform_buffer_sub_data )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform_buffer ), void*, usize );
	// void ( * set_index_buffer )( gs_resource( gs_command_buffer ), gs_resource( gs_index_buffer ) );

	// ============================================================
	// // Graphics Resource Construction
	// ============================================================
	gfx->construct_shader 						= &opengl_construct_shader;
	gfx->construct_uniform 						= &opengl_construct_uniform;
	gfx->construct_command_buffer 				= &opengl_construct_command_buffer;
	gfx->construct_frame_buffer 				= &opengl_construct_frame_buffer;
	gfx->construct_render_target 				= &opengl_construct_render_target;
	gfx->construct_vertex_buffer 				= &opengl_construct_vertex_buffer;
	gfx->update_vertex_buffer_data 				= &opengl_update_vertex_buffer_data;
	gfx->construct_index_buffer 				= &opengl_construct_index_buffer;
	gfx->construct_texture 						= &opengl_construct_texture;
	gfx->construct_texture_from_file 			= &opengl_construct_texture_from_file;
	gfx->update_texture_data 					= &opengl_update_texture_data;
	gfx->load_texture_data_from_file 			= &opengl_load_texture_data_from_file;
	// gs_resource( gs_uniform_buffer )( * construct_uniform_buffer )( gs_resource( gs_shader ), const char* uniform_name );

	// /*============================================================
	// // Graphics Resource Free Ops
	// ============================================================*/
	// void ( * free_vertex_attribute_layout_t_desc )( gs_resource( gs_vertex_attribute_layout_desc ) );
	// void ( * free_vertex_buffer )( gs_resource( gs_vertex_buffer ) );
	// void ( * free_index_buffer )( gs_resource( gs_index_buffer ) );
	// void ( * free_shader )( gs_resource( gs_shader ) );
	// void ( * free_uniform_buffer )( gs_resource( gs_uniform_buffer ) );

	// /*============================================================
	// // Graphics Update Ops
	// ============================================================*/

	/*============================================================
	// Graphics Debug Rendering Ops
	============================================================*/
	gfx->set_debug_draw_properties = &opengl_set_debug_draw_properties;
	gfx->draw_line = &opengl_draw_line;
	gfx->draw_square = &opengl_draw_square;
	gfx->submit_debug_drawing = &opengl_submit_debug_drawing;

	return gfx;
}
