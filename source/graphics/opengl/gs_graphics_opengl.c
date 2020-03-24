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

typedef enum gs_opengl_op_code
{
	gs_opengl_op_bind_shader = 0,
	gs_opengl_op_set_view_clear,
	gs_opengl_op_set_viewport,
	gs_opengl_op_set_depth_enabled,
	gs_opengl_op_bind_vertex_buffer,
	gs_opengl_op_bind_index_buffer,
	gs_opengl_op_bind_uniform,
	gs_opengl_op_bind_texture,
	gs_opengl_op_draw,
	gs_opengl_draw_indexed
} gs_opengl_op_code;

// Could make this a simple byte buffer, then read from that buffer for commands?
typedef struct command_buffer
{
	u32 num_commands;
	gs_byte_buffer commands;
} command_buffer;

typedef struct texture
{
	u16 width;
	u16 height;
	u32 id;	
	u32 num_comps;
	gs_texture_format texture_format;
} texture;

typedef struct shader
{
	u32 program_id;
} shader;

typedef struct uniform
{
	gs_uniform_type type;
	u32 location;
} uniform;

typedef struct index_buffer
{
	u32 ibo;
} index_buffer;

typedef struct vertex_buffer
{
	u32 vbo;
	u32 vao;		// Not sure if I need to do this as well, but we will for now...
} vertex_buffer;

typedef struct vertex_attribute_layout_desc
{
	gs_dyn_array( gs_vertex_attribute_type ) attributes;	
} vertex_attribute_layout_desc;

// Slot array declarations
gs_slot_array_decl( command_buffer );
gs_slot_array_decl( texture );
gs_slot_array_decl( shader );
gs_slot_array_decl( uniform );
gs_slot_array_decl( index_buffer );
gs_slot_array_decl( vertex_buffer );
gs_slot_array_decl( vertex_attribute_layout_desc );

// Define render resource data structure
typedef struct gs_opengl_render_data
{
	gs_slot_array( command_buffer ) 				command_buffers;
	gs_slot_array( texture ) 						textures;
	gs_slot_array( shader ) 						shaders;
	gs_slot_array( uniform ) 						uniforms;
	gs_slot_array( index_buffer )   				index_buffers;
	gs_slot_array( vertex_buffer )  				vertex_buffers;
	gs_slot_array( vertex_attribute_layout_desc ) 	vertex_layout_descs;
} gs_opengl_render_data;

#define __get_opengl_data_internal()\
	(gs_opengl_render_data*)(gs_engine_instance()->ctx.graphics->data)

#define __get_command_buffer_internal( data, cb )\
	(gs_slot_array_get_ptr(data->command_buffers, cb.id))

// Forward Decls;
void __reset_command_buffer_internal( command_buffer* cb );

/*============================================================
// Graphics Initilization / De-Initialization
============================================================*/

void opengl_init_default_state()
{
	// Init things...
}

gs_result opengl_init( struct gs_graphics_i* gfx )
{
	// Construct instance of render data
	struct gs_opengl_render_data* data = gs_malloc_init( gs_opengl_render_data );

	// Initialize data
	data->command_buffers 		= gs_slot_array_new( command_buffer );
	data->textures				= gs_slot_array_new( texture );
	data->shaders 				= gs_slot_array_new( shader );
	data->uniforms 				= gs_slot_array_new( uniform );
	data->index_buffers 		= gs_slot_array_new( index_buffer );
	data->vertex_buffers 		= gs_slot_array_new( vertex_buffer );
	data->vertex_layout_descs 	= gs_slot_array_new( vertex_attribute_layout_desc );

	// Set data
	gfx->data = data;

	// Init all default opengl state here before frame begins
	opengl_init_default_state();

	gs_println( "OpenGL::Vendor = %s", glGetString( GL_VENDOR ) ) ;
	gs_println( "OpenGL::Renderer = %s", glGetString( GL_RENDERER ) ) ;
	gs_println( "OpenGL::Version = %s", glGetString( GL_VERSION ) ) ;

	// Initialize all data here
	return gs_result_success;
}

