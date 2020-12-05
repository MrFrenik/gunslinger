#include "graphics/gs_graphics.h"
#include "graphics/gs_material.h"
#include "graphics/gs_camera.h"
#include "platform/gs_platform.h"
#include "base/gs_engine.h"

#define STB_DEFINE
#include <stb/stb.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// Error rate to calculate how many segments we need to draw a smooth circle,
// taken from https://stackoverflow.com/a/2244088
#ifndef smooth_circle_error_rate
    #define smooth_circle_error_rate  0.5f
#endif

const f32 deg2rad = gs_pi / 180.f;

gs_texture_parameter_desc gs_texture_parameter_desc_default()
{
	gs_texture_parameter_desc desc = {0};
	
	desc.texture_wrap_s 	= gs_repeat;
	desc.texture_wrap_t 	= gs_repeat;
	desc.min_filter 		= gs_linear;
	desc.mag_filter 		= gs_linear;
	desc.mipmap_filter 		= gs_linear;
	desc.generate_mips 		= true;
	desc.border_color[0]	= 0.f;
	desc.border_color[1]	= 0.f;
	desc.border_color[2]	= 0.f;
	desc.border_color[3]	= 0.f;
	desc.data 				= NULL;
	desc.texture_format 	= gs_texture_format_rgba8;
	desc.width 				= 0;
	desc.height 			= 0;
	desc.num_comps 			= 0;
	desc.flip_vertically_on_load = true;

	return desc;
}

void* gs_load_texture_data_from_file(const char* file_path, b32 flip_vertically_on_load)
{
	// Load texture data
	stbi_set_flip_vertically_on_load(flip_vertically_on_load);

	// For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param
	// Could optimize this later
	s32 width, height, num_comps;
	void* texture_data = stbi_load(file_path, (s32*)&width, (s32*)&height, &num_comps, STBI_rgb_alpha);

	if (!texture_data)
	{
		gs_println("Warning: could not load texture: %s", file_path);
		return NULL;
	}

	return texture_data;
}

gs_font_t __gs_construct_font_from_file(const char* file_path, f32 point_size)
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_font_t f = gs_default_val();

	stbtt_fontinfo font = gs_default_val();
	char* ttf = platform->read_file_contents(file_path, "rb", NULL);
	const u32 w = 512;
	const u32 h = 512;
	const u32 num_comps = 4;
	u8* alpha_bitmap = gs_malloc(w * h);
	u8* flipmap = gs_malloc(w * h * num_comps);
	memset(alpha_bitmap, 0, w * h);
	memset(flipmap, 0, w * h * num_comps);
   	s32 v = stbtt_BakeFontBitmap((u8*)ttf, 0, point_size, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f.glyphs); // no guarantee this fits!

   	// Flip texture
   	u32 r = h - 1;
   	for (u32 i = 0; i < h; ++i)
   	{
   		for (u32 j = 0; j < w; ++j)
   		{
   			u32 i0 = i * w + j;
   			u32 i1 = r * w * num_comps + j * num_comps;
   			u8 a = alpha_bitmap[i0];
   			flipmap[i1 + 0] = 255;
   			flipmap[i1 + 1] = 255;
   			flipmap[i1 + 2] = 255;
   			flipmap[i1 + 3] = a;
   		}
   		r--;
   	}

   	gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();
   	desc.width = w;
   	desc.height = h;
   	desc.num_comps = num_comps;
   	desc.data = flipmap;
   	desc.texture_format = gs_texture_format_rgba8;
   	desc.min_filter = gs_linear;

   	// Generate atlas texture for bitmap with bitmap data
   	f.texture = gfx->construct_texture(desc);

   	if (v == 0) {
	   	gs_println("Font Failed to Load: %s, %d", file_path, v);
   	}
   	else {
	   	gs_println("Font Successfully Load: %s, %d", file_path, v);
   	}

   	gs_free(ttf);
   	gs_free(alpha_bitmap);
   	gs_free(flipmap);

	return f;
}

gs_vec2 __gs_text_dimensions(const char* text, gs_font_t* ft)
{
	gs_vec2 pos = gs_v2(0.f, 0.f);
	gs_vec2 dims = gs_v2(pos.x, pos.y);
	while (text[0] != '\0')
	{
		char c = text[0];
		if (c >= 32 && c <= 127) 
		{
			stbtt_aligned_quad q = gs_default_val();
			stbtt_GetBakedQuad((stbtt_bakedchar*)ft->glyphs, ft->texture.width, ft->texture.height, c - 32, &pos.x, &pos.y, &q, 1);
			dims.x = pos.x;
			dims.y = gs_max(dims.y, (q.y1 - q.y0));
		}
		text++;
	}
	return dims;
}

void gs_rgb_to_hsv(u8 r, u8 g, u8 b, f32* h, f32* s, f32* v)
{
	f32 fR = (f32)r / 255.f; 
	f32 fG = (f32)g / 255.f; 
	f32 fB = (f32)b / 255.f;

	f32 fCMax = gs_max(gs_max(fR, fG), fB);
	f32 fCMin = gs_min(gs_min(fR, fG), fB);
	f32 fDelta = fCMax - fCMin;

	if(fDelta > 0.f) {

		if(fCMax == fR) 
		{
		  *h = 60.f * (fmod(((fG - fB) / fDelta), 6));
		} 
		else if(fCMax == fG) 
		{
		  *h = 60.f * (((fB - fR) / fDelta) + 2);
		} 
		else if(fCMax == fB) 
		{
		  *h = 60.f * (((fR - fG) / fDelta) + 4);
		}

		if(fCMax > 0.f) 
		{
		  *s = fDelta / fCMax;
		} 
		else 
		{
		  *s = 0;
		}

		*v = fCMax;
	} 
	else 
	{
		*h = 0;
		*s = 0;
		*v = fCMax;
	}

	if(*h < 0.f) 
	{
		*h = 360.f + *h;
	}
}

