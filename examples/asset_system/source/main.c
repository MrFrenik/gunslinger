#include <gs.h>

// Windows sound API

/*
	What is a sound? 
	Want some internal mechanism for storing a device
	Want some method for being able to apply arbitrary sound data from an application as a sound to be played
	Want some method for being able to load sound resource files, store them internally, then be able to play those at will via resource handles

	typedef struct gs_audio_i
	{
		gs_result ( * init )( struct gs_audio_i* );
		gs_result ( * shutdown )( struct gs_audio_i* );
		gs_result ( * update )();

		gs_resource( gs_sound )( * load_sound )( const char* file_name );

		void ( * play_sound )( gs_resource( gs_sound ) );

	} gs_audio_i;

*/

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
// Asset Subsystem
===================*/

typedef gs_resource( gs_texture ) 		 gs_texture_resource;
typedef gs_resource( gs_audio_source ) gs_audio_resource;

// Hash table declaration for u64 hashed string id to texture id
gs_hash_table_decl( u64, gs_texture_resource, gs_hash_u64, gs_hash_key_comp_std_type );
gs_hash_table_decl( u64, gs_audio_resource, gs_hash_u64, gs_hash_key_comp_std_type );

typedef struct asset_subsystem_t 
{
	gs_hash_table( u64, gs_texture_resource ) textures;
	gs_hash_table( u64, gs_audio_resource ) audio;
} asset_subsystem_t;

asset_subsystem_t asset_subsystem_new()
{
	asset_subsystem_t assets = {0};
	assets.textures = gs_hash_table_new( u64, gs_texture_resource );
	assets.audio = gs_hash_table_new( u64, gs_audio_resource );
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

	u32 num_dots = qual[0] == '.' && qual[1] == '.' ? 2 : 1;

	// Then need to remove the first 8 characters of string (.assets.)
	gs_util_string_substring( qual, buffer, sz, 7 + num_dots, gs_string_length( qual ) ); 

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

void asset_subsystem_load_audio_from_file( asset_subsystem_t* assets, const char* file_path )
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	char_buffer(qualified_name, 256);
	qualified_name_from_file( file_path, qualified_name, sizeof(qualified_name) );

	// We'll assert if the file doesn't exist
	gs_assert(platform->file_exists(file_path));

	gs_resource( gs_audio_source ) source = audio->load_audio_source_from_file( file_path );

	gs_hash_table_insert( assets->audio, gs_hash_str_64( qualified_name ), source );
}

gs_resource( gs_texture ) asset_subsystem_get_texture( asset_subsystem_t* assets, const char* name )
{
	u64 key = gs_hash_str_64( name );
	if ( gs_hash_table_exists( assets->textures, key ) ) {

		return gs_hash_table_get( assets->textures, key );
	}

	return (gs_resource_invalid( gs_texture ));
}

gs_resource( gs_audio_source ) asset_subsystem_get_audio( asset_subsystem_t* assets, const char* name )
{
	u64 key = gs_hash_str_64( name );
	if ( gs_hash_table_exists( assets->audio, key ) ) {

		return gs_hash_table_get( assets->audio, key );
	}

	return (gs_resource_invalid( gs_audio_source ));
}

/*======================
======================*/

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
_global gs_resource( gs_material ) g_mat_asset = {0};
_global asset_subsystem_t g_assets = {0};
_global gs_camera g_camera = {0};
_global gs_quad_batch_t g_qb = {0};

