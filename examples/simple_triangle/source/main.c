#include <gs.h>

// Globals
_global gs_command_buffer_t g_cb = {0};
_global gs_vertex_buffer_t g_vbo = {0};
_global gs_shader_t g_shader = {0};
_global gs_uniform_t u_color = {0}; 

const char* v_src = "\n"
"#version 330 core\n"
"layout( location = 0 ) in vec3 a_pos;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(a_pos, 1.0);\n"
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

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "Simple Triangle";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;

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
	g_cb = gs_command_buffer_new();

	// Construct shader from our source above
	g_shader = gfx->construct_shader( v_src, f_src );

	// Construct uniform for shader
	u_color = gfx->construct_uniform( g_shader, "u_color", gs_uniform_type_vec4 );

	// Vertex data layout for our mesh (for this shader, it's a single float3 attribute for position)
	gs_vertex_attribute_type layout[] = {
		gs_vertex_attribute_float3
	};

	// Vertex data for triangle
	f32 v_data[] = {
		0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f, 
		0.5f, -0.5f, 0.0f
	};

	// Construct vertex buffer
	g_vbo = gfx->construct_vertex_buffer( layout, sizeof(layout), v_data, sizeof(v_data) );

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if ( engine->ctx.platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	/*===============
	// Render scene
	================*/

	/*
	Note: 
		You'll notice that 'g_cb', our command buffer handle, is the first argument for every one of these api calls.
		All graphics api functions for immediate rendering will require a command buffer. 
		Every call you make adds these commands into this buffer and then will be processed later on in the frame
		for drawing. This allows you to make these calls at ANY point in your code before the final rendering submission occurs.
	*/

	// Graphics api instance
	gs_graphics_i* gfx = engine->ctx.graphics;
	gs_platform_i* platform = engine->ctx.platform;
	const gs_vec2 ws = platform->window_size(platform->main_window());
	const gs_vec2 fbs = platform->frame_buffer_size(platform->main_window());
	gs_command_buffer_t* cb = &g_cb;

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( cb, clear_color );
	gfx->set_view_port( cb, fbs.x, fbs.y );	

	// Bind shader
	gfx->bind_shader( cb, g_shader );

	// Bind uniform for triangle color
	f32 tri_color[4] = { 1.f, 0.6f, 0.1f, 1.f };
	gfx->bind_uniform( cb, u_color, &tri_color );

	// Bind vertex buffer
	gfx->bind_vertex_buffer( cb, g_vbo );

	// Draw
	gfx->draw( cb, 0, 3 );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( cb );

	return gs_result_in_progress;
}


/*
	gs_graphics_immediate_mode_i* gmi = &gfx->immediate;

	// Pass in a transform and do things with stuff!
	gmi->triangle( cb, transform );

	// Need to get 3d model loading
*/