 #ifndef __GS_GRAPHICS_SPRITE_H__
#define __GS_GRAPHICS_SPRITE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "graphics/gs_graphics.h"

/*================
// Sprite
=================*/

typedef struct gs_sprite_t 
{
	gs_resource( gs_texture ) texture;
	gs_color_t color;	
	gs_vec4 dimensions;	// x, y, width, height  	( should extend this to a gs_vqs transform, or something similar for easier transfomrations )
	gs_vec4 uv;			// x, y, width, height
} gs_sprite_t;

typedef struct gs_sprite_vert_t
{
	gs_vec2 position;
	gs_vec2 uv;
	gs_color_t color;
} gs_sprite_vert_t;

s32 gs_compare_sprites( const void* _s0, const void* _s1 );

/*================
// Sprite Batch
=================*/

// A collection of verts (which will just be quads), a vertex / index buffer for rendering, and a texture 
typedef struct gs_sprite_batch_t
{
	gs_dyn_array( gs_sprite_t ) sprites;
	gs_dyn_array( gs_sprite_vert_t ) vertices;
	gs_resource( gs_vertex_buffer ) vbo;
} gs_sprite_batch_t;

gs_sprite_batch_t gs_sprite_batch_new();
void gs_sprite_batch_begin( gs_sprite_batch_t* sb );
void gs_sprite_batch_add( gs_sprite_batch_t* sb, gs_sprite_t sprite );
void gs_sprite_batch_end( gs_sprite_batch_t* sb );
void gs_sprite_batch_submit( gs_sprite_batch_t* sb, gs_resource( gs_command_buffer ) cb, gs_resource( gs_uniform ) u_tex );
void gs_sprite_batch_free( gs_sprite_batch_t* sb );

#ifdef __cplusplus
}
#endif 	// c++

#endif // gs_sprite

/*
	Quad-batch
	Material -> shader
	Custom sorting method?

	Define custom material, custom shader, custom vertex decl? 
*/