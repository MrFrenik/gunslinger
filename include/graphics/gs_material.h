#ifndef __GS_MATERIAL_H__
#define __GS_MATERIAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "base/gs_engine.h"
#include "serialize/gs_byte_buffer.h"
#include "graphics/gs_graphics.h"

/*=====================
// Uniform Block
======================*/

typedef struct gs_uniform_block_t
{
	gs_byte_buffer data;
	gs_hash_table( u64, u32 ) offset_lookup_table;
	u32 count;
} gs_uniform_block_t;

#define __gs_uniform_data_block_uniform_decl( T, ... )\
	typedef struct gs_uniform_data_block_##T##_t\
	{\
		__VA_ARGS__\
	} gs_uniform_data_block_##T##_t;

__gs_uniform_data_block_uniform_decl( texture_sampler, gs_resource( gs_texture ) data; u32 slot; );
__gs_uniform_data_block_uniform_decl( mat4, gs_mat4 data; );
__gs_uniform_data_block_uniform_decl( vec4, gs_vec4 data; );
__gs_uniform_data_block_uniform_decl( vec3, gs_vec3 data; );
__gs_uniform_data_block_uniform_decl( vec2, gs_vec2 data; );
__gs_uniform_data_block_uniform_decl( float, f32 data; );
__gs_uniform_data_block_uniform_decl( int, s32 data; );

#define gs_uniform_block_type( T )\
	gs_uniform_data_block_##T##_t

_force_inline
gs_uniform_block_t __gs_uniform_block_t_new()
{
	gs_uniform_block_t ub = {0};
	ub.data = gs_byte_buffer_new();
	ub.offset_lookup_table = gs_hash_table_new(u64, u32);
	ub.count = 0;
	return ub;
}

_force_inline
void __gs_uniform_block_t_set_uniform( gs_uniform_block_t* u_block, gs_resource( gs_uniform ) uniform, const char* name, void* data, usize data_size )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Check for offset based on name, if doesn't exist create it and pass
	u64 hash_id = gs_hash_str_64( name );	
	b32 exists = true;

	if ( !gs_hash_table_exists( u_block->offset_lookup_table, hash_id ) )	
	{
		exists = false;

		// Seek to end of buffer
		gs_byte_buffer_seek_to_end( &u_block->data );

		// Insert buffer position into offset table (which should be end)
		gs_hash_table_insert( u_block->offset_lookup_table, hash_id, u_block->data.position );

		u_block->count++;
	}

	// Otherwise, we're going to overwrite existing data	
	u32 offset = gs_hash_table_get( u_block->offset_lookup_table, hash_id );

	// Set buffer to offset position
	u_block->data.position = offset;

	// Write type into data
	gs_byte_buffer_write( &u_block->data, gs_resource( gs_uniform ), uniform );
	// Write data size
	gs_byte_buffer_write( &u_block->data, usize, data_size);
	// Write data
	gs_byte_buffer_bulk_write( &u_block->data, data, data_size );

	// Subtract sizes since we were overwriting data and not appending if already exists
	if ( exists ) 
	{
		usize total_size = sizeof(u32) + sizeof(usize) + data_size;
		u_block->data.size -= total_size;
	}

	// Seek back to end
	gs_byte_buffer_seek_to_end( &u_block->data );
}

