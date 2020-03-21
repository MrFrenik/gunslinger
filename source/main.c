#include "gs.h"

// Global variables
_global gs_resource( gs_command_buffer ) 				g_cb = {0};
_global gs_resource( gs_vertex_buffer ) 				g_vb = {0};
_global gs_resource( gs_shader ) 						g_shader = {0};
_global gs_resource( gs_vertex_attribute_layout_desc )	g_vdesc = {0};
_global gs_resource( gs_index_buffer ) 					g_ib = {0};
_global gs_resource( gs_texture ) 						g_tex0 = {0};
_global gs_resource( gs_uniform ) 						g_s_tex0 = {0};
_global gs_resource( gs_uniform ) 						g_proj = {0};
_global gs_resource( gs_uniform ) 						g_view = {0};
_global gs_resource( gs_uniform ) 						g_model = {0};

float vert_data[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

u32 indices[] = {  
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

gs_vec3 cubePositions[] = {
  { 0.0f,  0.0f,  0.0f}, 
  { 2.0f,  5.0f, -15.0f}, 
  {-1.5f, -2.2f, -2.5f},  
  {-3.8f, -2.0f, -12.3f},  
  { 2.4f, -0.4f, -3.5f},  
  {-1.7f,  3.0f, -7.5f},  
  { 1.3f, -2.0f, -2.5f},  
  { 1.5f,  2.0f, -2.5f}, 
  { 1.5f,  0.2f, -1.5f}, 
  {-1.3f,  1.0f, -1.5f}  
};

const char* vert_src = "#version 330 core\n\
layout (location = 0) in vec3 aPos;\n\
layout (location = 1) in vec2 aTexCoord;\n\
out vec2 texCoord;\n\
uniform mat4 u_model;\n\
uniform mat4 u_view;\n\
uniform mat4 u_proj;\n\
void main()\n\
{\n\
    gl_Position = u_proj * u_view * u_model * vec4(aPos, 1.0);\n\
    texCoord = aTexCoord;\n\
}";

const char* frag_src ="#version 330 core\n\
out vec4 FragColor;\n\
in vec2 texCoord;\n\
uniform sampler2D s_tex0;\n\
void main()\n\
{\n\
    FragColor = texture(s_tex0, texCoord);\n\
}";

void render_scene()
{
	// Get instance of our graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Get platform time
	f32 t = gs_engine_instance()->ctx.platform->elapsed_time();

	// Test depth testing enabled
	gfx->set_depth_enabled( g_cb, true );

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

	gs_mat4 model = gs_mat4_identity();
	gs_mat4 view = gs_mat4_identity();
	gs_mat4 proj = gs_mat4_identity();
	view = gs_mat4_translate((gs_vec3){0.f, 0.f, -3.f});
	proj = gs_mat4_perspective(45.f, 800.f/600.f, 0.1f, 100.f);

	gfx->bind_uniform( g_cb, g_view, &view );
	gfx->bind_uniform( g_cb, g_proj, &proj );

	// Draw a bunch of different cubies
	gs_for_range_i( sizeof(cubePositions) / sizeof(gs_vec3))
	{
		gs_vqs xform = gs_vqs_default();
		gs_quat rot = gs_quat_angle_axis( t * 0.001f, (gs_vec3){1.f, 0.f, 0.f});
		rot = gs_quat_mul_quat( rot, gs_quat_angle_axis( t * 0.001f, (gs_vec3){0.f, 1.f, 0.f}));
		rot = gs_quat_mul_quat( rot, gs_quat_angle_axis( t * 0.001f, (gs_vec3){0.f, 0.f, 1.f}));
		xform.rotation = rot;
		xform.position = cubePositions[i];
		model = gs_vqs_to_mat4( &xform );

		// Bind matrix uniform
		gfx->bind_uniform( g_cb, g_model, &model );

		// Draw our cube
		gfx->draw( g_cb, 0, 36 );
	}

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
	gfx->add_vertex_attribute( g_vdesc, gs_vertex_attribute_float2 );

	// Construct vertex buffer using our layout
	g_vb = gfx->construct_vertex_buffer( g_vdesc, (void*)vert_data, sizeof( vert_data ) );

	// Construct index buffer for index drawing
	g_ib = gfx->construct_index_buffer( (void*)indices, sizeof( indices ) );

	// Construct texture from file path
	g_tex0 = gfx->construct_texture_from_file( "iu.png" );

	// Construct uniform for texture unit to be bound
	g_s_tex0 = gfx->construct_uniform( g_shader, "s_tex0", gs_uniform_type_sampler2d );

	// Construct uniform for xform mat
	g_proj = gfx->construct_uniform( g_shader, "u_proj", gs_uniform_type_mat4 );
	g_view = gfx->construct_uniform( g_shader, "u_view", gs_uniform_type_mat4 );
	g_model = gfx->construct_uniform( g_shader, "u_model", gs_uniform_type_mat4 );

	return gs_result_success;
}

gs_result app_update()
{
	// Render scene
	render_scene();

	// typedef struct gs_platform_time 
	// {
	// 	f64 max_fps;
	// 	f64 current;
	// 	f64 previous;
	// 	f64 update;
	// 	f64 render;
	// 	f64 delta;
	// 	f64 frame;
	// } gs_platform_time;

	gs_timed_action( 10, 
	{
		gs_platform_i* platform = gs_engine_instance()->ctx.platform;
		gs_println( "frame: %.2f, render: %.2f, update: %.2f, delta: %.2f ", 
			platform->time.frame, 
			platform->time.render,
			platform->time.update, 
			platform->time.delta 
		);
	});

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
	app.window_title 		= "Hello, Gunslinger";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.window_flags 		= gs_window_flags_resizable;
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

