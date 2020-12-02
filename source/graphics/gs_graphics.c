#include "graphics/gs_graphics.h"
#include "graphics/gs_material.h"
#include "graphics/gs_camera.h"
#include "platform/gs_platform.h"
#include "base/gs_engine.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

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

gs_vec2 __gs_text_dimensions(gs_command_buffer_t* cb, const char* text, gs_font_t* ft)
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

void __gs_draw_rect_2d_impl(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, gs_color_t color)
{
	gs_vec3 tl = gs_v3(a.x, a.y, 0.f);
	gs_vec3 tr = gs_v3(b.x, a.y, 0.f);
	gs_vec3 bl = gs_v3(a.x, b.y, 0.f);
	gs_vec3 br = gs_v3(b.x, b.y, 0.f); 

	gs_vec2 tl_uv = gs_v2(0.f, 1.f);
	gs_vec2 tr_uv = gs_v2(1.f, 1.f);
	gs_vec2 bl_uv = gs_v2(0.f, 0.f);
	gs_vec2 br_uv = gs_v2(1.f, 0.f);

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

void __gs_draw_rect_2d(gs_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.disable_texture_2d(cb);
	__gs_draw_rect_2d_impl(cb, gs_v2(x0, y0), gs_v2(x1, y1), color);
}

void __gs_draw_rect_2d_textured(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, u32 tex_id, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.enable_texture_2d(cb, tex_id);
	__gs_draw_rect_2d_impl(cb, a, b, color);
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

void __gs_draw_text(gs_command_buffer_t* cb, f32 x, f32 y, const char* text, gs_font_t* ft, gs_color_t color)
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

	gs_pipeline_state_t state = gs_pipeline_state_default();
	state.depth_enabled = true;
	state.face_culling = gs_face_culling_back;

	gs_camera_t c = gs_camera_perspective();

	gfx->immediate.push_state(cb, state);
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

	gfx->immediate.push_state(cb, gs_pipeline_state_default());
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





















