#include "gs.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// Global scope
_global gs_resource_handle window;

// Clear screen, that's it

gs_resource( gs_command_buffer ) 				g_cb = {0};
gs_resource( gs_vertex_buffer ) 				g_vb = {0};
gs_resource( gs_shader ) 						g_shader = {0};
gs_resource( gs_vertex_attribute_layout_desc)	g_vdesc = {0};

f32 vertices[] = 
{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

const char* vert_src = "\n\
#version 410\n\
layout (location = 0) in vec3 aPos;\n\
void main()\n\
{\n\
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
}";

const char* frag_src ="\n\
#version 410\n\
out vec4 FragColor;\n\
void main()\n\
{\n\
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n\
}";

void render()
{
	// Let's just clear some color, shall we?
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Get platform time
	f32 t = gs_engine_instance()->ctx.platform->elapsed_time() * 0.0001f;

	// Clear color
	f32 color[] = { 0.f, 0.f, 0.f, 1.f };
	gfx->set_view_clear( g_cb, (f32*)&color );

	// Bind shader
	gfx->bind_shader( g_cb, g_shader );

	// Bind our vertex buffer
	gfx->bind_vertex_buffer( g_cb, g_vb );

	// Draw
	gfx->draw( g_cb, 0, 3 );

	// Submit command buffer
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

	// Construct vertex buffer using our layout
	g_vb = gfx->construct_vertex_buffer( g_vdesc, (void*)vertices, sizeof( vertices ) );

	return gs_result_success;
}

gs_result app_update()
{
	gs_engine* engine = gs_engine_instance();
	gs_platform_i* platform = engine->ctx.platform;

	// Draw shit to screen? That'd be nice, wouldn't it?
	gs_timed_action( 10, {
		gs_println( "frame: %.2f, update: %.2f, render: %.2f, mp: <%.2f, %.2f>", 
				platform->time.frame, 
				platform->time.update, 
				platform->time.render, 
				platform->input.mouse.position.x, 
				platform->input.mouse.position.y 
		);
	});

	// Render scene
	render();

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	return gs_result_success;
}

int main( int argc, char** argv ) 
{
	struct gs_application_desc app;
	app.update = &app_update;
	app.shutdown = &app_shutdown;
	app.init = &app_init;

	// Construct internal instance of our engine
	gs_engine engine;
	gs_engine_init( &engine, app );

	// Run the engine loop
	gs_result res = engine.run();

	// Run engine
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;
}

/*
	gs_application_context ctx;

	Application context could hold settings for platform, including video and audiod

	// Want a way to create render tasks, submit those to a global render queue to be sorted and executed

	For example, for a simple forward renderer to render a cube: 

		// Use handles for all graphics resources
		gs_resource_handle index_buffer = gfx->create_index_buffer( data );		// You can use these graphics structures directly
		gs_resource_handle shader = gfx->load_shader( "shader" );				// Same thing here

		- Set render target / framebuffer
		- Set clear
		- Set view matrix
		- Set projection matrix
		- Set depth flags / stencil flags / etc. ( if not default )
		- Set shader
		- Set mesh to be bound
		- Set uniforms to be bound
		- Submit pass

*/