void gs_hsv_to_rgb(f32 h, f32 s, f32 v, u8* r, u8* g, u8* b)
{
	f32 fC = v * s; // Chroma
	f32 fHPrime = fmod(h / 60.0, 6);
	f32 fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	f32 fM = v - fC;
  
  if(0 <= fHPrime && fHPrime < 1) {
    *r = fC;
    *g = fX;
    *b = 0;
  } else if(1 <= fHPrime && fHPrime < 2) {
    *r = fX;
    *g = fC;
    *b = 0;
  } else if(2 <= fHPrime && fHPrime < 3) {
    *r = 0;
    *g = fC;
    *b = fX;
  } else if(3 <= fHPrime && fHPrime < 4) {
    *r = 0;
    *g = fX;
    *b = fC;
  } else if(4 <= fHPrime && fHPrime < 5) {
    *r = fX;
    *g = 0;
    *b = fC;
  } else if(5 <= fHPrime && fHPrime < 6) {
    *r = fC;
    *g = 0;
    *b = fX;
  } else {
    *r = 0;
    *g = 0;
    *b = 0;
  }
  
	*r += fM;
	*g += fM;
	*b += fM;
}

/*=====================
// Uniform Block
======================*/

gs_resource(gs_uniform_block_t) __gs_uniform_block_t_new()
{
	gs_uniform_block_i* uapi = gs_engine_instance()->ctx.graphics->uniform_i;
	gs_uniform_block_t ub = {0};
	ub.data = gs_byte_buffer_new();
	ub.offset_lookup_table = gs_hash_table_new(u64, u32);
	ub.uniforms = gs_hash_table_new(u64, gs_uniform_t);
	ub.count = 0;
	gs_resource(gs_uniform_block_t) res = {0};
	res.id = gs_slot_array_insert(uapi->uniform_blocks, ub);
	return res;
}

void __gs_uniform_block_t_set_uniform_from_shader(gs_resource(gs_uniform_block_t) u_block_h, gs_shader_t shader, gs_uniform_type type, const char* name, ...)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_uniform_block_i* uapi = gfx->uniform_i;

	// Either look for uniform or construct it
	// Look for uniform name in uniforms
	// Grab uniform from uniforms
	u64 hash_id = gs_hash_str_64(name);	
	gs_uniform_t uniform = {0};
	gs_uniform_block_t* u_block = gs_slot_array_get_ptr(uapi->uniform_blocks, u_block_h.id);

	if (!gs_hash_table_exists(u_block->offset_lookup_table, hash_id))	
	{
		// Construct or get existing uniform
		uniform = gfx->construct_uniform(shader, name, type);

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
		case gs_uniform_type_mat4:
		{ 
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(mat4) data = (gs_uniform_block_type(mat4)){va_arg(ap, gs_mat4)};
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(mat4)));
		} break;

		case gs_uniform_type_vec4: 
		{
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(vec4) data = (gs_uniform_block_type(vec4)){va_arg(ap, gs_vec4)};
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(vec4)));
		} break;

		case gs_uniform_type_vec3:
		{ 
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(vec3) data = (gs_uniform_block_type(vec3)){va_arg(ap, gs_vec3)};
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(vec3)));
		} break;

		case gs_uniform_type_vec2:
		{
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(vec2) data = (gs_uniform_block_type(vec2)){va_arg(ap, gs_vec2)};
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(vec2)));
		} break;

		case gs_uniform_type_float:
		{ 
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(float) data = (gs_uniform_block_type(float)){(float)va_arg(ap, double)};
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(float)));
		} break;

		case gs_uniform_type_int: 
		{
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(int) data = (gs_uniform_block_type(int)){va_arg(ap, int)};
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(int)));
		} break;

		case gs_uniform_type_sampler2d: 
		{
			va_list ap;
			va_start(ap, name);
			gs_uniform_block_type(texture_sampler) data = {0};
			data.data = va_arg(ap, u32);
			data.slot = va_arg(ap, u32);
			va_end(ap);
			uapi->set_uniform(u_block_h, uniform, name, &data, sizeof(gs_uniform_block_type(texture_sampler)));
		} break;
	};
}

void __gs_uniform_block_t_set_uniform(gs_resource(gs_uniform_block_t) u_block_h, gs_uniform_t uniform, const char* name, void* data, usize data_size)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Check for offset based on name, if doesn't exist create it and pass
	u64 hash_id = gs_hash_str_64(name);	
	b32 exists = true;

	gs_uniform_block_t* u_block = gs_slot_array_get_ptr(gfx->uniform_i->uniform_blocks, u_block_h.id);

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

void __gs_uniform_block_t_bind_uniforms(gs_command_buffer_t* cb, gs_resource(gs_uniform_block_t) uniforms)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_uniform_block_t* u_block = gs_slot_array_get_ptr(gfx->uniform_i->uniform_blocks, uniforms.id);

	// Set data to beginning
	gs_byte_buffer_seek_to_beg(&u_block->data);

	// Rip through uniforms, determine size, then bind accordingly
	gs_for_range_i(u_block->count)
	{
		// Grab uniform
		gs_uniform_t uniform;
		gs_byte_buffer_bulk_read(&u_block->data, &uniform, sizeof(gs_uniform_t));
		// Grab data size
		usize sz; gs_byte_buffer_read(&u_block->data, usize, &sz);
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
	api.set_uniform_from_shader = &__gs_uniform_block_t_set_uniform_from_shader;
	api.bind_uniforms = &__gs_uniform_block_t_bind_uniforms;
	api.uniform_blocks = gs_slot_array_new(gs_uniform_block_t);
	return api;	
}

