#include "graphics/gs_camera.h"
#include "platform/gs_platform.h"
#include "base/gs_engine.h"

gs_vec3 gs_camera_forward( gs_camera* cam )
{
	return ( gs_quat_rotate( cam->transform.rotation, ( gs_vec3 ){ 0.0f, 0.0f, 1.0f } ) );
} 

gs_vec3 gs_camera_up( gs_camera* cam )
{
	return ( gs_quat_rotate( cam->transform.rotation, ( gs_vec3 ){ 0.0f, 1.0f, 0.0f } ) );
}

gs_mat4 gs_camera_get_view( gs_camera* cam )
{
	// NOTE(john): mat4 lookat is messed up right now
	// gs_vec3 up = gs_camera_up( cam );
	// gs_vec3 forward = gs_camera_forward( cam );
	// gs_vec3 target = gs_vec3_add( forward, cam->transform.position );
	// return gs_mat4_look_at( cam->transform.position, target, up );

	gs_vec3 tscl = cam->transform.scale;
	gs_vec3 tpos = cam->transform.position;

	gs_mat4 scl = gs_mat4_scale((gs_vec3){1.f / tscl.x, 1.f / tscl.y, 1.f / tscl.z});	
	gs_mat4 rot = gs_quat_to_mat4(cam->transform.rotation);
	gs_mat4 tns = gs_mat4_translate((gs_vec3){-tpos.x, -tpos.y, -tpos.z});

	gs_mat4 ret = gs_mat4_mul(scl, rot);
	ret = gs_mat4_mul(ret, tns);
	return ret;
}

gs_mat4 gs_camera_get_projection( gs_camera* cam, s32 view_width, s32 view_height )
{
	gs_mat4 proj_mat = gs_mat4_identity();

	switch( cam->proj_type )
	{
		case gs_projection_type_perspective:
		{
			proj_mat = gs_mat4_perspective( cam->fov, cam->aspect_ratio, cam->near_plane, cam->far_plane );
		} break;

		case gs_projection_type_orthographic:
		{
			f32 distance = 0.5f * ( cam->far_plane - cam->near_plane );
			const f32 ortho_scale = cam->ortho_scale;
			const f32 aspect_ratio = cam->aspect_ratio;
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
			// );
		} break;
	}

	return proj_mat;
}

