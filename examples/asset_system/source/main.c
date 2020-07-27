#include <gs.h>

/*===================
// Useful Defines
===================*/

#define char_buffer( name, size )\
	char name[size];\
	memset(name, 0, size);

#define v4( _x, _y, _z, _w )\
	(gs_vec4){ _x, _y, _z, _w }

#define v3( _x, _y, _z )\
	(gs_vec3){ _x, _y, _z }

#define v2( _x, _y )\
	(gs_vec2){ _x, _y }

#define color( _r, _g, _b, _a )\
	(gs_color){ _r, _g, _b, _a }

/*===================
// Material
===================*/

typedef gs_resource( gs_texture ) gs_texture_resource;

#define gs_uniform_decl( T, name )\
	typedef struct gs_uniform_##name##_t\
	{\
		gs_resource( gs_uniform ) handle;\
		T value;\
		u32 slot;\
	} gs_uniform_##name##_t;

gs_uniform_decl( gs_texture_resource, texture );
gs_uniform_decl( gs_mat4, mat4 );
gs_uniform_decl( gs_vec4, vec4 );
gs_uniform_decl( gs_vec3, vec3 );
gs_uniform_decl( gs_vec2, vec2 );
gs_uniform_decl( float, float );
gs_uniform_decl( int, int );

gs_slot_array_decl( gs_uniform_texture_t );
gs_slot_array_decl( gs_uniform_mat4_t );
gs_slot_array_decl( gs_uniform_vec4_t );	
gs_slot_array_decl( gs_uniform_vec3_t );	
gs_slot_array_decl( gs_uniform_vec2_t );	
gs_slot_array_decl( gs_uniform_float_t );
gs_slot_array_decl( gs_uniform_int_t );

typedef struct gs_uniform_block_t
{
	gs_slot_array( gs_uniform_texture_t ) uniform_texture;	
	gs_slot_array( gs_uniform_mat4_t ) uniform_mat4;	
	gs_slot_array( gs_uniform_vec4_t ) uniform_vec4;	
	gs_slot_array( gs_uniform_vec3_t ) uniform_vec3;	
	gs_slot_array( gs_uniform_vec2_t ) uniform_vec2;	
	gs_slot_array( gs_uniform_float_t ) uniform_float;	
	gs_slot_array( gs_uniform_int_t ) uniform_int;	
	gs_hash_table( u64, u32 ) uniform_ids;					// -> go from name to id
} gs_uniform_block_t;

gs_uniform_block_t gs_uniform_block_new()
{
	gs_uniform_block_t block = {0};
	block.uniform_texture = gs_slot_array_new( gs_uniform_texture_t );
	block.uniform_mat4 = gs_slot_array_new( gs_uniform_mat4_t );
	block.uniform_vec4 = gs_slot_array_new( gs_uniform_vec4_t );
	block.uniform_vec3 = gs_slot_array_new( gs_uniform_vec3_t );
	block.uniform_vec2 = gs_slot_array_new( gs_uniform_vec2_t );
	block.uniform_float = gs_slot_array_new( gs_uniform_float_t );
	block.uniform_int = gs_slot_array_new( gs_uniform_int_t );
	block.uniform_ids = gs_hash_table_new( u64, u32 );

	return block;
}

// Could be something as simple as this?...
typedef struct gs_material_t
{
	gs_resource( gs_shader ) shader;
	gs_uniform_block_t uniforms;
} gs_material_t;

