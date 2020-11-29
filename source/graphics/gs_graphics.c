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

	u8 temp_bitmap[512*512];
	char* ttf = platform->read_file_contents(file_path, "rb", NULL);
   	s32 v = stbtt_BakeFontBitmap((u8*)ttf, 0, point_size, temp_bitmap, 512, 512, 32, 96, (stbtt_bakedchar*)f.glyphs); // no guarantee this fits!

   	gs_texture_parameter_desc desc = gs_texture_parameter_desc_default();
   	desc.data = temp_bitmap;
   	desc.texture_format = gs_texture_format_a8;
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

	return f;
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
void __gs_draw_line_3d(gs_command_buffer_t* cb, gs_vec3 start, gs_vec3 end, gs_vec3 normal, f32 thickness, gs_color_t color)
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
		gfx->immediate.vertex_3fv(cb, tl);
		gfx->immediate.vertex_3fv(cb, br);
		gfx->immediate.vertex_3fv(cb, bl);
		gfx->immediate.vertex_3fv(cb, tl);
		gfx->immediate.vertex_3fv(cb, tr);
		gfx->immediate.vertex_3fv(cb, br);
	}
	gfx->immediate.end(cb);
}

void __gs_draw_line_2d(gs_command_buffer_t* cb, gs_vec2 s, gs_vec2 e, f32 thickness, gs_color_t color)
{
	__gs_draw_line_3d(cb, gs_v3(s.x, s.y, 0.f), gs_v3(e.x, e.y, 0.f), gs_v3(0.f, 0.f, -1.f), thickness, color);
}

void __gs_draw_triangle_3d(gs_command_buffer_t* cb, gs_vec3 a, gs_vec3 b, gs_vec3 c, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);
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

void __gs_draw_rect_2d(gs_command_buffer_t* cb, gs_vec2 a, gs_vec2 b, gs_color_t color)
{
	gs_vec3 tl = gs_v3(a.x, a.y, 0.f);
	gs_vec3 tr = gs_v3(b.x, a.y, 0.f);
	gs_vec3 bl = gs_v3(a.x, b.y, 0.f);
	gs_vec3 br = gs_v3(b.x, b.y, 0.f); 

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);
		gfx->immediate.vertex_3fv(cb, tl);
		gfx->immediate.vertex_3fv(cb, br);
		gfx->immediate.vertex_3fv(cb, bl);
		gfx->immediate.vertex_3fv(cb, tl);
		gfx->immediate.vertex_3fv(cb, tr);
		gfx->immediate.vertex_3fv(cb, br);
	}
	gfx->immediate.end(cb);
}

void __gs_draw_box(gs_command_buffer_t* cb, gs_vec3 origin, gs_vec3 half_extents, gs_color_t color)
{
	f32 width = half_extents.x;
	f32 height = half_extents.y;
	f32 length = half_extents.z;
	f32 x = 0.f;
	f32 y = 0.f;
	f32 z = 0.f;

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.color_ubv(cb, color);
		gfx->immediate.push_matrix(cb, gs_matrix_model);
		{
			gfx->immediate.mat_mul(cb, gs_mat4_translate(origin));

	        // Front face
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z + length/2);  // Bottom Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z + length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z + length/2);  // Top Left

	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z + length/2);  // Top Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z + length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z + length/2);  // Bottom Right
	        
	        // Back face
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z - length/2);  // Bottom Left
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z - length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z - length/2);  // Bottom Right

	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z - length/2);  // Top Right
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z - length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z - length/2);  // Top Left

	        // Top face
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z - length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z + length/2);  // Bottom Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z + length/2);  // Bottom Right

	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z - length/2);  // Top Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z - length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z + length/2);  // Bottom Right

	        // Bottom face
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z - length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z + length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z + length/2);  // Bottom Left

	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z - length/2);  // Top Right
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z + length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z - length/2);  // Top Left

	        // Right face
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z - length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z - length/2);  // Top Right
	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z + length/2);  // Top Left

	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z + length/2);  // Bottom Left
	        gfx->immediate.vertex_3f(cb, x + width/2, y - height/2, z - length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x + width/2, y + height/2, z + length/2);  // Top Left

	        // Left face
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z - length/2);  // Bottom Right
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z + length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z - length/2);  // Top Right

	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z + length/2);  // Bottom Left
	        gfx->immediate.vertex_3f(cb, x - width/2, y + height/2, z + length/2);  // Top Left
	        gfx->immediate.vertex_3f(cb, x - width/2, y - height/2, z - length/2);  // Bottom Right
		}
		gfx->immediate.pop_matrix(cb);

	}
	gfx->immediate.end(cb);
}

