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
typedef struct gs_camera
{
	gs_vqs transform;
	f32 fov; 
	f32 aspect_ratio; 
	f32 near_plane; 
	f32 far_plane;
	f32 ortho_scale;
	gs_projection_type proj_type;
} gs_camera;

gs_mat4 gs_camera_get_view( gs_camera* cam );
gs_mat4 gs_camera_get_projection( gs_camera* cam, s32 view_width, s32 view_height );
gs_vec3 gs_camera_forward( gs_camera* cam );
gs_vec3 gs_camera_up( gs_camera* cam );

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_CAMERA_H__