#define __gs_material_uniform_new( mat, __type, handle, hash, v, slot )\
	do {\
		gs_uniform_##__type##_t uniform;\
		uniform.handle = handle;\
		uniform.value = v;\
		uniform.slot = slot;\
		u32 id = gs_slot_array_insert( mat->uniforms.uniform_##__type, uniform );\
		gs_hash_table_insert( mat->uniforms.uniform_ids, hash_id, id );\
	} while (0);

#define __gs_material_uniform_set_existing( mat, __type, hash, v, slot )\
	do {\
		u32 idx = gs_hash_table_get( mat->uniforms.uniform_ids, hash_id );\
		gs_uniform_##__type##_t* uniform = &gs_slot_array_get( mat->uniforms.uniform_##__type, idx );\
		uniform->value = v;\
		uniform->slot = slot;\
	} while (0);

void __gs_material_set_uniform_internal( gs_material_t* mat, gs_uniform_type T, const char* name, void* value, u32 slot )
{
	gs_graphics_i* __gfx = gs_engine_instance()->ctx.graphics;
	u64 hash_id = gs_hash_str_64( name );

	/* Insert if not exists */
	if ( !gs_hash_table_exists( mat->uniforms.uniform_ids, hash_id ) )
	{
		/* For now, just construct uniform */
		gs_resource( gs_uniform ) handle = __gfx->construct_uniform( mat->shader, name, T );
		switch ( T )
		{
			case gs_uniform_type_sampler2d: __gs_material_uniform_new( mat, texture, handle, hash_id, *(gs_resource(gs_texture)*)value, slot ); break;
			case gs_uniform_type_mat4: 		__gs_material_uniform_new( mat, mat4, handle, hash_id, *(gs_mat4*)value, slot ); break;
			case gs_uniform_type_vec4: 		__gs_material_uniform_new( mat, vec4, handle, hash_id, *(gs_vec4*)value, slot ); break;
			case gs_uniform_type_vec3: 		__gs_material_uniform_new( mat, vec3, handle, hash_id, *(gs_vec3*)value, slot ); break;
			case gs_uniform_type_vec2: 		__gs_material_uniform_new( mat, vec2, handle, hash_id, *(gs_vec2*)value, slot ); break;
			case gs_uniform_type_float: 	__gs_material_uniform_new( mat, float, handle, hash_id, *(f32*)value, slot ); break;
			case gs_uniform_type_int: 		__gs_material_uniform_new( mat, int, handle, hash_id, *(s32*)value, slot ); break;
		}
	}
	else
	{
		switch ( T )
		{
			case gs_uniform_type_sampler2d: __gs_material_uniform_set_existing( mat, texture, hash_id, *(gs_resource(gs_texture)*)value, slot ); break;
			case gs_uniform_type_mat4: 		__gs_material_uniform_set_existing( mat, mat4, hash_id, *(gs_mat4*)value, slot ); break;
			case gs_uniform_type_vec4: 		__gs_material_uniform_set_existing( mat, vec4, hash_id, *(gs_vec4*)value, slot ); break;
			case gs_uniform_type_vec3: 		__gs_material_uniform_set_existing( mat, vec3, hash_id, *(gs_vec3*)value, slot ); break;
			case gs_uniform_type_vec2: 		__gs_material_uniform_set_existing( mat, vec2, hash_id, *(gs_vec2*)value, slot ); break;
			case gs_uniform_type_float: 	__gs_material_uniform_set_existing( mat, float, hash_id, *(f32*)value, slot ); break;
			case gs_uniform_type_int: 		__gs_material_uniform_set_existing( mat, int, hash_id, *(s32*)value, slot ); break;
		}
	}
}

#define gs_material_set_uniform( mat, uniform_type, name, value_type, value, slot )\
	do {\
		value_type __v = (value);\
		__gs_material_set_uniform_internal( mat, uniform_type, name, &__v, slot );\
	} while(0)

gs_material_t gs_material_t_new( gs_resource( gs_shader ) shader )
{
	// I'd like a way to establish uniform blocks here automatically based on the shader, but that's fine for now...
	gs_material_t mat = {0};
	mat.shader = shader;
	mat.uniforms = gs_uniform_block_new();
	return mat;
}

#define __gs_material_bind_uniforms( cb, mat, T )\
	do {\
		gs_for_range_i( gs_slot_array_size( mat->uniforms.uniform_##T ) )\
		{\
			gs_uniform_##T##_t* uniform = &mat->uniforms.uniform_##T##.data[i];\
			gfx->bind_uniform( cb, uniform->handle, &uniform->value );\
		}\
	} while(0)

void gs_material_bind( gs_resource( gs_command_buffer ) cb, gs_material_t* mat )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Bind standard uniforms
	__gs_material_bind_uniforms( cb, mat, mat4 );
	__gs_material_bind_uniforms( cb, mat, vec4 );
	__gs_material_bind_uniforms( cb, mat, vec3 );
	__gs_material_bind_uniforms( cb, mat, vec2 );
	__gs_material_bind_uniforms( cb, mat, float );
	__gs_material_bind_uniforms( cb, mat, int );

	// Bind textures
	gs_for_range_i( gs_slot_array_size( mat->uniforms.uniform_texture ) )
	{
		gs_uniform_texture_t* uniform = &mat->uniforms.uniform_texture.data[i];
		gfx->bind_texture( cb, uniform->handle, uniform->value, uniform->slot );
	}
}

/*===================
// Mesh
===================*/

typedef struct gs_mesh_t
{
	gs_resource( gs_vertex_buffer ) vbo;
	u32 vertex_count;
} gs_mesh_t;

gs_mesh_t gs_mesh_t_new( gs_vertex_attribute_type* layout_data, u32 layout_size, void* v_data, usize v_data_size )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_mesh_t mesh = {0};	
	mesh.vbo = gfx->construct_vertex_buffer( layout_data, layout_size, v_data, v_data_size );
	mesh.vertex_count = v_data_size / sizeof(f32);
	return mesh;
}

/*===================
// Renderable
===================*/

typedef struct gs_renderable_t
{
	gs_mesh_t 		mesh;					// Should be an asset handle
	gs_material_t 	material;
	gs_vqs 			transform;
} gs_renderable_t;

void gs_renderable_submit( gs_resource( gs_command_buffer ) cb, gs_renderable_t* renderable )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->bind_vertex_buffer( cb, renderable->mesh.vbo );
	gfx->draw( cb, 0, renderable->mesh.vertex_count );
}