_global gs_resource( gs_audio_instance ) g_music_inst = {0};

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
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	// Construct asset subsystem
	g_assets = asset_subsystem_new();

	// Get appropriate file path for our texture (depending on where the app is running from)
	const char* tfp = platform->file_exists("./assets/gs.png") ? "./assets/gs.png" : "./../assets/gs.png";
	gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist

	// Load texture from file into asset subsystem
	asset_subsystem_load_texture_from_file( &g_assets, tfp );

	tfp = platform->file_exists("./assets/mario.png") ? "./assets/mario.png" : "./../assets/mario.png";
	gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist

	asset_subsystem_load_texture_from_file( &g_assets, tfp );

	// Construct camera parameters
	g_camera.transform = gs_vqs_default();
	g_camera.transform.position = (gs_vec3){0.f, 0.f, -1.f};
	g_camera.fov = 60.f;
	g_camera.near_plane = 0.1f;
	g_camera.far_plane = 1000.f;
	g_camera.ortho_scale = 2.f;
	g_camera.proj_type = gs_projection_type_orthographic;

	gs_vec4 color = v4(1.f, 0.f, 0.f, 1.f);

	// Construct material for quad batch
	g_mat_asset = gfx->construct_material( gfx->quad_batch_i->shader );

	gs_uniform_block_type( texture_sampler ) sampler;
	sampler.data = asset_subsystem_get_texture( &g_assets, "mario" );
	sampler.slot = 0;
	gfx->set_material_uniform( g_mat_asset, gs_uniform_type_sampler2d, "u_tex", &sampler, sizeof(sampler) );

	// Construct quad batch api and link up function pointers
	g_qb = gfx->quad_batch_i->new( g_mat_asset );

	const char* audio_file_names[] = 
	{
		"shotgun",
		"music",	
		"jump",	
		"van_door",
	};

	// Audio
	gs_for_range_i( sizeof(audio_file_names) / sizeof(const char*) )
	{
		char_buffer(tmp, 256);
		gs_snprintf(tmp, sizeof(tmp), "./assets/%s.ogg", audio_file_names[i]);
		char_buffer(tmp2, 256);
		gs_snprintf(tmp2, sizeof(tmp), "./../assets/%s.ogg", audio_file_names[i]);
		tfp = platform->file_exists(tmp) ? tmp : tmp2;
		gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist
		asset_subsystem_load_audio_from_file( &g_assets, tfp );
	}

	// Create an audio source instance that will be persistent in memory and loop
	gs_audio_instance_data_t sound = gs_audio_instance_data_new( asset_subsystem_get_audio( &g_assets, "music") );
	sound.volume = 0.2f;
	sound.loop = true;
	sound.playing = true;
	sound.persistent = true;
	g_music_inst = audio->play( sound );

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	 // Platform API
	gs_platform_i* platform = engine->ctx.platform;
	 // Graphics API
	gs_graphics_i* gfx = engine->ctx.graphics;
	// Quad Batch API
	gs_quad_batch_i* qbi = gfx->quad_batch_i;
	gs_audio_i* audio = engine->ctx.audio;

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

	if (platform->key_pressed(gs_keycode_p))
	{
		audio->play( gs_audio_instance_data_new( asset_subsystem_get_audio( &g_assets, "shotgun") ) );
	}

	if (platform->key_pressed(gs_keycode_space))
	{
		audio->play( gs_audio_instance_data_new( asset_subsystem_get_audio( &g_assets, "jump" ) ) );
	}

	if (platform->key_pressed(gs_keycode_v))
	{
		audio->play( gs_audio_instance_data_new( asset_subsystem_get_audio( &g_assets, "van_door" ) ) );
	}

	if ( platform->key_pressed(gs_keycode_up))
	{
		f32 vol = audio->get_volume(g_music_inst);
		vol += 0.05f;
		audio->set_volume(g_music_inst, gs_min(vol, 1.f));
	}
	if ( platform->key_pressed(gs_keycode_down))
	{
		f32 vol = audio->get_volume(g_music_inst);
		vol -= 0.05f;
		audio->set_volume(g_music_inst, gs_max(vol, 0.f));
	}

	if ( platform->key_pressed( gs_keycode_u ) )
	{
		// We'll reset the audio data
		gs_audio_instance_data_t sound = gs_audio_instance_data_new( asset_subsystem_get_audio( &g_assets, "music" ) );
		sound.volume = 0.2f;
		sound.loop = true;
		sound.playing = true;
		sound.persistent = true;
		audio->set_instance_data( g_music_inst, sound );
	}

	if ( platform->key_pressed( gs_keycode_r ) )
	{
		audio->restart( g_music_inst );
	}

	/*
		So I'd rather be able to do 

			audio->play_instance( source );
	*/

	// Add some stuff to the quad batch
	qbi->begin( &g_qb );
	{
		gs_for_range_i( 100 )
			gs_for_range_j( 100 )
			{
				gs_default_quad_info_t quad_info = {0};
				quad_info.transform = gs_vqs_default();
				quad_info.transform.position = v3(i, j, 0.f);
				quad_info.uv = v4(0.f, 0.f, 1.f, 1.f);
				quad_info.color = i % 2 == 0 ? v4(1.f, 1.f, 1.f, 1.f) : v4(1.f, 0.f, 0.f, 1.f);
				qbi->add( &g_qb, &quad_info, sizeof(quad_info) );
			}
	}
	qbi->end( &g_qb );

	/*===============
	// Render scene
	================*/

	// Main window size
	gs_vec2 ws = platform->window_size( platform->main_window() );

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( g_cb, clear_color );
	gfx->set_view_port( g_cb, ws.x, ws.y );
	gfx->set_depth_enabled( g_cb, false );
	gfx->set_blend_mode( g_cb, gs_blend_mode_src_alpha, gs_blend_mode_one_minus_src_alpha );

	// Create model/view/projection matrices from camera
	gs_mat4 model = gs_mat4_identity();
	gs_mat4 view_mtx = gs_camera_get_view( &g_camera );
	gs_mat4 proj_mtx = gs_camera_get_projection( &g_camera, ws.x, ws.y );
	gs_mat4 model_mtx = gs_mat4_scale((gs_vec3){1.f, 1.f, 1.f});

	// Set uniforms for quad batch material
	gs_uniform_block_type( mat4 ) u_model;
	u_model.data = model;
	gfx->set_material_uniform( g_qb.material, gs_uniform_type_mat4, "u_model", &u_model, sizeof(u_model) );

	gs_uniform_block_type( mat4 ) u_view;
	u_view.data = view_mtx;
	gfx->set_material_uniform( g_qb.material, gs_uniform_type_mat4, "u_view", &u_view, sizeof(u_view) );

	gs_uniform_block_type( mat4 ) u_proj;
	u_proj.data = proj_mtx;
	gfx->set_material_uniform( g_qb.material, gs_uniform_type_mat4, "u_proj", &u_proj, sizeof(u_proj) );

	// Need to submit quad batch
	qbi->submit( g_cb, &g_qb );

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