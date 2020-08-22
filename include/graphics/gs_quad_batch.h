#ifndef __GS_QUAD_BATCH_H__
#define __GS_QUAD_BATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "base/gs_engine.h"
#include "graphics/gs_graphics.h"
#include "graphics/gs_mesh.h"
#include "math/gs_math.h"
#include "serialize/gs_byte_buffer.h"

// Forward declare
struct gs_material_t;

extern const char* __gs_default_quad_batch_vertex_src();

 #define gs_quad_batch_default_vertex_src\
 	__gs_default_quad_batch_vertex_src()

extern const char* __gs_default_quad_batch_frag_src();

 #define gs_quad_batch_default_frag_src\
 	__gs_default_quad_batch_frag_src()

typedef struct gs_quad_batch_t
{
	gs_byte_buffer raw_vertex_data;
	gs_mesh_t mesh;
	struct gs_material_t* material;			// Pointer to a material instance
} gs_quad_batch_t;

/*
	typedef struct gs_quad_batch_t
	{
		gs_byte_buffer raw_vertex_data;
		gs_mesh_t mesh;	
		gs_material_t material;	
	} gs_quad_batch_t;
*/

// Default quad vertex structure
// Define your own to match custom materials
typedef struct gs_quad_batch_default_vert_t
{
	gs_vec3 position;
	gs_vec2 uv;
	gs_vec4 color;
} gs_quad_batch_default_vert_t;

typedef struct gs_quad_batch_vert_into_t
{
	gs_dyn_array( gs_vertex_attribute_type ) layout;
} gs_quad_batch_vert_info_t;

// Generic api for submitting custom data for a quad batch
/*
	Quad Batch API

	* Common functionality for quad batch
	* To define CUSTOM functionality, override specific function and information for API: 

		- gs_quad_batch_i.shader:	gs_resource( gs_shader )
			* Default shader used for quad batch material
			* Define custom vertex/fragment source and then set this shader to construct materials for batches

		- gs_quad_batch_i.vert_info: gs_quad_batch_vert_info_t
			* Holds a gs_dyn_array( gs_vertex_attribute_type ) for the vertex layout
			* Initialized by default
			* Reset this layout and then pass in custom vertex information for your custom shader and mesh layout

		- gs_quad_batch_i.add: func
			* This function requires certain parameters, and you can override this functionality with your specific data
				for adding information into the batch
					* vertex_data: void* 
						- all the data used for your add function
					* data_size: usize 
						- total size of data
			* I hope to make this part of the interface nicer in the future, but for now, this'll have to do.
*/
typedef struct gs_quad_batch_i
{
	gs_quad_batch_t ( * construct )( struct gs_material_t* );
	void ( * begin )( gs_quad_batch_t* );
	void ( * end )( gs_quad_batch_t* );
	void ( * add )( gs_quad_batch_t*, void* quad_data );
	void ( * submit )( gs_command_buffer_t*, gs_quad_batch_t* );
	void ( * free )( gs_quad_batch_t* );
	void ( * set_layout )( struct gs_quad_batch_i* api, void* layout, usize layout_size );
	void ( * set_shader )( struct gs_quad_batch_i* api, gs_shader_t );
	gs_shader_t shader;
	gs_quad_batch_vert_info_t vert_info;
} gs_quad_batch_i;

extern gs_quad_batch_t gs_quad_batch_new( struct gs_material_t* mat );
extern void __gs_quad_batch_default_begin( gs_quad_batch_t* batch );

typedef struct gs_default_quad_info_t
{
	gs_vqs transform;
	gs_vec4 uv;
	gs_vec4 color;
} gs_default_quad_info_t;

extern void __gs_quad_batch_add_raw_vert_data( gs_quad_batch_t* qb, void* data, usize data_size );
extern void __gs_quad_batch_default_add( gs_quad_batch_t* qb, void* quad_info_data );
extern void __gs_quad_batch_default_end( gs_quad_batch_t* batch );
extern void __gs_quad_batch_default_free( gs_quad_batch_t* batch );
extern void __gs_quad_batch_default_submit( gs_command_buffer_t* cb, gs_quad_batch_t* batch );
extern void __gs_quad_batch_i_set_layout( gs_quad_batch_i* api, void* layout, usize layout_size );
extern void __gs_quad_batch_i_set_shader( gs_quad_batch_i* api, gs_shader_t shader );
extern gs_quad_batch_i __gs_quad_batch_i_new();

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_QUAD_BATCH_H__
