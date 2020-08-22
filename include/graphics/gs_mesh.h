#ifndef __GS_MESH_H__
#define __GS_MESH_H__

#ifdef __cplusplus
extern "C" {
#endif

/*===================
// Mesh
===================*/

typedef struct gs_mesh_t
{
	gs_vertex_buffer_t vbo;
	u32 vertex_count;
} gs_mesh_t;

_force_inline
gs_mesh_t gs_mesh_t_new( gs_vertex_attribute_type* layout_data, usize layout_size, void* v_data, usize v_data_size )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_mesh_t mesh = {0};	
	mesh.vbo = gfx->construct_vertex_buffer( layout_data, layout_size, v_data, v_data_size );
	mesh.vertex_count = v_data_size / sizeof(f32);
	return mesh;
}

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_MESH_H__