/*===============================
// Immediate Operations
===============================*/

// Line as a quad (not sure about this, actually)
// Might want to use GL_LINES instead
void __gs_draw_line_3d_ext(gs_command_buffer_t* cb, gs_vec3 start, gs_vec3 end, gs_vec3 normal, f32 thickness, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Need cross for left and right on the plane
	gs_vec3 cross = gs_vec3_scale(gs_vec3_norm(gs_vec3_cross(gs_vec3_norm(gs_vec3_sub(end, start)), normal)), thickness / 2.f);

	gs_vec3 tl = gs_vec3_add(start, cross);	// 0
	gs_vec3 tr = gs_vec3_sub(start, cross); // 1
	gs_vec3 bl = gs_vec3_add(end, cross);	// 2
	gs_vec3 br = gs_vec3_sub(end, cross);	// 3

	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);
		gfx->immediate.disable_texture_2d(cb);
		gfx->immediate.vertex_3fv(cb, tl);
		gfx->immediate.vertex_3fv(cb, br);
		gfx->immediate.vertex_3fv(cb, bl);
		gfx->immediate.vertex_3fv(cb, tl);
		gfx->immediate.vertex_3fv(cb, tr);
		gfx->immediate.vertex_3fv(cb, br);
	}
	gfx->immediate.end(cb);
}

// Thickness line
void __gs_draw_line_2d_ext(gs_command_buffer_t* cb, gs_vec2 s, gs_vec2 e, f32 thickness, gs_color_t color)
{
	__gs_draw_line_3d_ext(cb, gs_v3(s.x, s.y, 0.f), gs_v3(e.x, e.y, 0.f), gs_v3(0.f, 0.f, -1.f), thickness, color);
}

void __gs_draw_line_3d(gs_command_buffer_t* cb, gs_vec3 s, gs_vec3 e, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_lines);
	{
		gfx->immediate.color_ubv(cb, color);
		gfx->immediate.disable_texture_2d(cb);
		gfx->immediate.vertex_3fv(cb, s);
		gfx->immediate.vertex_3fv(cb, e);
	}
	gfx->immediate.end(cb);
}

void __gs_draw_line_2d(gs_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, gs_color_t color)
{
	__gs_draw_line_3d(cb, gs_v3(x0, y0, 0.f), gs_v3(x1, y1, 0.f), color);
}

void __gs_draw_triangle_3d(gs_command_buffer_t* cb, gs_vec3 a, gs_vec3 b, gs_vec3 c, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);
		gfx->immediate.disable_texture_2d(cb);
		gfx->immediate.vertex_3fv(cb, a);
		gfx->immediate.vertex_3fv(cb, b);
		gfx->immediate.vertex_3fv(cb, c);
	}
	gfx->immediate.end(cb);
}

void __gs_draw_triangle_3d_ext(gs_command_buffer_t* cb, gs_vec3 a, gs_vec3 b, gs_vec3 c, gs_mat4 m, gs_color_t color)
{
	gs_vec4 _a = gs_mat4_mul_vec4(m, gs_v4(a.x, a.y, a.z, 1.f));
	gs_vec4 _b = gs_mat4_mul_vec4(m, gs_v4(b.x, b.y, b.z, 1.f));
	gs_vec4 _c = gs_mat4_mul_vec4(m, gs_v4(c.x, c.y, c.z, 1.f));
	__gs_draw_triangle_3d(cb, gs_v3(_a.x, _a.y, _a.z), gs_v3(_b.x, _b.y, _b.z), gs_v3(_c.x, _c.y, _c.z), color);
}

void __gs_draw_triangle_2d(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, gs_vec2 c, gs_color_t color)
{
	__gs_draw_triangle_3d(
		cb,
		gs_v3(a.x, a.y, 0.f), 
		gs_v3(b.x, b.y, 0.f), 
		gs_v3(c.x, c.y, 0.f), 
		color
	);
}

// Draw a plane
void __gs_draw_rect_3d(gs_command_buffer_t* cb, gs_vec3 p, gs_vec3 n, gs_color_t color)
{
	// Most intuitive way to draw a plane?
	// 3 points? 2 points (corners) and a normal?
	// Normal with a transformation matrix? (applied to a rect)
	// How to do da plane?
	// point, normal, scale?
}

void __gs_draw_rect_2d_impl(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color)
{
	gs_vec3 tl = gs_v3(a.x, a.y, 0.f);
	gs_vec3 tr = gs_v3(b.x, a.y, 0.f);
	gs_vec3 bl = gs_v3(a.x, b.y, 0.f);
	gs_vec3 br = gs_v3(b.x, b.y, 0.f); 

	gs_vec2 tl_uv = gs_v2(uv0.x, uv1.y);
	gs_vec2 tr_uv = gs_v2(uv1.x, uv1.y);
	gs_vec2 bl_uv = gs_v2(uv0.x, uv0.y);
	gs_vec2 br_uv = gs_v2(uv1.x, uv0.y);

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);

		gfx->immediate.texcoord_2fv(cb, tl_uv);
		gfx->immediate.vertex_3fv(cb, tl);

		gfx->immediate.texcoord_2fv(cb, br_uv);
		gfx->immediate.vertex_3fv(cb, br);

		gfx->immediate.texcoord_2fv(cb, bl_uv);
		gfx->immediate.vertex_3fv(cb, bl);

		gfx->immediate.texcoord_2fv(cb, tl_uv);
		gfx->immediate.vertex_3fv(cb, tl);

		gfx->immediate.texcoord_2fv(cb, tr_uv);
		gfx->immediate.vertex_3fv(cb, tr);

		gfx->immediate.texcoord_2fv(cb, br_uv);
		gfx->immediate.vertex_3fv(cb, br);
	}
	gfx->immediate.end(cb);
}

