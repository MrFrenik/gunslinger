#include "graphics/gs_graphics.h"
#include "common/gs_containers.h"
#include "serialize/gs_byte_buffer.h"
#include "math/gs_math.h"
#include "common/gs_util.h"
#include "base/gs_engine.h"

// Not sure if this will be available by default...
#include <glad/glad.h>

// gs_declare_resource_type( gs_command_buffer );
// gs_declare_resource_type( gs_uniform_buffer );
// gs_declare_resource_type( gs_vertex_buffer );
// gs_declare_resource_type( gs_index_buffer );
// gs_declare_resource_type( gs_texture );
// gs_declare_resource_type( gs_shader );
// gs_declare_resource_type( gs_uniform );
// gs_declare_resource_type( gs_vertex_attribute_layout_desc );
// gs_declare_resource_type( gs_render_target );
// gs_declare_resource_type( gs_frame_buffer );

// Need to define the above types (not sure if they should be entirely renderer specific; thinking so...)

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
	gs_opengl_op_bind_set_uniform,
	gs_opengl_op_set_view_clear,
	gs_opengl_op_bind_vertex_buffer,
	gs_opengl_op_draw
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

void opengl_bind_set_bind_uniform( gs_resource( gs_command_buffer ) cb_handle, gs_resource( gs_uniform ) u_handle, void* u_data )
{
	// Get data from graphics api
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Grab command buffer ptr from command buffer slot array
	command_buffer* cb = __get_command_buffer_internal( data, cb_handle );

	// Grab uniform from handle
	uniform u = gs_slot_array_get( data->uniforms, u_handle.id );

	// Write out op code
	gs_byte_buffer_write( &cb->commands, u32, gs_opengl_op_bind_set_uniform );
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

		default:
		{
			gs_println( "Invalid uniform type passed" );
			gs_assert( false );
		}
	}

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

			case gs_opengl_op_set_view_clear: 
			{
				// Read color from buffer (as vec4)
				gs_vec4 col = gs_byte_buffer_read( &cb->commands, gs_vec4 );
				// Set clear color
				glClearColor( col.x, col.y, col.z, col.w );
				// Clear screen
				glClear( GL_COLOR_BUFFER_BIT );
				// glViewport(0, 0, 800, 600);
			} break;

			case gs_opengl_op_bind_vertex_buffer:
			{
				// Read out vao
				u32 vao = gs_byte_buffer_read( &cb->commands, u32 );

				// Bind vao
				glBindVertexArray( vao );
			} break;

			case gs_opengl_op_bind_shader: 
			{
				// Read in shader id
				u32 program_id = gs_byte_buffer_read( &cb->commands, u32 );
				// Bind program
				glUseProgram( program_id );
			} break;

			case gs_opengl_op_bind_set_uniform:
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
						glUniformMatrix4fv( location, 16, false, (f32*)(val.elements) );
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

gs_resource( gs_vertex_attribute_layout_desc ) opengl_construct_vertex_attribute_layout_desc()
{
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Construct new command buffer, then insert into slot array
	u32 vd_handle = gs_slot_array_insert( data->vertex_layout_descs, (vertex_attribute_layout_desc){0} );
	vertex_attribute_layout_desc* vd = gs_slot_array_get_ptr( data->vertex_layout_descs, vd_handle );

	// Set data
	vd->attributes = gs_dyn_array_new( gs_vertex_attribute_type );

	// Set resource handle
	gs_resource( gs_vertex_attribute_layout_desc ) handle = {0};
	handle.id = vd_handle;

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

u32 calculate_vertex_size_in_bytes( vertex_attribute_layout_desc* desc )
{
	// Iterate through all formats in delcarations and calculate total size
	u32 sz = 0;

	gs_for_range_i( gs_dyn_array_size( desc->attributes ) )
	{
		gs_vertex_attribute_type type = desc->attributes[ i ];
		sz += get_byte_size_of_vertex_attribute( type );
	}

	return sz;
}

s32 get_byte_offest( vertex_attribute_layout_desc* desc, u32 index )
{
	gs_assert( desc );
	gs_assert( index < gs_dyn_array_size( desc->attributes ) );

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
		total_offset += get_byte_size_of_vertex_attribute( desc->attributes[ i ] );
	} 

	return total_offset;
}

