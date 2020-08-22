#ifndef __GS_CAMERA_H__
#define __GS_CAMERA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "math/gs_math.h"
#include "base/gs_object.h"

typedef enum 
{
	gs_projection_type_orthographic,
	gs_projection_type_perspective
} gs_projection_type;

// TODO(john): enums need to be supported with the reflection generation
typedef struct gs_camera_t
{
	gs_vqs transform;
	f32 fov; 
	f32 aspect_ratio; 
	f32 near_plane; 
	f32 far_plane;
	f32 ortho_scale;
	gs_projection_type proj_type;
} gs_camera_t;

gs_mat4 gs_camera_get_view( gs_camera_t* cam );
gs_mat4 gs_camera_get_projection( gs_camera_t* cam, s32 view_width, s32 view_height );
gs_mat4 gs_camera_get_view_projection( gs_camera_t* cam, s32 view_width, s32 view_height );
gs_vec3 gs_camera_forward( gs_camera_t* cam );
gs_vec3 gs_camera_backward( gs_camera_t* cam );
gs_vec3 gs_camera_up( gs_camera_t* cam );
gs_vec3 gs_camera_down( gs_camera_t* cam );
gs_vec3 gs_camera_right( gs_camera_t* cam );
gs_vec3 gs_camera_left( gs_camera_t* cam );
gs_vec3 gs_camera_unproject( gs_camera_t* cam, gs_vec3 coords, s32 view_width, s32 view_height );
void gs_camera_offset_orientation( gs_camera_t* cam, f32 yaw, f32 picth );

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_CAMERA_H__