void __gs_draw_rect_2d_lines_impl(gs_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, gs_color_t color)
{
	__gs_draw_line_2d(cb, x0, y0, x1, y0, color);	// tl -> tr
	__gs_draw_line_2d(cb, x1, y0, x1, y1, color);	// tr -> br
	__gs_draw_line_2d(cb, x1, y1, x0, y1, color);	// br -> bl
	__gs_draw_line_2d(cb, x0, y1, x0, y0, color);	// bl -> tl
}

void __gs_draw_rect_2d_lines(gs_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_rect_2d_lines_impl(cb, x0, y0, x1, y1, color);
}

void __gs_draw_rect_2d(gs_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_rect_2d_impl(cb, gs_v2(x0, y0), gs_v2(x1, y1), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color);
}

void __gs_draw_rect_2dv(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_rect_2d_impl(cb, a, b, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color);
}

void __gs_draw_rect_2d_textured(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, u32 tex_id, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.enable_texture_2d(cb, tex_id);
	__gs_draw_rect_2d_impl(cb, a, b, gs_v2(0.f, 0.f), gs_v2(0.f, 0.f), color);
}

void __gs_draw_rect_2d_textured_ext(gs_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, 
	f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.enable_texture_2d(cb, tex_id);
	__gs_draw_rect_2d_impl(cb, gs_v2(x0, y0), gs_v2(x1, y1), gs_v2(u0, v0), gs_v2(u1, v1), color);
}

void __gs_draw_box_vqs_impl(gs_command_buffer_t* cb, gs_vqs xform, gs_color_t color)
{
	f32 width = 0.5f;
	f32 height = 0.5f;
	f32 length = 0.5f;
	f32 x = 0.f;
	f32 y = 0.f;
	f32 z = 0.f;

	// Preapply matrix transformations to all verts
	gs_mat4 mat = gs_vqs_to_mat4(&xform);

	gs_vec3 v0 = gs_v3(x - width/2, y - height/2, z + length/2);
	gs_vec3 v1 = gs_v3(x + width/2, y - height/2, z + length/2);
	gs_vec3 v2 = gs_v3(x - width/2, y + height/2, z + length/2);
	gs_vec3 v3 = gs_v3(x + width/2, y + height/2, z + length/2);
	gs_vec3 v4 = gs_v3(x - width/2, y - height/2, z - length/2);
	gs_vec3 v5 = gs_v3(x - width/2, y + height/2, z - length/2);
	gs_vec3 v6 = gs_v3(x + width/2, y - height/2, z - length/2);
	gs_vec3 v7 = gs_v3(x + width/2, y + height/2, z - length/2);

	gs_vec2 uv0 = gs_v2(0.f, 0.f);
	gs_vec2 uv1 = gs_v2(1.f, 0.f);
	gs_vec2 uv2 = gs_v2(0.f, 1.f);
	gs_vec2 uv3 = gs_v2(1.f, 1.f);

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.push_matrix(cb, gs_matrix_model);
		{
    		gfx->immediate.mat_mul(cb, mat);
			gfx->immediate.color_ubv(cb, color);
			
	        // Front face
	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v2);  // Top Left

	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v3);  // Top Right
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v2);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        
	        // Back face
	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v6);  // Bottom Left
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v5);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v7);  // Bottom Right

	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v6);  // Top Right
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v5);  // Top Left

	        // Top face
	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v7);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Bottom Left
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v3);  // Bottom Right

	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v7);  // Top Right
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v5);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right

	        // Bottom face
	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left

	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Top Right
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v1);  // Top Left

	        // Right face
	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v7);  // Top Right
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v3);  // Top Left

	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Left
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v7);  // Top Left

	        // Left face
	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v5);  // Top Right

	        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Bottom Left
	        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v0);  // Top Left
	        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right
		}
		gfx->immediate.pop_matrix(cb);
	}
	gfx->immediate.end(cb);

}

void __gs_draw_box_impl(gs_command_buffer_t* cb, gs_vec3 origin, gs_vec3 half_extents, gs_color_t color)
{
	f32 width = half_extents.x;
	f32 height = half_extents.y;
	f32 length = half_extents.z;
	f32 x = origin.x;
	f32 y = origin.y;
	f32 z = origin.z;

	gs_vec3 v0 = gs_v3(x - width/2, y - height/2, z + length/2);
	gs_vec3 v1 = gs_v3(x + width/2, y - height/2, z + length/2);
	gs_vec3 v2 = gs_v3(x - width/2, y + height/2, z + length/2);
	gs_vec3 v3 = gs_v3(x + width/2, y + height/2, z + length/2);
	gs_vec3 v4 = gs_v3(x - width/2, y - height/2, z - length/2);
	gs_vec3 v5 = gs_v3(x - width/2, y + height/2, z - length/2);
	gs_vec3 v6 = gs_v3(x + width/2, y - height/2, z - length/2);
	gs_vec3 v7 = gs_v3(x + width/2, y + height/2, z - length/2);

	gs_vec2 uv0 = gs_v2(0.f, 0.f);
	gs_vec2 uv1 = gs_v2(1.f, 0.f);
	gs_vec2 uv2 = gs_v2(0.f, 1.f);
	gs_vec2 uv3 = gs_v2(1.f, 1.f);

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);

        // Front face
        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v2);  // Top Left

        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v3);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v2);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        
        // Back face
        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v6);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v5);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v7);  // Bottom Right

        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v6);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v5);  // Top Left

        // Top face
        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v7);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v3);  // Bottom Right

        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v7);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v5);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right

        // Bottom face
        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left

        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v1);  // Top Left

        // Right face
        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v7);  // Top Right
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v3);  // Top Left

        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v1);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v7);  // Top Left

        // Left face
        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv2); gfx->immediate.vertex_3fv(cb, v5);  // Top Right

        gfx->immediate.texcoord_2fv(cb, uv0); gfx->immediate.vertex_3fv(cb, v4);  // Bottom Left
        gfx->immediate.texcoord_2fv(cb, uv1); gfx->immediate.vertex_3fv(cb, v0);  // Top Left
        gfx->immediate.texcoord_2fv(cb, uv3); gfx->immediate.vertex_3fv(cb, v2);  // Bottom Right
	}
	gfx->immediate.end(cb);
}

