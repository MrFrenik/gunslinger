#include <gs.h>

typedef struct app_data_t 
{
	gs_command_buffer_t cb;			
	gs_font_t font;
	gs_texture_t texture;
} app_data_t;

app_data_t g_app_data = {0}; 

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();   // Use to shutdown your application

int main(int argc, char** argv)
{
	gs_application_desc_t app = {0};
	app.window_title 		= "Simple Immediate Rendering";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;
	app.user_data 			= &g_app_data;

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

gs_result app_init()
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	app_data_t* data = gs_engine_user_data(app_data_t);
	data->cb = gs_command_buffer_new();

	data->font = gfx->construct_font_from_file("./assets/font.ttf", 64.f);

	gs_texture_parameter_desc_t desc = gs_texture_parameter_desc_default();
	data->texture = gfx->construct_texture_from_file("./assets/gs.png", &desc);
	gs_free(desc.data);

	return gs_result_success;
}

gs_result app_update()
{
	// Cache global pointers
	gs_platform_i* platform = gs_engine_subsystem(platform);
	app_data_t* ad = gs_engine_user_data(app_data_t);
	gs_command_buffer_t* cb = &ad->cb;

	// Elapsed run time of program
	const f32 _t = platform->elapsed_time();

	// If we press the escape key, exit the application
	if (platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	static f32 _z = -3.f;
	if (platform->key_pressed(gs_keycode_e))
	{
		_z -= 0.1f;
	}
	if (platform->key_pressed(gs_keycode_q))
	{
		_z += 0.1f;
	}

	/*===================
	// Render scene Here
	===================*/
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);

	gfx->immediate.begin_drawing(cb);
	{
		gfx->immediate.clear(cb, 0.1f, 0.1f, 0.1f, 1.f);

		gs_camera_t cam = gfx->immediate.begin_3d(cb);
		{
			gs_vqs box_xform = gs_vqs_default();
			gs_vqs local_xform = gs_vqs_default();

			gfx->immediate.push_matrix(cb, gs_matrix_model);
			{
				gs_vec3 bp = gs_v3(0.f, 0.f, 0.f);
				gs_vec3 he = gs_v3_s(0.5f);

				box_xform.position = gs_v3(0.f, 0.f, _z);	
				box_xform.rotation = gs_quat_mul_list(3,
					gs_quat_angle_axis(_t * 0.001f, gs_y_axis),
					gs_quat_angle_axis(_t * 0.001f, gs_x_axis),
					gs_quat_angle_axis(_t * 0.001f, gs_z_axis)
				);
				gfx->immediate.mat_mul_vqs(cb, box_xform);
				gfx->immediate.draw_box_textured(cb, bp, he, ad->texture.id, gs_color_white);

				// Draw bounding box surrounding it
				gfx->immediate.draw_box_lines(cb, bp, gs_vec3_add(he, gs_v3_s(0.01f)), gs_color_white);
				gfx->immediate.draw_sphere(cb, bp, 0.45f, gs_color_alpha(gs_color_white, 100));
				gfx->immediate.draw_sphere_lines(cb, bp, 0.46f, gs_color_white);

				// Draw coordinate axis of box
				gfx->immediate.draw_line_3d(cb, gs_v3(0.f, 0.f, 0.f), gs_v3(1.f, 0.f, 0.f), gs_color_red);
				gfx->immediate.draw_line_3d(cb, gs_v3(0.f, 0.f, 0.f), gs_v3(0.f, 1.f, 0.f), gs_color_green);
				gfx->immediate.draw_line_3d(cb, gs_v3(0.f, 0.f, 0.f), gs_v3(0.f, 0.f, 1.f), gs_color_blue);
			}
			gfx->immediate.pop_matrix(cb);

			f32 rs[3] = 
			{
				0.6f, 
				1.f, 
				2.f
			};

			gs_color_t colors[3] = 
			{
				gs_color_purple,
				gs_color_blue,
				gs_color_orange
			};

			f32 ts0 = _t * 0.0001f;
			f32 ts1 = _t * 0.0004f;
			f32 ts2 = _t * 0.0006f;
			gs_vec3 planet_positions[3] = 
			{
				gs_vec3_add(box_xform.position, gs_vec3_scale(gs_v3(cos(ts0), sin(ts0), cos(ts0)), rs[0])),
				gs_vec3_add(box_xform.position, gs_vec3_scale(gs_v3(cos(ts1), sin(ts1), cos(ts1)), rs[1])),
				gs_vec3_add(box_xform.position, gs_vec3_scale(gs_v3(cos(ts2), sin(ts2), cos(ts2)), rs[2]))
			};

			// Planet
			gs_for_range_i(3)
			{
				gfx->immediate.push_matrix(cb, gs_matrix_model);
				{
					local_xform.position = gs_vec3_add(planet_positions[(i + 1) % 3], planet_positions[i]);
					local_xform.rotation = gs_quat_angle_axis(_t * 0.001f, gs_y_axis);
					gfx->immediate.mat_mul_vqs(cb, local_xform);
					gfx->immediate.draw_sphere(cb, gs_v3(0.f, 0.f, 0.f), 0.1f, colors[i]);
					gfx->immediate.draw_sphere_lines(cb, gs_v3(0.f, 0.f, 0.f), 0.15f, gs_color_white);
				}
				gfx->immediate.pop_matrix(cb);
			}

			gs_vqs local_xforms[3] = 
			{
				gs_vqs_ctor(gs_v3(-0.2f, 0.5f, 0.f), gs_quat_angle_axis(gs_deg_to_rad(-180.f), gs_x_axis), gs_v3_s(0.001f)),
				gs_vqs_ctor(gs_v3(-0.1f, 0.0f, 0.5f), gs_quat_angle_axis(gs_deg_to_rad(-180.f), gs_x_axis), gs_v3_s(0.001f)),
				gs_vqs_ctor(gs_v3( 0.5f, 0.0f, 0.f), gs_quat_angle_axis(gs_deg_to_rad(-180.f), gs_x_axis), gs_v3_s(0.001f))
			};

			// Make our text a child transform of the box	
			gfx->immediate.push_state_attr(cb, gs_face_culling, gs_face_culling_disabled);
			{
				gs_snprintfc(rot_buffer, 256, "rotation: %.2f, %.2f, %.2f, %.2f", 
					box_xform.rotation.x, box_xform.rotation.y, box_xform.rotation.z, box_xform.rotation.w); 
				gs_snprintfc(trans_buffer, 256, "trans: %.2f, %.2f, %.2f",
					box_xform.position.x, box_xform.position.y, box_xform.position.z);
				gs_snprintfc(scale_buffer, 256, "scale: %.2f, %.2f, %.2f", 
					box_xform.scale.x, box_xform.scale.y, box_xform.scale.z);

				char* buffers[3] = 
				{
					rot_buffer, 
					trans_buffer, 
					scale_buffer
				};

				gs_for_range_i(3)
				{
					gfx->immediate.push_matrix(cb, gs_matrix_model);
					{
						gfx->immediate.mat_mul_vqs(cb, gs_vqs_absolute_transform(&local_xforms[i], &box_xform));
						gfx->immediate.draw_text(cb, 0.f, 0.f, buffers[i], &ad->font, gs_color_green);
					}
					gfx->immediate.pop_matrix(cb);
				}
			}
			gfx->immediate.pop_state_attr(cb);
		}
		gfx->immediate.end_3d(cb);

		gs_vec2 ws = platform->window_size(platform->main_window());
		gs_snprintfc(fps_text, 256, "fps: %.2f", 1000.f / platform->time.frame);
		gfx->immediate.draw_text(cb, 10.f, 50.f, fps_text, &ad->font, gs_color_white);
	}
	gfx->immediate.end_drawing(cb);

	// Final command buffer submit to draw
	gfx->submit_command_buffer(cb);

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	return gs_result_success;
}







