#include "graphics/gs_material.h"

gs_uniform_block_t __gs_uniform_block_t_new()
{
	gs_uniform_block_t ub = {0};
	ub.data = gs_byte_buffer_new();
	ub.offset_lookup_table = gs_hash_table_new(u64, u32);
	ub.uniforms = gs_hash_table_new(u64, gs_uniform_t);
	ub.count = 0;
	return ub;
}

void __gs_uniform_block_t_set_uniform(gs_uniform_block_t* u_block, gs_uniform_t uniform, const char* name, void* data, usize data_size)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Check for offset based on name, if doesn't exist create it and pass
	u64 hash_id = gs_hash_str_64(name);	
	b32 exists = true;

	if (!gs_hash_table_exists(u_block->offset_lookup_table, hash_id))	
	{
		exists = false;

		// Seek to end of buffer
		gs_byte_buffer_seek_to_end(&u_block->data);

		// Insert buffer position into offset table (which should be end)
		gs_hash_table_insert(u_block->offset_lookup_table, hash_id, u_block->data.position);

		u_block->count++;
	}

	// Otherwise, we're going to overwrite existing data	
	u32 offset = gs_hash_table_get(u_block->offset_lookup_table, hash_id);

	// Set buffer to offset position
	u_block->data.position = offset;

	// Write type into data
	gs_byte_buffer_write(&u_block->data, gs_uniform_t, uniform);
	// Write data size
	gs_byte_buffer_write(&u_block->data, usize, data_size);
	// Write data
	gs_byte_buffer_bulk_write(&u_block->data, data, data_size);

	// Subtract sizes since we were overwriting data and not appending if already exists
	if (exists) 
	{
		usize total_size = sizeof(gs_uniform_t) + sizeof(usize) + data_size;
		u_block->data.size -= total_size;
	}

	// Seek back to end
	gs_byte_buffer_seek_to_end(&u_block->data);
}

void __gs_uniform_block_t_bind_uniforms(gs_command_buffer_t* cb, gs_uniform_block_t* u_block)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Set data to beginning
	gs_byte_buffer_seek_to_beg(&u_block->data);

	// Rip through uniforms, determine size, then bind accordingly
	gs_for_range_i(u_block->count)
	{
		// Grab uniform
		gs_uniform_t uniform;
		gs_byte_buffer_bulk_read(&u_block->data, &uniform, sizeof(gs_uniform_t));
		// Grab data size
		usize sz = gs_byte_buffer_read(&u_block->data, usize);
		// Grab type
		gs_uniform_type type = uniform.type;

		// Grab data based on type and bind
		switch (type)
		{
			case gs_uniform_type_sampler2d:
			{
				gs_uniform_block_type(texture_sampler) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_texture_id(cb, uniform, value.data, value.slot);
			} break;

			case gs_uniform_type_mat4:
			{
				gs_uniform_block_type(mat4) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_uniform(cb, uniform, &value.data);
			} break;

			case gs_uniform_type_vec4:
			{
				gs_uniform_block_type(vec4) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_uniform(cb, uniform, &value.data);
			} break;

			case gs_uniform_type_vec3:
			{
				gs_uniform_block_type(vec3) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_uniform(cb, uniform, &value.data);
			} break;

			case gs_uniform_type_vec2:
			{
				gs_uniform_block_type(vec2) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_uniform(cb, uniform, &value.data);
			} break;

			case gs_uniform_type_float:
			{
				gs_uniform_block_type(float) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_uniform(cb, uniform, &value.data);
			} break;

			case gs_uniform_type_int:
			{
				gs_uniform_block_type(int) value = {0};
				gs_byte_buffer_bulk_read(&u_block->data, &value, sz);
				gfx->bind_uniform(cb, uniform, &value.data);
			} break;
 		}
	}
}

gs_uniform_block_i __gs_uniform_block_i_new()
{
	gs_uniform_block_i api = {0};
	api.construct = &__gs_uniform_block_t_new;
	api.set_uniform = &__gs_uniform_block_t_set_uniform;
	api.bind_uniforms = &__gs_uniform_block_t_bind_uniforms;
	return api;	
}

 gs_material_t gs_material_new(gs_shader_t shader)
{
	gs_material_t mat = {0};
	mat.shader = shader;
	mat.uniforms = gs_engine_instance()->ctx.graphics->uniform_i->construct();
	return mat;
}

void __gs_material_i_set_uniform(gs_material_t* mat, gs_uniform_type type, const char* name, void* data)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_uniform_block_i* uapi = gfx->uniform_i;

	// Either look for uniform or construct it
	// Look for uniform name in uniforms
	// Grab uniform from uniforms
	u64 hash_id = gs_hash_str_64(name);	
	gs_uniform_t uniform = {0};
	gs_uniform_block_t* u_block = &mat->uniforms;

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

	uapi->set_uniform(&mat->uniforms, uniform, name, data, sz);
}

void __gs_material_i_bind_uniforms(gs_command_buffer_t* cb, gs_material_t* mat)
{
	gs_engine_instance()->ctx.graphics->uniform_i->bind_uniforms(cb, &mat->uniforms);
}

gs_material_i __gs_material_i_new()
{
	gs_material_i api = {0};
	api.construct = &gs_material_new;
	api.set_uniform = &__gs_material_i_set_uniform;
	api.bind_uniforms = &__gs_material_i_bind_uniforms;
	return api;
}
