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

			gs_for_range_i(10)
			{
				f32 s = (sin(_t * 0.001f) * 0.5f + 0.5f) * 0.5f * i;
				xform.scale = gs_v3_s(10.f + i * 0.05f);
				id->draw_box_lines_vqs(cb, xform, gs_color_orange);
			}

			// Draw 3d lines to represent the forward, up, right local axis of box
			id->push_matrix(cb, gs_matrix_model);

				xform.scale = gs_v3_s(6.f);
				id->mat_mul_vqs(cb, xform);

				// Z axis
				id->draw_line_3d(cb, gs_v3_s(0.f), gs_z_axis, gs_color_blue);
				id->draw_line_3d(cb, gs_v3_s(0.f), gs_vec3_scale(gs_z_axis, -1.f), gs_color_blue);

				// X axis
				id->draw_line_3d(cb, gs_v3_s(0.f), gs_x_axis, gs_color_red);
				id->draw_line_3d(cb, gs_v3_s(0.f), gs_vec3_scale(gs_x_axis, -1.f), gs_color_red);

				// Y axis
				id->draw_line_3d(cb, gs_v3_s(0.f), gs_y_axis, gs_color_green);
				id->draw_line_3d(cb, gs_v3_s(0.f), gs_vec3_scale(gs_y_axis, -1.f), gs_color_green);
			id->pop_matrix(cb);

			/*==========
			// Sphere
			==========*/
			// Draw sphere rotating around box
			f32 r = 2.f;
			xform.position = gs_v3(cosf(_t * 0.001f) * 10, 0.f, sinf(_t * 0.001f) * 10 - 25.f);
			xform.scale = gs_v3_s(r); 
			xform.rotation = gs_quat_mul(xform.rotation, gs_quat_angle_axis(_t * 0.001f, gs_z_axis));
			id->draw_sphere(
				cb, 
				xform.position,
				r,
				gs_color_alpha(gs_color_red, 255)
			);

			// Draw sphere lines
			xform.scale = gs_v3_s(r + (sin(_t * 0.001f) * 0.5f + 0.5f) * 2.f);
			id->draw_sphere_lines_vqs(
				cb, 
				xform,
				gs_color_alpha(gs_color_white, 255)
			);

			/*===========
			// Circle
			===========*/
			// id->push_state_attr(cb, gs_face_culling, gs_face_culling_disabled);
			// {
			// 	id->push_matrix(cb, gs_matrix_model); 
			// 	{
			// 		id->mat_mul_vqs(cb, xform);
			// 		id->draw_circle(cb, gs_v2(0.f, 0.f), 1.f, 32, gs_color_green);
			// 	}
			// 	id->pop_matrix(cb);
			// }
			// id->pop_state_attr(cb);
		}
		id->pop_camera(cb);
		id->pop_state(cb);

		// Back to 2d state
		gfx->set_depth_enabled(cb, false);
		gfx->set_face_culling(cb, gs_face_culling_disabled);

		/*===========
		// Rects
		===========*/
		id->draw_rect(cb, gs_v2(500.f, 500.f), gs_v2(600.f, 550.f), gs_color(0.f, 1.f, 0.f, 1.f));
		id->draw_rect_textured(cb, gs_v2(600.f, 400.f), gs_v2(750.f, 550.f), g_texture.id, gs_color_green);

		/*=====================
		// 2D Lines (Thickness)
		======================*/
		gs_vec2 mp = platform->mouse_position();
		id->draw_line_ext(cb, gs_v2(mp.x, 0.f), gs_v2(mp.x, ws.y), 3.f, gs_color_red);
		id->draw_line_ext(cb, gs_v2(0.f, mp.y), gs_v2(ws.x, mp.y), 3.f, gs_color_red);

		/*===========
		// Triangles
		===========*/
		// Transformed triangle
		id->push_matrix(cb, gs_matrix_model);
			id->mat_transf(cb, mp.x, mp.y, 0.f);
			id->mat_rotateq(cb, gs_quat_angle_axis(_t * 0.001f, gs_z_axis));
			id->mat_scalef(cb, 80.f, 80.f, 1.f);
			f32 a0 = gs_deg_to_rad(30.f);
			f32 a1 = gs_deg_to_rad(60.f);
			id->draw_triangle(cb, gs_v2(0.f, 0.f), gs_v2(cos(a0), sin(a0)), gs_v2(cos(a1), sin(a1)), gs_color_blue);
		id->pop_matrix(cb);

		/*==========
		// Text
		==========*/
		char buffer[256];
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
