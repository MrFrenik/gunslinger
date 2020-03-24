#include <gs.h>

#include "sdnoise1234.h"

// Heightmap Terrain Demo
// Ripped from Sebastian Lague's demo, converted/modified to C

// Struct Defines
typedef struct model_t
{
	gs_resource( gs_vertex_buffer ) vbo;
	u32 vertex_count;
} model_t;

typedef struct color_t
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;
} color_t;

typedef struct terrain_type
{
	f32 height;
	color_t color;
} terrain_type;

typedef struct terrain_vert_data_t
{
	gs_vec3 position;
	gs_vec3 normal;
	gs_vec2 tex_coord;
} terrain_vert_data_t;

typedef struct terrain_mesh_data_packet_t
{
	f32* data;
	usize sz;
	u32 count;
} terrain_mesh_data_packet_t;

// Constants
_global f32 scale = 100.f;
_global u32 octaves = 4;
_global f32 persistence = 0.5f; 
_global f32 lacunarity = 2.f;
_global const u32 map_width = 200;
_global const u32 map_height = 200;

// Globals
_global gs_resource( gs_shader ) 			shader = {0};
_global gs_resource( gs_uniform ) 			u_noise_tex = {0};
_global gs_resource( gs_texture ) 			noise_tex = {0};
_global gs_resource( gs_command_buffer ) 	cb = {0};
_global gs_resource( gs_uniform )			u_proj = {0};
_global gs_resource( gs_uniform )			u_view = {0};
_global gs_resource( gs_uniform )			u_model = {0};
_global gs_resource( gs_uniform )			u_view_pos = {0};
_global model_t 							terrain_model = {0};

// Function Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

void render_scene();
void generate_terrain_mesh( f32* noise_data, u32 width, u32 height );

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "Terrain Demo";
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

f32* generate_noise_map( u32 width, u32 height, f32 scale, u32 octaves, f32 persistence, f32 lacunarity, f32 x_offset, f32 y_offset )
{
	f32* noise_map = gs_malloc( width * height * sizeof(f32) );
	gs_assert( noise_map );

	f32 max_noise_height = f32_min;
	f32 min_noise_height = f32_max;

	for ( s32 y = 0; y < height; y++ ) 
	{
		for (s32 x = 0; x < width; x++) 
		{
			f32 amplitude = 1.f;
			f32 frequency = 1.f;
			f32 noise_height = 0.f;

			for ( u32 i = 0; i < octaves; ++i )
			{
				f32 sample_x = ((x + x_offset) / scale) * frequency;
				f32 sample_y = ((y + y_offset) / scale) * frequency;

				f32 p_val = sdnoise2( sample_x, sample_y, NULL, NULL );
				noise_height += p_val * amplitude;

				amplitude *= persistence;
				frequency *= lacunarity;
			}

			noise_map[ y * width + x ] = noise_height;

			if ( noise_height > max_noise_height )
				max_noise_height = noise_height;
			else if ( noise_height < min_noise_height )
				min_noise_height = noise_height;
		}
	}

	// Renormalize ranges between [0.0, 1.0]
	for ( s32 y = 0; y < height; ++y )
	{
		for ( s32 x = 0; x < width; ++x )
		{
			noise_map[ y * width + x ] = gs_map_range( min_noise_height, max_noise_height, 
				0.0f, 1.f, noise_map[ y * width + x ] );
		}
	}

	return noise_map;
}

color_t* generate_color_map( f32* noise_map, u32 width, u32 height )
{
	gs_assert( noise_map );

	color_t* color_map= gs_malloc( width * height * sizeof(color_t) );
	gs_assert( color_map );

	terrain_type regions[] = {
		{0.3f, {10, 20, 150, 255}},		// Deep Water
		{0.5f, {10, 50, 250, 255}},		// Shallow Water
		{0.53f, {255, 255, 153, 255}},	// Sand/Beach
		{0.6f, {100, 170, 40, 255}},	// Grass
		{0.65f, {100, 140, 30, 255}},	// Grass2
		{0.8f, {153, 102, 10, 255}},	// Rock
		{0.85f, {51, 26, 0, 255}},		// Rock2
		{1.0f, {200, 190, 210, 255}}	// Snow
	};

	u32 num_regions = sizeof(regions) / sizeof(terrain_type);

	// We'll then create a color map from these noise values
	for ( s32 y = 0; y < height; y++ ) 
	{
		for ( s32 x = 0; x < width; x++ ) 
		{
			u32 idx = y * width + x;
			f32 p = noise_map[ idx ];
			for ( u32 i = 0; i < num_regions; ++i ) {
				if ( p <= regions[ i ].height ) {
					color_map[ idx ] = regions[ i ].color;
					break;
				}
			}
		}
	}

	return color_map;
}