gs_renderable_t gs_renderable_new()
{
	gs_renderable_t renderable = {0};
	renderable.transform = gs_vqs_default();
	return renderable;	
}

/*===================
// Asset Subsystem
===================*/

// Hash table declaration for u64 hashed string id to texture id
gs_hash_table_decl( u64, gs_texture_resource, gs_hash_u64, gs_hash_key_comp_std_type );

typedef struct asset_subsystem_t 
{
	gs_hash_table( u64, gs_texture_resource ) textures;
} asset_subsystem_t;

asset_subsystem_t asset_subsystem_new()
{
	asset_subsystem_t assets = {0};
	assets.textures = gs_hash_table_new( u64, gs_texture_resource );
	return assets;
}

void qualified_name_from_file( const char* file_path, char* buffer, usize sz )
{
	// Get file name
	char_buffer(file_name, 256);
	gs_util_get_file_name( file_name, sizeof(file_name), file_path );

	// Remove / from file name
	char_buffer(fnq, 256);
	gs_util_string_remove_character( file_name, fnq, sizeof(fnq), '/' );

	// Then get parent directory
	char_buffer(dir, 256);
	gs_util_get_dir_from_file( dir, sizeof(dir), file_path );

	// Need to format parent directory now (remove all dots)
	char_buffer(qd, 256);
	gs_util_string_remove_character( dir, qd, sizeof(qd), '.' );

	// Now need to replace all '/' with '.'
	char_buffer(d, 256);
	gs_util_string_replace( qd, d, sizeof(d), '/', '.' );

	// Now need to combine file name with qualified directory
	char_buffer(qual, 256);
	gs_snprintf( qual, sizeof(qual), "%s%s", d, fnq );

	// Then need to remove the first 8 characters of string (.assets.)
	gs_util_string_substring( qual, buffer, sz, 8, gs_string_length( qual ) );

	gs_println( "qualified name: %s", buffer );
}

void asset_subsystem_load_texture_from_file( asset_subsystem_t* assets, const char* file_path )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	char_buffer(qualified_name, 256);
	qualified_name_from_file( file_path, qualified_name, sizeof(qualified_name) );

	// We'll assert if the file doesn't exist
	gs_assert(platform->file_exists(file_path));

	// Load texture from file and pass into description format
	gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();
	desc.texture_wrap_s = gs_clamp_to_edge;
	desc.texture_wrap_t = gs_clamp_to_edge;
	desc.generate_mips = false;
	desc.min_filter = gs_nearest;
	desc.mag_filter = gs_nearest;

	desc.data = gfx->load_texture_data_from_file( file_path, true, desc.texture_format, &desc.width, 
										&desc.height, &desc.num_comps );

	// Construct GPU texture
	gs_resource( gs_texture ) tex = gfx->construct_texture( desc );

	// Free texture data
	gs_free(desc.data);

	// Insert texture into assets
	gs_hash_table_insert( assets->textures, gs_hash_str_64( qualified_name ), tex );
}

gs_resource( gs_texture ) asset_subsystem_get_texture( asset_subsystem_t* assets, const char* name )
{
	u64 key = gs_hash_str_64( name );
	if ( gs_hash_table_exists( assets->textures, key ) ) {

		return gs_hash_table_get( assets->textures, key );
	}

	return (gs_resource( gs_texture )){0};
}

//========================================================================

/*=============
// Quad Batch
==============*/

const char* v_qb_src = "\n"
"#version 110\n"
"in vec3 a_pos;\n"
"in vec2 a_uv;\n"
"in vec4 a_color;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_proj;\n"
"varying vec2 uv;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);\n"
"	uv = a_uv;\n"
"	color = a_color;\n"
"}";

