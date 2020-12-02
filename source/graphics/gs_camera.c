#include "graphics/gs_camera.h"
#include "platform/gs_platform.h"
#include "base/gs_engine.h"

gs_camera_t gs_camera_default()
{
	// Construct default camera parameters
	gs_camera_t cam = gs_default_val();
	cam.transform = gs_vqs_default();
	cam.transform.position = (gs_vec3){0.f, 0.f, -1.f};
	cam.fov = 60.f;
	cam.near_plane = 0.1f;
	cam.far_plane = 1000.f;
	cam.ortho_scale = 1.f;
	cam.proj_type = gs_projection_type_orthographic;
	return cam;
}

gs_camera_t gs_camera_perspective()
{
	gs_camera_t cam = gs_camera_default();
	cam.proj_type = gs_projection_type_perspective;
	return cam;
}

gs_vec3 gs_camera_forward(gs_camera_t* cam)
{
	return (gs_quat_rotate(cam->transform.rotation, (gs_vec3){0.0f, 0.0f, -1.0f}));
} 

gs_vec3 gs_camera_backward(gs_camera_t* cam)
{
	return (gs_quat_rotate(cam->transform.rotation, (gs_vec3){0.0f, 0.0f, 1.0f}));
} 

gs_vec3 gs_camera_up(gs_camera_t* cam)
{
	return (gs_quat_rotate(cam->transform.rotation, (gs_vec3){0.0f, 1.0f, 0.0f}));
}

gs_vec3 gs_camera_down(gs_camera_t* cam)
{
	return (gs_quat_rotate(cam->transform.rotation, (gs_vec3){0.0f, -1.0f, 0.0f}));
}

gs_vec3 gs_camera_right(gs_camera_t* cam)
{
	return (gs_quat_rotate(cam->transform.rotation, (gs_vec3){1.0f, 0.0f, 0.0f}));
}

gs_vec3 gs_camera_left(gs_camera_t* cam)
{
	return (gs_quat_rotate(cam->transform.rotation, (gs_vec3){-1.0f, 0.0f, 0.0f}));
}

gs_vec3 gs_camera_unproject(gs_camera_t* cam, gs_vec3 coords, s32 view_width, s32 view_height)
{
	gs_vec3 wc = gs_default_val();

	// Get inverse of view projection from camera
	gs_mat4 inverse_vp = gs_mat4_inverse(gs_camera_get_view_projection(cam, view_width, view_height));	

	f32 w_x = (f32)coords.x;
	f32 w_y = (f32)coords.y;
	f32 w_z = (f32)coords.z;

	// Transform from ndc
	gs_vec4 in;
	in.x = (w_x / (f32)view_width) * 2.f - 1.f;
	in.y = 1.f - (w_y / (f32)view_height) * 2.f;
	in.z = 2.f * w_z - 1.f;
	in.w = 1.f;

	// To world coords
	gs_vec4 out = gs_mat4_mul_vec4(inverse_vp, in);
	if (out.w == 0.f)
	{
		// Avoid div by zero
		return wc;
	}

	out.w = 1.f / out.w;
	wc = (gs_vec3) {
		out.x * out.w,
		out.y * out.w,
		out.z * out.w
	};

	return wc;
}

gs_mat4 gs_camera_get_view_projection(gs_camera_t* cam, s32 view_width, s32 view_height)
{
	gs_mat4 view = gs_camera_get_view(cam);
	gs_mat4 proj = gs_camera_get_projection(cam, view_width, view_height);
	return gs_mat4_mul(proj, view);	
}

gs_mat4 gs_camera_get_view(gs_camera_t* cam)
{
	gs_vec3 up = gs_camera_up(cam);
	gs_vec3 forward = gs_camera_forward(cam);
	gs_vec3 target = gs_vec3_add(forward, cam->transform.position);
	return gs_mat4_look_at(cam->transform.position, target, up);
}

gs_mat4 gs_camera_get_projection(gs_camera_t* cam, s32 view_width, s32 view_height)
{
	gs_mat4 proj_mat = gs_mat4_identity();

	switch(cam->proj_type)
	{
		case gs_projection_type_perspective:
		{
			proj_mat = gs_mat4_perspective(cam->fov, (f32)view_width / (f32)view_height, cam->near_plane, cam->far_plane);
		} break;

		// Don't like this...
		case gs_projection_type_orthographic:
		{
			f32 _ar = (f32)view_width / (f32)view_height;
			f32 distance = 0.5f * (cam->far_plane - cam->near_plane);
			const f32 ortho_scale = cam->ortho_scale;
			const f32 aspect_ratio = _ar;
			proj_mat = gs_mat4_ortho
			(
				-ortho_scale * aspect_ratio, 
				ortho_scale * aspect_ratio, 
				-ortho_scale, 
				ortho_scale, 
				-distance, 
				distance	
			);
			// (
			// 	0.f, 
			// 	view_width, 
			// 	view_height, 
			// 	0.f, 
			// 	cam->near_plane, 
			// 	cam->far_plane	
			//);
		} break;
	}

	return proj_mat;
}

void gs_camera_offset_orientation(gs_camera_t* cam, f32 yaw, f32 pitch)
{
	gs_quat x = gs_quat_angle_axis(gs_deg_to_rad(yaw), (gs_vec3){0.f, 1.f, 0.f});		// Absolute up
	gs_quat y = gs_quat_angle_axis(gs_deg_to_rad(pitch), gs_camera_right(cam));			// Relative right
	cam->transform.rotation = gs_quat_mul(gs_quat_mul(x, y), cam->transform.rotation);
}







