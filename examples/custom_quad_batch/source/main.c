#include <gs.h>

/*
	Custom Quad Batch Example

	Quad Batch API

	* Common functionality for quad batch
	* To define CUSTOM functionality, override specific function and information for API: 

		- gs_quad_batch_i.shader:	gs_resource( gs_shader )
			* Default shader used for quad batch material
			* Define custom vertex/fragment source and then set this shader to construct materials for batches

		- gs_quad_batch_i.vert_info: gs_quad_batch_vert_info_t
			* Holds a gs_dyn_array( gs_vertex_attribute_type ) for the vertex layout
			* Initialized by default
			* Reset this layout and then pass in custom vertex information for your custom shader and mesh layout

		- gs_quad_batch_i.add: func
			* This function requires certain parameters, and you can override this functionality with your specific data
				for adding information into the batch
					* vertex_data: void* 
						- all the data used for your add function
					* data_size: usize 
						- total size of data
			* I hope to make this part of the interface nicer in the future, but for now, this'll have to do.

	For this example, we'll implement a custom material and vertex layout for our quad batch. 
	This material will be associated with a custom shader that has the following vertex information: 
		- Vec3: Position
		- Vec2: UV
		- Vec4: Color
		- Vec4: ColorTwo

	The fragment shader will blend color, colortwo and then multiply that into a passed in texture for the final fragment color.
	We'll also use a custom uniform variable for alpha, 'u_alpha', which will be controlled by the platform time to fade
		the batch in and out.
*/

/*=================================
// Custom Quad Batch Implementation
=================================*/

const char* quad_batch_custom_vert_src ="\n"
	"#version 330 core\n"
	"layout( location = 0 ) in vec3 a_pos;\n"
	"layout( location = 1 ) in vec2 a_uv;\n"
	"layout( location = 2 ) in vec4 a_color;\n"
	"layout( location = 3 ) in vec4 a_color_two;\n"
	"uniform mat4 u_model;\n"
	"uniform mat4 u_view;\n"
	"uniform mat4 u_proj;\n"
	"out vec2 uv;\n"
	"out vec4 color;\n"
	"out vec4 color_two;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);\n"
	"	uv = a_uv;\n"
	"	color = a_color;\n"
	"	color_two = a_color_two;\n"
	"}";

const char* quad_batch_custom_frag_src = "\n"
	"#version 330\n"
	"uniform sampler2D u_tex;\n"
	"uniform float u_alpha;\n"
	"in vec2 uv;\n"
	"in vec4 color;\n"
	"in vec4 color_two;\n"
	"out vec4 frag_color;\n"
	"void main()\n"
	"{\n"
	"	frag_color = vec4((mix(color_two, color, 0.5) * texture(u_tex, uv)).rgb, u_alpha);\n"
	"}";

typedef struct quad_batch_custom_vert_t
{
	gs_vec3 position;
	gs_vec2 uv;
	gs_vec4 color;
	gs_vec4 color_two;
} quad_batch_custom_vert_t;

typedef struct quad_batch_custom_info_t
{
	gs_vqs transform;
	gs_vec4 uv;
	gs_vec4 color;
	gs_vec4 color_two;
} quad_batch_custom_info_t;

void quad_batch_custom_add( gs_quad_batch_t* qb, void* quad_batch_custom_info_data )
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	quad_batch_custom_info_t* quad_info = (quad_batch_custom_info_t*)(quad_batch_custom_info_data);
	if ( !quad_info ) {
		gs_assert( false );
	}

	gs_vqs transform = quad_info->transform;
	gs_vec4 uv = quad_info->uv;
	gs_vec4 color = quad_info->color;
	gs_vec4 color_two = quad_info->color_two;

	gs_mat4 model = gs_vqs_to_mat4( &transform );

	gs_vec3 _tl = (gs_vec3){-0.5f, -0.5f, 0.f};
	gs_vec3 _tr = (gs_vec3){ 0.5f, -0.5f, 0.f};
	gs_vec3 _bl = (gs_vec3){-0.5f,  0.5f, 0.f};
	gs_vec3 _br = (gs_vec3){ 0.5f,  0.5f, 0.f};
	gs_vec4 position = {0};
	quad_batch_custom_vert_t tl = {0};
	quad_batch_custom_vert_t tr = {0};
	quad_batch_custom_vert_t bl = {0};
	quad_batch_custom_vert_t br = {0};

	// Top Left
	position = gs_mat4_mul_vec4( model, (gs_vec4){_tl.x, _tl.y, _tl.z, 1.0f} );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	tl.position = (gs_vec3){position.x, position.y, position.z};
	tl.uv = (gs_vec2){uv.x, uv.y};
	tl.color = color;
	tl.color_two = color_two;

	// Top Right
	position = gs_mat4_mul_vec4( model, (gs_vec4){_tr.x, _tr.y, _tr.z, 1.0f} );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	tr.position = (gs_vec3){position.x, position.y, position.z};
	tr.uv = (gs_vec2){uv.z, uv.y};
	tr.color = color;
	tr.color_two = color_two;

	// Bottom Left
	position = gs_mat4_mul_vec4( model, (gs_vec4){_bl.x, _bl.y, _bl.z, 1.0f} );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	bl.position = (gs_vec3){position.x, position.y, position.z};
	bl.uv = (gs_vec2){uv.x, uv.w};
	bl.color = color;
	bl.color_two = color_two;

	// Bottom Right
	position = gs_mat4_mul_vec4( model, (gs_vec4){_br.x, _br.y, _br.z, 1.0f} );
	position = gs_vec4_scale( position, 1.0f / position.w ); 
	br.position = (gs_vec3){position.x, position.y, position.z};
	br.uv = (gs_vec2){uv.z, uv.w};
	br.color = color;
	br.color_two = color_two;

	quad_batch_custom_vert_t verts[] = {
		tl, br, bl, tl, tr, br
	};

	__gs_quad_batch_add_raw_vert_data( qb, verts, sizeof(verts) );
}