const char* f_qb_src = "\n"
"#version 110\n"
"uniform sampler2D u_tex;"
"varying vec2 uv;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	gl_FragColor = color * texture2D(u_tex, uv);\n"
"}";

typedef struct gs_quad_batch_t
{
	gs_byte_buffer raw_vertex_data;
	gs_mesh_t mesh;
	gs_material_t material;
} gs_quad_batch_t;

// Generic api for submitting custom data for a quad batch
typedef struct gs_quad_batch_api
{
	gs_quad_batch_t ( * new )();
	void ( * begin )( gs_quad_batch_t* );
	void ( * end )( gs_quad_batch_t* );
	void ( * add )( gs_quad_batch_t*, void* vertex_data, usize data_size );
	void ( * submit )( gs_resource( gs_command_buffer ), gs_quad_batch_t*, void* submit_data );
	void ( * free )( gs_quad_batch_t* );
} gs_quad_batch_api;

// Custom Quad Batch API implementation
typedef struct quad_vert_t
{
	gs_vec3 position;
	gs_vec2 uv;
	gs_vec4 color;
} quad_vert_t;

typedef struct quad_batch_submit_data_t
{
	gs_mat4 proj_mtx;
	gs_mat4 view_mtx;
	gs_resource( gs_uniform ) u_view;
	gs_resource( gs_uniform ) u_proj;
	gs_resource( gs_uniform ) u_model;
} quad_batch_submit_data_t;

gs_quad_batch_t __gs_quad_batch_new( gs_material_t mat )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	gs_quad_batch_t qb = {0};
	qb.raw_vertex_data = gs_byte_buffer_new();
	qb.material = mat;

	// Vertex data layout for a sprite vertex
	gs_vertex_attribute_type layout[] = {

		gs_vertex_attribute_float3,		// Position
		gs_vertex_attribute_float2,		// UV
		gs_vertex_attribute_float4		// Color
	};
	// Count of our vertex attribute array
	u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

	// The data for will be empty for now
	qb.mesh.vbo = gfx->construct_vertex_buffer( layout, layout_count, NULL, 0 );

	return qb;
}

void __gs_quad_batch_begin( gs_quad_batch_t* batch )
{
	// Reset position of vertex data
	gs_byte_buffer_clear( &batch->raw_vertex_data );	
	batch->mesh.vertex_count = 0;
}

void __gs_quad_batch_add( gs_quad_batch_t* batch, void* data, usize data_size )
{
	// In here, you need to know how to read/structure your data to parse the package
	u32 vert_count = data_size / sizeof(quad_vert_t);
	gs_byte_buffer_bulk_write( &batch->raw_vertex_data, data, data_size );
	batch->mesh.vertex_count += vert_count;
}

void __gs_quad_batch_end( gs_quad_batch_t* batch )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->update_vertex_buffer_data( batch->mesh.vbo, batch->raw_vertex_data.buffer, batch->raw_vertex_data.size );
}

void __gs_quad_batch_free( gs_quad_batch_t* batch )
{
	// Free byte buffer
	gs_byte_buffer_free( &batch->raw_vertex_data );	
}

void __gs_quad_batch_submit( gs_resource( gs_command_buffer ) cb, gs_quad_batch_t* batch, void* submit_data )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Bind material shader
	gfx->bind_shader( cb, batch->material.shader );

	quad_batch_submit_data_t* sd = (quad_batch_submit_data_t*)submit_data;
	gs_mat4 model = gs_mat4_identity();
	gs_material_set_uniform( &batch->material, gs_uniform_type_mat4, "u_model", gs_mat4, model, 0 );
	gs_material_set_uniform( &batch->material, gs_uniform_type_mat4, "u_view", gs_mat4, sd->view_mtx, 0 );
	gs_material_set_uniform( &batch->material, gs_uniform_type_mat4, "u_proj", gs_mat4, sd->proj_mtx, 0 );

	// Bind material
	gs_material_bind( cb, &batch->material );

	// Bind quad batch mesh vbo
	gfx->bind_vertex_buffer( cb, batch->mesh.vbo );

	// Draw
	gfx->draw( cb, 0, batch->mesh.vertex_count );
}

gs_quad_batch_api __gs_construct_quad_batch_api()
{
	gs_quad_batch_api api = {0};
	api.begin = &__gs_quad_batch_begin;
	api.end = &__gs_quad_batch_end;
	api.add = &__gs_quad_batch_add;
	api.begin = &__gs_quad_batch_begin;
	api.new = &__gs_quad_batch_new;
	api.free = &__gs_quad_batch_free;
	api.submit = &__gs_quad_batch_submit;
	return api;
}

