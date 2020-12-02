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
			gs_quat rot = gs_quat_mul_list(
				3,
				gs_quat_angle_axis(_t * 0.0001f, gs_x_axis),
				gs_quat_angle_axis(_t * 0.0001f, gs_y_axis),
				gs_quat_angle_axis(_t * 0.0005f, gs_z_axis)
			);
			gs_vqs xform = (gs_vqs){gs_v3(0.f, 0.f, -20.f), rot, gs_v3_s((sinf(_t * 0.001f) * 0.5f + 0.5f) * 5.f)};
			gfx->immediate.draw_box_textured_vqs(cb, xform, g_texture.id, gs_color_white);

			gfx->immediate.push_state_attr(cb, gs_face_culling, gs_face_culling_disabled);
			gfx->immediate.push_matrix(cb, gs_matrix_model);
			{
				gs_vqs x0 = xform;
				x0.scale = gs_v3_s(1.f);
				gfx->immediate.mat_mul_vqs(cb, x0);
				gfx->immediate.draw_line_ext(cb, gs_v2(0.f, 0.f), gs_v2(0.f, 10.f), 0.1f, gs_color_red);
				gfx->immediate.draw_line_ext(cb, gs_v2(0.f, 0.f), gs_v2(10.f, 0.f), 0.1f, gs_color_green);
				gfx->immediate.mat_rotateq(cb, gs_quat_angle_axis(gs_deg_to_rad(90.f), gs_x_axis));
				gfx->immediate.draw_line_ext(cb, gs_v2(0.f, 0.f), gs_v2(0.f, 10.f), 0.1f, gs_color_blue);
			}
			gfx->immediate.pop_matrix(cb);

			gfx->immediate.push_matrix(cb, gs_matrix_model);
			{
				gs_vqs x0 = xform;
				x0.scale = gs_vec3_scale(x0.scale, 0.5f);	
				gfx->immediate.mat_mul_vqs(cb, x0);
				gfx->immediate.draw_sphere_lines(cb, gs_v3(0.f, 0.f, 0.f), 1.f, gs_color_purple);
			}
			gfx->immediate.pop_matrix(cb);

			// Children 
			gs_vec3 positions[3] = {
				gs_v3(0.f, 1.f, 0.f),
				gs_v3(1.f, 0.f, 0.f),
				gs_v3(0.f, 0.f, 1.f)
			};

			gs_color_t colors[3] = {
				gs_color_red,
				gs_color_green,
				gs_color_blue
			};

			gs_for_range_i(3)
			{
				gfx->immediate.push_matrix(cb, gs_matrix_model);
				{
					gs_vqs local = gs_vqs_default();
					local.position = positions[i];
					local.scale = gs_v3_s(0.5f);
					gfx->immediate.mat_mul_vqs(cb, gs_vqs_absolute_transform(&local, &xform));
					gfx->immediate.draw_sphere_lines(cb, gs_v3(0.f, 0.f, 0.f), 1.f, colors[i]);
				}
				gfx->immediate.pop_matrix(cb);
			}

			// Children 
			gs_vec3 tri_positions[3] = {
				gs_v3(0.f, 2.f, 0.f),
				gs_v3(2.f, 0.f, 0.f),
				gs_v3(0.f, 0.f, 2.f)
			};

			gs_for_range_i(3)
			{
				gfx->immediate.push_matrix(cb, gs_matrix_model);
				{
					gs_vqs local = gs_vqs_default();
					local.position = tri_positions[i];
					local.scale = gs_v3_s(2.f);
					local.rotation = gs_quat_angle_axis(_t * 0.01f, gs_y_axis);
					gfx->immediate.mat_mul_vqs(cb, gs_vqs_absolute_transform(&local, &xform));
					gfx->immediate.draw_triangle(cb, gs_v2(0.f, 0.f), gs_v2(1.f, 0.f), gs_v2(0.f, 1.f), gs_color_orange);
				}
				gfx->immediate.pop_matrix(cb);
			}
		}
		gfx->immediate.end_3d(cb);
	} 
	gfx->immediate.end_drawing(cb);

	// Submit command buffer for rendering
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}