gs_result opengl_update()
{
	gs_opengl_render_data* data = __get_opengl_data_internal();

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

void __reset_command_buffer_internal( command_buffer* cb )
{
	// Clear byte buffer of commands
	gs_byte_buffer_clear( &cb->commands );

	// Set num commands to 0
	cb->num_commands = 0;
}

void opengl_reset_command_buffer( gs_resource( gs_command_buffer ) cb_handle )
{
	// Get data from graphics api
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	__reset_command_buffer_internal( cb );
}

void opengl_bind_shader( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_shader ) s_handle )
{
	// Get data from graphics api
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab shader from handle
	shader s = gs_slot_array_get( data->shaders, s_handle.id );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab uniform from handle
	uniform u = gs_slot_array_get( data->uniforms, u_handle.id );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Get texture
	texture tex = gs_slot_array_get( data->textures, tex_handle.id );

	// Grab uniform from handle
	uniform u = gs_slot_array_get( data->uniforms, u_handle.id );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Get vertex buffer data
	vertex_buffer vb = gs_slot_array_get( data->vertex_buffers, vb_handle.id );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Get index buffer data
	index_buffer ib = gs_slot_array_get( data->index_buffers, ib_handle.id );

	// Write op code
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_index_buffer );
	// Write out ibo
	gs_byte_buffer_write( &cb->commands, u32, ib.ibo );

	// Increase command amount
	cb->num_commands++;
}

void opengl_set_viewport( gs_resource( gs_command_buffer ) cb_handle, u32 width, u32 height )
{
	// Get data from graphics api
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Write op into buffer
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_set_viewport );
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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Write draw command
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_draw );	
	// Write start
	gs_byte_buffer_write( &cb->commands, u32, start );
	// Write count
	gs_byte_buffer_write( &cb->commands, u32, count );

	// Increase command amount
	cb->num_commands++;
}