void gs_quad_batch_add( gs_quad_batch_api* api, gs_quad_batch_t* qb, gs_vqs transform, gs_vec4 uv, gs_vec4 color )
{
	// For now, no rotation (just position and scale)	
	gs_mat4 model = gs_vqs_to_mat4( &transform );

	gs_vec3 _tl = v3(-0.5f, -0.5f, 0.f);
	gs_vec3 _tr = v3( 0.5f, -0.5f, 0.f);
	gs_vec3 _bl = v3(-0.5f,  0.5f, 0.f);
	gs_vec3 _br = v3( 0.5f,  0.5f, 0.f);
	gs_vec4 position = {0};
	quad_vert_t tl = {0};
	quad_vert_t tr = {0};
	quad_vert_t bl = {0};
	quad_vert_t br = {0};

	// Top Left
	position = gs_mat4_mul_vec4( model, v4(_tl.x, _tl.y, _tl.z, 1.0f) );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	tl.position = v3(position.x, position.y, position.z);
	tl.uv = v2(uv.x, uv.y);
	tl.color = color;

	// Top Right
	position = gs_mat4_mul_vec4( model, v4(_tr.x, _tr.y, _tr.z, 1.0f) );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	tr.position = v3(position.x, position.y, position.z);
	tr.uv = v2(uv.z, uv.y);
	tr.color = color;

	// Bottom Left
	position = gs_mat4_mul_vec4( model, v4(_bl.x, _bl.y, _bl.z, 1.0f) );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	bl.position = v3(position.x, position.y, position.z);
	bl.uv = v2(uv.x, uv.w);
	bl.color = color;

	// Bottom Right
	position = gs_mat4_mul_vec4( model, v4(_br.x, _br.y, _br.z, 1.0f) );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	br.position = v3(position.x, position.y, position.z);
	br.uv = v2(uv.z, uv.w);
	br.color = color;

	api->add( qb, &tl, sizeof(tl) );
	api->add( qb, &br, sizeof(br) );
	api->add( qb, &bl, sizeof(bl) );
	api->add( qb, &tl, sizeof(tl) );
	api->add( qb, &tr, sizeof(tr) );
	api->add( qb, &br, sizeof(br) );
}

/*
	Quad Vertex Definition (can this be user defined)? Possibly? Just need to pass 

	Would like to be able to have a definition and utilities that could be used with custom user materials and vertex definitions
	But not sure that's a possibility

	Quad batch (single material)
	typedef struct gs_quad_batch_t
	{
		gs_material_t material;
	} gs_quad_batch_t;

	quad_batch_begin();
	quad_batch_end();
	quad_batch_submit();
*/

/*======================
// Sprite Batch Material
======================*/

const char* v_sb_src = "\n"
"#version 110\n"
"in vec2 a_pos;\n"
"in vec2 a_uv;\n"
"in vec4 a_color;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_proj;\n"
"varying vec2 uv;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	gl_Position = u_proj * u_view * u_model * vec4(a_pos, 0.0, 1.0);\n"
"	uv = a_uv;\n"
"	color = a_color;\n"
"}";

const char* f_sb_src = "\n"
"#version 110\n"
"uniform sampler2D u_tex;"
"uniform vec4 u_color;"
"varying vec2 uv;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	gl_FragColor = u_color * texture2D(u_tex, uv);\n"
"}";

// Globals
_global gs_resource( gs_vertex_buffer ) g_vbo = {0};
_global gs_resource( gs_index_buffer ) g_ibo = {0};
_global gs_resource( gs_command_buffer ) g_cb = {0};
_global gs_resource( gs_shader ) g_shader = {0};
_global gs_resource( gs_uniform ) u_model = {0};
_global gs_resource( gs_uniform ) u_view = {0};
_global gs_resource( gs_uniform ) u_proj = {0};
_global gs_resource( gs_uniform ) u_tex = {0};
_global gs_resource( gs_texture ) g_tex = {0};
_global asset_subsystem_t g_assets = {0};
_global gs_camera g_camera = {0};
_global gs_material_t g_mat0 = {0};
_global gs_material_t g_mat1 = {0};
_global gs_mesh_t g_quad = {0};
_global gs_renderable_t g_rend0 = {0};
_global gs_renderable_t g_rend1 = {0};

