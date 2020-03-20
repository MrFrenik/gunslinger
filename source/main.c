#include "gs.h"

// Global variables
_global gs_resource( gs_command_buffer ) 				g_cb = {0};
_global gs_resource( gs_vertex_buffer ) 				g_vb = {0};
_global gs_resource( gs_shader ) 						g_shader = {0};
_global gs_resource( gs_vertex_attribute_layout_desc )	g_vdesc = {0};
_global gs_resource( gs_index_buffer ) 					g_ib = {0};
_global gs_resource( gs_texture ) 						g_tex0 = {0};
_global gs_resource( gs_texture ) 						g_tex1 = {0};
_global gs_resource( gs_texture ) 						g_tex2 = {0};
_global gs_resource( gs_uniform ) 						g_s_tex0 = {0};
_global gs_resource( gs_uniform ) 						g_s_tex1 = {0};
_global gs_resource( gs_uniform ) 						g_s_tex2 = {0};

f32 vert_data[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
};

u32 indices[] = {  
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

const char* vert_src = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 aColor;\n\
layout (location = 2) in vec2 aTexCoord;\n\
out vec3 outColor;\n\
out vec2 texCoord;\n\
void main()\n\
{\n\
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
    outColor = aColor;\n\
    texCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);\n\
}";

const char* frag_src ="#version 330 core\n\
uniform float u_time;\n\
out vec4 FragColor;\n\
in vec3 outColor;\n\
in vec2 texCoord;\n\
uniform sampler2D s_tex0;\n\
uniform sampler2D s_tex1;\n\
uniform sampler2D s_tex2;\n\
void main()\n\
{\n\
    vec4 s0 = texture(s_tex0, texCoord);\n\
    vec4 s1 = texture(s_tex1, texCoord);\n\
    vec4 s2 = texture(s_tex2, texCoord);\n\
    if ( texCoord.x <= 0.3 ) {\n\
	    FragColor = s0;\n\
    } else if ( texCoord.x <= 0.6 ) {\n\
	    FragColor = s1;\n\
    } else {\n\
	    FragColor = s2;\n\
    }\n\
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

	// Bind index buffer
	gfx->bind_index_buffer( g_cb, g_ib );

	// Bind textures
	gfx->bind_texture( g_cb, g_s_tex0, g_tex0, 0 );
	gfx->bind_texture( g_cb, g_s_tex1, g_tex1, 1 );
	gfx->bind_texture( g_cb, g_s_tex2, g_tex2, 2 );

	// Draw our triangle
	gfx->draw_indexed( g_cb, 6 );

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
	gfx->add_vertex_attribute( g_vdesc, gs_vertex_attribute_float2 );

	// Construct vertex buffer using our layout
	g_vb = gfx->construct_vertex_buffer( g_vdesc, (void*)vert_data, sizeof( vert_data ) );

	// Construct index buffer for index drawing
	g_ib = gfx->construct_index_buffer( (void*)indices, sizeof( indices ) );

	// Construct texture from file path
	g_tex0 = gfx->construct_texture_from_file( "test0.png" );
	g_tex1 = gfx->construct_texture_from_file( "test1.png" );
	g_tex2 = gfx->construct_texture_from_file( "test2.jpg" );

	// Construct uniform for texture unit to be bound
	g_s_tex0 = gfx->construct_uniform( g_shader, "s_tex0", gs_uniform_type_sampler2d );
	g_s_tex1 = gfx->construct_uniform( g_shader, "s_tex1", gs_uniform_type_sampler2d );
	g_s_tex2 = gfx->construct_uniform( g_shader, "s_tex2", gs_uniform_type_sampler2d );

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

