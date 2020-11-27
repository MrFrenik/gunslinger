#include <gs.h>

// Globals
_global gs_command_buffer_t g_cb = gs_default_val();

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

	// Get framebuffer and window sizes
	const gs_vec2 fbs = platform->frame_buffer_size(platform->main_window());
	const gs_vec2 ws = platform->window_size(platform->main_window());

	// If we press the escape key, exit the application
	if (engine->ctx.platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	/*===============
	// Render scene
	================*/

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear(cb, clear_color);
	gfx->set_view_port(cb, fbs.x, fbs.y);

	// Elapsed run time of program
	const f32 _t = platform->elapsed_time();

	gfx->immediate.begin_drawing(cb);	// Maybe don't need to do this?
	{
		/*========================
		// Push Vertices Directly
		========================*/
		gfx->immediate.begin(cb, gs_triangles);
		{
			gfx->immediate.color_ubv(cb, gs_color_white);
			gfx->immediate.vertex_3fv(cb, gs_v3(100.f, 100.f, 0.f));
			gfx->immediate.vertex_3fv(cb, gs_v3(150.f, 100.f, 0.f));
			gfx->immediate.vertex_3fv(cb, gs_v3(150.f, 150.f, 0.f));
		}
		gfx->immediate.end(cb);

		/*==========
		// 3D
		==========*/
		gfx->immediate.push_camera(cb, gs_camera_perspective());
		{
			// Want to abstract these into a default "3d state"
			gfx->set_depth_enabled(cb, true);
			gfx->set_face_culling(cb, gs_face_culling_back);

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
			gfx->immediate.draw_box_ext(cb, xform, gs_color_alpha(gs_color_white, 50));

			// // Rotation
			xform.rotation = gs_quat_mul(
				xform.rotation, 
				gs_quat_angle_axis(_t * 0.001f, gs_z_axis)
			);
			gfx->immediate.push_matrix(cb, gs_matrix_model);
				gfx->immediate.mat_mul(cb, gs_quat_to_mat4(xform.rotation));
				gfx->immediate.draw_box(cb, gs_v3(5.f, 5.f, -20.f), gs_v3(2.f, 2.f, 2.f), gs_color_white);
			gfx->immediate.pop_matrix(cb);

			/*==========
			// Sphere
			==========*/
			// Draw sphere rotating around box
			gfx->immediate.draw_sphere(
				cb, 
				gs_v3(cosf(_t * 0.001f) * 10, 0.f, sinf(_t * 0.001f) * 10 - 25.f), 
				2.f, 
				gs_color_alpha(gs_color_orange, 255)
			);
		}
		gfx->immediate.pop_camera(cb);

		// Back to 2d state
		gfx->set_depth_enabled(cb, false);
		gfx->set_face_culling(cb, gs_face_culling_disabled);

		/*===========
		// Triangles
		===========*/
		// Transformed triangle
		gfx->immediate.push_matrix(cb, gs_matrix_model);
			gfx->immediate.mat_transf(cb, 200.f, 200.f, 0.f);
			gfx->immediate.mat_rotateq(cb, gs_quat_angle_axis(_t * 0.001f, gs_z_axis));
			gfx->immediate.mat_scalef(cb, 200c.f, 200c.f, 1.f);
			gfx->immediate.draw_triangle(cb, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), gs_v2(0.f, 1.f), gs_color_blue);
		gfx->immediate.pop_matrix(cb);

		/*===========
		// Rects
		===========*/
		gfx->immediate.draw_rect(cb, gs_v2(500.f, 500.f), gs_v2(600.f, 550.f), gs_color(0.f, 1.f, 0.f, 1.f));

		/*==========
		// Lines
		==========*/
		gs_vec2 mp = platform->mouse_position();
		gfx->immediate.draw_line(cb, gs_v2(mp.x, 0.f), gs_v2(mp.x, ws.y), 1.f, gs_color_red);
		gfx->immediate.draw_line(cb, gs_v2(0.f, mp.y), gs_v2(ws.x, mp.y), 1.f, gs_color_red);
	} 
	gfx->immediate.end_drawing(cb);

	// Submit command buffer for rendering
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}
