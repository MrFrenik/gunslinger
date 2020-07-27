#include <gs.h>

/*
	Simple Sprite Batch - 
*/

/*===================
// Useful Defines
===================*/

#define color( _r, _g, _b, _a )\
	(gs_color_t){ _r, _g, _b, _a }

#define v2( _x, _y )\
	(gs_vec2){ _x, _y }

#define v3( _x, _y, _z )\
	(gs_vec3){ _x, _y, _z }

#define v4( _x, _y, _z, _w )\
	(gs_vec4){ _x, _y, _z, _w }

#define char_buffer( name, size )\
	char name[size];\
	memset(name, 0, size);

s32 random_val( s32 lower, s32 upper )
{
	if ( upper < lower ) {
		s32 tmp = lower;
		lower = upper;
		upper = tmp;
	}
	return ( rand() % (upper - lower + 1) + lower );
}

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
_global gs_sprite_batch_t g_sprite_batch = {0};
_global gs_camera g_camera = {0};

// Let's try to do a SDF shader

const char* v_src = "\n"
"#version 110\n"
"in vec2 a_pos;\n"
"in vec2 a_uv;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_proj;\n"
"varying vec2 _uv;\n"
"void main()\n"
"{\n"
"	gl_Position = u_proj * u_view * u_model * vec4(a_pos, 0.0, 1.0);\n"
"	_uv = a_uv;\n"
"}";

const char* f_src = "\n"
"#version 110\n"
"vec3 rgb(float r, float g, float b) {\n"
"	return vec3(r / 255.0, g / 255.0, b / 255.0);\n"
"}\n"
"float circle(vec2 uv, vec2 pos, float rad) {\n"
"	float d = length(pos - uv) - rad;\n"
"	float t = clamp(d, 0.0, 1.0);\n"
"	return t;\n"
"}\n"
"varying vec2 _uv;\n"
"vec2 resolution = vec2(800.0, 600.0);"
"void main()\n"
"{\n"
"	vec2 uv = gl_FragCoord.xy;\n"
"	vec2 center = resolution.xy * 0.5;\n"
"	float radius = 0.25 * resolution.y;\n"
"	vec4 bg = vec4(rgb(210.0, 222.0, 228.0), 1.0);\n"
"	float cir = circle(uv, center, radius);\n"
"	gl_FragColor = mix(bg, vec4(1.0, 0.0, 0.0, 1.0 - cir), 1.0 - cir);\n"
"}";

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "SDF";
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

	g_sprite_batch = gs_sprite_batch_new();

	// Construct shader from our source above
	g_shader = gfx->construct_shader( v_src, f_src );

	// Construct uniform for shader
	u_view = gfx->construct_uniform( g_shader, "u_view", gs_uniform_type_mat4 );
	u_model = gfx->construct_uniform( g_shader, "u_model", gs_uniform_type_mat4 );
	u_proj = gfx->construct_uniform( g_shader, "u_proj", gs_uniform_type_mat4 );

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

	u32 i_data[] = {

		0, 3, 2,	// First Triangle
		0, 1, 3		// Second Triangle
	};

	// Construct vertex and index buffers
	g_vbo = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );
	g_ibo = gfx->construct_index_buffer( i_data, sizeof(i_data) );

	// This is a descriptor for our texture. It includes various metrics, such as the width, height, texture format, 
	// and holds the actual uncompressed texture data for the texture. After using it for loading a raw texture 
	// from file, it's the responsibility of the user to free the data.
	gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();

	// Get appropriate file path for our texture (depending on where the app is running from)
	const char* tfp = platform->file_exists("./assets/gs.png") ? "./assets/gs.png" : "./../assets/gs.png";
	gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist

	// Load texture from file and pass into description format
	desc.data = gfx->load_texture_data_from_file( tfp, true, desc.texture_format, &desc.width, 
										&desc.height, &desc.num_comps );

	// Assert that our texture data is valid (it should be)
	gs_assert(desc.data != NULL);

	// Now we can pass this descriptor to our graphics api to construct our GPU texture
	g_tex = gfx->construct_texture( desc );

	// We can now safely release the memory for our descriptor
	gs_free( desc.data );
	desc.data = NULL;

	// Construct camera parameters
	g_camera.transform = gs_vqs_default();
	g_camera.transform.position = (gs_vec3){0.f, 0.f, -1.f};
	g_camera.fov = 60.f;
	g_camera.near_plane = 0.1f;
	g_camera.far_plane = 1000.f;
	g_camera.ortho_scale = 0.001f;
	g_camera.proj_type = gs_projection_type_orthographic;

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

	// if ( platform->key_down( gs_keycode_q ) ) {
	// 	g_camera.ortho_scale += 0.1f;
	// }
	// if ( platform->key_down( gs_keycode_e ) ) {
	// 	g_camera.ortho_scale -= 0.1f;
	// }

/*
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
	*/

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

	// Bind shader
	gfx->bind_shader( g_cb, g_shader );

	// Create model/view/projection matrices from camera
	gs_mat4 view_mtx = gs_camera_get_view( &g_camera );
	gs_mat4 proj_mtx = gs_camera_get_projection( &g_camera, ws.x, ws.y );
	gs_mat4 model_mtx = gs_mat4_scale((gs_vec3){1.f, 1.f, 1.f});

	// Bind matrix uniforms
	gfx->bind_uniform( g_cb, u_proj, &proj_mtx );
	gfx->bind_uniform( g_cb, u_view, &view_mtx );
	gfx->bind_uniform( g_cb, u_model, &model_mtx );

	// Bind buffers
	gfx->bind_vertex_buffer( g_cb, g_vbo );
	gfx->bind_index_buffer( g_cb, g_ibo );

	// Draw
	gfx->draw_indexed( g_cb, 6, 0 );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb );

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}