terrain_mesh_data_packet_t generate_terrain_mesh_data( f32* noise_data, u32 width, u32 height )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	gs_dyn_array( gs_vec3 ) positions = gs_dyn_array_new( gs_vec3 );
	gs_dyn_array( gs_vec3 ) normals = gs_dyn_array_new( gs_vec3 );
	gs_dyn_array( gs_vec2 ) uvs = gs_dyn_array_new( gs_vec2 );
	gs_dyn_array( u32 ) tris = gs_dyn_array_new( u32 );

	// Generate triangles, calculate normals, calculate uvs
	f32 top_left_x = (f32)(width - 1) / -2.f;
	f32 top_left_z = (f32)(height - 1) / 2.f;

	// Generate mesh data
	for ( u32 y = 0; y < height; ++y )
	{
		for ( u32 x = 0; x < width; ++x )
		{
			u32 idx = y * width + x;

			// Want to define some way of being able to pass in a curve to evaluate data for this
			f32 nd = noise_data[ idx ];
			f32 mult = gs_map_range( 0.f, 1.f, 1.f, 30.f, nd );
			gs_dyn_array_push( positions, ((gs_vec3){top_left_x + x, nd * mult, top_left_z - y }) );
			gs_dyn_array_push( uvs, ((gs_vec2){ x / (f32)width, y / (f32)height }) );

			if ( x < (width - 1) && y < (height - 1) ) {
				// Add triangle 
				gs_dyn_array_push( tris, idx );
				gs_dyn_array_push( tris, idx + width );
				gs_dyn_array_push( tris, idx + width + 1 );
				// Add triangle
				gs_dyn_array_push( tris, idx + width + 1 );
				gs_dyn_array_push( tris, idx + 1 );
				gs_dyn_array_push( tris, idx );
			}
		}
	}

	// Now that we have positions, uvs, and triangles, need to calculate normals for each triangle
	// For now, just put normal as UP, cause normals are going to take more time to do
	// Go through each triangle, calculate normal
	for ( u32 i = 0; i < gs_dyn_array_size( tris ); i += 3 )
	{
		u32 idx_0 = tris[ i ];
		u32 idx_1 = tris[ i + 1 ];
		u32 idx_2 = tris[ i + 2 ];

		gs_vec3 pos_0 = positions[ idx_0 ];
		gs_vec3 pos_1 = positions[ idx_1 ];
		gs_vec3 pos_2 = positions[ idx_2 ];

		// Calculate vector a = normalize( pos_1 - pos_0 )
		gs_vec3 a = gs_vec3_norm( gs_vec3_sub( pos_1, pos_0 ) );

		// Calculate vector b = normalize( pos_2 - pos_0 )
		gs_vec3 b = gs_vec3_norm( gs_vec3_sub( pos_2, pos_0 ) );

		// Calculate normal 
		gs_vec3 n = gs_vec3_norm( gs_vec3_cross( b, a ) );

		gs_dyn_array_push( normals, n );
	}

	// Batch vertex data together
	usize vert_data_size = gs_dyn_array_size( tris ) * sizeof(terrain_vert_data_t);
	f32* vertex_data = gs_malloc( vert_data_size );

	// Have to interleave data
	u32 n_idx = 0;
	gs_for_range_i( gs_dyn_array_size( tris ) )
	{
		u32 base_idx = i * 8;
		u32 idx = tris[ i ];
		gs_vec3 pos = positions[ idx ];
		gs_vec3 norm = normals[ n_idx ];
		gs_vec2 uv = uvs[ idx ];

		vertex_data[ base_idx + 0 ] = pos.x;
		vertex_data[ base_idx + 1 ] = pos.y;
		vertex_data[ base_idx + 2 ] = pos.z;
		vertex_data[ base_idx + 3 ] = norm.x;
		vertex_data[ base_idx + 4 ] = norm.y;
		vertex_data[ base_idx + 5 ] = norm.z;
		vertex_data[ base_idx + 6 ] = uv.x;
		vertex_data[ base_idx + 7 ] = uv.y;

		if ( i % 3 == 0 ) 
			n_idx++;
	}

	terrain_mesh_data_packet_t packet;
	packet.data = vertex_data;
	packet.sz = vert_data_size;
	packet.count = gs_dyn_array_size( tris );

	// Free used memory
	gs_dyn_array_free( positions );
	gs_dyn_array_free( uvs );
	gs_dyn_array_free( normals );
	gs_dyn_array_free( tris );

	return packet;
}

