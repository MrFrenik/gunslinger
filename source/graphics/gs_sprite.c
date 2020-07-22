#include "graphics/gs_sprite.h"
#include "base/gs_engine.h"

s32 gs_compare_sprites( const void* _s0, const void* _s1 )
{
	gs_sprite_t* s0 = (gs_sprite_t*)_s0;
	gs_sprite_t* s1 = (gs_sprite_t*)_s1;

	// Compare textures
	return ( s0->texture.id - s1->texture.id );
}

gs_sprite_batch_t gs_sprite_batch_new()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	gs_sprite_batch_t sb = {0};
	sb.sprites = gs_dyn_array_new( gs_sprite_t );
	sb.vertices = gs_dyn_array_new( gs_sprite_vert_t );

	// Vertex data layout for a sprite vertex
	gs_vertex_attribute_type layout[] = {

		gs_vertex_attribute_float2,		// Position
		gs_vertex_attribute_float2,		// UV
		gs_vertex_attribute_float4		// Color
	};
	// Count of our vertex attribute array
	u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

	// The data for will be empty for now
	sb.vbo = gfx->construct_vertex_buffer( layout, layout_count, NULL, 0 );

	return sb;
}

void gs_sprite_batch_begin( gs_sprite_batch_t* sb )
{
	// Clear all previous data
	gs_dyn_array_clear( sb->sprites );
	gs_dyn_array_clear( sb->vertices );
}

void gs_sprite_batch_add( gs_sprite_batch_t* sb, gs_sprite_t sprite )
{
	gs_dyn_array_push( sb->sprites, sprite );
}

void gs_sprite_batch_end( gs_sprite_batch_t* sb )
{
	if (gs_dyn_array_empty( sb->sprites ) ) {
		return;
	}

	// Sort all sprites by their texture, fill out vertex data for rendering
	qsort( sb->sprites, gs_dyn_array_size( sb->sprites ), sizeof( gs_sprite_t ), gs_compare_sprites );	
}

void gs_sprite_vertices( gs_sprite_t* s, gs_sprite_vert_t* tl, gs_sprite_vert_t* tr, gs_sprite_vert_t* bl, gs_sprite_vert_t* br )
{
	*tl = (gs_sprite_vert_t) 
	{
		(gs_vec2){ s->dimensions.x, s->dimensions.y },
		(gs_vec2){ s->uv.x, s->uv.y },
		s->color
	};

	*tr = (gs_sprite_vert_t)
	{
		(gs_vec2){ s->dimensions.x + s->dimensions.z, s->dimensions.y },
		(gs_vec2){ s->uv.x + s->uv.z, s->uv.y },
		s->color
	};

	*bl = (gs_sprite_vert_t)
	{
		(gs_vec2){ s->dimensions.x, s->dimensions.y + s->dimensions.w },
		(gs_vec2){ s->uv.x, s->uv.y + s->uv.w },
		s->color
	};

	*br = (gs_sprite_vert_t)
	{
		(gs_vec2){ s->dimensions.x + s->dimensions.z, s->dimensions.y + s->dimensions.w },
		(gs_vec2){ s->uv.x + s->uv.z, s->uv.y + s->uv.w },
		s->color
	};
}

void gs_sprite_batch_submit( gs_sprite_batch_t* sb, 
	gs_resource( gs_command_buffer ) cb, gs_resource( gs_uniform ) u_tex )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Rip through sprites, continue to add until you find a texture switch. Then draw what you have currently,
	// clear the buffer, continue to add. Rinse, repeat until empty.

	if ( gs_dyn_array_empty( sb->sprites ) ) {
		return;
	}

	// Add vertices for first sprite
	gs_sprite_t* s = &sb->sprites[ 0 ];

	// Cache current texture
	gs_resource( gs_texture ) cur_tex = s->texture; 

	// Verts to construct
	gs_sprite_vert_t tl = {0}, tr = {0}, bl = {0}, br = {0};

	// Submit to command buffer
	gfx->bind_texture( cb, u_tex, cur_tex, 0 );

	for ( u32 i = 0; i < gs_dyn_array_size( sb->sprites ); ++i )
	{
		s = &sb->sprites[ i ];	

		// Need to render now
		if ( s->texture.id != cur_tex.id )
		{
			// Update vertex buffer and render
			u32 vert_size = sizeof(gs_sprite_vert_t) * gs_dyn_array_size(sb->vertices);
			gfx->update_vertex_data( cb, sb->vbo, sb->vertices, vert_size );
			gfx->bind_vertex_buffer( cb, sb->vbo );
			gfx->draw( cb, 0, gs_dyn_array_size(sb->vertices) );

			// Flush vertices
			gs_dyn_array_clear( sb->vertices );

			// Set current texture to this texture
			cur_tex = s->texture;

			// Bind texture
			gfx->bind_texture( cb, u_tex, cur_tex, 0 );
		}

		// Add vertices for this sprite to vertex buffer
		gs_sprite_vertices( s, &tl, &tr, &bl, &br );
		
		gs_dyn_array_push( sb->vertices, tl );
		gs_dyn_array_push( sb->vertices, bl );
		gs_dyn_array_push( sb->vertices, br );
		gs_dyn_array_push( sb->vertices, br );
		gs_dyn_array_push( sb->vertices, tr );
		gs_dyn_array_push( sb->vertices, tl );
	}

	// Final render
	usize vert_size = sizeof(gs_sprite_vert_t) * gs_dyn_array_size(sb->vertices);
	gfx->update_vertex_data( cb, sb->vbo, sb->vertices, vert_size );
	gfx->bind_vertex_buffer( cb, sb->vbo );
	gfx->draw( cb, 0, gs_dyn_array_size(sb->vertices) );
	gs_dyn_array_clear( sb->vertices );
}

void gs_sprite_batch_free( gs_sprite_batch_t* sb )
{
	gs_dyn_array_free( sb->sprites );
	gs_dyn_array_free( sb->vertices );
}
