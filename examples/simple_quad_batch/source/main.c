#include <gs.h>

/*================================================================
	* Copyright: 2020 John Jackson
	* simple_quad_batch

	The purpose of this example is to demonstrate how to construct
	and rennder a simple quad batch using the default quad batch
	api provided in the graphics subsystem.

	Included: 
		* Constructing a new quad batch
		* Adding quad batch info into the batch
		* Setting up uniforms for the default quad batch material
		* Submitting the quad batch to be rendered

	Press `esc` to exit the application.
================================================================*/

// Globals
_global gs_command_buffer_t g_cb = {0};
_global gs_texture_t 		g_tex = {0};
_global gs_camera_t 		g_camera = {0};
_global gs_quad_batch_t 	g_batch = {0};

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction
void update_camera();

int main(int argc, char** argv)
{
	gs_application_desc_t app = {0};
	app.window_title 		= "Simple Quad Batch";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;

	// Construct internal instance of our engine
	gs_engine_t* engine = gs_engine_construct(app);

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if (res != gs_result_success) 
	{
		gs_println("Error: Engine did not successfully finish running.");
		return -1;
	}

	gs_println("Gunslinger exited successfully.");

	return 0;	
}

// Here, we'll initialize all of our application data, which in this case is our graphics resources
gs_result app_init()
{
	// Cache instance of graphics/platform apis from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Construct command buffer (the command buffer is used to allow for immediate drawing at any point in our program)
	g_cb = gs_command_buffer_new();

	// Get appropriate file path for our texture (depending on where the app is running from)
	const char* tfp = platform->file_exists("./assets/gs.png") ? "./assets/gs.png" : "./../assets/gs.png";
	gs_assert(platform->file_exists(tfp));	// We'll assert if the file doesn't exist

	// Now we can pass this descriptor to our graphics api to construct our GPU texture
	g_tex = gfx->construct_texture_from_file(platform->file_exists("./assets/gs.png") ? "./assets/gs.png" : "./../assets/gs.png", 
											  NULL);

	// Construct camera parameters
	g_camera.transform = gs_vqs_default();
	g_camera.transform.position = (gs_vec3){0.f, 0.f, -1.f};
	g_camera.fov = 60.f;
	g_camera.near_plane = 0.1f;
	g_camera.far_plane = 1000.f;
	g_camera.ortho_scale = 2.f;
	g_camera.proj_type = gs_projection_type_orthographic;

	// Construct quad batch api and link up function pointers
	g_batch = gs_quad_batch_new(NULL);

	// Set material texture uniform
	gfx->set_material_uniform_sampler2d(g_batch.material, "u_tex", g_tex, 0);

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine_t* engine = gs_engine_instance();

	// Platform api 
	gs_platform_i* platform = engine->ctx.platform;
	gs_graphics_i* gfx = engine->ctx.graphics;

	// Cache pointers
	gs_quad_batch_t* qb = &g_batch;
	gs_command_buffer_t* cb = &g_cb;

	// If we press the escape key, exit the application
	if (platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	const f32 dt = platform->time.delta;
	const f32 t = platform->elapsed_time();

	update_camera();

	// Add some stuff to the quad batch
	gfx->quad_batch_begin(qb);
	{
		gs_for_range_i(100) {
			gs_for_range_j(100) {
				gs_default_quad_info_t quad_info = {0};
				quad_info.transform = gs_vqs_default();
				quad_info.transform.position = (gs_vec3){i, j, 0.f};
				quad_info.uv = (gs_vec4){0.f, 0.f, 1.f, 1.f};
				quad_info.color = i % 2 == 0 ? (gs_vec4){1.f, 1.f, 1.f, 1.f} : (gs_vec4){1.f, 0.f, 0.f, 1.f};
				gfx->quad_batch_add(qb, &quad_info);
			}
		}
	}
	gfx->quad_batch_end(qb);

	/*===============
	// Render scene
	================*/

	// Main window size
	gs_vec2 ws = platform->window_size(platform->main_window());
	gs_vec2 fbs = platform->frame_buffer_size(platform->main_window());

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear(cb, clear_color);
	gfx->set_viewport(cb, 0.f, 0.f, fbs.x, fbs.y);
	gfx->set_depth_enabled(cb, false);
	gfx->set_blend_mode(cb, gs_blend_mode_src_alpha, gs_blend_mode_one_minus_src_alpha);

	// Create model/view/projection matrices from camera
	gs_mat4 view_mtx = gs_camera_get_view(&g_camera);
	gs_mat4 proj_mtx = gs_camera_get_projection(&g_camera, ws.x, ws.y);
	gs_mat4 model_mtx = gs_mat4_scale((gs_vec3){1.f, 1.f, 1.f});

	// Set necessary dynamic uniforms for quad batch material (defined in default shader in gs_quad_batch.h)
	gfx->set_material_uniform_mat4(qb->material, "u_model", model_mtx);
	gfx->set_material_uniform_mat4(qb->material, "u_view", view_mtx);
	gfx->set_material_uniform_mat4(qb->material, "u_proj", proj_mtx);

	// Need to submit quad batch
	gfx->quad_batch_submit(cb, qb);

	gfx->immediate.begin_drawing(cb);
	{
		gfx->immediate.draw_text(cb, 10.f, 20.f, "Example: Simple Quad Batch", gs_color_white);
	}
	gfx->immediate.end_drawing(cb);

	// Submit command buffer for rendering
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}

void update_camera()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	if (platform->key_down(gs_keycode_q)) {
		g_camera.ortho_scale += 0.1f;
	}
	if (platform->key_down(gs_keycode_e)) {
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
	gs_println("Goodbye, Gunslinger.");
	return gs_result_success;
}
