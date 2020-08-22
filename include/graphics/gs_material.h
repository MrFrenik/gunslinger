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

gs_hash_table_decl( u64, gs_uniform_t, gs_hash_u64, gs_hash_key_comp_std_type );

typedef struct gs_uniform_block_t
{
	gs_byte_buffer data;
	gs_hash_table( u64, u32 ) offset_lookup_table;
	gs_hash_table( u64, gs_uniform_t ) uniforms;
	u32 count;
} gs_uniform_block_t;

#define __gs_uniform_data_block_uniform_decl( T, ... )\
	typedef struct gs_uniform_data_block_##T##_t\
	{\
		__VA_ARGS__\
	} gs_uniform_data_block_##T##_t;

__gs_uniform_data_block_uniform_decl( texture_sampler, u32 data; u32 slot; );
__gs_uniform_data_block_uniform_decl( mat4, gs_mat4 data; );
__gs_uniform_data_block_uniform_decl( vec4, gs_vec4 data; );
__gs_uniform_data_block_uniform_decl( vec3, gs_vec3 data; );
__gs_uniform_data_block_uniform_decl( vec2, gs_vec2 data; );
__gs_uniform_data_block_uniform_decl( float, f32 data; );
__gs_uniform_data_block_uniform_decl( int, s32 data; );

#define gs_uniform_block_type( T )\
	gs_uniform_data_block_##T##_t

extern gs_uniform_block_t __gs_uniform_block_t_new();

extern void __gs_uniform_block_t_set_uniform( gs_uniform_block_t* u_block, gs_uniform_t uniform, const char* name, void* data, usize data_size );
extern void __gs_uniform_block_t_bind_uniforms( gs_command_buffer_t* cb, gs_uniform_block_t* u_block );

typedef struct gs_uniform_block_i
{
	gs_uniform_block_t ( *construct )();
	void ( *set_uniform )( gs_uniform_block_t* u_block, gs_uniform_t uniform, const char* name, void* data, usize data_size );
	void ( *bind_uniforms )( gs_command_buffer_t* cb, gs_uniform_block_t* u_block );
} gs_uniform_block_i;

gs_uniform_block_i __gs_uniform_block_i_new();

/*=====================
// Material
======================*/

typedef struct gs_material_t
{
	gs_shader_t shader;
	gs_uniform_block_t uniforms;
} gs_material_t;

extern gs_material_t gs_material_new( gs_shader_t shader );
extern void __gs_material_i_set_uniform( gs_material_t* mat, gs_uniform_type type, const char* name, void* data );
extern void __gs_material_i_bind_uniforms( gs_command_buffer_t* cb, gs_material_t* mat );

typedef struct gs_material_i
{
	gs_material_t ( *construct )( gs_shader_t );
	void ( *set_uniform )( gs_material_t*, gs_uniform_type, const char* name, void* data );
	void ( *bind_uniforms )( gs_command_buffer_t*, gs_material_t* );
} gs_material_i;

gs_material_i __gs_material_i_new();

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_MATERIAL_H__