gs_result app_init()
{
	// Graphics api instance
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	// Platform api instance
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	
	// Create noise map
	f32* noise_map = generate_noise_map( map_width, map_height, scale, octaves, persistence, lacunarity, 0.f, 0.f );

	// Create color map from noise
	color_t* color_map = generate_color_map( noise_map, map_width, map_height );

	// Generate terrain mesh data from noise
	terrain_mesh_data_packet_t mesh = generate_terrain_mesh_data( noise_map, map_width, map_height );

	gs_vertex_attribute_type layout[] = {
		gs_vertex_attribute_float3,
		gs_vertex_attribute_float3,
		gs_vertex_attribute_float2
	};
	u32 layout_count = sizeof(layout) / sizeof(gs_vertex_attribute_type);

	// Create mesh 
	terrain_model.vbo = gfx->construct_vertex_buffer( layout, layout_count, mesh.data, mesh.sz );
	terrain_model.vertex_count = mesh.count;

	// Make our noise texture for gpu
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.width = map_width;
	t_desc.height = map_height;
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.mipmap_filter = gs_nearest;
	t_desc.data = color_map; 

	// Construct texture
	noise_tex = gfx->construct_texture( t_desc );

	// Construct shader
	char* v_src = platform->read_file_contents( "../assets/shaders/terrain.v.glsl", "r", NULL );
	char* f_src = platform->read_file_contents( "../assets/shaders/terrain.f.glsl", "r", NULL );
	shader = gfx->construct_shader( v_src, f_src );

	// Construct uniforms
	u_noise_tex = gfx->construct_uniform( shader, "s_noise_tex", gs_uniform_type_sampler2d );
	u_proj = gfx->construct_uniform( shader, "u_proj", gs_uniform_type_mat4 );
	u_view = gfx->construct_uniform( shader, "u_view", gs_uniform_type_mat4 );
	u_model = gfx->construct_uniform( shader, "u_model", gs_uniform_type_mat4 );
	u_view_pos = gfx->construct_uniform( shader, "u_view_pos", gs_uniform_type_vec3 );

	// Construct command buffer for rendering
	cb = gfx->construct_command_buffer();

	// Free data
	gs_free( noise_map );
	gs_free( color_map );
	gs_free( mesh.data );
	gs_free( v_src );
	gs_free( f_src );

	return gs_result_success;
}

void update_terrain()
{
	static f32 t = 0.f;
	const f32 speed = 10.f;
	t += gs_engine_instance()->ctx.platform->time.delta;
	f32 _scale = (sin(t) * 0.5f + 0.5f + 50.f) * 2.f;
	f32 _persistence = (sin(t * 0.6f) * 0.5f + 0.5f);
	f32 _lacunarity = (sin(t * 0.2f) * 0.5f + 0.5f + 1.f);

	// Create noise map
	f32* noise_map = generate_noise_map( map_width, map_height, _scale, octaves, _persistence, _lacunarity, t * speed, t );

	// Create color map from noise
	color_t* color_map = generate_color_map( noise_map, map_width, map_height );

	// Make our noise texture for gpu
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.width = map_width;
	t_desc.height = map_height;
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.mipmap_filter = gs_nearest;
	t_desc.data = color_map; 

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Update texture (let's just glTexImage2d for now)...
	gfx->update_texture_data( noise_tex, t_desc );

	// Generate terrain mesh data from noise
	terrain_mesh_data_packet_t mesh = generate_terrain_mesh_data( noise_map, map_width, map_height );

	// Create mesh 
	gfx->update_vertex_buffer_data( terrain_model.vbo, mesh.data, mesh.sz );

	// Free data
	gs_free( noise_map );
	gs_free( color_map );
	gs_free( mesh.data );
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	gs_timed_action( 20, 
	{
		gs_println( "Frame: %.2f", engine->ctx.platform->time.frame );
	});

	// If we press the escape key, exit the application
	if ( engine->ctx.platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	// Want to update the data for the terrain over time (to give the effect of it scrolling)
	update_terrain();

	// Render terrain
	render_scene();

	// Otherwise, continue
	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	return gs_result_success;
}

void render_scene()
{
	// Grab graphics api instance
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( cb, clear_color );

	// Set depth flags
	gfx->set_depth_enabled( cb, true );

	// Bind shader
	gfx->bind_shader( cb, shader );

	// Bind texture
	gfx->bind_texture( cb, u_noise_tex, noise_tex, 0 );	

	static f32 t = 0.f;
	t += 0.1f * gs_engine_instance()->ctx.platform->time.delta;
	gs_mat4 model = gs_mat4_identity();
	gs_vqs xform = gs_vqs_default();
	gs_quat rot = gs_quat_angle_axis( gs_deg_to_rad(30.f), (gs_vec3){1.f, 0.f, 0.f});
	rot = gs_quat_mul_quat( rot, gs_quat_angle_axis( t, (gs_vec3){0.f, 1.f, 0.f}));
	xform.rotation = rot;
	model = gs_vqs_to_mat4( &xform );
	gs_mat4 view = gs_mat4_identity();
	gs_mat4 proj = gs_mat4_identity();
	view = gs_mat4_translate((gs_vec3){0.f, -10.f, -250.f});
	proj = gs_mat4_perspective(45.f, 800.f/600.f, 0.01f, 1000.f);

	f32 t_s = t * 10.f;

	gs_vec3 vp = (gs_vec3){0.f, -10.f, -250.f};
	gfx->bind_uniform( cb, u_view_pos, &vp );
	gfx->bind_uniform( cb, u_view, &view );
	gfx->bind_uniform( cb, u_proj, &proj );
	gfx->bind_uniform( cb, u_model, &model );

	// Bind vertex buffer of terrain
	gfx->bind_vertex_buffer( cb, terrain_model.vbo );

	// Draw
	gfx->draw( cb, 0, terrain_model.vertex_count );

	// Submit command buffer to graphics api for final render
	gfx->submit_command_buffer( cb );

}