void __gs_draw_box(gs_command_buffer_t* cb, gs_vec3 origin, gs_vec3 half_extents, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_box_impl(cb, origin, half_extents, color);
}

void __gs_draw_box_vqs(gs_command_buffer_t* cb, gs_vqs xform, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_box_vqs_impl(cb, xform, color);
}

void __gs_draw_box_textured_vqs(gs_command_buffer_t* cb, gs_vqs xform, u32 tex_id, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.enable_texture_2d(cb, tex_id);
	__gs_draw_box_vqs_impl(cb, xform, color);
}

void __gs_draw_box_textured(gs_command_buffer_t* cb, gs_vec3 origin, gs_vec3 half_extents, u32 tex_id, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.enable_texture_2d(cb, tex_id);
	__gs_draw_box_impl(cb, origin, half_extents, color);
}

void __gs_draw_box_lines_impl(gs_command_buffer_t* cb, gs_vec3 origin, gs_vec3 half_extents, gs_color_t color)
{
	// Draw individual 3d lines using vqs
	f32 width = half_extents.x;
	f32 height = half_extents.y;
	f32 length = half_extents.z;
	f32 x = origin.x;
	f32 y = origin.y;
	f32 z = origin.z;

	gs_vec3 v0 = gs_v3(x - width/2, y - height/2, z + length/2);
	gs_vec3 v1 = gs_v3(x + width/2, y - height/2, z + length/2);
	gs_vec3 v2 = gs_v3(x - width/2, y + height/2, z + length/2);
	gs_vec3 v3 = gs_v3(x + width/2, y + height/2, z + length/2);
	gs_vec3 v4 = gs_v3(x - width/2, y - height/2, z - length/2);
	gs_vec3 v5 = gs_v3(x - width/2, y + height/2, z - length/2);
	gs_vec3 v6 = gs_v3(x + width/2, y - height/2, z - length/2);
	gs_vec3 v7 = gs_v3(x + width/2, y + height/2, z - length/2);

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_lines);
	{
		gfx->immediate.color_ubv(cb, color);

		// Front face
		gfx->immediate.draw_line_3d(cb, v0, v1, color);
		gfx->immediate.draw_line_3d(cb, v1, v3, color);
		gfx->immediate.draw_line_3d(cb, v3, v2, color);
		gfx->immediate.draw_line_3d(cb, v2, v0, color);

		// Back face
		gfx->immediate.draw_line_3d(cb, v4, v6, color);
		gfx->immediate.draw_line_3d(cb, v6, v7, color);
		gfx->immediate.draw_line_3d(cb, v7, v5, color);
		gfx->immediate.draw_line_3d(cb, v5, v4, color);

		// Right face
		gfx->immediate.draw_line_3d(cb, v1, v6, color);
		gfx->immediate.draw_line_3d(cb, v6, v7, color);
		gfx->immediate.draw_line_3d(cb, v7, v3, color);
		gfx->immediate.draw_line_3d(cb, v3, v1, color);

		// Left face
		gfx->immediate.draw_line_3d(cb, v4, v6, color);
		gfx->immediate.draw_line_3d(cb, v6, v1, color);
		gfx->immediate.draw_line_3d(cb, v1, v0, color);
		gfx->immediate.draw_line_3d(cb, v0, v4, color);

		// Bottom face
		gfx->immediate.draw_line_3d(cb, v5, v7, color);
		gfx->immediate.draw_line_3d(cb, v7, v3, color);
		gfx->immediate.draw_line_3d(cb, v3, v2, color);
		gfx->immediate.draw_line_3d(cb, v2, v5, color);

		// Top face
		gfx->immediate.draw_line_3d(cb, v0, v4, color);
		gfx->immediate.draw_line_3d(cb, v4, v5, color);
		gfx->immediate.draw_line_3d(cb, v5, v2, color);
		gfx->immediate.draw_line_3d(cb, v2, v0, color);
    }
    gfx->immediate.end(cb);
}

