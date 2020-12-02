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

	g_font = gfx->construct_font_from_file("./assets/font.ttf", 32.f);

	gs_texture_parameter_desc_t desc = gs_texture_parameter_desc_default();	
	g_texture = gfx->construct_texture_from_file("./assets/gs.png", &desc);
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
	gs_command_buffer_t* cb = &g_cb;

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

	gfx->immediate.begin_drawing(cb);
	{
		// Clear screen
		gfx->immediate.clear(cb, 0.1f, 0.1f, 0.1f, 1.f);

		gfx->immediate.begin_3d(cb);
		{
			gs_vqs xform = (gs_vqs){gs_v3(0.f, 0.f, -20.f), gs_quat_angle_axis(_t * 0.001f, gs_y_axis), gs_v3_s(10.f)};
			gfx->immediate.draw_box_textured_vqs(cb, xform, g_texture.id, gs_color_white);
		}
		gfx->immediate.end_3d(cb);
		
		gfx->immediate.begin_2d(cb);
		{
			gfx->immediate.draw_line(cb, gs_v2(0.f, 0.f), gs_v2(500.f, 500.f), gs_color_red);
		}
		gfx->immediate.end_2d(cb);
	} 
	gfx->immediate.end_drawing(cb);

	// Submit command buffer for rendering
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}