_global gs_quad_batch_api g_qb_api = {0};
_global gs_quad_batch_t g_qb = {0};
_global gs_resource( gs_shader ) g_qb_shader = {0};
_global gs_material_t g_qb_mat = {0};

const char* v_src = "\n"
"#version 110\n"
"in vec2 a_pos;\n"
"in vec2 a_uv;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_proj;\n"
"varying vec2 uv;\n"
"void main()\n"
"{\n"
"	gl_Position = u_proj * u_view * u_model * vec4(a_pos, 0.0, 1.0);\n"
"	uv = a_uv;\n"
"}";

const char* f_src = "\n"
"#version 110\n"
"uniform sampler2D u_tex;"
"uniform vec4 u_color;"
"varying vec2 uv;\n"
"void main()\n"
"{\n"
"	gl_FragColor = u_color * texture2D(u_tex, uv);\n"
"}";

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "AssetSystem";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;

	// Construct internal instance of our engine
	gs_engine* engine = gs_engine_construct( app );

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
		return -1;
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;	
}

// Here, we'll initialize all of our application data, which in this case is our graphics resources
gs_result app_init()
{
	// Cache instance of graphics/platform apis from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	// Construct asset subsystem
	g_assets = asset_subsystem_new();

	// Construct shader from our source above
	g_shader = gfx->construct_shader( v_src, f_src );

	// Construct uniform for shader
	u_view = gfx->construct_uniform( g_shader, "u_view", gs_uniform_type_mat4 );
	u_model = gfx->construct_uniform( g_shader, "u_model", gs_uniform_type_mat4 );
	u_proj = gfx->construct_uniform( g_shader, "u_proj", gs_uniform_type_mat4 );
	u_tex = gfx->construct_uniform( g_shader, "u_tex", gs_uniform_type_sampler2d );

	// Vertex data layout for our mesh
	gs_vertex_attribute_type layout[] = {

		gs_vertex_attribute_float2,		// Position
		gs_vertex_attribute_float2		// UV
	};

	// Count of our vertex attribute array
	u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

	// Vertex data for quad
	f32 v_data[] = 
	{
		// Positions  UVs
		-0.5f, -0.5f,  0.0f, 0.0f,	// Top Left
		 0.5f, -0.5f,  1.0f, 0.0f,	// Top Right 
		-0.5f,  0.5f,  0.0f, 1.0f,  // Bottom Left
		 0.5f,  0.5f,  1.0f, 1.0f   // Bottom Right
	};

	f32 quad_v_data[] = 
	{
		// Positions  UVs
		-0.5f, -0.5f,  0.0f, 0.0f,	// Top Left
		 0.5f,  0.5f,  1.0f, 1.0f,   // Bottom Right
		-0.5f,  0.5f,  0.0f, 1.0f,  // Bottom Left
		-0.5f, -0.5f,  0.0f, 0.0f,	// Top Left
		 0.5f, -0.5f,  1.0f, 0.0f,	// Top Right 
		 0.5f,  0.5f,  1.0f, 1.0f   // Bottom Right
	};

	u32 i_data[] = {

		0, 3, 2,	// First Triangle
		0, 1, 3		// Second Triangle
	};

	// Construct vertex and index buffers
	g_vbo = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );
	g_ibo = gfx->construct_index_buffer( i_data, sizeof(i_data) );

	// Construct quad
	g_quad = gs_mesh_t_new( layout, layout_count, quad_v_data, sizeof(quad_v_data) );

	// This is a descriptor for our texture. It includes various metrics, such as the width, height, texture format, 
	// and holds the actual uncompressed texture data for the texture. After using it for loading a raw texture 
	// from file, it's the responsibility of the user to free the data.
	// gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();

	// Get appropriate file path for our texture (depending on where the app is running from)
	const char* tfp = platform->file_exists("./assets/gs.png") ? "./assets/gs.png" : "./../assets/gs.png";
	gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist

	// Load texture from file into asset subsystem
	asset_subsystem_load_texture_from_file( &g_assets, tfp );

	tfp = platform->file_exists("./assets/mario.png") ? "./assets/mario.png" : "./../assets/mario.png";
	gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist

	asset_subsystem_load_texture_from_file( &g_assets, tfp );

	// Grab texture
	g_tex = asset_subsystem_get_texture( &g_assets, "gs" );

	// Construct camera parameters
	g_camera.transform = gs_vqs_default();
	g_camera.transform.position = (gs_vec3){0.f, 0.f, -1.f};
	g_camera.fov = 60.f;
	g_camera.near_plane = 0.1f;
	g_camera.far_plane = 1000.f;
	g_camera.ortho_scale = 2.f;
	g_camera.proj_type = gs_projection_type_orthographic;

	gs_vec4 color = v4(1.f, 0.f, 0.f, 1.f);

	// Setup materials
	g_mat0 = gs_material_t_new( g_shader );
	gs_material_set_uniform( &g_mat0, gs_uniform_type_sampler2d, "u_tex", gs_resource( gs_texture ), g_tex, 0 );
	gs_material_set_uniform( &g_mat0, gs_uniform_type_vec4, "u_color", gs_vec4, v4(1.f, 0.f, 0.f, 1.f), 0 );

	// Setup materials
	g_mat1 = gs_material_t_new( g_shader );
	gs_material_set_uniform( &g_mat1, gs_uniform_type_sampler2d, "u_tex", gs_resource( gs_texture ), asset_subsystem_get_texture( &g_assets, "mario" ), 0 );
	gs_material_set_uniform( &g_mat1, gs_uniform_type_vec4, "u_color", gs_vec4, v4(1.f, 1.f, 1.f, 1.f), 0 );

	g_rend0 = gs_renderable_new();
	g_rend0.mesh = g_quad;
	g_rend0.material = g_mat0;
	g_rend0.transform.position = v3(-2.f, 0.f, 0.f);

	g_rend1 = gs_renderable_new();
	g_rend1.mesh = g_quad;
	g_rend1.material = g_mat1;
	g_rend1.transform.position = v3(2.f, 0.f, 0.f);

	// Construct quad batch api and link up function pointers
	g_qb_api = __gs_construct_quad_batch_api();

	g_qb_shader = gfx->construct_shader( v_qb_src, f_qb_src );
	g_qb_mat = gs_material_t_new( g_qb_shader );
	gs_material_set_uniform( &g_qb_mat, gs_uniform_type_sampler2d, "u_tex", gs_resource( gs_texture ), asset_subsystem_get_texture( &g_assets, "mario" ), 0 );
	g_qb = g_qb_api.new( g_qb_mat );

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// Platform api 
	gs_platform_i* platform = engine->ctx.platform;

	// If we press the escape key, exit the application
	if ( platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	const f32 dt = platform->time.delta;
	const f32 t = platform->elapsed_time();

	/*=================
	// Camera controls
	==================*/

	if ( platform->key_down( gs_keycode_q ) ) {
		g_camera.ortho_scale += 0.1f;
	}
	if ( platform->key_down( gs_keycode_e ) ) {
		g_camera.ortho_scale -= 0.1f;
	}

	if (platform->key_down(gs_keycode_a)) {
		g_camera.transform.position.x -= 0.1f;
	}
	if (platform->key_down(gs_keycode_d)) {
		g_camera.transform.position.x += 0.1f;
	}
	if (platform->key_down(gs_keycode_w)) {
		g_camera.transform.position.y += 0.1f;
	}
	if (platform->key_down(gs_keycode_s)) {
		g_camera.transform.position.y -= 0.1f;
	}

	// Add some stuff to the quad batch
	g_qb_api.begin( &g_qb );
	{
		gs_for_range_i( 10000 )
		{
			gs_vqs xform = gs_vqs_default();
			xform.position = v3(i, 0.f, 0.f);
			gs_vec4 uv = v4(0.f, 0.f, 1.f, 1.f);
			gs_vec4 color = i % 2 == 0 ? v4(1.f, 1.f, 1.f, 1.f) : v4(1.f, 0.f, 0.f, 1.f);
			gs_quad_batch_add( &g_qb_api, &g_qb, xform, uv, color );
		}
	}
	g_qb_api.end( &g_qb );

	/*===============
	// Render scene
	================*/

	// Graphics api instance
	gs_graphics_i* gfx = engine->ctx.graphics;

	// Main window size
	gs_vec2 ws = platform->window_size( platform->main_window() );

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( g_cb, clear_color );
	gfx->set_view_port( g_cb, ws.x, ws.y );
	gfx->set_depth_enabled( g_cb, false );
	gfx->set_blend_mode( g_cb, gs_blend_mode_src_alpha, gs_blend_mode_one_minus_src_alpha );

	// Create model/view/projection matrices from camera
	gs_mat4 view_mtx = gs_camera_get_view( &g_camera );
	gs_mat4 proj_mtx = gs_camera_get_projection( &g_camera, ws.x, ws.y );
	gs_mat4 model_mtx = gs_mat4_scale((gs_vec3){1.f, 1.f, 1.f});

	quad_batch_submit_data_t sd = {0};
	sd.view_mtx = view_mtx;
	sd.proj_mtx = proj_mtx;
	sd.u_model = u_model;
	sd.u_proj = u_proj;
	sd.u_view = u_view;

	// Need to submit quad batch
	g_qb_api.submit( g_cb, &g_qb, &sd );

	// gs_resource( gs_shader ) cur_shader = g_rend0.material.shader;

	// // For each renderable in scene
	// // Bind material if necessary
	// // Bind mesh if necessary
	// // Bind material uniforms if necessary
	// // Submit renderable

	// // Sort renderables by shader program
	// // if ( cur_shader.id != g_rend0.material.shader.id ) 
	// {
	// 	model_mtx = gs_vqs_to_mat4(&g_rend0.transform);

	// 	// Bind shader
	// 	gfx->bind_shader( g_cb, g_rend0.material.shader );

	// 	// Bind matrix uniforms
	// 	gfx->bind_uniform( g_cb, u_proj, &proj_mtx );
	// 	gfx->bind_uniform( g_cb, u_view, &view_mtx );
	// 	gfx->bind_uniform( g_cb, u_model, &model_mtx );
	// }

	// // Bind uniforms for material
	// gs_material_bind( g_cb, &g_rend0.material );

	// // Submit renderable mesh
	// gs_renderable_submit( g_cb, &g_rend0 );

	// // if ( cur_shader.id != g_rend1.material.shader.id ) 
	// {
	// 	model_mtx = gs_vqs_to_mat4(&g_rend1.transform);

	// 	// Bind shader
	// 	gfx->bind_shader( g_cb, g_rend1.material.shader );

	// 	// Bind matrix uniforms
	// 	gfx->bind_uniform( g_cb, u_proj, &proj_mtx );
	// 	gfx->bind_uniform( g_cb, u_view, &view_mtx );
	// 	gfx->bind_uniform( g_cb, u_model, &model_mtx );
	// }

	// // Bind uniforms for material
	// gs_material_bind( g_cb, &g_rend1.material );

	// // Submit renderable mesh
	// gs_renderable_submit( g_cb, &g_rend1 );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb );

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}