void __gs_draw_box_lines_vqs_impl(gs_command_buffer_t* cb, gs_vqs xform, gs_color_t color)
{
	// Draw individual 3d lines using vqs
	f32 width = 0.5f;
	f32 height = 0.5f;
	f32 length = 0.5f;
	f32 x = 0.f;
	f32 y = 0.f;
	f32 z = 0.f;

	// Preapply matrix transformations to all verts
	gs_mat4 mat = gs_vqs_to_mat4(&xform);

	gs_vec3 v0 = gs_v3(x - width/2, y - height/2, z + length/2);
	gs_vec3 v1 = gs_v3(x + width/2, y - height/2, z + length/2);
	gs_vec3 v2 = gs_v3(x - width/2, y + height/2, z + length/2);
	gs_vec3 v3 = gs_v3(x + width/2, y + height/2, z + length/2);
	gs_vec3 v4 = gs_v3(x - width/2, y - height/2, z - length/2);
	gs_vec3 v5 = gs_v3(x - width/2, y + height/2, z - length/2);
	gs_vec3 v6 = gs_v3(x + width/2, y - height/2, z - length/2);
	gs_vec3 v7 = gs_v3(x + width/2, y + height/2, z - length/2);

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_lines);
	{
		gfx->immediate.push_matrix(cb, gs_matrix_model);
		{
    		gfx->immediate.mat_mul(cb, mat);
			gfx->immediate.color_ubv(cb, color);

    		// Front face
    		gfx->immediate.draw_line_3d(cb, v0, v1, color);
    		gfx->immediate.draw_line_3d(cb, v1, v3, color);
    		gfx->immediate.draw_line_3d(cb, v3, v2, color);
    		gfx->immediate.draw_line_3d(cb, v2, v0, color);

    		// Back face
    		gfx->immediate.draw_line_3d(cb, v4, v6, color);
    		gfx->immediate.draw_line_3d(cb, v6, v7, color);
    		gfx->immediate.draw_line_3d(cb, v7, v5, color);
    		gfx->immediate.draw_line_3d(cb, v5, v4, color);

    		// Right face
    		gfx->immediate.draw_line_3d(cb, v1, v6, color);
    		gfx->immediate.draw_line_3d(cb, v6, v7, color);
    		gfx->immediate.draw_line_3d(cb, v7, v3, color);
    		gfx->immediate.draw_line_3d(cb, v3, v1, color);

    		// Left face
    		gfx->immediate.draw_line_3d(cb, v4, v6, color);
    		gfx->immediate.draw_line_3d(cb, v6, v1, color);
    		gfx->immediate.draw_line_3d(cb, v1, v0, color);
    		gfx->immediate.draw_line_3d(cb, v0, v4, color);

    		// Bottom face
    		gfx->immediate.draw_line_3d(cb, v5, v7, color);
    		gfx->immediate.draw_line_3d(cb, v7, v3, color);
    		gfx->immediate.draw_line_3d(cb, v3, v2, color);
    		gfx->immediate.draw_line_3d(cb, v2, v5, color);

    		// Top face
    		gfx->immediate.draw_line_3d(cb, v0, v4, color);
    		gfx->immediate.draw_line_3d(cb, v4, v5, color);
    		gfx->immediate.draw_line_3d(cb, v5, v2, color);
    		gfx->immediate.draw_line_3d(cb, v2, v0, color);
    	}
    	gfx->immediate.pop_matrix(cb);
    }
    gfx->immediate.end(cb);
}

void __gs_draw_box_lines(gs_command_buffer_t* cb, gs_vec3 origin, gs_vec3 half_extents, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_box_lines_impl(cb, origin, half_extents, color);
}
 
void __gs_draw_box_lines_vqs(gs_command_buffer_t* cb, gs_vqs xform, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_box_lines_vqs_impl(cb, xform, color);
}

/*=========
// Sphere
=========*/

void __gs_draw_sphere_impl(gs_command_buffer_t* cb, gs_vec3 center, f32 radius, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

    s32 rings  = 16;
    s32 slices = 16;

    gfx->immediate.begin(cb, gs_triangles);
    {
		gfx->immediate.color_ubv(cb, color);
		{
	        for (s32 i = 0; i < (rings + 2); i++)
	        {
	            for (s32 j = 0; j < slices; j++)
	            {
	                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
	                           radius * sinf(deg2rad*(270+(180/(rings + 1))*i)),
	                           radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));

	                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
	                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
	                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));

	                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*(j*360/slices)),
	                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
	                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*(j*360/slices)));

	                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
	                           radius * sinf(deg2rad*(270+(180/(rings + 1))*i)),
	                           radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));

	                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i)))*sinf(deg2rad*((j+1)*360/slices)),
	                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i))),
	                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i)))*cosf(deg2rad*((j+1)*360/slices)));

	                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
	                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
	                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));
	            }
	        }
		}
    }
    gfx->immediate.end(cb);
}

void __gs_draw_sphere(gs_command_buffer_t* cb, gs_vec3 center, f32 radius, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_sphere_impl(cb, center, radius, color);
}

void __gs_draw_sphere_lines_impl(gs_command_buffer_t* cb, gs_vec3 origin, f32 radius, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	const s32 rings = 16;
	const s32 slices = 16;
    s32 numVertex = (rings + 2) * slices * 6;

	gfx->immediate.begin(cb, gs_lines);
	{
		gfx->immediate.color_ubv(cb, color);
        gs_for_range_i(rings + 2)
        {
        	gs_for_range_j(slices)
            {
                gfx->immediate.vertex_3f(
                	cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
                           radius * sinf(deg2rad*(270+(180/(rings + 1))*i)),
                           radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices))
                    );
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));

                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*(j*360/slices)),
                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*(j*360/slices)));

                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*(j*360/slices)),
                           radius * sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                           radius * cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*(j*360/slices)));
                gfx->immediate.vertex_3f(cb, radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
                           radius * sinf(deg2rad*(270+(180/(rings + 1))*i)),
                           radius * cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));
            }
        }
	}
    gfx->immediate.end(cb);
}

