#include <gs.h>

// Globals
_global gs_resource( gs_vertex_buffer ) g_vbo = {0};
_global gs_resource( gs_command_buffer ) g_cb = {0};
_global gs_resource( gs_shader ) g_shader = {0};
_global gs_resource( gs_uniform ) u_color = {0}; 
_global gs_resource( gs_uniform ) u_model = {0};
_global gs_resource( gs_uniform ) u_view = {0};
_global gs_resource( gs_uniform ) u_proj = {0};

const char* v_src = "\n"
"#version 330 core\n"
"layout( location = 0 ) in vec3 a_pos;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_proj;\n"
"void main()\n"
"{\n"
"	gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"uniform vec4 u_color;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"	frag_color = u_color;\n"
"}";

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "Simple Cube";
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
	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	// Construct shader from our source above
	g_shader = gfx->construct_shader( v_src, f_src );

	// Construct uniform for shader
	u_color = gfx->construct_uniform( g_shader, "u_color", gs_uniform_type_vec4 );
	u_view = gfx->construct_uniform( g_shader, "u_view", gs_uniform_type_mat4 );
	u_model = gfx->construct_uniform( g_shader, "u_model", gs_uniform_type_mat4 );
	u_proj = gfx->construct_uniform( g_shader, "u_proj", gs_uniform_type_mat4 );

	// Vertex data layout for our mesh (for this shader, it's a single float3 attribute for position)
	gs_vertex_attribute_type layout[] = 
	{
		gs_vertex_attribute_float3
	};

	// Count of our vertex attribute array
	u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

	// Vertex data for cube
	float v_data[] = {
		// Position
        -0.5f, -0.5f, -0.5f,		// First face
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,		// Second face
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,		// Third face
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,		// Fourth face
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,		// Fifth face
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,		// Sixth face
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };
	// Construct vertex buffer
	g_vbo = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );

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

	/*===============
	// Render scene
	================*/

	// Graphics api instance
	gs_graphics_i* gfx = engine->ctx.graphics;

	// Main window size
	gs_vec2 ws = platform->window_size( platform->main_window() );
	gs_vec2 fbs = platform->frame_buffer_size( platform->main_window() );

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( g_cb, clear_color );
	gfx->set_view_port( g_cb, fbs.x, fbs.y );
	gfx->set_depth_enabled( g_cb, true );
	gfx->set_face_culling( g_cb, gs_face_culling_disabled );

	// Bind shader
	gfx->bind_shader( g_cb, g_shader );

	// Bind uniform for triangle color
	f32 color[4] = { sin(t * 0.001f), cos(t * 0.002f), 0.1f, 1.f };
	gfx->bind_uniform( g_cb, u_color, &color );

	// Create model/view/projection matrices
	gs_mat4 view_mtx = gs_mat4_mul(gs_mat4_rotate(15.f, (gs_vec3){1.f, 0.f, 0.f}),
									gs_mat4_translate((gs_vec3){0.f, -0.9f, -2.5f}));
	gs_mat4 proj_mtx = gs_mat4_perspective(60.f, ws.x / ws.y, 0.01f, 1000.f);
	gs_mat4 model_mtx = gs_mat4_rotate(t * 0.01f, (gs_vec3){0.f, 1.f, 0.f});

	// Bind matrix uniforms
	gfx->bind_uniform( g_cb, u_proj, &proj_mtx );
	gfx->bind_uniform( g_cb, u_view, &view_mtx );
	gfx->bind_uniform( g_cb, u_model, &model_mtx );

	// Bind vertex buffer
	gfx->bind_vertex_buffer( g_cb, g_vbo );

	// Draw
	gfx->draw( g_cb, 0, 36 );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb );

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}