void __gs_draw_box_ext(gs_command_buffer_t* cb, gs_vqs xform, gs_color_t color)
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

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gfx->immediate.begin(cb, gs_triangles);
	{
		gfx->immediate.push_matrix(cb, gs_matrix_model);
		{
    		gfx->immediate.mat_mul(cb, mat);
			gfx->immediate.color_ubv(cb, color);
			
	        // Front face
	        gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left
	        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v2);  // Top Left

	        gfx->immediate.vertex_3fv(cb, v3);  // Top Right
	        gfx->immediate.vertex_3fv(cb, v2);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        
	        // Back face
	        gfx->immediate.vertex_3fv(cb, v4);  // Bottom Left
	        gfx->immediate.vertex_3fv(cb, v5);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right

	        gfx->immediate.vertex_3fv(cb, v7);  // Top Right
	        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v5);  // Top Left

	        // Top face
	        gfx->immediate.vertex_3fv(cb, v5);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v2);  // Bottom Left
	        gfx->immediate.vertex_3fv(cb, v3);  // Bottom Right

	        gfx->immediate.vertex_3fv(cb, v7);  // Top Right
	        gfx->immediate.vertex_3fv(cb, v5);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v3);  // Bottom Right

	        // Bottom face
	        gfx->immediate.vertex_3fv(cb, v4);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left

	        gfx->immediate.vertex_3fv(cb, v6);  // Top Right
	        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v4);  // Top Left

	        // Right face
	        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v7);  // Top Right
	        gfx->immediate.vertex_3fv(cb, v3);  // Top Left

	        gfx->immediate.vertex_3fv(cb, v1);  // Bottom Left
	        gfx->immediate.vertex_3fv(cb, v6);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v3);  // Top Left

	        // Left face
	        gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
	        gfx->immediate.vertex_3fv(cb, v2);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v5);  // Top Right

	        gfx->immediate.vertex_3fv(cb, v0);  // Bottom Left
	        gfx->immediate.vertex_3fv(cb, v2);  // Top Left
	        gfx->immediate.vertex_3fv(cb, v4);  // Bottom Right
		}
		gfx->immediate.pop_matrix(cb);
	}
	gfx->immediate.end(cb);
}

void __gs_draw_sphere(gs_command_buffer_t* cb, gs_vec3 center, f32 radius, gs_color_t color)
{
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

    s32 rings  = 16;
    s32 slices = 16;

    // Deg to rad
    const f32 deg2rad = gs_pi / 180.f;

    gfx->immediate.begin(cb, gs_triangles);
    {
    	gs_vqs xform = gs_vqs_default();
    	xform.position = center;
    	xform.scale = gs_v3(radius, radius, radius);

    	gfx->immediate.push_matrix(cb, gs_matrix_model);
    	{
    		gfx->immediate.mat_mul(cb, gs_vqs_to_mat4(&xform));
			gfx->immediate.color_ubv(cb, color);
			{
		        for (s32 i = 0; i < (rings + 2); i++)
		        {
		            for (s32 j = 0; j < slices; j++)
		            {
		                gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
		                           sinf(deg2rad*(270+(180/(rings + 1))*i)),
		                           cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));

		                gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
		                           sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
		                           cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));

		                gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*(j*360/slices)),
		                           sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
		                           cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*(j*360/slices)));

		                gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*i))*sinf(deg2rad*(j*360/slices)),
		                           sinf(deg2rad*(270+(180/(rings + 1))*i)),
		                           cosf(deg2rad*(270+(180/(rings + 1))*i))*cosf(deg2rad*(j*360/slices)));

		                gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i)))*sinf(deg2rad*((j+1)*360/slices)),
		                           sinf(deg2rad*(270+(180/(rings + 1))*(i))),
		                           cosf(deg2rad*(270+(180/(rings + 1))*(i)))*cosf(deg2rad*((j+1)*360/slices)));

		                gfx->immediate.vertex_3f(cb, cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*sinf(deg2rad*((j+1)*360/slices)),
		                           sinf(deg2rad*(270+(180/(rings + 1))*(i+1))),
		                           cosf(deg2rad*(270+(180/(rings + 1))*(i+1)))*cosf(deg2rad*((j+1)*360/slices)));
		            }
		        }
			}
		}
        gfx->immediate.pop_matrix(cb);
    }
    gfx->immediate.end(cb);
}

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