_force_inline
void __gs_uniform_block_t_bind_uniforms( gs_resource( gs_command_buffer ) cb, gs_uniform_block_t* u_block )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Set data to beginning
	gs_byte_buffer_seek_to_beg( &u_block->data );

	// Rip through uniforms, determine size, then bind accordingly
	gs_for_range_i( u_block->count )
	{
		// Grab uniform
		gs_resource( gs_uniform ) uniform;
		gs_byte_buffer_bulk_read( &u_block->data, &uniform, sizeof(gs_resource(gs_uniform)) );
		// Grab data size
		usize sz = gs_byte_buffer_read( &u_block->data, usize );
		// Grab type
		gs_uniform_type type = gfx->uniform_type( uniform );

		// Grab data based on type and bind
		switch ( type )
		{
			case gs_uniform_type_sampler2d:
			{
				gs_uniform_block_type( texture_sampler ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_texture( cb, uniform, value.data, value.slot );
			} break;

			case gs_uniform_type_mat4:
			{
				gs_uniform_block_type( mat4 ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_uniform( cb, uniform, &value.data );
			} break;

			case gs_uniform_type_vec4:
			{
				gs_uniform_block_type( vec4 ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_uniform( cb, uniform, &value.data );
			} break;

			case gs_uniform_type_vec3:
			{
				gs_uniform_block_type( vec3 ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_uniform( cb, uniform, &value.data );
			} break;

			case gs_uniform_type_vec2:
			{
				gs_uniform_block_type( vec2 ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_uniform( cb, uniform, &value.data );
			} break;

			case gs_uniform_type_float:
			{
				gs_uniform_block_type( float ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_uniform( cb, uniform, &value.data );
			} break;

			case gs_uniform_type_int:
			{
				gs_uniform_block_type( int ) value = {0};
				gs_byte_buffer_bulk_read( &u_block->data, &value, sz );
				gfx->bind_uniform( cb, uniform, &value.data );
			} break;
 		}
	}
}

typedef struct gs_uniform_block_i
{
	gs_uniform_block_t ( *new )();
	void ( *set_uniform )( gs_uniform_block_t* u_block, gs_resource( gs_uniform ), const char* name, void* data, usize data_size );
	void ( *bind_uniforms )( gs_resource( gs_command_buffer ) cb, gs_uniform_block_t* u_block );
} gs_uniform_block_i;

_force_inline
gs_uniform_block_i __gs_uniform_block_i_new()
{
	gs_uniform_block_i api = {0};
	api.new = &__gs_uniform_block_t_new;
	api.set_uniform = &__gs_uniform_block_t_set_uniform;
	api.bind_uniforms = &__gs_uniform_block_t_bind_uniforms;
	return api;	
}

/*=====================
// Material
======================*/

typedef struct gs_material_t
{
	gs_resource( gs_shader ) shader;
	gs_uniform_block_t uniforms;
} gs_material_t;

_force_inline
 gs_material_t __gs_material_t_new( gs_resource( gs_shader ) shader )
{
	gs_material_t mat = {0};
	mat.shader = shader;
	mat.uniforms = gs_engine_instance()->ctx.graphics->uniform_i->new();
	return mat;
}

_force_inline
void __gs_material_i_set_uniform( gs_material_t* mat, gs_uniform_type type, const char* name, void* data )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_uniform_block_i* uapi = gfx->uniform_i;

	// Construct or get existing uniform
	gs_resource( gs_uniform ) uniform = gfx->construct_uniform( mat->shader, name, type );

	usize sz = 0;
	switch ( type )
	{
		case gs_uniform_type_mat4: sz = sizeof(gs_uniform_block_type( mat4 )); break;
		case gs_uniform_type_vec4: sz = sizeof(gs_uniform_block_type( vec4 )); break;
		case gs_uniform_type_vec3: sz = sizeof(gs_uniform_block_type( vec3 )); break;
		case gs_uniform_type_vec2: sz = sizeof(gs_uniform_block_type( vec2 )); break;
		case gs_uniform_type_float: sz = sizeof(gs_uniform_block_type( float )); break;
		case gs_uniform_type_int: sz = sizeof(gs_uniform_block_type( int )); break;
		case gs_uniform_type_sampler2d: sz = sizeof(gs_uniform_block_type( texture_sampler )); break;
	};

	uapi->set_uniform( &mat->uniforms, uniform, name, data, sz );
}

_force_inline
void __gs_material_i_bind_uniforms( gs_resource( gs_command_buffer ) cb, gs_material_t* mat )
{
	gs_engine_instance()->ctx.graphics->uniform_i->bind_uniforms( cb, &mat->uniforms );
}

typedef struct gs_material_i
{
	gs_material_t ( *new )( gs_resource( gs_shader ) );
	void ( *set_uniform )( gs_material_t*, gs_uniform_type, const char* name, void* data );
	void ( *bind_uniforms )( gs_resource( gs_command_buffer ), gs_material_t* );
} gs_material_i;

_force_inline
gs_material_i __gs_material_i_new()
{
	gs_material_i api = {0};
	api.new = &__gs_material_t_new;
	api.set_uniform = &__gs_material_i_set_uniform;
	api.bind_uniforms = &__gs_material_i_bind_uniforms;
	return api;
}

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_MATERIAL_H__