// For now, just rip through command buffer. Later on, will add all command buffers into a queue to be processed on rendering thread.
void opengl_submit_command_buffer( gs_resource( gs_command_buffer ) cb_handle )
{
	// Get data from graphics api
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

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
			case gs_opengl_op_draw:
			{
				// Read start
				u32 start = gs_byte_buffer_read( &cb->commands, u32 );
				// Read count
				u32 count = gs_byte_buffer_read( &cb->commands, u32 );

				// Draw ( this assumes a vao is set, which is not correct )...for now, will assume
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

			case gs_opengl_op_set_viewport: 
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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Construct new command buffer, then insert into slot array
	u32 cb_handle = gs_slot_array_insert( data->command_buffers, (command_buffer){0} );
	command_buffer* cb = gs_slot_array_get_ptr( data->command_buffers, cb_handle );

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Construct new vertex buffer, then insert into slot array
	u32 vb_handle = gs_slot_array_insert( data->vertex_buffers, (vertex_buffer){0} );
	vertex_buffer* vb = gs_slot_array_get_ptr( data->vertex_buffers, vb_handle );

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

gs_resource( gs_index_buffer ) opengl_construct_index_buffer( void* indices, usize sz )
{
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Construct new vertex buffer, then insert into slot array
	u32 ib_handle = gs_slot_array_insert( data->index_buffers, (index_buffer){0} );
	index_buffer* ib = gs_slot_array_get_ptr( data->index_buffers, ib_handle );

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
	// Error shit... skip for now
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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Shader to fill out
	shader s = {0};

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
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Uniform to fill out
	uniform u = {0};

	// Grab shader from data
	shader s = gs_slot_array_get( data->shaders, s_handle.id );

	// Grab location of uniform
	u32 location = glGetUniformLocation( s.program_id, uniform_name );

	if ( location >= u32_max )
	{
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

gs_resource( gs_texture ) opengl_construct_texture( gs_texture_parameter_desc desc )
{
	gs_opengl_render_data* data = __get_opengl_data_internal();

	if ( desc.data == NULL )
	{
		gs_println( "Texture data cannot be null." );
		gs_assert( false );
	}

	// Texture to fill out
	texture tex = {0};

	gs_texture_format texture_format = desc.texture_format;
	u32 width = desc.width;
	u32 height = desc.height;
	u32 num_comps = desc.num_comps;

	// Standard texture format
	switch( texture_format )
	{
		case gs_texture_format_ldr:
		{
			void* texture_data = desc.data;

			glGenTextures( 1, &( tex.id ) );
			glBindTexture( GL_TEXTURE_2D, tex.id );

			// Generate texture depending on number of components in texture data
			switch ( num_comps )
			{
				case 3: 
				{
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data ); 
				} break;

				default:
				case 4: 
				{
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data ); 
				} break;
			}

			s32 mag_filter = desc.mag_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;
			s32 min_filter = desc.min_filter == gs_nearest ? GL_NEAREST : GL_LINEAR;

			if ( desc.generate_mips )
			{
				if ( desc.min_filter == gs_nearest ) {
					min_filter = desc.mipmap_filter == gs_nearest ? GL_NEAREST_MIPMAP_NEAREST : 
						GL_NEAREST_MIPMAP_LINEAR;
				} else {
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

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );

			if ( desc.generate_mips )
			{
				glGenerateMipmap( GL_TEXTURE_2D );
			}

			tex.width 			= width;
			tex.height 			= height;
			tex.num_comps 		= num_comps;
			tex.texture_format 	= texture_format;

			glBindTexture( GL_TEXTURE_2D, 0 );

			// gs_free( texture_data );
			// texture_data = NULL;

		} break;

		// TODO(john): support this format
		case gs_texture_format_hdr:
		default:
		{
			gs_assert( false );
		} break;
	}

	// Push back texture for handle
	u32 t_handle = gs_slot_array_insert( data->textures, tex );

	gs_resource( gs_texture ) handle = {0};
	handle.id = t_handle;
	return handle;
}

void* opengl_load_texture_data_from_file( const char* file_path, b32 flip_vertically_on_load, 
				u32* width, u32* height, u32* num_comps , gs_texture_format* texture_format )
{
	// Load texture data
	stbi_set_flip_vertically_on_load( flip_vertically_on_load );

	// Load in texture data using stb for now
	char temp_file_extension_buffer[ 16 ] = {0}; 
	gs_util_get_file_extension( temp_file_extension_buffer, sizeof( temp_file_extension_buffer ), file_path );

	if ( gs_string_compare_equal( temp_file_extension_buffer, "hdr" ) ) {
		*texture_format = gs_texture_format_hdr;
	} else {
		*texture_format = gs_texture_format_ldr;
	}

	// Texture data to fill out
	void* texture_data = NULL;

	switch ( *texture_format )
	{
		case gs_texture_format_ldr:
		{
			// Load texture data
			stbi_set_flip_vertically_on_load( true );

			// For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param
			// Could optimize this later
			texture_data = ( u8* )stbi_load( file_path, (s32*)width, (s32*)height, (s32*)num_comps, STBI_rgb_alpha );

			if ( !texture_data )
			{
				gs_println( "Warning: could not load texture: %s", file_path );
			}
		} break;

		// TODO(john): support this format
		case gs_texture_format_hdr:
		default:
		{
			gs_assert( false );
		} break;
	}

	return texture_data;
}

gs_resource( gs_texture ) opengl_construct_texture_from_file( const char* file_path )
{
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Texture to fill out
	texture tex = {0};

	// Load in texture data using stb for now
	char temp_file_extension_buffer[ 16 ] = {0}; 
	gs_texture_format texture_format;
	gs_util_get_file_extension( temp_file_extension_buffer, sizeof( temp_file_extension_buffer ), file_path );

	if ( gs_string_compare_equal( temp_file_extension_buffer, "hdr" ) ) {
		texture_format = gs_texture_format_hdr;
	} else {
		texture_format = gs_texture_format_ldr;
	}

	// Fields to load and store
	s32 width, height, num_comps, len;

	void* texture_data;

	// Standard texture format
	switch( texture_format )
	{
		case gs_texture_format_ldr:
		{
			// Load texture data
			stbi_set_flip_vertically_on_load( true );

			// For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param
			// Could optimize this later
			texture_data = ( u8* )stbi_load( file_path, ( s32* )&width, ( s32* )&height, &num_comps, STBI_rgb_alpha );

			if ( !texture_data )
			{
				gs_println( "Warning: could not load texture: %s", file_path );
			}

			// TODO(): For some reason, required components is not working, so just default to 4 for now
			num_comps = 4;

			glGenTextures( 1, &( tex.id ) );
			glBindTexture( GL_TEXTURE_2D, tex.id );

			// Generate texture depending on number of components in texture data
			switch ( num_comps )
			{
				case 3: 
				{
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data ); 
				} break;

				default:
				case 4: 
				{
					glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data ); 
				} break;
			}

			s32 MAG_PARAM = GL_LINEAR;
			s32 MIN_PARAM = GL_LINEAR_MIPMAP_LINEAR;
			b32 genMips = true;

			// Anisotropic filtering ( not supported by default, need to figure this out )
			// f32 aniso = 0.0f; 
			// glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso );
			// glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAG_PARAM );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MIN_PARAM );

			if ( genMips )
			{
				glGenerateMipmap( GL_TEXTURE_2D );
			}

			tex.width 		= width;
			tex.height 		= height;
			tex.num_comps 	= num_comps;
			tex.texture_format = texture_format;

			glBindTexture( GL_TEXTURE_2D, 0 );

			gs_free( texture_data );
			texture_data = NULL;
		} break;

		// TODO(john): support this format
		case gs_texture_format_hdr:
		default:
		{
			gs_assert( false );
		} break;
	}

	// Push back texture for handle
	u32 t_handle = gs_slot_array_insert( data->textures, tex );

	gs_resource( gs_texture ) handle = {0};
	handle.id = t_handle;
	return handle;
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
	gfx->reset_command_buffer 	= &opengl_reset_command_buffer;
	gfx->bind_shader	      	= &opengl_bind_shader;
	gfx->bind_vertex_buffer 	= &opengl_bind_vertex_buffer;
	gfx->bind_index_buffer 		= &opengl_bind_index_buffer;
	gfx->bind_texture 			= &opengl_bind_texture;
	gfx->bind_uniform 			= &opengl_bind_uniform;
	gfx->set_view_clear 		= &opengl_set_view_clear;
	gfx->set_viewport 			= &opengl_set_viewport;
	gfx->set_depth_enabled 		= &opengl_set_depth_enabled;
	gfx->draw 					= &opengl_draw;
	gfx->draw_indexed 			= &opengl_draw_indexed;
	gfx->submit_command_buffer 	= &opengl_submit_command_buffer;

	// void ( * set_uniform_buffer_sub_data )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform_buffer ), void*, usize );
	// void ( * set_index_buffer )( gs_resource( gs_command_buffer ), gs_resource( gs_index_buffer ) );

	// ============================================================
	// // Graphics Resource Construction
	// ============================================================
	gfx->construct_shader 						= &opengl_construct_shader;
	gfx->construct_uniform 						= &opengl_construct_uniform;
	gfx->construct_command_buffer 				= &opengl_construct_command_buffer;
	gfx->construct_vertex_buffer 				= &opengl_construct_vertex_buffer;
	gfx->construct_index_buffer 				= &opengl_construct_index_buffer;
	gfx->construct_texture 						= &opengl_construct_texture;
	gfx->construct_texture_from_file 			= &opengl_construct_texture_from_file;
	gfx->load_texture_data_from_file 			= &opengl_load_texture_data_from_file;
	// gs_resource( gs_uniform_buffer )( * construct_uniform_buffer )( gs_resource( gs_shader ), const char* uniform_name );

	// /*============================================================
	// // Graphics Resource Free Ops
	// ============================================================*/
	// void ( * free_vertex_attribute_layout_desc )( gs_resource( gs_vertex_attribute_layout_desc ) );
	// void ( * free_vertex_buffer )( gs_resource( gs_vertex_buffer ) );
	// void ( * free_index_buffer )( gs_resource( gs_index_buffer ) );
	// void ( * free_shader )( gs_resource( gs_shader ) );
	// void ( * free_uniform_buffer )( gs_resource( gs_uniform_buffer ) );

	// /*============================================================
	// // Graphics Update Ops
	// ============================================================*/

	return gfx;
}
