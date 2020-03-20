#include "gs.h"

// Global variables
_global gs_resource( gs_command_buffer ) 				g_cb = {0};
_global gs_resource( gs_vertex_buffer ) 				g_vb = {0};
_global gs_resource( gs_shader ) 						g_shader = {0};
_global gs_resource( gs_vertex_attribute_layout_desc )	g_vdesc = {0};

f32 vert_data[] = 
{
    // positions         // colors
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top
};

const char* vert_src = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 aColor;\n\
out vec3 outColor;\n\
void main()\n\
{\n\
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
    outColor = aColor;\n\
}";

const char* frag_src ="#version 330 core\n\
uniform float u_time;\n\
out vec4 FragColor;\n\
in vec3 outColor;\n\
void main()\n\
{\n\
    FragColor = vec4(outColor, 1.f);\n\
}";

void render_scene()
{
	// Get instance of our graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Get platform time
	f32 t = gs_engine_instance()->ctx.platform->elapsed_time() * 0.0001f;

	// Set clear color and clear screen buffer
	f32 color[] = { 0.2f, 0.3f, 0.3f, 1.0f };
	gfx->set_view_clear( g_cb, (f32*)&color );

	// Bind shader
	gfx->bind_shader( g_cb, g_shader );

	// Bind our vertex buffer
	gfx->bind_vertex_buffer( g_cb, g_vb );

	// Draw our triangle
	gfx->draw( g_cb, 0, 3 );

	// Submit command buffer to graphics api for rendering later in frame
	gfx->submit_command_buffer( g_cb );
}

gs_result app_init()
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Construct command buffer for pass
	g_cb = gfx->construct_command_buffer();

	// Construct shader
	g_shader = gfx->construct_shader( vert_src, frag_src );

	// Construct vertex layout decl
	g_vdesc = gfx->construct_vertex_attribute_layout_desc();
	gfx->add_vertex_attribute( g_vdesc, gs_vertex_attribute_float3 );
	gfx->add_vertex_attribute( g_vdesc, gs_vertex_attribute_float3 );

	// Construct vertex buffer using our layout
	g_vb = gfx->construct_vertex_buffer( g_vdesc, (void*)vert_data, sizeof( vert_data ) );

	return gs_result_success;
}

gs_result app_update()
{
	// Render scene
	render_scene();
	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	return gs_result_success;
}

int main( int argc, char** argv ) 
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size and name as well as update, initialization, and shutdown functions to be run. 
	// Later on, it'll include descriptions about plugins as well.
	gs_application_desc app = {0};
	app.window_title 		= "Hello Gunslinger";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;

	// Construct internal instance of our engine
	gs_engine* engine = gs_engine_construct( app );

	// Run the engine loop
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;
}

