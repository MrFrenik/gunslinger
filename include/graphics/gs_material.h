#ifndef __GS_MATERIAL_H__
#define __GS_MATERIAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "base/gs_engine.h"
#include "serialize/gs_byte_buffer.h"
#include "graphics/gs_graphics.h"

/*=====================
// Material
======================*/

typedef struct gs_material_t
{
	gs_shader_t shader;
	gs_resource(gs_uniform_block_t) uniforms;
} gs_material_t;

extern gs_material_t gs_material_new(gs_shader_t shader);
extern void __gs_material_i_set_uniform(gs_material_t* mat, gs_uniform_type type, const char* name, void* data);
extern void __gs_material_i_bind_uniforms(gs_command_buffer_t* cb, gs_material_t* mat);

typedef struct gs_material_i
{
	gs_material_t (*construct)(gs_shader_t);
	void (*set_uniform)(gs_material_t*, gs_uniform_type, const char* name, void* data);
	void (*bind_uniforms)(gs_command_buffer_t*, gs_material_t*);
} gs_material_i;

gs_material_i __gs_material_i_new();

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_MATERIAL_H__
