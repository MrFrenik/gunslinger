#include <gs.h>

// Globals
_global gs_command_buffer_t g_cb = gs_default_val();
_global gs_font_t 			g_font = gs_default_val();
_global gs_texture_t 		g_texture = gs_default_val();

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application

int main(int argc, char** argv)
{
	gs_application_desc_t app = {0};
	app.window_title 		= "Simple Immediate Rendering";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;

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
	// Construct command buffer (the command buffer is used to allow for immediate drawing at any point in our program)
	g_cb = gs_command_buffer_new();

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	g_font = gfx->construct_font_from_file("./../assets/font.ttf", 32.f);

	gs_texture_parameter_desc_t desc = gs_texture_parameter_desc_default();	
	g_texture = gfx->construct_texture_from_file("./../assets/gs.png", &desc);
	gs_free(desc.data);

	return gs_result_success;
}

// Main update loop
gs_result app_update()
{
	// Cache global pointers
	gs_engine_t* engine = gs_engine_instance();
	gs_graphics_i* gfx = engine->ctx.graphics;
	gs_platform_i* platform = engine->ctx.platform;
	gs_graphics_immediate_draw_i* id = &gfx->immediate;
	gs_command_buffer_t* cb = &g_cb;

	// Get window sizes
	const gs_vec2 ws = platform->window_size(platform->main_window());

	// If we press the escape key, exit the application
	if (platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	/*===============
	// Render scene
	================*/

	// Elapsed run time of program
	const f32 _t = platform->elapsed_time();

	id->begin_drawing(cb);	// Maybe don't need to do this?
	{
		/*==========
		// 3D
		==========*/
		gs_pipeline_state_t state = gs_pipeline_state_default();
		state.depth_enabled = true;
		state.face_culling = gs_face_culling_back;
		id->push_state(cb, state);
		id->push_camera(cb, gs_camera_perspective());
		{
			/*==========
			// Box
			==========*/
			gs_vqs xform = gs_vqs_default();
			xform.position = gs_v3(0.f, 0.f, -20.f);
			xform.scale = gs_vec3_scale(gs_v3(1.f, 1.f, 1.f), 10.f);
			xform.rotation = gs_quat_mul_list(
				3,
				gs_quat_angle_axis(_t * 0.001f, gs_y_axis),
				gs_quat_angle_axis(_t * 0.0001f, gs_x_axis),
				gs_quat_angle_axis(_t * 0.0005f, gs_z_axis)
			);
			id->draw_box_textured_vqs(cb, xform, g_texture.id, gs_color_white); 
		}
		id->pop_camera(cb);
		id->pop_state(cb); 
	} 
	id->end_drawing(cb);

	// Submit command buffer for rendering
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}