void __gs_draw_sphere_lines_vqs(gs_command_buffer_t* cb, gs_vqs xform, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	const s32 rings = 16;
	const s32 slices = 16;
    s32 numVertex = (rings + 2) * slices * 6;

	gfx->immediate.begin(cb, gs_lines);
	{
		gfx->immediate.color_ubv(cb, color);
	    gfx->immediate.push_matrix(cb, gs_matrix_model);
	    {
	    	gfx->immediate.mat_mul_vqs(cb, xform);
            gs_for_range_i(rings + 2)
            {
            	gs_for_range_j(slices)
                {
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
                               sinf(deg2rad*(270+(180/(rings + 1))*i)),
                               cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
                               sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                               cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));

                    gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
                               sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                               cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*(j*360/slices)),
                               sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                               cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*(j*360/slices)));

                    gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*(j*360/slices)),
                               sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
                               cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*(j*360/slices)));
                    gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
                               sinf(deg2rad*(270+(180/(rings + 1))*i)),
                               cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));
                }
            }
	    }
	    gfx->immediate.pop_matrix(cb);
	}
    gfx->immediate.end(cb);
}

// Draw sphere wires
void __gs_draw_sphere_lines(gs_command_buffer_t* cb, gs_vec3 center, f32 radius, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	__gs_draw_sphere_lines_impl(cb, center, radius, color);
}

/*=========
// Circle
=========*/

void __gs_draw_circle_sector_impl(gs_command_buffer_t* cb, gs_vec2 center, f32 radius, s32 start_angle, s32 end_angle, s32 segments, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

    if (radius <= 0.0f) {
    	radius = 0.1f;
    }

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        s32 tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        f32 th = acosf(2*powf(1 - smooth_circle_error_rate/radius, 2) - 1);
        segments = (s32)((end_angle - start_angle)*ceilf(2*gs_pi/th)/360);
        if (segments <= 0) {
        	segments = 4;
        }
    }

    f32 step = (f32)(end_angle - start_angle)/(f32)segments;
    f32 angle = (f32)start_angle;

    gfx->immediate.begin(cb, gs_triangles);
    {
	    gfx->immediate.color_ubv(cb, color);
		gs_for_range_i(segments)
	    {
	        gfx->immediate.vertex_2f(cb, center.x, center.y);
	        gfx->immediate.vertex_2f(cb, center.x + sinf(deg2rad*angle)*radius, center.y + cosf(deg2rad*angle)*radius);
	        gfx->immediate.vertex_2f(cb, center.x + sinf(deg2rad*(angle + step))*radius, center.y + cosf(deg2rad*(angle + step))*radius);
	        angle += step;
	    }
    }
    gfx->immediate.end(cb);
}

void __gs_draw_circle_sector(gs_command_buffer_t* cb, gs_vec2 center, f32 radius, s32 start_angle, s32 end_angle, s32 segments, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_circle_sector_impl(cb, center, radius, start_angle, end_angle, segments, color);
}

void __gs_draw_circle(gs_command_buffer_t* cb, gs_vec2 center, f32 radius, s32 segments, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_circle_sector_impl(cb, center, radius, 0.f, 360.f, segments, color);
}

/*=========
// Text
=========*/

void __gs_draw_text_ext(gs_command_buffer_t* cb, f32 x, f32 y, const char* text, gs_font_t* ft, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.enable_texture_2d(cb, ft->texture.id);
		gfx->immediate.color_ubv(cb, color);
		while (text[0] != '\0')
		{
			char c = text[0];
			if (c >= 32 && c <= 127) 
			{
				stbtt_aligned_quad q = gs_default_val();
				stbtt_GetBakedQuad((stbtt_bakedchar*)ft->glyphs, ft->texture.width, ft->texture.height, c - 32, &x, &y, &q, 1);

				gs_vec3 v0 = gs_v3(q.x0, q.y0, 0.f);	// TL
				gs_vec3 v1 = gs_v3(q.x1, q.y0, 0.f);	// TR
				gs_vec3 v2 = gs_v3(q.x0, q.y1, 0.f);	// BL
				gs_vec3 v3 = gs_v3(q.x1, q.y1, 0.f);	// BR

				gs_vec2 uv0 = gs_v2(q.s0, 1.f - q.t0);	// TL
				gs_vec2 uv1 = gs_v2(q.s1, 1.f - q.t0);	// TR
				gs_vec2 uv2 = gs_v2(q.s0, 1.f - q.t1);	// BL
				gs_vec2 uv3 = gs_v2(q.s1, 1.f - q.t1);	// BR

				gfx->immediate.texcoord_2fv(cb, uv0); 
				gfx->immediate.vertex_3fv(cb, v0);

		        gfx->immediate.texcoord_2fv(cb, uv3); 
				gfx->immediate.vertex_3fv(cb, v3); 

		        gfx->immediate.texcoord_2fv(cb, uv2); 
				gfx->immediate.vertex_3fv(cb, v2); 

				gfx->immediate.texcoord_2fv(cb, uv0); 
				gfx->immediate.vertex_3fv(cb, v0);

		        gfx->immediate.texcoord_2fv(cb, uv1); 
				gfx->immediate.vertex_3fv(cb, v1); 

		        gfx->immediate.texcoord_2fv(cb, uv3); 
				gfx->immediate.vertex_3fv(cb, v3); 
			}
			text++;
		}
	}
	gfx->immediate.end(cb);
}

void __gs_draw_text(gs_command_buffer_t* cb, f32 x, f32 y, const char* text, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	gs_font_t ft = gfx->default_font();
	gfx->immediate.draw_text_ext(cb, x, y, text, &ft, color);
}

/*============
// Matrix Ops
============*/

void __gs_push_camera(gs_command_buffer_t* cb, gs_camera_t camera)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	// Just grab main window for now. Will need to grab top of viewport stack in future
	gs_platform_i* p = gs_engine_instance()->ctx.platform;
	gs_vec2 ws = p->window_size(p->main_window());	
	gfx->immediate.push_matrix(cb, gs_matrix_vp);
	gfx->immediate.mat_mul(cb, gs_camera_get_view_projection(&camera, ws.x, ws.y));
}