/*
	Simple 2d game

	Sprite batch
	Asset system for textures

	// Add sprites to batch during play
	// Update renderable mesh with sprite batch mesh
*/

/*

// Keep arrays for various uniform types?

typedef gs_uniform_block_t
{
	gs_slot_array( gs_uniform_texture ) uniform_texture;	
	gs_slot_array( gs_uniform_mat4 ) uniform_mat4;	
	gs_slot_array( gs_uniform_vec4 ) uniform_vec4;	
	gs_slot_array( gs_uniform_vec3 ) uniform_vec3;	
	gs_slot_array( gs_uniform_vec2 ) uniform_vec2;	
	gs_slot_array( gs_uniform_float ) uniform_float;	
	gs_hash_table( u64, u32 ) uniform_ids;					-> go from name to id
} gs_uniform_block_t;


// Could do a byte buffer for uniform information... Uniform block could come from shader itself
typedef gs_uniform_block_t
{
	gs_hash_table( u64, u32 ) 	data_offsets;				// Used for being able to actually set the data in the buffer
	hash table for offsets into data (based on name)
	data block
	to upload all uniforms, rip through data and upload to bound shader
} gs_uniform_block_t;

material_set_uniform( name, value )\
	if (exists(hash_str_64(name))) {
		data_offset = gs_hash_table_get(data_offsets, hash_str_64(name));
		cur_position = data_block.position;
		data_block.position = data_offset;
		write( type, value , data_block );
		data_block.position = cur_position;
	}


typedef gs_material_t
{
	gs_resource( gs_shader ) 	shader;
	gs_uniform_block_t 			uniforms;
} gs_material_t;

// Need inputs for the material to bind to shader
typedef gs_material_t
{
	gs_resource( gs_shader ) shader;
	gs_hash_table( u32, gs_uniform_texture ) uniform_texture;		-> wasted space
	gs_hash_table( u32, gs_uniform_vec4 ) 	 uniform_vec4;
	gs_hash_table( u32, gs_uniform_vec3 ) 	 uniform_vec3;
	gs_hash_table( u32, gs_uniform_vec2 ) 	 uniform_vec2;
	gs_hash_table( u32, gs_uniform_float ) 	 uniform_float;
	gs_hash_table( u32, gs_uniform_mat4 ) 	 uniform_mat4;
} gs_material_t;

// Need a way to keep up with uniforms, of course
gs_shader_t
{
} gs_shader;


*/