#define int_2_void_p(i) (void*)(uintptr_t)(i)

void opengl_add_vertex_attribute( gs_resource( gs_vertex_attribute_layout_desc ) vd_handle, gs_vertex_attribute_type type )
{
	gs_opengl_render_data* data = __get_opengl_data_internal();
	gs_dyn_array_push( ( gs_slot_array_get( data->vertex_layout_descs, vd_handle.id ) ).attributes, type );
}

gs_resource( gs_vertex_buffer ) opengl_construct_vertex_buffer( gs_resource( gs_vertex_attribute_layout_desc ) v_desc_handle, 
	void* v_data, usize v_data_size )
{
	gs_opengl_render_data* data = __get_opengl_data_internal();

	// Construct new vertex buffer, then insert into slot array
	u32 vb_handle = gs_slot_array_insert( data->vertex_buffers, (vertex_buffer){0} );
	vertex_buffer* vb = gs_slot_array_get_ptr( data->vertex_buffers, vb_handle );

	vertex_attribute_layout_desc* v_desc = gs_slot_array_get_ptr( data->vertex_layout_descs, v_desc_handle.id );

	// Create and bind vertex array
	glGenVertexArrays( 1, (u32*)&vb->vao );
	glBindVertexArray( (u32)vb->vao );

	// Create and upload mesh data
	glGenBuffers( 1, (u32*)&vb->vbo );
	glBindBuffer( GL_ARRAY_BUFFER, (u32)vb->vbo );
	glBufferData( GL_ARRAY_BUFFER, v_data_size, v_data, GL_STATIC_DRAW );

	u32 total_size = calculate_vertex_size_in_bytes( v_desc );

	// Bind vertex attrib pointers for vao using layout descriptor
	gs_for_range_i( gs_dyn_array_size( v_desc->attributes ) )
	{
		glEnableVertexAttribArray( i );
		gs_vertex_attribute_type type = v_desc->attributes[ i ];
		u32 byte_offset = get_byte_offest( v_desc, i );

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
	}

	// Unbind buffer and array
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	// Set resource handle
	gs_resource( gs_vertex_buffer ) handle = {0};
	handle.id = vb_handle;

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
	gfx->set_view_clear 		= &opengl_set_view_clear;
	gfx->draw 					= &opengl_draw;
	gfx->submit_command_buffer 	= &opengl_submit_command_buffer;
	gfx->bind_set_uniform 		= &opengl_bind_set_bind_uniform;

	// void ( * set_uniform_buffer_sub_data )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform_buffer ), void*, usize );
	// void ( * set_uniform )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform ), void* );
	// void ( * set_index_buffer )( gs_resource( gs_command_buffer ), gs_resource( gs_index_buffer ) );

	// ============================================================
	// // Graphics Resource Construction
	// ============================================================
	// gs_resource( gs_shader )( * construct_shader )( const char* vert_src, const char* frag_src );
	// gs_resource( gs_uniform )( * construct_uniform )( gs_resource( gs_shader ), const char* uniform_name );
	// gs_resource( gs_uniform_buffer )( * construct_uniform_buffer )( gs_resource( gs_shader ), const char* uniform_name );
	gfx->construct_shader 						= &opengl_construct_shader;
	gfx->construct_uniform 						= &opengl_construct_uniform;
	gfx->construct_command_buffer 				= &opengl_construct_command_buffer;
	gfx->construct_vertex_attribute_layout_desc = &opengl_construct_vertex_attribute_layout_desc;
	gfx->construct_vertex_buffer 				= &opengl_construct_vertex_buffer;

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
	gfx->add_vertex_attribute = &opengl_add_vertex_attribute;

	return gfx;
}