void __gs_pop_camera(gs_command_buffer_t* cb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.pop_matrix(cb);
}

gs_camera_t __gs_begin_3d(gs_command_buffer_t* cb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_camera_t c = gs_camera_perspective();
	gfx->immediate.push_state(cb, gfx->immediate.default_pipeline_state(gs_immediate_mode_3d));
	gfx->immediate.push_camera(cb, c);
	return c;
}

void __gs_end_3d(gs_command_buffer_t* cb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.pop_camera(cb);
	gfx->immediate.pop_state(cb);
}

gs_camera_t __gs_begin_2d(gs_command_buffer_t* cb)
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	gs_vec2 ws = platform->window_size(platform->main_window());
	gs_vec2 hws = gs_vec2_scale(ws, 0.5f);
	gs_camera_t c = gs_camera_default();
	c.transform.position = gs_vec3_add(c.transform.position, gs_v3(hws.x, hws.y, 1.f));

	f32 l = -ws.x / 2.f; 
	f32 r = ws.x / 2.f;
	f32 b = ws.y / 2.f;
	f32 t = -ws.y / 2.f;
	gs_mat4 ortho = gs_mat4_transpose(gs_mat4_ortho(
		l, r, b, t, 0.01f, 1000.f
	));
	ortho = gs_mat4_mul(ortho, gs_camera_get_view(&c));

	gfx->immediate.push_state(cb, gfx->immediate.default_pipeline_state(gs_immediate_mode_2d));
	gfx->immediate.push_matrix(cb, gs_matrix_vp);
	gfx->immediate.mat_mul(cb, ortho);

	return c;
}

void __gs_end_2d(gs_command_buffer_t* cb)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.pop_camera(cb);
	gfx->immediate.pop_state(cb);
}

void __gs_mat_rotatef(gs_command_buffer_t* cb, f32 rad, f32 x, f32 y, f32 z)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_mat4_rotate(rad, gs_v3(x, y, z)));
}

void __gs_mat_rotatev(gs_command_buffer_t* cb, f32 rad, gs_vec3 v)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_mat4_rotate(rad, v));
}

void __gs_mat_rotateq(gs_command_buffer_t* cb, gs_quat q)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_quat_to_mat4(q));
}

void __gs_mat_transf(gs_command_buffer_t* cb, f32 x, f32 y, f32 z)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_mat4_translate(gs_v3(x, y, z)));
}

void __gs_mat_transv(gs_command_buffer_t* cb, gs_vec3 v)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_mat4_translate(v));
}

void __gs_mat_scalef(gs_command_buffer_t* cb, f32 x, f32 y, f32 z)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_mat4_scale(gs_v3(x, y, z)));
}

void __gs_mat_scalev(gs_command_buffer_t* cb, gs_vec3 v)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_mat4_scale(v));
}

void __gs_mat_mul_vqs(gs_command_buffer_t* cb, gs_vqs xform)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.mat_mul(cb, gs_vqs_to_mat4(&xform));
}

static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c);
static void         Decode85(const unsigned char* src, unsigned char* dst);

gs_font_t __gs_construct_default_font()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_font_t f = gs_default_val();

	stbtt_fontinfo font = gs_default_val();
	const char* compressed_ttf_data_base85 = GetDefaultCompressedFontDataTTFBase85();

	s32 compressed_ttf_size = (((s32)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf_data = gs_malloc((usize)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf_data);

    const u32 buf_decompressed_size = stb_decompress_length((unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char*)gs_malloc(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (unsigned char*)compressed_ttf_data, (u32)compressed_ttf_size);

	const u32 w = 512;
	const u32 h = 512;
	const u32 num_comps = 4;
	u8* alpha_bitmap = gs_malloc(w * h);
	u8* flipmap = gs_malloc(w * h * num_comps);
	memset(alpha_bitmap, 0, w * h);
	memset(flipmap, 0, w * h * num_comps);
   	s32 v = stbtt_BakeFontBitmap((u8*)buf_decompressed_data, 0, 16.f, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f.glyphs); // no guarantee this fits!

   	// Flip texture
   	u32 r = h - 1;
   	for (u32 i = 0; i < h; ++i)
   	{
   		for (u32 j = 0; j < w; ++j)
   		{
   			u32 i0 = i * w + j;
   			u32 i1 = r * w * num_comps + j * num_comps;
   			u8 a = alpha_bitmap[i0];
   			flipmap[i1 + 0] = 255;
   			flipmap[i1 + 1] = 255;
   			flipmap[i1 + 2] = 255;
   			flipmap[i1 + 3] = a;
   		}
   		r--;
   	}

   	gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();
   	desc.width = w;
   	desc.height = h;
   	desc.num_comps = num_comps;
   	desc.data = flipmap;
   	desc.texture_format = gs_texture_format_rgba8;
   	desc.min_filter = gs_linear;

   	// Generate atlas texture for bitmap with bitmap data
   	f.texture = gfx->construct_texture(desc);

    gs_free(compressed_ttf_data);
   	gs_free(buf_decompressed_data);
   	gs_free(alpha_bitmap);
   	gs_free(flipmap);

	return f;
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------

static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    {return c >= '\\' ? c-36 : c-35;}
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85 * (Decode85Byte(src[1]) + 85 * (Decode85Byte(src[2]) + 85 * (Decode85Byte(src[3]) + 85 * Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

static const char proggy_clean_ttf_compressed_data_base85[11980 + 1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return proggy_clean_ttf_compressed_data_base85;
}