/*============================
// Main Program
=============================*/

// Globals
_global gs_resource( gs_command_buffer ) g_cb = {0};
_global gs_resource( gs_texture ) g_tex = {0};
_global gs_resource( gs_shader ) g_batch_shader = {0};
_global gs_resource( gs_material ) g_batch_mat = {0};
_global gs_resource( gs_quad_batch ) g_batch = {0};
_global gs_camera g_camera = {0};

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction
void update_camera();

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "Custom Quad Batch";
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
	gs_quad_batch_i* qb = gfx->quad_batch_i;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	// Load texture from file
	g_tex = gfx->construct_texture_from_file( platform->file_exists( "./assets/gs.png" ) ? "./assets/gs.png" : "./../assets/gs.png", 
											gs_texture_parameter_desc_default() );

	// Construct camera parameters
	g_camera.transform = gs_vqs_default();
	g_camera.transform.position = (gs_vec3){0.f, 0.f, -1.f};
	g_camera.fov = 60.f;
	g_camera.near_plane = 0.1f;
	g_camera.far_plane = 1000.f;
	g_camera.ortho_scale = 2.f;
	g_camera.proj_type = gs_projection_type_orthographic;

	// Initialize quad batch API with custom implementation data
	gs_vertex_attribute_type qb_layout[] = {
		gs_vertex_attribute_float3,	// Position
		gs_vertex_attribute_float2,	// UV
		gs_vertex_attribute_float4,	// Color
		gs_vertex_attribute_float4	// Color2
	};

	g_batch_shader = gfx->construct_shader( quad_batch_custom_vert_src, quad_batch_custom_frag_src );
	qb->set_shader( qb, g_batch_shader );
	qb->set_layout( qb, qb_layout, sizeof(qb_layout) );
	qb->add = &quad_batch_custom_add;

	// Setup quad batch
	g_batch_mat = gfx->construct_material( gfx->quad_batch_i->shader );

	// Set texture uniform for material
	gfx->set_material_uniform_sampler2d( g_batch_mat, "u_tex", g_tex, 0 );

	// Construct quad batch api and link up function pointers
	g_batch = gfx->construct_quad_batch( g_batch_mat );

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// Platform api 
	gs_platform_i* platform = engine->ctx.platform;
	gs_graphics_i* gfx = engine->ctx.graphics;

	// If we press the escape key, exit the application
	if ( platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	const f32 dt = platform->time.delta;
	const f32 t = platform->elapsed_time();

	update_camera();

	// Add 10k items to batch
	gfx->quad_batch_begin( g_batch );
	{
		gs_for_range_i( 100 ) {
			gs_for_range_j( 100 )
			{
				// Instance of our custom quad batch info struct
				quad_batch_custom_info_t quad_info = {0};

				quad_info.transform = gs_vqs_default();
				quad_info.transform.position = (gs_vec3){i, j, 0.f};
				quad_info.uv = (gs_vec4){0.f, 0.f, 1.f, 1.f};
				quad_info.color = i % 2 == 0 ? (gs_vec4){1.f, 1.f, 1.f, 1.f} : (gs_vec4){1.f, 0.f, 0.f, 1.f};
				quad_info.color_two = (gs_vec4){0.f, 1.f, 0.f, 1.f};

				gfx->quad_batch_add( g_batch, &quad_info );
			}
		}
	}
	gfx->quad_batch_end( g_batch );

	/*===============
	// Render scene
	================*/

	// Main window size
	gs_vec2 ws = platform->window_size( platform->main_window() );
	gs_vec2 fbs = platform->frame_buffer_size( platform->main_window() );

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( g_cb, clear_color );
	gfx->set_view_port( g_cb, fbs.x, fbs.y );
	gfx->set_depth_enabled( g_cb, false );
	gfx->set_blend_mode( g_cb, gs_blend_mode_src_alpha, gs_blend_mode_one_minus_src_alpha );

	// Create model/view/projection matrices from camera
	gs_mat4 view_mtx = gs_camera_get_view( &g_camera );
	gs_mat4 proj_mtx = gs_camera_get_projection( &g_camera, ws.x, ws.y );
	gs_mat4 model_mtx = gs_mat4_scale((gs_vec3){1.f, 1.f, 1.f});

	// Set necessary dynamic uniforms for quad batch material (defined in default shader in gs_quad_batch.h)
	gfx->set_material_uniform_mat4( g_batch_mat, "u_model", model_mtx );
	gfx->set_material_uniform_mat4( g_batch_mat, "u_view", view_mtx );
	gfx->set_material_uniform_mat4( g_batch_mat, "u_proj", proj_mtx );
	gfx->set_material_uniform_float( g_batch_mat, "u_alpha", sin(t * 0.001f) * 0.5f + 0.5f );

	// Need to submit quad batch
	gfx->quad_batch_submit( g_cb, g_batch );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb );

	return gs_result_in_progress;
}

void update_camera()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

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
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}
