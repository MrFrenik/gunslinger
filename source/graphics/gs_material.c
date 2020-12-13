#include "base/gs_engine.h"
#include "graphics/gs_graphics.h"

gs_resource(gs_material_t) gs_material_new(gs_shader_t shader)
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	gs_material_t mat = {0};
	mat.shader = shader;
	mat.uniforms = gfx->uniform_i->construct();
	gs_resource(gs_material_t) handle = gs_resource_cache_insert(gfx->material_cache, mat);
	return handle;
}

void __gs_material_i_set_uniform(gs_resource(gs_material_t) handle, gs_uniform_type type, const char* name, void* data)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_uniform_block_i* uapi = gfx->uniform_i;

	gs_material_t* mat = gs_resource_cache_get_ptr(gfx->material_cache, handle);

	// Either look for uniform or construct it
	// Look for uniform name in uniforms
	// Grab uniform from uniforms
	u64 hash_id = gs_hash_str_64(name);	
	gs_uniform_t uniform = {0};
	gs_uniform_block_t* u_block = gs_slot_array_get_ptr(uapi->uniform_blocks, mat->uniforms.id);

	if (!gs_hash_table_exists(u_block->offset_lookup_table, hash_id))	
	{
		// Construct or get existing uniform
		uniform = gfx->construct_uniform(mat->shader, name, type);

		// Insert buffer position into offset table (which should be end)
		gs_hash_table_insert(u_block->uniforms, hash_id, uniform);
	}
	else 
	{
		uniform = gs_hash_table_get(u_block->uniforms, hash_id);
	}

	usize sz = 0;
	switch (type)
	{
		case gs_uniform_type_mat4: sz = sizeof(gs_uniform_block_type(mat4)); break;
		case gs_uniform_type_vec4: sz = sizeof(gs_uniform_block_type(vec4)); break;
		case gs_uniform_type_vec3: sz = sizeof(gs_uniform_block_type(vec3)); break;
		case gs_uniform_type_vec2: sz = sizeof(gs_uniform_block_type(vec2)); break;
		case gs_uniform_type_float: sz = sizeof(gs_uniform_block_type(float)); break;
		case gs_uniform_type_int: sz = sizeof(gs_uniform_block_type(int)); break;
		case gs_uniform_type_sampler2d: sz = sizeof(gs_uniform_block_type(texture_sampler)); break;
	};

	uapi->set_uniform(mat->uniforms, uniform, name, data, sz);
}

void __gs_material_i_bind_uniforms(gs_command_buffer_t* cb, gs_resource(gs_material_t) handle)
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	gs_material_t* mat = gs_resource_cache_get_ptr(gfx->material_cache, handle);
	gfx->uniform_i->bind_uniforms(cb, mat->uniforms);
}

gs_material_i __gs_material_i_new()
{
	gs_material_i api = {0};
	api.construct = &gs_material_new;
	api.set_uniform = &__gs_material_i_set_uniform;
	api.bind_uniforms = &__gs_material_i_bind_uniforms;
	return api;
}
