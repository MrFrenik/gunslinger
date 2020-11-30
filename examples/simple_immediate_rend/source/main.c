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
	gs_graphics_immediate_draw_i* id = &gfx->immediate;
	gs_command_buffer_t* cb = &g_cb;

	// Get framebuffer and window sizes
	const gs_vec2 fbs = platform->frame_buffer_size(platform->main_window());
	const gs_vec2 ws = platform->window_size(platform->main_window());

	// If we press the escape key, exit the application
	if (platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	/*===============
	// Render scene
	================*/

	// Set clear color and clear screen
	f32 clear_color[4] = {0.2f, 0.2f, 0.2f, 1.f};
	gfx->set_view_clear(cb, clear_color);
	gfx->set_view_port(cb, fbs.x, fbs.y);

	// Elapsed run time of program
	const f32 _t = platform->elapsed_time();

	id->begin_drawing(cb);	// Maybe don't need to do this?
	{
		/*========================
		// Push Vertices Directly
		========================*/
		id->begin(cb, gs_triangles);
		{
			id->disable_texture_2d(cb);
			id->color_ubv(cb, gs_color_white);
			id->vertex_3fv(cb, gs_v3(100.f, 100.f, 0.f));
			id->vertex_3fv(cb, gs_v3(150.f, 100.f, 0.f));
			id->vertex_3fv(cb, gs_v3(150.f, 150.f, 0.f));
		}
		id->end(cb);

		/*==========
		// 3D
		==========*/
		id->push_camera(cb, gs_camera_perspective());
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
			id->draw_box_ext(cb, xform, gs_color_alpha(gs_color_white, 50));

			// Draw cube with push matrix for rotation
			id->push_matrix(cb, gs_matrix_model);
				gs_mat4 rot = gs_quat_to_mat4(gs_quat_mul(xform.rotation, gs_quat_angle_axis(_t * 0.0001f, gs_z_axis)));
				id->mat_mul(cb, rot);
				id->draw_box(cb, gs_v3(5.f, 5.f, -20.f), gs_v3(2.f, 2.f, 2.f), gs_color_white);
			id->pop_matrix(cb);

			id->begin(cb, gs_lines);
			{
			}
			id->end(cb);

			/*==========
			// Sphere
			==========*/
			// Draw sphere rotating around box
			id->draw_sphere(
				cb, 
				gs_v3(cosf(_t * 0.001f) * 10, 0.f, sinf(_t * 0.001f) * 10 - 25.f), 
				2.f, 
				gs_color_alpha(gs_color_orange, 255)
			);
		}
		id->pop_camera(cb);

		// Back to 2d state
		gfx->set_depth_enabled(cb, false);
		gfx->set_face_culling(cb, gs_face_culling_disabled);

		/*===========
		// Triangles
		===========*/
		// Transformed triangle
		id->push_matrix(cb, gs_matrix_model);
			id->mat_transf(cb, 200.f, 200.f, 0.f);
			id->mat_rotateq(cb, gs_quat_angle_axis(_t * 0.001f, gs_z_axis));
			id->mat_scalef(cb, 200.f, 200.f, 1.f);
			id->draw_triangle(cb, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), gs_v2(0.f, 1.f), gs_color_blue);
		id->pop_matrix(cb);

		/*===========
		// Rects
		===========*/
		id->draw_rect(cb, gs_v2(500.f, 500.f), gs_v2(600.f, 550.f), gs_color(0.f, 1.f, 0.f, 1.f));
		id->draw_rect_textured(cb, gs_v2(600.f, 400.f), gs_v2(750.f, 550.f), g_texture.id, gs_color_green);

		/*==========
		// Lines
		==========*/
		gs_vec2 mp = platform->mouse_position();
		id->draw_line(cb, gs_v2(mp.x, 0.f), gs_v2(mp.x, ws.y), 1.f, gs_color_red);
		id->draw_line(cb, gs_v2(0.f, mp.y), gs_v2(ws.x, mp.y), 1.f, gs_color_red);

		/*==========
		// Text
		==========*/
		s8 buffer[256];
		memset(buffer, 0, 256);
		gs_snprintf(buffer, 256, "Frame: %.2f ms", platform->time.frame);
		gs_vec2 td = gfx->text_dimensions(cb, buffer, &g_font);
		id->draw_rect(cb, gs_v2(300.f, 500.f), gs_v2(300.f + td.x, 500.f - td.y), gs_color_red);
		id->draw_text(cb, gs_v2(300.f, 500.f), buffer, &g_font, gs_color_white);
	} 
	id->end_drawing(cb);

	// Submit command buffer for rendering
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}
